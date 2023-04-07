/**
 * @file
 *
 * Header file for declaring the Message class.
 */

#ifndef MESSAGE_HPP
#define MESSAGE_HPP

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
  const std::string & getRenderedHeader();

  bool hasError() const;
  Message & setStatusCode(size_t statusCode);
  size_t getStatusCode() const;
  Message & setErrorMessage(const std::string & errorMessage);
  const std::string & getErrorMessage() const;
  Message & setMethod(const std::string & method);
  const std::string & getMethod() const;
  Message & setTarget(const std::string & method);
  const std::string & getTarget() const;
  Message & setVersion(const std::string & method);
  const std::string & getVersion() const;
  void addFieldValue(const std::string & name, const std::string & value);
  const std::map<std::string, std::vector<std::string>> & getFields() const;

  private:
  bool headerIsRendered;
  bool errorIsSet;
  Type type;
  size_t statusCode;
  std::string renderedHeader;
  std::string errorMessage;
  std::string method;
  std::string target;
  std::string version;
  std::map<std::string, std::vector<std::string>> headers;
};

std::ostream & operator<<(std::ostream & out, Message & message);

}

#endif // MESSAGE_HPP

