/**
 * @file
 *
 * Header file for declaring the Client class.
 */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <ghoti.io/pool.hpp>
#include <map>
#include <memory>
#include <string>
#include <thread>

namespace Ghoti::Wave {
class ClientSession;
class Client {
  public:
    enum ErrorCode {
      NO_ERROR,
      CLIENT_ALREADY_RUNNING,
      START_FAILED,
    };
  Client();
  ~Client();
  ErrorCode getErrorCode() const;
  const std::string & getErrorMessage() const;
  bool isRunning() const;
  Client & setPort(uint16_t port);
  uint16_t getPort() const;
  Client & setAddress(const char * ip);
  const std::string & getAddress() const;
  int getSocketHandle() const;
  Client & start();
  Client & stop();
  void dispatchLoop(std::stop_token stoken);

  private:
  Ghoti::Pool::Pool workers;
  std::map<int, std::shared_ptr<Ghoti::Wave::ClientSession>> sessions;
  std::jthread dispatchThread;
  ErrorCode errorCode;
  std::string errorMessage;
  bool running;
  int hSocket;
  std::string address;
  uint16_t port;
};

}

#endif // CLIENT_HPP

