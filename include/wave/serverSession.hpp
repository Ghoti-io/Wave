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
#include <map>
#include "message.hpp"
#include <string>

namespace Ghoti::Wave {
class Server;
class ServerSession {
  public:
  ServerSession(int hClient, Server * server);
  ~ServerSession();
  bool hasReadDataWaiting();
  bool hasWriteDataWaiting();
  bool isFinished();
  void read();
  std::unique_ptr<std::mutex> controlMutex;
  std::unique_ptr<std::condition_variable> controlConditionVariable;
  void write();

  private:
  int hClient;
  int sequence;
  size_t writeOffset;
  bool working;
  bool finished;
  Parser parser;
  Server * server;
  std::map<uint64_t, std::pair<std::shared_ptr<Message>, std::shared_ptr<Message>>> messages;
  std::queue<uint64_t> pipeline;
};

}

#endif // SERVERSESSION_HPP

