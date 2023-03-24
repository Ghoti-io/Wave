/**
 * @file
 * Define the Ghoti::Wave::Request class.
 */

#include "request.hpp"

using namespace std;
using namespace Ghoti::Wave;

Request::Request() :
  errorIsSet{false},
  statusCode{},
  errorMessage{},
  method{},
  target{},
  version{},
  headers{} {
}

bool Request::hasError() const {
  return this->errorIsSet;
}

Request & Request::setStatusCode(size_t statusCode) {
  this->statusCode = statusCode;
  return *this;
}

size_t Request::getStatusCode() const {
  return this->statusCode;
}

Request & Request::setErrorMessage(const std::string & errorMessage) {
  this->errorMessage = errorMessage;
  this->errorIsSet = true;
  return *this;
}

const std::string & Request::getErrorMessage() const {
  return this->errorMessage;
}

Request & Request::setMethod(const std::string & method) {
  this->method = method;
  return *this;
}

const std::string & Request::getMethod() const {
  return this->method;
}

Request & Request::setTarget(const std::string & target) {
  this->target = target;
  return *this;
}

const std::string & Request::getTarget() const {
  return this->target;
}

Request & Request::setVersion(const std::string & version) {
  this->version = version;
  return *this;
}

const std::string & Request::getVersion() const {
  return this->version;
}

void Request::addFieldLine(const std::string & name, const std::string & value) {
  this->headers[name] = value;
}

ostream & Ghoti::Wave::operator<<(ostream & out, Request & request) {
  out << "Request:" << endl;
  out << "  Method: " << request.getMethod() << endl;
  out << "  Target: " << request.getTarget() << endl;
  out << "  Version: " << request.getVersion() << endl;
  out << "  StatusCode: " << request.getStatusCode() << endl;
  out << "  Error Message: " << request.getErrorMessage() << endl;
  return out;
}

