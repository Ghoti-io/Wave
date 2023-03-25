/**
 * @file
 *
 * Header file for declaring the Request class.
 */

#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <map>
#include <ostream>
#include <string>
#include <vector>

namespace Ghoti::Wave {
class Request {
  public:
  Request();

  bool hasError() const;
  Request & setStatusCode(size_t statusCode);
  size_t getStatusCode() const;
  Request & setErrorMessage(const std::string & errorMessage);
  const std::string & getErrorMessage() const;
  Request & setMethod(const std::string & method);
  const std::string & getMethod() const;
  Request & setTarget(const std::string & method);
  const std::string & getTarget() const;
  Request & setVersion(const std::string & method);
  const std::string & getVersion() const;
  void addFieldLine(const std::string & name, const std::string & value);

  private:
  bool errorIsSet;
  size_t statusCode;
  std::string errorMessage;
  std::string method;
  std::string target;
  std::string version;
  std::map<std::string, std::vector<std::string>> headers;
};

std::ostream & operator<<(std::ostream & out, Request & request);

}

#endif // REQUEST_HPP

