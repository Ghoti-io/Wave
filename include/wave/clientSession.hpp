/**
 * @file
 *
 * Header file for declaring the ClientSession class.
 */

#ifndef CLIENTSESSION_HPP
#define CLIENTSESSION_HPP

#include <condition_variable>
#include <ghoti.io/pool.hpp>
#include <memory>
#include <mutex>
#include <ostream>
#include "parser.hpp"
#include "message.hpp"
#include <string>

namespace Ghoti::Wave {
class Client;
class ClientSession {
  public:
  ClientSession(int hServer, Client * client);
  ~ClientSession();
  bool hasDataWaiting();
  bool isFinished();
  void read();
  std::unique_ptr<std::mutex> controlMutex;
  std::unique_ptr<std::condition_variable> controlConditionVariable;

  private:
  int hServer;
  Client * client;
  bool working;
  bool finished;
  bool messageReady;
  Parser parser;
};

}

#endif // CLIENTSESSION_HPP

