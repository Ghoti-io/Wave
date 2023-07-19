/**
 * @file
 *
 * Define the Ghoti::Wave::ClientSession class.
 */

#include <cassert>
#include <iostream>
#include <poll.h>
#include <sstream>
#include <set>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ghoti.io/pool.hpp>
#include <sys/socket.h>
#include "wave/clientSession.hpp"
#include "wave/message.hpp"

using namespace std;
using namespace Ghoti::Pool;
using namespace Ghoti::Wave;

#define SEND_OFFSET_T uint32_t
#define CURRENT_CHUNK_T uint32_t

/**
 * Track the phase of the message send/receive lifetime.
 *
 * Helper class only for use in the Ghoti::Wave::ClientSession object file.
 */
enum Phase {
  NEW,               ///< The Message has not started being transmitted or
                     ///<   received yet.
  SEND_HEADER,       ///< The Header is being sent.
  SEND_FIXED,        ///< A fixed-length message is being sent.
  SEND_MULTIPART,    ///< A Multipart message is being sent.
  SEND_CHUNK_HEADER, ///< A Chunk Header is being sent.
  SEND_CHUNK_BODY,   ///< A Chunk body is being sent.
  SEND_STREAM,       ///< A streaming message is being sent.
  FINISHED,          ///< The message is finished.
  ERROR,             ///< There was an error performing the write.
};

/**
 * Message attributes available (to avoid writing a lot of getters/setters.
 *
 * Helper class only for use in the Ghoti::Wave::ClientSession object file.
 */
enum class Attribute {
  SEND_OFFSET,     ///< `uint32_t` Offset of the currently sent part.
  CURRENT_CHUNK,   ///< `uint32_t` The current chunk being transferred.
};

/**
 * Helper definition tracking the state of a message that is being transferred.
 *
 * <Phase, writeOffset, currentChunk>
 */
struct WriteState {
  Phase phase;
  uint32_t writeOffset;
  uint32_t currentChunk;
};

ClientSession::ClientSession(int hServer, Client * client) :
  controlMutex{make_unique<mutex>()},
  hServer{hServer},
  requestSequence{0},
  writeSequence{0},
  readSequence{0},
  working{false},
  finished{false},
  parser{},
  client{client},
  messages{} {
  this->parser.setInheritFrom(this);
}

ClientSession::~ClientSession() {
  close(this->hServer);
}

bool ClientSession::hasReadDataWaiting() {
  // It may be that the socket is currently in use by another thread.  If so,
  // then do not wait for a response, but rather just return false so as not
  // to block the calling thread.
  bool dataIsWaiting{false};

  if (this->controlMutex->try_lock()) {
    if (!this->working) {
      // See if there is anything waiting to be read on the socket.
      pollfd pollFd{this->hServer, POLLIN | POLLERR, 0};
      if (poll(&pollFd, 1, 0)) {
        if (pollFd.revents & POLLIN) {
          // Read Data is waiting.
          dataIsWaiting = true;
          this->working = true;
        }
        else if ((pollFd.revents & POLLERR) && (errno != EINPROGRESS)) {
          cout << "Error in hasReadDataWaiting" << endl;
          cout << strerror(errno) << endl;
          dataIsWaiting = true;
          this->working = true;
        }
      }
      else {
        // It Timed out.
        // Nothing to do.
      }
    }
    this->controlMutex->unlock();
  }
  return dataIsWaiting;
}

bool ClientSession::hasWriteDataWaiting() {
  // It may be that the socket is currently in use by another thread.  If so,
  // then do not wait for a response, but rather just return false so as not
  // to block the calling thread.
  bool dataIsWaiting{false};

  if (this->controlMutex->try_lock()) {
    if (!this->working && (this->writeSequence <= this->messages.size())) {
      // See if there is anything waiting to be read on the socket.
      pollfd pollFd{this->hServer, POLLOUT | POLLERR, 0};
      if (poll(&pollFd, 1, 0)) {
        if (pollFd.revents & POLLOUT) {
          // Socket can be written to.
          dataIsWaiting = true;
          this->working = true;
        }
        if ((pollFd.revents & POLLERR) && (errno != EINPROGRESS)) {
          cout << "Error in hasWriteDataWaiting" << endl;
          cout << strerror(errno) << endl;
          dataIsWaiting = true;
          this->working = true;
        }
        else {
          // There was an error, but it is EINPROGRESS.
        }
      }
      else {
        // It timed out.
        // Nothing to do.
      }
    }
    this->controlMutex->unlock();
  }
  return dataIsWaiting;
}

bool ClientSession::isFinished() {
  scoped_lock lock{*this->controlMutex};
  return this->finished;
}

void ClientSession::read() {
  scoped_lock lock{*this->controlMutex};

  while (1) {
    auto maxBufferSize = *this->getParameter<uint32_t>(ClientParameter::MAXBUFFERSIZE);
    assert(maxBufferSize);
    vector<char> bufferVector(maxBufferSize);
    char * buffer{bufferVector.data()};
    ssize_t byte_count = recv(this->hServer, buffer, maxBufferSize, 0);
    if (byte_count > 0) {
      this->parser.processChunk(buffer, byte_count);

      // Notify the requester that we have a response.
      while (!this->parser.messages.empty()) {
        auto temp = this->parser.messages.front();
        this->parser.messages.pop();

        auto & [request, response, writeState] = this->messages[this->readSequence];
        response->adoptContents(*temp);
        this->messages.erase(this->readSequence);
        ++this->readSequence;
      }
    }
    else if (byte_count == 0) {
      // There was an orderly shutdown.
      close(this->hServer);
      this->finished = true;
      break;
    }
    else {
      // there was an error.
      switch (errno) {
#if EAGAIN != EWOULDBLOCK
        // https://man7.org/linux/man-pages/man2/recv.2.html
        // POSIX.1 allows either error to be returned for this case, and does
        // not require these constants to have the same value, so a portable
        // application should check for both possibilities.
        //
        // In C++, however, the two cases cannot coexist if EAGAIN and
        // EWOULDBLOCK have the same value, therefore one must be wrapped in a
        // #if preprocessor block.
        case EAGAIN:
#endif
        case EWOULDBLOCK: {
          break;
        }
        default: {
          //cout << strerror(errno) << endl;
          close(this->hServer);
          this->finished = true;
        }
      }
      break;
    }
  }
  //[[maybe_unused]] size_t cursor{0};
  //[[maybe_unused]] size_t count = send(hServer, response.c_str(), response.length(), 0);

  /*
  cout << this->hServer << ":(" << this->input.length() << ") " << this->input << endl;
  if (this->input.length() > 255) {
    close(this->hServer);
  }
  */
  this->working = false;
}

/**
 * Helper macro for repeatedly writing different parts of the message.
 *
 * Requires the following variables to be used in the calling code block:
 *  - `writeOffset`: How much of `source` has already been written.
 *  - `this` : The ClientSession object.
 *  - `phase` : The Phase of transfer of the message.
 *
 * @param source The text to be written.
 * @param completedTarget The new Phase to transition to, if all of `source`
 *   has been fully and successfully written.
 */
#define ATTEMPT_WRITE(source, completedTarget) \
  auto attemptedWriteLength = (source).length() - writeOffset; \
  auto bytesWritten = ::write(this->hServer, string{source}.c_str() + writeOffset, attemptedWriteLength); \
  if (bytesWritten == -1) { \
    phase = ERROR; \
  } \
  else { \
    writeOffset += bytesWritten; \
    if (writeOffset == source.length()) { \
      phase = (completedTarget); \
    } \
    else { \
      writeOffset += bytesWritten; \
    } \
  }


void ClientSession::write() {
  scoped_lock lock{*this->controlMutex};

  if (this->writeSequence < this->requestSequence) {
    // Attempt to write out some of the response.
    auto & [request, response, anyState] = this->messages[this->writeSequence];
    auto & [phase, writeOffset, currentChunk] = any_cast<WriteState &>(anyState);

    if (phase == NEW) {
      // Default to FIXED if no other transport has been declared.
      if (request->getTransport() == Message::Transport::UNDECLARED) {
        request->setTransport(Message::Transport::FIXED);
      }
      phase = SEND_HEADER;
    }

    switch (request->getTransport()) {
      case Message::Transport::FIXED: {
        switch (phase) {
          case SEND_HEADER: {
            auto header = request->getRenderedHeader1() + "Content-Length: " + to_string(request->getContentLength()) + "\r\n\r\n";

            // Write out as much as possible.
            ATTEMPT_WRITE(header, request->getContentLength()
              ? SEND_FIXED
              : FINISHED);
            break;
          }
          case SEND_FIXED: {
            auto message = request->getMessageBody().getText();
            ATTEMPT_WRITE(message, FINISHED);
            break;
          }
          case FINISHED: {
            // Move to the next message.
            ++writeSequence;
            break;
          }
          case ERROR: {
            // There was an error writing to the socket.
            cerr << "Error writing request: " << strerror(errno) << endl;
            this->finished = true;
            close(this->hServer);
            return;
          }
          default: {}
        };
      }
      default: {}
    }
  }
  this->working = false;
}

void ClientSession::enqueue(shared_ptr<Message> request, shared_ptr<Message> response) {
  this->messages[this->requestSequence] = {request, response, WriteState{
    .phase = NEW,
    .writeOffset = 0,
    .currentChunk = 0,
  }};
  ++this->requestSequence;
}

