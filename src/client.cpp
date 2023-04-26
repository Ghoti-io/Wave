/**
 * @file
 *
 * Define the Ghoti::Wave::Client class.
 */

#include <arpa/inet.h>
#include <cstring>
#include <ghoti.io/pool.hpp>
#include <iostream>
#include <sys/socket.h>
#include <sstream>
#include "client.hpp"
#include "clientSession.hpp"

using namespace std;
using namespace Ghoti::Pool;
using namespace Ghoti::Wave;

/**
 * Helper function to create a ClientSession connection to the provided
 * domain and port.
 *
 * @param domain The domain to which the connection points
 * @param port The connection port of the target domain
 * @param client A pointer to the client class
 * @param response A response message which can hold an error message
 * @return A shared pointer to the client session (empty upon failure)
 */
std::shared_ptr<ClientSession> createClientSession(const std::string & domain, size_t port, Client * client, shared_ptr<Message> response) {
  int hSocket;
  // Open a new connection.
  sockaddr_in client_address;
  //socklen_t addrlen{sizeof(client_address)};
  client_address.sin_family = AF_INET;
  client_address.sin_addr.s_addr = INADDR_ANY;
  client_address.sin_port = htons(port);

  // Verify that the address is valid.
  char processed_address[INET_ADDRSTRLEN];
  auto isValid = inet_pton(AF_INET, domain.c_str(), &(client_address.sin_addr));
  if (isValid != 1) {
    response->setMessage("Error parsing client listen address: `" + domain + "`");
    response->setReady(false);
    return {};
  }
  inet_ntop(AF_INET, &(client_address.sin_addr), processed_address, INET_ADDRSTRLEN);
  //this->address = processed_address;

  // Create the socket.
  if ((hSocket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) < 0) {
    response->setMessage("Failed to create a TCP socket");
    response->setReady(false);
    return {};
  }

  if (connect(hSocket, (sockaddr*)&client_address, sizeof(client_address)) < 0) {
    if (errno != EINPROGRESS) {
      response->setMessage(string("Connection Failed: ") + strerror(errno));
      response->setReady(false);
      return {};
    }
    // The TCP handshake is still in progress.
  }

  return make_shared<ClientSession>(hSocket, client);
}

void Client::dispatchLoop(stop_token stopToken) {
  // Create the worker pool queue.
  Pool::Pool pool{1};
  pool.start();

  while (!stopToken.stop_requested()) {
    bool workDone{false};

    // Poll existing connections.
    // Must loop through domains, then sessions.
    for (auto & [domain, portMap] : this->domains) {
      for (auto & [port, sessionsPair] : portMap) {
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
  auto & domain = message->getDomain();
  auto port = message->getPort();

  // Set up an empty domain/port queue if it does not yet exist.
  if (!this->domains.contains(domain)) {
    this->domains[domain] = {};
  }
  if (!this->domains[domain].contains(port)) {
    this->domains[domain][port] = {};
  }

  //auto clientSession = createClientSession(message->getDomain(), message->getPort(), this, response);

  // Add the request to the domain/port queue.
  auto & [sessions, requestQueue] = this->domains[domain][port];
  auto response = make_shared<Message>(Message::Type::RESPONSE);
  requestQueue.push_back({message, response});

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


