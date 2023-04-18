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
    bool workDone{false};

    // Poll existing connections.
    // Must loop through domains, then sessions.
    for (auto & [domain, sessionsPair] : this->domains) {
      auto & [sessions, requestQueue] = sessionsPair;
      for (auto & session : sessions) {
        // Service existing requests.
        if (session->hasReadDataWaiting()) {
          pool.enqueue({[=](){
            session->read();
          }});
          workDone = true;
        }
        else if (session->hasWriteDataWaiting()) {
          pool.enqueue({[=](){
            session->write();
          }});
          workDone = true;
        }

        // Remove any requests that are dead.
        if (session->isFinished()) {
          sessions.erase(session);
        }
      }
    }

    if (!workDone) {
      this_thread::sleep_for(1ms);
    }
  }

  // TODO: Make session cleanup more elegant.
  // Specifically, make sure that all client sessions are stopped.
  this->domains.clear();

  // Stop and join the worker threads.
  pool.join();
}

Client::Client() : running{true} {
  this->dispatchThread = jthread{[&] (stop_token stoken) {
    this->dispatchLoop(stoken);
  }};
}

Client::~Client() {
  this->stop();
}

bool Client::isRunning() const {
  return this->running;
}

shared_ptr<Message> Client::sendRequest(shared_ptr<Message> message) {
  // The response which will eventually be returned from this function,
  // if there is an error.
  auto response = make_shared<Message>(Message::Type::RESPONSE);

  // Determine whether or not a suitable session already exists.
  string domainPort = message->getDomain() + ":" + to_string(message->getPort());

  string errorMessage{};

  if (this->domains.contains(domainPort)) {
    auto & [sessions, requestQueue] = this->domains[domainPort];
    for (auto & session : sessions) {
      // TODO: Handle multiple sessions, if needed.
      //if (session->isIdle())
      return session->enqueue(message);
    }
  }
  else {
    int hSocket;
    // Open a new connection.
    sockaddr_in client_address;
    //socklen_t addrlen{sizeof(client_address)};
    client_address.sin_family = AF_INET;
    client_address.sin_addr.s_addr = INADDR_ANY;

    // Verify that the address is valid.
    char processed_address[INET_ADDRSTRLEN];
    auto isValid = inet_pton(AF_INET, message->getDomain().c_str(), &(client_address.sin_addr));
    if (isValid != 1) {
      response->setMessage("Error parsing client listen address: `" + message->getDomain() + "`");
      response->setReady(false);
      return response;
    }
    inet_ntop(AF_INET, &(client_address.sin_addr), processed_address, INET_ADDRSTRLEN);
    //this->address = processed_address;

    // Create the socket.
    if ((hSocket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) < 0) {
      response->setMessage("Failed to create a TCP socket");
      response->setReady(false);
      return response;
    }

    /*
    // Get the socket number that was bound to.
    if (getsockname(this->hSocket, (sockaddr *)&client_address, &addrlen) < 0) {
      this->errorCode = ErrorCode::START_FAILED;
      this->errorMessage = "Could not get the socket number";
      return *this;
    }
    this->port = ntohs(client_address.sin_port);
    */

    if (connect(hSocket, (sockaddr*)&client_address, sizeof(client_address)) < 0) {
      response->setMessage("Connection Failed");
      response->setReady(false);
      return response;
    }
  }

  return response;
}

Client& Client::stop() {
  // Stop the dispatch thread.
  if (this->dispatchThread.joinable()) {
    this->dispatchThread.request_stop();
    this->dispatchThread.join();
    this->running = false;
  }
  return *this;
}

