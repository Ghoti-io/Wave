/**
 * @file
 *
 * Header file for declaring the ServerSession class.
 */

#ifndef SERVERSESSION_HPP
#define SERVERSESSION_HPP

#include <condition_variable>
#include <ghoti.io/pool.hpp>
#include <memory>
#include <mutex>
#include <ostream>
#include "parser.hpp"
#include "message.hpp"
#include <string>

namespace Ghoti::Wave {
class Server;
class ServerSession {
  public:
  ServerSession(int hClient, Server * server);
  ~ServerSession();
  bool hasDataWaiting();
  bool isFinished();
  void read();
  std::unique_ptr<std::mutex> controlMutex;
  std::unique_ptr<std::condition_variable> controlConditionVariable;

  private:
  int hClient;
  Server * server;
  bool working;
  bool finished;
  bool messageReady;
  Parser parser;
};

}

#endif // SERVERSESSION_HPP

