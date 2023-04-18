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

namespace Ghoti::Wave {
class Message {
  public:
  enum Type {
    REQUEST,
    RESPONSE,
  };

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
  const std::string & getRenderedHeader1();

  bool hasError() const;
  Message & setStatusCode(size_t statusCode);
  size_t getStatusCode() const;
  Message & setErrorMessage(const std::string & Message);
  Message & setMessage(const std::string & Message);
  const std::string & getMessage() const;
  Message & setMethod(const std::string & method);
  const std::string & getMethod() const;
  Message & setTarget(const std::string & method);
  const std::string & getTarget() const;
  Message & setVersion(const std::string & method);
  const std::string & getVersion() const;
  void addFieldValue(const std::string & name, const std::string & value);
  const std::map<std::string, std::vector<std::string>> & getFields() const;
  Type getType() const;
  Message & setMessageBody(const std::string & body);
  const std::string & getMessageBody() const;
  size_t getContentLength() const;
  Message & setPort(size_t port);
  size_t getPort() const;
  Message & setDomain(const std::string & domain);
  const std::string & getDomain() const;
  void setReady(bool isError);
  std::future<bool> & getReadyFuture();
  Message & setId(uint32_t id);
  uint32_t getId() const;

  private:
  bool headerIsRendered;
  bool errorIsSet;
  Type type;
  uint32_t id;
  size_t port;
  size_t statusCode;
  size_t contentLength;
  std::string renderedHeader;
  std::string message;
  std::string method;
  std::string domain;
  std::string target;
  std::string version;
  std::string messageBody;
  std::map<std::string, std::vector<std::string>> headers;
  std::promise<bool> readyPromise;
  std::future<bool> readyFuture;
};

std::ostream & operator<<(std::ostream & out, Message & message);

}

#endif // MESSAGE_HPP

