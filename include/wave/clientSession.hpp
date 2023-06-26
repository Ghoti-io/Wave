/**
 * @file
 *
 * Header file for declaring the ClientSession class.
 */

#ifndef GHOTI_WAVE_CLIENTSESSION_HPP
#define GHOTI_WAVE_CLIENTSESSION_HPP

#include <condition_variable>
#include <ghoti.io/pool.hpp>
#include <memory>
#include <mutex>
#include <ostream>
#include <map>
#include <string>
#include "wave/client.hpp"
#include "wave/hasParameters.hpp"
#include "wave/message.hpp"
#include "wave/parser.hpp"

namespace Ghoti::Wave {
/**
 * Represents a connection to a particular domain/port pair.
 */
class ClientSession : public HasClientParameters {
  public:
  /**
   * Sessings parameters which influence the behavior of Wave and its
   * components.
   */
  enum class Parameter {
    MAXBUFFERSIZE, ///< The read/write buffer size used when interacting with
                   ///<   sockets.
  };

  /**
   * The constructor.
   *
   * The parent Client object will do the work of establishing the socket
   * connection.  Once the connection is established, then this class takes
   * over the communication.
   *
   * @param hServer The socket handle to the Server to which this session will
   *   communicate.
   * @param client A pointer to the parent Client object.
   */
  ClientSession(int hServer, Client * client);

  /**
   * The destructor.
   */
  ~ClientSession();

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
   * Performs a read from the session.
   *
   * This function is intended to be called by the client session's dispatch
   * thread.
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
   * Performs a write to the session.
   *
   * This function is intended to be called by the client session's dispatch
   * thread.
   */
  void write();

  /**
   * Add a request/response pair to the session's queue.
   *
   * The response object was created by the Client, and we will write our
   * results into it as the request is processed.
   *
   * @param request The HTTP request Message.
   * @param response The HTTP response Message.
   */
  void enqueue(std::shared_ptr<Message> request, std::shared_ptr<Message> response);

  private:
  /**
   * The socket handle to the server.
   */
  int hServer;

  /**
   * The index number of the next request to be enqueued.
   *
   * A session may have multiple messages enqueued before the connection has
   * been established.  This variable ensures that messages are handled in
   * the order requested.
   */
  size_t requestSequence;

  /**
   * The index number of the current request being written.
   *
   * A session must send requests in the order that they were enqueued.  This
   * variable tracks which message will be sent next.
   */
  size_t writeSequence;

  /**
   * A byte offset, used to track how many bytes of a message have been
   * written, so that individual write attempts do not duplicate data.
   */
  size_t writeOffset;

  /**
   * The index number of the current request being received.
   *
   * A session may send many requests before a single response is completely
   * received.  This variable tracks the reponse order so that it can be paired
   * to the correct request.
   */
  size_t readSequence;

  /**
   * Tracks whether or not the session has work queued.
   */
  bool working;

  /**
   * Tracks whether or not the session has completed all pending communications.
   */
  bool finished;

  /**
   * The parser object used to parse the raw HTTP stream.
   */
  ResponseParser parser;

  /**
   * A pointer to the client object.
   */
  Client * client;

  /**
   * Tracks message/response pairs.
   *
   * messages[request sequence #] = <request, response>
   */
  std::map<uint64_t, std::pair<std::shared_ptr<Message>, std::shared_ptr<Message>>> messages;
};

}

#endif // GHOTI_WAVE_CLIENTSESSION_HPP

