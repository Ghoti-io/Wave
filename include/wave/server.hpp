/**
 * @file
 * Header file for declaring the Server class.
 */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <thread>
#include <ghoti.io/pool.hpp>

namespace Ghoti::Wave {
class Server {
  public:
  Server();
  ~Server();

  private:
  Ghoti::Pool::Pool workers;
  std::jthread dispatchThread;
};

};

#endif // SERVER_HPP

