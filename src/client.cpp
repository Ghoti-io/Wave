/**
 * @file
 *
 * Define the Ghoti::Wave::Client class.
 */

#include <arpa/inet.h>
#include <ghoti.io/pool.hpp>
#include <iostream>
#include <sys/socket.h>
#include <sstream>
#include "client.hpp"
#include "clientSession.hpp"

using namespace std;
using namespace Ghoti::Pool;
using namespace Ghoti::Wave;

void Client::dispatchLoop(stop_token stopToken) {
  // Create the worker pool queue.
  Pool::Pool pool{1};
  pool.start();

  while (!stopToken.stop_requested()) {
    // Poll existing connections
    for (auto it = this->sessions.begin(); it != this->sessions.end();) {
      auto session = it->second;

      // Service existing requests.
      if (session->hasReadDataWaiting()) {
        pool.enqueue({[=](){
          session->read();
        }});
      }
      else if (session->hasWriteDataWaiting()) {
        pool.enqueue({[=](){
          session->write();
        }});
      }

      // Remove any requests that are dead.
      if (session->isFinished()) {
        it = this->sessions.erase(it);
      }
      else {
        ++it;
      }
    }

    // Service new requests.
    sockaddr_in client;
    socklen_t clientLength = sizeof(client);
    int hClient = accept4(this->hSocket, (sockaddr *)&client, &clientLength, SOCK_NONBLOCK);
    if (hClient < 0) {
      this_thread::sleep_for(1ms);
    }
    else {
      this->sessions.emplace(hClient, make_shared<ClientSession>(hClient, this));
    }
  }

  // TODO: Make session cleanup more elegant.
  this->sessions.clear();

  // Stop and join the worker threads.
  pool.join();
}

Client::Client() : errorCode{ErrorCode::NO_ERROR}, errorMessage{}, running{false}, hSocket{0}, address{"127.0.0.1"}, port{0} {}

Client::~Client() {
  this->stop();
}

Client::ErrorCode Client::getErrorCode() const {
  return this->errorCode;
}

const std::string& Client::getErrorMessage() const {
  return this->errorMessage;
}

bool Client::isRunning() const {
  return this->running;
}

Client& Client::setPort(uint16_t port) {
  if (this->running) {
    this->errorCode = ErrorCode::CLIENT_ALREADY_RUNNING;
    this->errorMessage = "Could not set port of client because it is already running.";
  }
  else {
    this->port = port;
  }
  return *this;
}

uint16_t Client::getPort() const {
  return this->port;
}

Client & Client::setAddress(const char * ip) {
  if (this->running) {
    this->errorCode = ErrorCode::CLIENT_ALREADY_RUNNING;
    this->errorMessage = "Could not set client listening address because client is already running.";
  }
  else {
    this->address = ip;
  }
  return *this;
}

const string & Client::getAddress() const {
  return this->address;
}

int Client::getSocketHandle() const {
  return this->hSocket;
}

Client& Client::start() {
  sockaddr_in client_address;
  socklen_t addrlen{sizeof(client_address)};
  client_address.sin_family = AF_INET;
  client_address.sin_addr.s_addr = INADDR_ANY;

  // Verify that the address is valid.
  char processed_address[INET_ADDRSTRLEN];
  auto isValid = inet_pton(AF_INET, this->address.c_str(), &(client_address.sin_addr));
  if (isValid != 1) {
    this->errorCode = ErrorCode::START_FAILED;
    this->errorMessage = "Error parsing client listen address: `" + this->address + "`";
    return *this;
  }
  inet_ntop(AF_INET, &(client_address.sin_addr), processed_address, INET_ADDRSTRLEN);
  this->address = processed_address;

  // Create the socket.
  if ((this->hSocket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) < 0) {
    this->errorCode = ErrorCode::START_FAILED;
    this->errorMessage = "Failed to create a TCP socket";
    return *this;
  }

  // Set the socket options.
  int opt = 1;
  if (setsockopt(this->hSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
    this->errorCode = ErrorCode::START_FAILED;
    this->errorMessage = "Filed to set socket options";
    return *this;
  }

  // Bind to the port.
  client_address.sin_port = htons(this->port);
  if (bind(this->hSocket, (sockaddr *)&client_address, addrlen) < 0) {
    this->errorCode = ErrorCode::START_FAILED;
    this->errorMessage = "Failed to bind to socket";
    return *this;
  }

  // Get the socket number that was bound to.
  if (getsockname(this->hSocket, (sockaddr *)&client_address, &addrlen) < 0) {
    this->errorCode = ErrorCode::START_FAILED;
    this->errorMessage = "Could not get the socket number";
    return *this;
  }
  this->port = ntohs(client_address.sin_port);

  // Start listening.
  if (listen(this->hSocket, 10) < 0) {
    this->errorCode = ErrorCode::START_FAILED;
    this->errorMessage = "Failed to listen on port " + to_string(port);
    return *this;
  }

  // Start the dispatch thread.
  this->dispatchThread = jthread{[&] (stop_token stoken) {
    this->dispatchLoop(stoken);
  }};

  this->running = true;

  return *this;
}

Client& Client::stop() {
  // Stop the dispatch thread.
  if (this->dispatchThread.joinable()) {
    this->dispatchThread.request_stop();
    this->dispatchThread.join();
    this->running = false;
  }
  if (this->hSocket) {
    close(this->hSocket);
    this->hSocket = 0;
  }
  return *this;
}


