/**
 * @file
 * Define the Ghoti::Wave::Server class.
 */

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sstream>
#include "server.hpp"

using namespace std;
using namespace Ghoti::Pool;
using namespace Ghoti::Wave;

static void dispatchLoop(stop_token stoken) {
  while (1) {
    if (stoken.stop_requested()) {
      return;
    }
    return;
  }
}

Server::Server() {
  // Create the socket.
  int sock_fd;
  uint16_t port;
  if ((sock_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) < 0) {
    throw std::runtime_error("Failed to create a TCP socket");
  }
  // Start listening.
  int opt = 1;
  sockaddr_in server_address;
  if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
    throw runtime_error("Filed to set socket options");
  }

  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  inet_pton(AF_INET, "localhost", &(server_address.sin_addr.s_addr));
  server_address.sin_port = htons(port);

  if (bind(sock_fd, (sockaddr *)&server_address, sizeof(server_address)) < 0) {
    throw runtime_error("Failed to bind to socket");
  }

  if (listen(sock_fd, 10) < 0) {
    ostringstream msg;
    msg << "Failed to listen on port " << port;
    throw runtime_error(msg.str());
  }

  this->dispatchThread = jthread{dispatchLoop};
}

Server::~Server() {
  // Stop the dispatch thread.
  if (this->dispatchThread.joinable()) {
    this->dispatchThread.request_stop();
  }
  // The dispatch thread will join upon destruction of this object.
}

