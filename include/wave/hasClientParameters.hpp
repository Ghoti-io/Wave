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
 * Identifies a class as accepting ClientParameters.
 */
using HasClientParameters = Ghoti::Util::HasParameters<Ghoti::Wave::ClientParameter>;

}
#endif // GHOTI_WAVE_HASCLIENTPARAMETERS_HPP

