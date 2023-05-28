/**
 * @file
 *
 * Header file for declaring the Client class.
 */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <future>
#include <ghoti.io/pool.hpp>
#include <ghoti.io/shared_string_view.hpp>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <string>
#include <thread>

namespace Ghoti::Wave {
class ClientSession;
class Message;

/**
 * Represents a client and all of its HTTP connections.
 *
 * This class is currently only used for testing (so that the HTTP connection
 * can be controlled explicitly), but that does not mean that it can't be used
 * for more.
 *
 * The Client object can establish connections to a server, receive message
 * requests and forward them to the appropriate session, and report on the
 * status of the connections.
 *
 * This class exists primarily for testing the server, and as such offers
 * fine-grained control of enabling and disabling features.
 */
class Client {
  public:
  /**
   * The constructor.
   */
  Client();

  /**
   * The destructor.
   */
  ~Client();

  /**
   * Indicates whether or not the client and its thread pools are currently
   * active.
   *
   * @return Whether or not the client and its thread pools are currently
   *   active.
   */
  bool isRunning() const;

  /**
   * Instructs the client to start its thread pool and begin processing the
   * requests in its queue.
   *
   * @return The Client object.
   */
  Client & start();

  /**
   * Instructs the client to gracefully shut down its thread pool.
   *
   * @return The Client object.
   */
  Client & stop();

  /**
   * The dispatch loop used by the thread pool to process sending requests and
   * receiving responses.
   *
   * @param stoken The jthread stop token, used to alert the thread that it
   *   should gracefully shut down.
   */
  void dispatchLoop(std::stop_token stoken);

  /**
   * Enqueues a message to be sent to a client.  This returns a shared pointer
   * to a Message which will contain the response when the request is
   * completed.
   *
   * @param message The request to be sent to a client.
   * @return A shared pointer to a Message the will eventually contain the
   *   response when the request is completed.
   */
  std::shared_ptr<Message> sendRequest(std::shared_ptr<Message> message);

  private:
  /**
   * The thread pool worker queue.
   */
  Ghoti::Pool::Pool workers;

  /**
   * Stores all connections and their request queues.
   *
   * domains[domain][port] = {set{ClientSession}, queue{{request, response}}}
   */
  std::map<Ghoti::shared_string_view, std::map<size_t, std::pair<std::set<std::shared_ptr<Ghoti::Wave::ClientSession>>, std::queue<std::pair<std::shared_ptr<Message>, std::shared_ptr<Message>>>>>> domains;

  /**
   * The thread that runs the read/write processing queues.
   */
  std::jthread dispatchThread;

  /**
   * Whether or not the client is processing read/write actions from the
   * sockets.
   */
  bool running;
};

}

#endif // CLIENT_HPP

