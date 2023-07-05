/**
 * @file
 *
 * Header file for declaring the HasClientParameters class.
 */

#ifndef GHOTI_WAVE_HASCLIENTPARAMETERS_HPP
#define GHOTI_WAVE_HASCLIENTPARAMETERS_HPP

#include <ghoti.io/util/hasParameters.hpp>

namespace Ghoti::Wave {

/**
 * Sessings parameters which influence the behavior of Wave and its
 * components.
 */
enum class ClientParameter {
  MAXBUFFERSIZE, ///< The read/write buffer size used when interacting with
                 ///<   sockets.
  MEMCHUNKSIZELIMIT, ///< The maximum size in bytes allowed for a chunk before
                     ///<   converting the chunk to a file.
};

/**
 * Base class to provide consistent defaults to Server and ServerSession
 * classes.
 */
class HasClientParameters: public Ghoti::Util::HasParameters<Ghoti::Wave::ClientParameter> {
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
  virtual Ghoti::Util::ErrorOr<std::any> getParameterDefault(const Ghoti::Wave::ClientParameter & parameter) override;
};

}
#endif // GHOTI_WAVE_HASCLIENTPARAMETERS_HPP

