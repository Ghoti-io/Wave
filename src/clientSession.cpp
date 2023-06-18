/**
 * @file
 *
 * Define the Ghoti::Wave::ClientSession class.
 */

#include <arpa/inet.h>
#include "clientSession.hpp"
#include <ghoti.io/pool.hpp>
#include <iostream>
#include "macros.hpp"
#include "message.hpp"
#include <poll.h>
#include <sys/socket.h>
#include <sstream>
#include <set>
#include <string.h>
#include <unistd.h>

using namespace std;
using namespace Ghoti::Pool;
using namespace Ghoti::Wave;

ClientSession::ClientSession(int hServer, Client * client) :
  controlMutex{make_unique<mutex>()},
  hServer{hServer},
  requestSequence{0},
  writeSequence{0},
  writeOffset{0},
  readSequence{0},
  working{false},
  finished{false},
  parser{Parser::Type::RESPONSE},
  client{client},
  messages{} {
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
    auto maxBufferSize = *this->getParameter<uint32_t>(Parameter::MAXBUFFERSIZE);
    vector<char> bufferVector(maxBufferSize);
    char * buffer{bufferVector.data()};
    ssize_t byte_count = recv(this->hServer, buffer, maxBufferSize, 0);
    if (byte_count > 0) {
      this->parser.processChunk(buffer, byte_count);

      // Notify the requester that we have a response.
      while (!this->parser.messages.empty()) {
        auto temp = this->parser.messages.front();
        this->parser.messages.pop();

        auto [request, response] = this->messages[this->readSequence];
        // TODO: The value should be true only if the response is from the
        // server, and false otherwise (client closed before finishing, etc.)
        response->adoptContents(*temp);
        response->setReady(true);
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

void ClientSession::write() {
  scoped_lock lock{*this->controlMutex};

  if (this->writeSequence < this->requestSequence) {
    // Attempt to write out some of the response.
    auto & [request, response] = this->messages[this->writeSequence];
    auto assembledMessage = request->getRenderedHeader1() + string{"Content-Length: "} + to_string(request->getContentLength()) + "\r\n\r\n";
    if (request->getContentLength()) {
      assembledMessage += request->getMessageBody().getText();
    }

    // Write out as much as possible.
    auto bytesWritten = ::write(this->hServer, string{assembledMessage}.c_str() + this->writeOffset, assembledMessage.length() - this->writeOffset);

    // Detect any errors.
    if (bytesWritten == -1) {
      cout << "Error writing request: " << strerror(errno) << endl;
      this->finished = true;
      close(this->hServer);
      return;
    }

    // Advance the internal pointer.
    this->writeOffset += bytesWritten;

    // If everything has been written, then move to the next message.
    if (this->writeOffset == assembledMessage.length()) {
      ++this->writeSequence;
    }
  }
  this->working = false;
}

void ClientSession::enqueue(shared_ptr<Message> request, shared_ptr<Message> response) {
  this->messages[this->requestSequence] = {request, response};
  ++this->requestSequence;
}

