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

void Request::addFieldValue(const std::string & name, const std::string & value) {
  this->headers[name].push_back(value);
}

const map<string, vector<string>> & Request::getFields() const {
  return this->headers;
}

ostream & Ghoti::Wave::operator<<(ostream & out, Request & request) {
  out << "Request:" << endl;
  out << "  Method: " << request.getMethod() << endl;
  out << "  Target: " << request.getTarget() << endl;
  out << "  Version: " << request.getVersion() << endl;
  out << "  StatusCode: " << request.getStatusCode() << endl;
  out << "  Error Message: " << request.getErrorMessage() << endl;
  if (request.getFields().size()) {
    out << "  Fields:" << endl;
    for (auto & [name, values] : request.getFields()) {
      out << "    " << name << ": ";
      size_t i = values.size();

      for (auto & value : values) {
        out << '"' << value << '"' << (--i ? "," : "");
      }
      out << endl;
    }
  }
  return out;
}

