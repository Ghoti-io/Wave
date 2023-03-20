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
    enum ErrorCode {
      NO_ERROR,
      SERVER_ALREADY_RUNNING,
      START_FAILED,
    };
  Server();
  ~Server();
  ErrorCode getErrorCode() const;
  const std::string& getErrorMessage() const;
  bool isRunning() const;
  Server& setPort(uint16_t port);
  uint16_t getPort() const;
  Server& setAddress(const char * ip);
  const char * getAddress() const;
  Server& start();
  Server& stop();

  private:
  Ghoti::Pool::Pool workers;
  std::jthread dispatchThread;
  ErrorCode errorCode;
  std::string errorMessage;
  bool running;
  int hSocket;
  std::string address;
  uint16_t port;
};

}

#endif // SERVER_HPP

