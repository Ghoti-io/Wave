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
#include "request.hpp"
#include <string>

namespace Ghoti::Wave {
class Server;
class Session {
  public:
  Session(int hClient, Server * server);
  bool hasDataWaiting();
  bool isFinished();
  void read();
  void processChunk(const char * buffer, size_t len);
  std::unique_ptr<std::mutex> controlMutex;
  std::unique_ptr<std::condition_variable> controlConditionVariable;
  void parseRequestTarget(const std::string & target);

  private:
  enum ReadStateMajor {
    NEW_HEADER,
    FIELD_LINE,
    MESSAGE_BODY,
  };
  enum ReadStateMinor {
    BEGINNING_OF_LINE,
    CRLF,
    AFTER_CRLF,
    METHOD,
    AFTER_METHOD,
    REQUEST_TARGET,
    AFTER_REQUEST_TARGET,
    HTTP_VERSION,
    AFTER_HTTP_VERSION,
  };
  int hClient;
  Server * server;
  bool working;
  bool finished;
  bool requestReady;
  ReadStateMajor readStateMajor;
  ReadStateMinor readStateMinor;
  size_t majorStart;
  size_t minorStart;
  std::string input;
  std::string errorMessage;
  std::string tempFieldName;
  Request currentRequest;
};

}

#endif // SESSION_HPP

