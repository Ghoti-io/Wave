/**
 * @file
 * Define the Ghoti::Wave::Message class.
 */

#include "message.hpp"

using namespace std;
using namespace Ghoti::Wave;

Message::Message() :
  errorIsSet{false},
  statusCode{},
  errorMessage{},
  method{},
  target{},
  version{},
  headers{} {
}

bool Message::hasError() const {
  return this->errorIsSet;
}

Message & Message::setStatusCode(size_t statusCode) {
  this->statusCode = statusCode;
  return *this;
}

size_t Message::getStatusCode() const {
  return this->statusCode;
}

Message & Message::setErrorMessage(const std::string & errorMessage) {
  this->errorMessage = errorMessage;
  this->errorIsSet = true;
  return *this;
}

const std::string & Message::getErrorMessage() const {
  return this->errorMessage;
}

Message & Message::setMethod(const std::string & method) {
  this->method = method;
  return *this;
}

const std::string & Message::getMethod() const {
  return this->method;
}

Message & Message::setTarget(const std::string & target) {
  this->target = target;
  return *this;
}

const std::string & Message::getTarget() const {
  return this->target;
}

Message & Message::setVersion(const std::string & version) {
  this->version = version;
  return *this;
}

const std::string & Message::getVersion() const {
  return this->version;
}

void Message::addFieldValue(const std::string & name, const std::string & value) {
  this->headers[name].push_back(value);
}

const map<string, vector<string>> & Message::getFields() const {
  return this->headers;
}

ostream & Ghoti::Wave::operator<<(ostream & out, Message & message) {
  out << "Message:" << endl;
  out << "  Method: " << message.getMethod() << endl;
  out << "  Target: " << message.getTarget() << endl;
  out << "  Version: " << message.getVersion() << endl;
  out << "  StatusCode: " << message.getStatusCode() << endl;
  out << "  Error Message: " << message.getErrorMessage() << endl;
  if (message.getFields().size()) {
    out << "  Fields:" << endl;
    for (auto & [name, values] : message.getFields()) {
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

