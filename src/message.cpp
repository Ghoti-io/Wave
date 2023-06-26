/**
 * @file
 * Define the Ghoti::Wave::Message class.
 */

#include <iostream>
#include "wave/message.hpp"
#include "wave/parsing.hpp"

using namespace std;
using namespace Ghoti;
using namespace Ghoti::Wave;

static shared_string_view defaultMethod{"GET"};

Message::Message(Type type) :
  headerIsRendered{false},
  errorIsSet{false},
  parsingIsFinished{false},
  type{type},
  transport{UNDECLARED},
  id{0},
  port{0},
  statusCode{},
  contentLength{0},
  message{},
  method{defaultMethod},
  domain{},
  target{},
  version{},
  messageBody{},
  headers{},
  readySemaphore{0} {
}

void Message::adoptContents(Message & source) {
  // The semaphore cannot be moved, so we have to do things the hard way.
  this->headerIsRendered = move(source.headerIsRendered);
  this->errorIsSet = move(source.errorIsSet);
  this->parsingIsFinished = move(source.parsingIsFinished);
  this->type = move(source.type);
  this->transport = move(source.transport);
  this->id = move(source.id);
  this->port = move(source.port);
  this->statusCode = move(source.statusCode);
  this->contentLength = move(source.contentLength);
  this->message = move(source.message);
  this->method = move(source.method);
  this->domain = move(source.domain);
  this->target = move(source.target);
  this->version = move(source.version);
  this->messageBody = move(source.messageBody);
  this->headers = move(source.headers);
  // The semaphore is not copied, but we can synchronize the state.
  if (source.readySemaphore.try_acquire()) {
    this->readySemaphore.release();
  }
}

const shared_string_view & Message::getRenderedHeader1() {
  if (!this->headerIsRendered) {
    if (this->type == REQUEST) {
      this->renderedHeader = this->getMethod()
        + " " + this->getTarget()
        + " HTTP/1.1\r\n";
    }
    else {
      this->renderedHeader = "HTTP/1.1 "
        + to_string(this->statusCode)
        + " " + (this->message.length() ? string{this->message} : "OK")
        + "\r\n";
    }
    for (auto & [field, values] : this->headers) {
      if (values.size()) {
        // Output the field name as provided.
        this->renderedHeader += field + ": ";

        // Convert the field name to uppercase for use by isListField().
        auto temp = string{field};
        transform(temp.begin(), temp.end(), temp.begin(), ::toupper);

        // Wrap the field values with double quotes only when necessary.
        if (!isListField(temp) && (values.size() == 1)) {
          // Only use double quotes if necessary.
          // https://www.rfc-editor.org/rfc/rfc9110.html#section-5.6.4-5
          if (fieldValueQuotesNeeded(values[0])) {
            this->renderedHeader += '"' + fieldValueEscape(values[0]) + '"';
          }
          else {
            this->renderedHeader += values[0];
          }
          this->renderedHeader += "\r\n";
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

Message & Message::setTransport(Transport transport) {
  this->transport = transport;
  return *this;
}

Message::Transport Message::getTransport() const {
  return this->transport;
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

Message & Message::addFieldValue(const shared_string_view & name, const shared_string_view & value) {
  if (!this->headerIsRendered) {
    this->headers[name].push_back(value);
  }
  return *this;
}

const map<shared_string_view, vector<shared_string_view>> & Message::getFields() const {
  return this->headers;
}

Message::Type Message::getType() const {
  return this->type;
}

Message & Message::setMessageBody(Blob && messageBody) {
  this->contentLength = messageBody.length();
  this->messageBody = move(messageBody);
  this->transport = Message::Transport::FIXED;
  return *this;
}

const Blob & Message::getMessageBody() const {
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

void Message::setReady(bool parsingIsFinished) {
  this->parsingIsFinished = parsingIsFinished;
  this->readySemaphore.release();
}

bool Message::isFinished() const noexcept {
  return this->parsingIsFinished;
}

binary_semaphore & Message::getReadySemaphore() {
  return this->readySemaphore;
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
      << "  Method: " << message.getMethod() << endl
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
        out << value << (--i ? "," : "");
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

