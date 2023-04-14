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
#include <map>
#include "message.hpp"
#include <string>

namespace Ghoti::Wave {
class Client;
class ClientSession {
  public:
  ClientSession(int hServer, Client * client);
  ~ClientSession();
  bool hasReadDataWaiting();
  bool hasWriteDataWaiting();
  bool isFinished();
  void read();
  std::unique_ptr<std::mutex> controlMutex;
  std::unique_ptr<std::condition_variable> controlConditionVariable;
  void write();

  private:
  int hServer;
  int sequence;
  size_t writeOffset;
  bool working;
  bool finished;
  Parser parser;
  Client * client;
  std::map<uint64_t, std::pair<std::shared_ptr<Message>, std::shared_ptr<Message>>> messages;
  std::queue<uint64_t> pipeline;
};

}

#endif // CLIENTSESSION_HPP

