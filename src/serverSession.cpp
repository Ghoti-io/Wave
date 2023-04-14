/**
 * @file
 *
 * Define the Ghoti::Wave::ServerSession class.
 */

#include <arpa/inet.h>
#include <ghoti.io/pool.hpp>
#include <iostream>
#include "macros.hpp"
#include "message.hpp"
#include <poll.h>
#include <sys/socket.h>
#include <sstream>
#include <set>
#include "server.hpp"
#include "serverSession.hpp"
#include <string.h>
#include <unistd.h>

using namespace std;
using namespace Ghoti::Pool;
using namespace Ghoti::Wave;

ServerSession::ServerSession(int hClient, Server * server) :
  controlMutex{make_unique<mutex>()},
  hClient{hClient},
  requestSequence{0},
  writeOffset{0},
  working{false},
  finished{false},
  parser{Parser::Type::REQUEST},
  server{server},
  messages{},
  pipeline{} {
  cout << "Open: " << this->hClient << endl;
}

ServerSession::~ServerSession() {
  cout << "Close: " << this->hClient << endl;
  close(this->hClient);
}

bool ServerSession::hasReadDataWaiting() {
  // It may be that the socket is currently in use by another thread.  If so,
  // then do not wait for a response, but rather just return false so as not
  // to block the calling thread.
  bool dataIsWaiting{false};

  if (this->controlMutex->try_lock()) {
    if (!this->working) {
      // See if there is anything waiting to be read on the socket.
      pollfd pollFd{this->hClient, POLLIN | POLLERR, 0};
      if (poll(&pollFd, 1, 0)) {
        dataIsWaiting = true;
        this->working = true;
      }
    }
    this->controlMutex->unlock();
  }
  return dataIsWaiting;
}

bool ServerSession::hasWriteDataWaiting() {
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

bool ServerSession::isFinished() {
  scoped_lock lock{*this->controlMutex};
  return this->finished;
}

void ServerSession::read() {
  scoped_lock lock{*this->controlMutex};

  while (1) {
    char buffer[MAXBUFFERSIZE] = {0};
    ssize_t byte_count = recv(hClient, buffer, MAXBUFFERSIZE, 0);
    if (byte_count > 0) {
      this->parser.processChunk(buffer, byte_count);

      // Enqueue the completed messages for processing.
      while (!this->parser.messages.empty()) {
        auto temp = this->parser.messages.front();
        cout << temp;
        this->parser.messages.pop();
        auto response = make_shared<Message>(Message::Type::RESPONSE);
        response->setStatusCode(200)
          .setMessageBody("Hello World!");
        cout << *response;
        this->messages[this->requestSequence] = {make_shared<Message>(temp), response};
        this->pipeline.push(this->requestSequence);
        ++this->requestSequence;
      }
    }
    else if (byte_count == 0) {
      // There was an orderly shutdown.
      close(this->hClient);
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
          close(this->hClient);
          this->finished = true;
        }
      }
      break;
    }
  }
  //[[maybe_unused]] size_t cursor{0};
  //[[maybe_unused]] size_t count = send(hClient, response.c_str(), response.length(), 0);

  /*
  cout << this->hClient << ":(" << this->input.length() << ") " << this->input << endl;
  if (this->input.length() > 255) {
    close(this->hClient);
  }
  */
  this->working = false;
}

void ServerSession::write() {
  scoped_lock lock{*this->controlMutex};

  if (this->pipeline.size()) {
    // Attempt to write out some of the response.
    auto currentRequest = this->pipeline.front();
    auto [request, response] = this->messages[currentRequest];
    auto assembledMessage = response->getRenderedHeader1() + "Content-Length: " + to_string(response->getContentLength()) + "\r\n\r\n";
    if (response->getContentLength()) {
      assembledMessage += response->getMessageBody();
    }

    // Write out as much as possible.
    auto bytesWritten = ::write(this->hClient, assembledMessage.c_str() + this->writeOffset, assembledMessage.length() - this->writeOffset);

    // Detect any errors.
    if (bytesWritten == -1) {
      cout << "Error writing response: " << strerror(errno) << endl;
      this->finished = true;
      close(this->hClient);
      return;
    }

    // Advance the internal pointer.
    this->writeOffset += bytesWritten;

    // If everything has been written, then remove this message from the
    // pipeline queue.
    if (this->writeOffset == assembledMessage.length()) {
      this->messages.erase(currentRequest);
      this->pipeline.pop();
    }
  }
}

