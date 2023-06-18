/**
 * @file
 * Header file for declaring the Client class.
 */

#ifndef GHOTI_WAVE_MACROS_HPP
#define GHOTI_WAVE_MACROS_HPP


namespace Ghoti::Wave {
  /**
   * Sessings parameters which influence the behavior of Wave and its
   * components.
   */
  enum class Parameter {
    MAXBUFFERSIZE, ///< The read/write buffer size used when interacting with
                   ///<   sockets.
  };
};

#endif // GHOTI_WAVE_MACROS_HPP


