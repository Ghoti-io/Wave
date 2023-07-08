/**
 * @file
 *
 * Header file for declaring the HasServerParameters class.
 */

#ifndef GHOTI_WAVE_HASSERVERPARAMETERS_HPP
#define GHOTI_WAVE_HASSERVERPARAMETERS_HPP

#include <ghoti.io/util/hasParameters.hpp>

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
 * Identifies a class as accepting ClientParameters.
 */
using HasServerParameters = Ghoti::Util::HasParameters<Ghoti::Wave::ServerParameter>;

}
#endif // GHOTI_WAVE_HASSERVERPARAMETERS_HPP

