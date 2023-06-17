/**
 * @file
 *
 * Header file for declaring the Message class.
 */

#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <future>
#include <map>
#include <ostream>
#include <string>
#include <vector>
#include <ghoti.io/shared_string_view.hpp>

namespace Ghoti::Wave {

/**
 * Represents a HTTP message.
 */
class Message {
  public:

  /**
   * Indicates whether the message is a request or a response.
   */
  enum Type {
    REQUEST,  ///< A HTTP Request.
    RESPONSE, ///< A HTTP Response.
  };

  /**
   * Indicate the transport type of the message.
   */
  enum Transport {
    UNDECLARED, ///< The transport type has not been declared, and the Message
                ///<   should not be considered to be safe for processing.
    FIXED,      ///< The Message is a fixed-length and should not be processed
                ///<   until the full length has been received.
    MULTIPART,  ///< The Message is multipart, each part being separated by a
                ///<   boundary.  The message should not be processed until all
                ///<   parts have been received.
    CHUNKED,    ///< The Message is sent using a chunked encoding.  The chunks
                ///<   can be processed as they arrive, asynchronously.
    STREAM,     ///< The Message did not have a declared (fixed) length.  The
                ///<   received bytes may be processed asynchronously.
  };

  /**
   * The constructor.
   *
   * Messages must have an associated type.
   *
   * @param type The Message::Type of the HTTP message.
   */
  Message(Type type);

  /**
   * Move the contents of `source` into the `this` object, except for the
   * promise and future attributes.
   *
   * This method is necessary because the parser may have already started
   * populating a Message object.  A client, however, must supply the Message
   * object so that the client can know when the promise/future is fulfilled.
   * The only way to accomplish this is to provide a way for the parser to have
   * a provided Message "adopt" the contents of an existing message, but not
   * bother the associated promise/future of the target.
   *
   * @param source The Message whose contents will be adopted into `this`.
   */
  void adoptContents(Message & source);

  /**
   * Get the HTTP/1.1 rendered header as a string.
   *
   * @return A string containing the HTTP/1.1 rendered header.
   */
  const Ghoti::shared_string_view & getRenderedHeader1();

  /**
   * Indicates that the message has an error.
   *
   * @return `true` if there is an error, `false` otherwise.
   */
  bool hasError() const;

  /**
   * Set the Message::Transport type of the Message.
   *
   * @param type The transport type of the Message.
   * @return The Message object.
   */
  Message & setTransport(Message::Transport transport);

  /**
   * Get the Message::Transport type of the Message.
   *
   * @return The transport type of the Message.
   */
  Message::Transport getTransport() const;

  /**
   * Set the status code of the message.
   *
   * Per the HTTP spec, this must be a 3-digit number.
   *
   * @param statusCode The status code of the message.
   * @return The Message object.
   */
  Message & setStatusCode(size_t statusCode);

  /**
   * Get the status code of the message.
   *
   * @return The status code of the message.
   */
  size_t getStatusCode() const;

  /**
   * Set an error message description.
   *
   * @param message The error message description.
   * @return The Message object.
   */
  Message & setErrorMessage(const Ghoti::shared_string_view & message);

  /**
   * Set a status message.
   *
   * @param The status message description.
   * @return The Message object.
   */
  Message & setMessage(const Ghoti::shared_string_view & message);

  /**
   * Get the status message.
   *
   * @return The status message.
   */
  const Ghoti::shared_string_view & getMessage() const;

  /**
   * Set the HTTP method of the message.
   *
   * @param method The HTTP method.
   * @return The Message object.
   */
  Message & setMethod(const Ghoti::shared_string_view & method);

  /**
   * Get the HTTP method of the message.
   *
   * @return The HTTP method.
   */
  const Ghoti::shared_string_view & getMethod() const;

  /**
   * Set the URL target of the message.
   *
   * @param target The URL target.
   * @return The Message object.
   */
  Message & setTarget(const Ghoti::shared_string_view & target);

  /**
   * Get the URL target of the message.
   *
   * @return The URL target.
   */
  const Ghoti::shared_string_view & getTarget() const;

  /**
   * Set the HTTP version of the message.
   *
   * @param version The HTTP version.
   * @return The Message object.
   */
  Message & setVersion(const Ghoti::shared_string_view & version);

  /**
   * Get the HTTP version of the message.
   *
   * @return The HTTP version.
   */
  const Ghoti::shared_string_view & getVersion() const;

  /**
   * Add a header key/value pair.
   *
   * @param name The field name.
   * @param value The field value.
   * @return The Message object.
   */
  Message & addFieldValue(const Ghoti::shared_string_view & name, const Ghoti::shared_string_view & value);

  /**
   * Get the map of all header field key/value pairs.
   *
   * fields[field name] = [field value]
   */
  const std::map<Ghoti::shared_string_view, std::vector<Ghoti::shared_string_view>> & getFields() const;

  /**
   * Get the Message::Type of the message.
   *
   * @return The Message::Type of the message.
   */
  Type getType() const;

  /**
   * Set the content body of the message.
   *
   * Sets the transport type to Message::Transport::FIXED.
   *
   * @param body The content body.
   * @return The Message object.
   */
  Message & setMessageBody(const Ghoti::shared_string_view & body);

  /**
   * Get the content body of the message.
   *
   * @return The content body.
   */
  const Ghoti::shared_string_view & getMessageBody() const;

  /**
   * Get the content length of the message body.
   *
   * @return The content length of the message body.
   */
  size_t getContentLength() const;

  /**
   * Set the port to which the message is targeted.
   *
   * @param port The target port.
   * @return The Message object.
   */
  Message & setPort(size_t port);

  /**
   * Get the port to which the message is targeted.
   *
   * @return The target port.
   */
  size_t getPort() const;

  /**
   * Set the domain to which the message is targeted.
   *
   * @param domain The target domain.
   * @return The Message object.
   */
  Message & setDomain(const Ghoti::shared_string_view & domain);

  /**
   * Get the domain to which the message is targeted.
   *
   * @return The target domain.
   */
  const Ghoti::shared_string_view & getDomain() const;

  /**
   * Notify the associated promise/future that the message is completed.
   *
   * Note that a value of `true` only indicates that the message completed and
   * the response was parsed.  It does not indicate anything about the response
   * code of the message (e.g., the status code may be 404, but because the
   * response actually came from the server, this value should be `true`).
   *
   * A value of `false` indicates that there was a problem either in delivering
   * the message (such as a write failure) or an error in parsing the response.
   *
   * @param requestCompleted `true` if this is the result of a successful HTTP
   *   request, otherwise `false`.
   */
  void setReady(bool isError);

  /**
   * Get the future which will indicate when the message has been fully
   * processed.
   *
   * @return The future used to monitor the status of the message.
   */
  std::future<bool> & getReadyFuture();

  /**
   * Set the ID of the message.
   *
   * @param id The ID number of the message.
   * @return The Message object.
   */
  Message & setId(uint32_t id);

  /**
   * Get the ID of the message.
   *
   * @return The ID number of the message.
   */
  uint32_t getId() const;

  private:
  /**
   * Used to track whether or not the header has been rendered to a string.
   */
  bool headerIsRendered;

  /**
   * Tracks whether or not an error has been set.
   */
  bool errorIsSet;

  /**
   * The Message::Type of the message.
   */
  Type type;

  /**
   * The Message::Transport type of the message.
   */
  Transport transport;

  /**
   * The ID number of the message.
   */
  uint32_t id;

  /**
   * The port to which the message is targeted.
   */
  size_t port;

  /**
   * The status code of the message.
   */
  size_t statusCode;

  /**
   * The contentLength of the message.
   */
  size_t contentLength;

  /**
   * A cached version of the HTTP/1.1 header.
   */
  Ghoti::shared_string_view renderedHeader;

  /**
   * The status message.
   */
  Ghoti::shared_string_view message;

  /**
   * The HTTP method.
   */
  Ghoti::shared_string_view method;

  /**
   * The domain target of the message.
   */
  Ghoti::shared_string_view domain;

  /**
   * The URL target of the message.
   */
  Ghoti::shared_string_view target;

  /**
   * The HTTP version of the message.
   */
  Ghoti::shared_string_view version;

  /**
   * The content body of the message.
   */
  Ghoti::shared_string_view messageBody;

  /**
   * A collection of headers and their associated values.
   *
   * `headers[field name] = [field value]`
   */
  std::map<Ghoti::shared_string_view, std::vector<Ghoti::shared_string_view>> headers;

  /**
   * The promise used for asynchronous notification of when the message has
   * been processed.
   */
  std::promise<bool> readyPromise;

  /**
   * The future used for asynchronous notification of when the message has
   * been processed.
   */
  std::future<bool> readyFuture;
};

/**
 * Helper function to output a Message to a stream.
 *
 * @param out The output stream.
 * @param message The Message to be inserted into the stream.
 *
 * @return The output stream.
 */
std::ostream & operator<<(std::ostream & out, Message & message);

}

#endif // MESSAGE_HPP

