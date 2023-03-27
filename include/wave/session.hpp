/**
 * @file
 *
 * Header file for declaring the Session class.
 */

#ifndef SESSION_HPP
#define SESSION_HPP

#include <condition_variable>
#include <ghoti.io/pool.hpp>
#include <memory>
#include <mutex>
#include <ostream>
#include "parser.hpp"
#include "request.hpp"
#include <string>

namespace Ghoti::Wave {
class Server;
class Session {
  public:
  Session(int hClient, Server * server);
  ~Session();
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
  bool requestReady;
  Parser parser;
};

}

#endif // SESSION_HPP

