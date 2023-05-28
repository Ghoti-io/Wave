/**
 * @file
 * Define the Ghoti::Wave::Message class.
 */

#include "message.hpp"
#include "parsing.hpp"

using namespace std;
using namespace Ghoti;
using namespace Ghoti::Wave;

Message::Message(Type type) :
  headerIsRendered{false},
  errorIsSet{false},
  type{type},
  id{0},
  port{0},
  statusCode{},
  contentLength{0},
  message{},
  method{},
  domain{},
  target{},
  version{},
  messageBody{},
  headers{},
  readyPromise{},
  readyFuture{this->readyPromise.get_future()} {
}

void Message::adoptContents(Message & source) {
  auto tempPromise = move(this->readyPromise);
  auto tempFuture = move(this->readyFuture);
  *this = move(source);
  this->readyPromise = move(tempPromise);
  this->readyFuture = move(tempFuture);
}

const shared_string_view & Message::getRenderedHeader1() {
  if (!this->headerIsRendered) {
    this->renderedHeader = "HTTP/1.1 "
      + to_string(this->statusCode)
      + " " + (this->message.length() ? string{this->message} : "OK")
      + "\r\n";
    for (auto & [field, values] : this->headers) {
      if (values.size()) {
        // Output the field name as provided.
        this->renderedHeader += field + ": ";

        // Convert the field name to uppercase for use by isListField().
        auto temp = string{field};
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

Message & Message::setErrorMessage(const shared_string_view & errorMessage) {
  if (!this->headerIsRendered) {
    this->message = errorMessage;
    this->errorIsSet = true;
  }
  return *this;
}

Message & Message::setMessage(const shared_string_view & message) {
  if (!this->headerIsRendered) {
    this->message = message;
  }
  return *this;
}

const shared_string_view & Message::getMessage() const {
  return this->message;
}

Message & Message::setMethod(const shared_string_view & method) {
  if (!this->headerIsRendered) {
    this->method = method;
  }
  return *this;
}

const shared_string_view & Message::getMethod() const {
  return this->method;
}

Message & Message::setTarget(const shared_string_view & target) {
  if (!this->headerIsRendered) {
    this->target = target;
  }
  return *this;
}

const shared_string_view & Message::getTarget() const {
  return this->target;
}

Message & Message::setVersion(const shared_string_view & version) {
  if (!this->headerIsRendered) {
    this->version = version;
  }
  return *this;
}

const shared_string_view & Message::getVersion() const {
  return this->version;
}

void Message::addFieldValue(const shared_string_view & name, const shared_string_view & value) {
  if (!this->headerIsRendered) {
    this->headers[name].push_back(value);
  }
}

const map<shared_string_view, vector<shared_string_view>> & Message::getFields() const {
  return this->headers;
}

Message::Type Message::getType() const {
  return this->type;
}

Message & Message::setMessageBody(const shared_string_view & messageBody) {
  this->messageBody = messageBody;
  this->contentLength = messageBody.length();
  return *this;
}

const shared_string_view & Message::getMessageBody() const {
  return this->messageBody;
}

size_t Message::getContentLength() const {
  return this->contentLength;
}

Message & Message::setPort(size_t port) {
  this->port = port;
  return *this;
}

size_t Message::getPort() const {
  return this->port;
}

Message & Message::setDomain(const shared_string_view & domain) {
  this->domain = domain;
  return *this;
}

const shared_string_view & Message::getDomain() const {
  return this->domain;
}

void Message::setReady(bool isError) {
  this->readyPromise.set_value(isError);
}

future<bool> & Message::getReadyFuture() {
  return this->readyFuture;
}

Message & Message::setId(uint32_t id) {
  this->id = id;
  return *this;
}

uint32_t Message::getId() const {
  return this->id;
}

ostream & Ghoti::Wave::operator<<(ostream & out, Message & message) {
  if (message.getType() == Message::Type::REQUEST) {
    out << "Request:" << endl
      << "  Domain: " << message.getDomain() << endl
      << "  Port: " << message.getPort() << endl
      << "  Target: " << message.getTarget() << endl;
  }
  else {
    out << "Response:" << endl;
    out << "  StatusCode: " << message.getStatusCode() << endl;
  }

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
    out << "  Content-Length: " << contentLength << endl;
    out << "  Message: " << message.getMessage() << endl;
    if (contentLength) {
      out << message.getMessageBody();
    }
   out << endl;
  }
  return out;
}

