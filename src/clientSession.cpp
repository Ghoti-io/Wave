/**
 * @file
 *
 * Define the Ghoti::Wave::ClientSession class.
 */

#include <arpa/inet.h>
#include "client.hpp"
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
  working{false},
  finished{false},
  parser{Parser::Type::RESPONSE},
  client{client},
  messages{},
  pipeline{} {
  cout << "Client Open: " << this->hServer << endl;
}

ClientSession::~ClientSession() {
  cout << "Client Close: " << this->hServer << endl;
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
        dataIsWaiting = true;
        this->working = true;
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
    dataIsWaiting = this->pipeline.size();
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
    char buffer[MAXBUFFERSIZE] = {0};
    ssize_t byte_count = recv(hServer, buffer, MAXBUFFERSIZE, 0);
    if (byte_count > 0) {
      this->parser.processChunk(buffer, byte_count);

      // Notify the requester that we have a response.
      while (!this->parser.messages.empty()) {
        auto temp = this->parser.messages.front();
        this->parser.messages.pop();
        cout << "RECEIVED:" << endl;
        cout << *temp;

        auto currentRequest = this->pipeline.front();
        auto & [request, response] = this->messages[currentRequest];
        // TODO: The value should be true only if the response is from the
        // server, and false otherwise (client closed before finishing, etc.)
        response->setReady(true);

        this->messages.erase(currentRequest);
        this->pipeline.pop();
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
    auto & [request, responsePromise] = this->messages[this->writeSequence];
    auto assembledMessage = request->getRenderedHeader1() + "Content-Length: " + to_string(request->getContentLength()) + "\r\n\r\n";
    if (request->getContentLength()) {
      assembledMessage += request->getMessageBody();
    }

    // Write out as much as possible.
    auto bytesWritten = ::write(this->hServer, assembledMessage.c_str() + this->writeOffset, assembledMessage.length() - this->writeOffset);

    // Detect any errors.
    if (bytesWritten == -1) {
      cout << "Error writing request: " << strerror(errno) << endl;
      this->finished = true;
      close(this->hServer);
      return;
    }

    // Advance the internal pointer.
    this->writeOffset += bytesWritten;

    // If everything has been written, then remove this message from the
    // pipeline queue.
    if (this->writeOffset == assembledMessage.length()) {
      this->messages.erase(this->writeSequence);
      this->pipeline.pop();
      ++this->writeSequence;
    }
  }
}

shared_ptr<Message> ClientSession::enqueue(shared_ptr<Message> request) {
  auto response = make_shared<Message>(Message::Type::RESPONSE);
  this->messages[this->requestSequence] = {request, response};
  this->pipeline.push(this->requestSequence);
  ++this->requestSequence;
  return response;
}

