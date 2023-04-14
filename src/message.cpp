/**
 * @file
 * Define the Ghoti::Wave::Message class.
 */

#include "message.hpp"
#include "parsing.hpp"

using namespace std;
using namespace Ghoti::Wave;

Message::Message(Type type) :
  headerIsRendered{false},
  errorIsSet{false},
  type{type},
  statusCode{},
  contentLength{0},
  message{},
  method{},
  target{},
  version{},
  messageBody{},
  headers{} {
}

const string & Message::getRenderedHeader1() {
  if (!this->headerIsRendered) {
    this->renderedHeader = "HTTP/1.1 "
      + to_string(this->statusCode)
      + " " + (this->message.length() ? this->message : "OK")
      + "\r\n";
    for (auto & [field, values] : this->headers) {
      if (values.size()) {
        // Output the field name as provided.
        this->renderedHeader += field + ": ";

        // Convert the field name to uppercase for use by isListField().
        auto temp = field;
        transform(temp.begin(), temp.end(), temp.begin(), ::toupper);

        // Wrap the field values with double quotes only when necessary.
        if (!isListField(temp)) {
          this->renderedHeader += field[0] + "\r\n";
        }
        else {
          bool isFirst{true};
          for (auto & value : values) {
            this->renderedHeader += isFirst ? "" : ", ";
            // Only use double quotes if necessary.
            // https://www.rfc-editor.org/rfc/rfc9110.html#section-5.6.4-5
            if (fieldValueQuotesNeeded(value)) {
              this->renderedHeader += '"' + fieldValueEscape(value) + '"';
            }
            else {
              this->renderedHeader += value;
            }
            isFirst = false;
          }
          this->renderedHeader += "\r\n";
        }
      }
    }
    this->headerIsRendered = true;
  }
  return this->renderedHeader;
}

bool Message::hasError() const {
  return this->errorIsSet;
}

Message & Message::setStatusCode(size_t statusCode) {
  if (!this->headerIsRendered) {
    this->statusCode = statusCode;
  }
  return *this;
}

size_t Message::getStatusCode() const {
  return this->statusCode;
}

Message & Message::setErrorMessage(const std::string & errorMessage) {
  if (!this->headerIsRendered) {
    this->message = errorMessage;
    this->errorIsSet = true;
  }
  return *this;
}

Message & Message::setMessage(const std::string & message) {
  if (!this->headerIsRendered) {
    this->message = message;
  }
  return *this;
}

const std::string & Message::getMessage() const {
  return this->message;
}

Message & Message::setMethod(const std::string & method) {
  if (!this->headerIsRendered) {
    this->method = method;
  }
  return *this;
}

const std::string & Message::getMethod() const {
  return this->method;
}

Message & Message::setTarget(const std::string & target) {
  if (!this->headerIsRendered) {
    this->target = target;
  }
  return *this;
}

const std::string & Message::getTarget() const {
  return this->target;
}

Message & Message::setVersion(const std::string & version) {
  if (!this->headerIsRendered) {
    this->version = version;
  }
  return *this;
}

const std::string & Message::getVersion() const {
  return this->version;
}

void Message::addFieldValue(const std::string & name, const std::string & value) {
  if (!this->headerIsRendered) {
    this->headers[name].push_back(value);
  }
}

const map<string, vector<string>> & Message::getFields() const {
  return this->headers;
}

Message::Type Message::getType() const {
  return this->type;
}

Message & Message::setMessageBody(const std::string & messageBody) {
  this->messageBody = messageBody;
  this->contentLength = messageBody.length();
  return *this;
}

const string & Message::getMessageBody() const {
  return this->messageBody;
}

size_t Message::getContentLength() const {
  return this->contentLength;
}

ostream & Ghoti::Wave::operator<<(ostream & out, Message & message) {
  if (message.getType() == Message::Type::REQUEST) {
    out << "Request:" << endl;
    out << "  Method: " << message.getMethod() << endl;
    out << "  Target: " << message.getTarget() << endl;
    out << "  Version: " << message.getVersion() << endl;
  }
  else {
    out << "Response:" << endl;
    out << "  StatusCode: " << message.getStatusCode() << endl;
  }

  out << "  Message: " << message.getMessage() << endl;
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
  if (message.getType() == Message::Type::RESPONSE) {
    size_t contentLength = message.getContentLength();
    out << "Content-Length: " << contentLength << endl;
    if (contentLength) {
      out << endl << message.getMessageBody();
    }
   out << endl;
  }
  return out;
}

