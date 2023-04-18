/**
 * @file
 *
 * Header file for declaring the Client class.
 */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <future>
#include <ghoti.io/pool.hpp>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <thread>

namespace Ghoti::Wave {
class ClientSession;
class Message;
class Client {
  public:
  Client();
  ~Client();
  bool isRunning() const;
  Client & start();
  Client & stop();
  void dispatchLoop(std::stop_token stoken);
  std::shared_ptr<Message> sendRequest(std::shared_ptr<Message> message);

  private:
  Ghoti::Pool::Pool workers;
  std::map<std::string, std::pair<std::set<std::shared_ptr<Ghoti::Wave::ClientSession>>, std::vector<std::shared_ptr<Message>>>> domains;
  std::jthread dispatchThread;
  bool running;
};

}

#endif // CLIENT_HPP

