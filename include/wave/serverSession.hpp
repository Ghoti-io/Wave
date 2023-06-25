/**
 * @file
 *
 * Header file for declaring the ServerSession class.
 */

#ifndef GHOTI_WAVE_SERVERSESSION_HPP
#define GHOTI_WAVE_SERVERSESSION_HPP

#include <condition_variable>
#include <ghoti.io/pool.hpp>
#include <memory>
#include <map>
#include <mutex>
#include <ostream>
#include <string>
#include "wave/hasParameters.hpp"
#include "wave/message.hpp"
#include "wave/parser.hpp"
#include "wave/server.hpp"

namespace Ghoti::Wave {

/**
 * Represents a persistent connection with a client.
 */
class ServerSession : public HasParameters<Ghoti::Wave::Server::Parameter> {
  public:
  /**
   * The constructor.
   *
   * @param hClient The socket handle to the client connection.
   * @param server A pointer to the parent Server object.
   */
  ServerSession(int hClient, Server * server);

  /**
   * The destructor.
   */
  ~ServerSession();

  /**
   * Checks to see whether or not the session has data waiting to be read from
   * the socket.
   *
   * This is non-blocking mutex controlled.  If the session is currently
   * working, then this function will return false.
   *
   * @return Whether or not the session has data waiting to be read from the
   *   socket.
   */
  bool hasReadDataWaiting();

  /**
   * Checks to see whether or not the session has data waiting to be written to
   * the socket.
   *
   * This is non-blocking mutex controlled.  If the session is currently
   * working, then this function will return false.
   *
   * @return Whether or not the session has data waiting to be written to the
   *   socket.
   */
  bool hasWriteDataWaiting();

  /**
   * Indicates whether or not the session has completed all communications and
   * may be terminated.
   *
   * @return `true` if all communications have completed, `false` otherwise.
   */
  bool isFinished();

  /**
   * Perform a read from the session.
   *
   * This function is intended to be called by the server's thread pool worker
   * queue, probably in a lambda expression.
   */
  void read();

  /**
   * Used to synchronize access to the session to make it thread safe.
   */
  std::unique_ptr<std::mutex> controlMutex;

  /**
   * Used to synchronize access to the session to make it thread safe.
   */
  std::unique_ptr<std::condition_variable> controlConditionVariable;

  /**
   * Perform a write to the session.
   *
   * This function is intended to be called by the server's thread pool worker
   * queue, probably in a lambda expression.
   */
  void write();

  private:
  /**
   * The socket handle to the client.
   */
  int hClient;

  /**
   * A monotonically increasing counter to track request/response pairs.
   */
  size_t requestSequence;

  /**
   * A byte offset used to track how many bytes of a message have been written,
   * so that individual write attempts do not duplicate data.
   */
  size_t writeOffset;

  /**
   * Tracks whether or not the session has work queued.
   */
  bool working;

  /**
   * Tracks whether or not the session has completed all pending
   * communications.
   */
  bool finished;

  /**
   * The parser object used to parse the raw HTTP stream.
   */
  Parser parser;

  /**
   * A pointer to the server object.
   */
  Server * server;

  /**
   * Tracks request/response pairs.
   *
   * `messages[request sequence #] = <request, response>`
   */
  std::map<uint64_t, std::pair<std::shared_ptr<Message>, std::shared_ptr<Message>>> messages;

  /**
   * Simple queue to track which request sequence # should be parsed next.
   */
  std::queue<uint64_t> pipeline;
};

}

#endif // GHOTI_WAVE_SERVERSESSION_HPP

