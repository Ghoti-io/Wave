/**
 * @file
 *
 * Header file for declaring the HasServerParameters class.
 */

#ifndef GHOTI_WAVE_HASSERVERPARAMETERS_HPP
#define GHOTI_WAVE_HASSERVERPARAMETERS_HPP

#include "wave/hasParameters.hpp"

namespace Ghoti::Wave {
class ServerSession;

/**
 * Sessings parameters which influence the behavior of a Server.
 */
enum class ServerParameter {
  MAXBUFFERSIZE, ///< The read/write buffer size used when interacting with
                 ///<   sockets.
  MEMCHUNKSIZELIMIT, ///< The maximum size in bytes allowed for a chunk before
                     ///<   converting the chunk to a file.
};

/**
 * Base class to provide consistent defaults to Server and ServerSession
 * classes.
 */
class HasServerParameters : public Ghoti::Wave::HasParameters<Ghoti::Wave::ServerParameter> {
  public:
  /**
   * Provide a default value for the provided parameter key.
   *
   * The default behavior of this function is to only return an empty optional
   * value.  The intent is for this to be overridden by subclasses.
   *
   * @param parameter The parameter key to fetch.
   * @return The associated value.
   */
  virtual std::optional<std::any> getParameterDefault(const Ghoti::Wave::ServerParameter & parameter) override;
};

}
#endif // GHOTI_WAVE_HASSERVERPARAMETERS_HPP

