/**
 * @file
 *
 * Header file for declaring the Server class.
 */

#ifndef GHOTI_WAVE_SERVER_HPP
#define GHOTI_WAVE_SERVER_HPP

#include <ghoti.io/pool.hpp>
#include <any>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include "wave/hasServerParameters.hpp"

namespace Ghoti::Wave {
class ServerSession;

/**
 * The base Server class.
 *
 * This class is the foundation of the Ghoti.io HTTP server.  It serves as the
 * interface to control and expand the server programmatically.
 */
class Server : public Ghoti::Wave::HasServerParameters {
  public:
  /**
   * These are the error codes that the Server may generate when control
   * functions fail.
   */
  enum ErrorCode {
    NO_ERROR,               ///< No error.
    SERVER_ALREADY_RUNNING, ///< The change could not be applied because the
                            ///<   server is already running.
    START_FAILED,           ///< The server could not be started.
  };

  /**
   * The constructor.
   *
   * The constructor only creates the server object.  It does not begin
   * listening for connections.  In order to begin listening for connections,
   * the Server.start() function must be called.
   *
   * By default, the server will bind to "127.0.0.1" and a port number assigned
   * by the operating system.  This default functionality can be changed by
   * using Server.setAddress() and Server.setPort(), respectively.
   */
  Server();

  /**
   * The destructor.
   *
   * The destructor will call Server.stop().
   */
  ~Server();

  /**
   * Clears any error code and error message that may be set.
   *
   * Error messages are not cleared automatically.  This function must be
   * called explicitly.
   *
   * @returns The server object.
   */
  Server & clearError();

  /**
   * Returns the Server::ErrorCode error that was most recently generated.
   *
   * Calling the function does not clear the error.  The error must be cleared
   * explicitly by calling Server::clearError().
   *
   * @returns The Server::ErrorCode error that was most recently generated.
   */
  ErrorCode getErrorCode() const;

  /**
   * Returns an error message string that was most recently generated.
   *
   * Calling the function does not clear the error.  The error must be cleared
   * explicitly by calling Server::clearError().
   *
   * @returns The error message string that was most recently generated.
   */
  const std::string & getErrorMessage() const;

  /**
   * Returns whether or not the server is running.
   *
   * @returns True/False whether or not the server is running.
   */
  bool isRunning() const;

  /**
   * Set the port that the server is listening on.
   *
   * This setting cannot be changed if the server is running.  If the server is
   * running, then an error will be set.
   *
   * @param port The port number that the server should listen on.
   * @returns The server object.
   */
  Server & setPort(uint16_t port);

  /**
   * Return the server's current port setting.
   *
   * This setting does not imply that the server is active.
   *
   * @returns The current port number.
   */
  uint16_t getPort() const;

  /**
   * Set the ip address that the server is listening on.
   *
   * This setting cannot be changed if the server is running.  If the server is
   * running, then an error will be set.
   *
   * @param ip The ip address that the server should listen on.
   * @returns The server object.
   */
  Server & setAddress(const char * ip);

  /**
   * Return the server's current ip address setting.
   *
   * This setting does not imply that the server is active.
   *
   * @returns The current ip address.
   */
  const std::string & getAddress() const;

  /**
   * Returns the socket handle of the server (if set).
   *
   * @returns The socket handle of the server.
   */
  int getSocketHandle() const;

  /**
   * Start the server listening on the designated ip address and port.
   *
   * @returns The server object.
   */
  Server & start();

  /**
   * Signal the server to stop listening and terminate its thread pool.
   *
   * @returns The server object.
   */
  Server & stop();

  /**
   * The Dispatch loop used by the thread pool to handle asynchronous reading
   * and writing of the server ports.
   *
   * @param stop_token The stop token provided by the jthread to indicate that
   *   the thread should be safely shut down.
   */
  void dispatchLoop(std::stop_token stoken);

  /**
   * Provide a default value for the provided parameter key.
   *
   * The default behavior of this function is to only return an empty optional
   * value.  The intent is for this to be overridden by subclasses.  This is an
   * example of one such override.
   *
   * @param parameter The parameter key to fetch.
   * @return The associated value.
   */
  virtual Ghoti::Util::ErrorOr<std::any> getParameterDefault(const Ghoti::Wave::ServerParameter & parameter) override;

  private:
  /**
   * The thread pool worker queue.
   */
  Ghoti::Pool::Pool workers;

  /**
   * Stores active sessions.
   *
   * The sessions are keyed by the socket handle to which the session is
   * associated.
   */
  std::map<int, std::shared_ptr<Ghoti::Wave::ServerSession>> sessions;

  /**
   * The dispatch thread used to monitor for new connections and to dispatch
   * read/write tasks as needed by the sessions.
   */
  std::jthread dispatchThread;

  /**
   * The most recently generated error code.
   */
  ErrorCode errorCode;

  /**
   * The most recently generated error message.
   */
  std::string errorMessage;

  /**
   * Stores whether or not the server is set to be running.
   */
  bool running;

  /**
   * The socket handle to which the running server is attached.
   */
  int hSocket;

  /**
   * The ip address that the server is configured to use.
   */
  std::string address;

  /**
   * The port that the server is configured to use.
   */
  uint16_t port;
};

}

#endif // GHOTI_WAVE_SERVER_HPP

