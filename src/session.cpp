/**
 * @file
 *
 * Define the Ghoti::Wave::Session class.
 */

#include <arpa/inet.h>
#include <ghoti.io/pool.hpp>
#include <iostream>
#include <poll.h>
#include <sys/socket.h>
#include <sstream>
#include <set>
#include "server.hpp"
#include "session.hpp"
#include <string.h>

using namespace std;
using namespace Ghoti::Pool;
using namespace Ghoti::Wave;

Session::Session(int hClient, Server * server) :
  controlMutex{make_unique<mutex>()},
  hClient{hClient},
  server{server},
  working{false},
  finished{false},
  requestReady{false} {
  cout << "Open: " << this->hClient << endl;
}

Session::~Session() {
  cout << "Close: " << this->hClient << endl;
  close(this->hClient);
}

bool Session::hasDataWaiting() {
  bool dataIsWaiting{false};
  if (this->controlMutex->try_lock()) {
    if (!this->working) {
      // See if there is anything waiting to be read on the socket.
      pollfd pollFd{this->hClient, POLLIN, 0};
      if (poll(&pollFd, 1, 0)) {
        dataIsWaiting = true;
        this->working = true;
      }
    }
    this->controlMutex->unlock();
  }
  return dataIsWaiting;
}

bool Session::isFinished() {
  scoped_lock lock{*this->controlMutex};
  return this->finished;
}

void Session::read() {
  scoped_lock lock{*this->controlMutex};

  while (1) {
    char buffer[MAXBUFFERSIZE] = {0};
    ssize_t byte_count = recv(hClient, buffer, MAXBUFFERSIZE, 0);
    if (byte_count > 0) {
      this->parser.processChunk(buffer, byte_count);

      // Enqueue the completed messages for processing.
      while (!this->parser.messages.empty()) {
        auto temp = this->parser.messages.front();
        this->parser.messages.pop();
        cout << temp;
      }
      /*
      if (this->requestReady) {
        // Yeah, none of this is right.  It's just for testing.
        close(this->hClient);
        this->finished = true;
        this->currentRequest = Request();
      }
      */
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
