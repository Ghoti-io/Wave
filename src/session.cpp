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

// https://www.rfc-editor.org/rfc/rfc9110#name-overview
// PATCH - https://www.rfc-editor.org/rfc/rfc5789
static set<string> requestMethods{"GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE", "PATCH"};

Session::Session(int hClient, Server * server) :
  controlMutex{make_unique<mutex>()},
  hClient{hClient},
  server{server},
  working{false},
  finished{false},
  requestReady{false},
  readStateMajor{NEW_HEADER},
  readStateMinor{BEGINNING_OF_LINE},
  majorStart{0},
  minorStart{0},
  input{} {

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
      this->processChunk(buffer, byte_count);
      if (this->requestReady) {
        // Yeah, none of this is right.  It's just for testing.
        cout << this->currentRequest;
        close(this->hClient);
        this->finished = true;
        this->currentRequest = Request();
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

void Session::parseRequestTarget([[maybe_unused]]const std::string & target) {
  // Parse origin-form
  // https://datatracker.ietf.org/doc/html/rfc9112#name-origin-form
  //
  // Parse absolute-form
  // https://datatracker.ietf.org/doc/html/rfc9112#name-absolute-form
  //
  // Parse authority-form
  // https://datatracker.ietf.org/doc/html/rfc9112#name-authority-form
  //
  // Parse asterisk-form
  // https://datatracker.ietf.org/doc/html/rfc9112#name-asterisk-form
}

#define SET_MINOR_STATE(nextState) \
  this->readStateMinor = nextState; \
  this->minorStart = cursor;

#define SET_MAJOR_STATE(nextState) \
  this->readStateMajor = nextState; \
  this->majorStart = cursor; \
  SET_MINOR_STATE(BEGINNING_OF_LINE);

#define SKIP_WHITESPACE(nextState, required) \
  while ((cursor < input_length) && ( \
      isspace(this->input[cursor]) \
      && (this->input[cursor] != '\n') \
      && (this->input[cursor] != '\r'))) { \
    ++cursor; \
  } \
  if ((cursor < input_length) && (!required || (cursor > this->minorStart))) { \
    SET_MINOR_STATE(nextState); \
  }

#define READ_CRLF(nextState, statusCode, errorMessage) \
  size_t len = cursor - this->minorStart; \
  while ((cursor < input_length) && (len < 2)) { \
    if (((len == 0) && (this->input[cursor] != '\r')) \
      || ((len == 1) && (this->input[cursor] != '\n'))) { \
      this->currentRequest.setStatusCode(statusCode).setErrorMessage(errorMessage); \
    } \
    if (!this->currentRequest.hasError() && (len == 1)) { \
      SET_MINOR_STATE(nextState); \
      break; \
    } \
    ++cursor; \
    ++len; \
  }

void Session::processChunk(const char * buffer, size_t len) {
  cout << "Processing (" << len << "): " << string(buffer, len) << endl;
  size_t cursor = this->input.length();
  this->input += string(buffer, len);
  size_t input_length = this->input.length();
  while (!this->currentRequest.hasError() && (cursor < input_length)) {
    switch (this->readStateMajor) {
      case NEW_HEADER:
        // https://datatracker.ietf.org/doc/html/rfc9112#name-request-line
        // request-line   = method SP request-target SP HTTP-version
        switch (this->readStateMinor) {
          case BEGINNING_OF_LINE: {
            SKIP_WHITESPACE(METHOD, false);
            break;
          }
          case METHOD: {
            while ((cursor < input_length) && isgraph(this->input[cursor])) {
              ++cursor;
            }
            if (cursor < input_length) {
              // Finished reading Method.
              string method = this->input.substr(this->minorStart, cursor - this->minorStart);
              if (requestMethods.contains(method)) {
                // Finished reading a valid method.
                this->currentRequest.setMethod(method);
                SET_MINOR_STATE(AFTER_METHOD);
              }
              else {
                // https://www.rfc-editor.org/rfc/rfc9110#section-9.1-10
                this->currentRequest.setStatusCode(501).setErrorMessage("Unrecognized method");
              }
            }
            break;
          }
          case AFTER_METHOD: {
            SKIP_WHITESPACE(REQUEST_TARGET, true);
            break;
          }
          case REQUEST_TARGET: {
            while ((cursor < input_length) && isgraph(this->input[cursor])) {
              ++cursor;
            }
            if (cursor < input_length) {
              // Finished reading request target.
              string target = this->input.substr(this->minorStart, cursor - this->minorStart);
              this->parseRequestTarget(target);
              this->currentRequest.setTarget(target);
              SET_MINOR_STATE(AFTER_REQUEST_TARGET);
            }
            break;
          }
          case AFTER_REQUEST_TARGET: {
            SKIP_WHITESPACE(HTTP_VERSION, true);
            break;
          }
          case HTTP_VERSION: {
            while ((cursor < input_length) && isgraph(this->input[cursor])) {
              ++cursor;
            }
            if (cursor < input_length) {
              // Finished reading request target.
              string version = this->input.substr(this->minorStart, cursor - this->minorStart);
              this->currentRequest.setVersion(version);
              SET_MINOR_STATE(AFTER_HTTP_VERSION);
            }
            break;
          }
          case AFTER_HTTP_VERSION: {
            SKIP_WHITESPACE(CRLF, false);
            break;
          }
          case CRLF: {
            READ_CRLF(AFTER_CRLF, 400, "Error reading request line.");
            break;
          }
          case AFTER_CRLF: {
              SET_MAJOR_STATE(START_LINE);
            break;
          }
          default: {
            this->currentRequest.setStatusCode(400).setErrorMessage("Error reading request line.");
          }
        }
      break;
      case START_LINE:
        switch (this->readStateMinor) {
          default: {
            // Placeholder of where to start next.
            this->currentRequest.setErrorMessage("foo");
            this->requestReady = true;
          }
        }
      break;
      case FIELD_LINE:
        switch (this->readStateMinor) {
          default: {
          }
        }
      break;
      case MESSAGE_BODY:
        switch (this->readStateMinor) {
          default: {
          }
        }
      break;
      default: {
      }
    }
  }
}

