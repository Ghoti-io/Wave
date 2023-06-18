/**
 * @file
 * Header file for declaring the Client class.
 */

#ifndef MACROS_HPP
#define MACROS_HPP


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

#endif // MACROS_HPP


