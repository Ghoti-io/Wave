/**
 * @file
 *
 * Header file for declaring the Session class.
 */

#ifndef PARSER_HPP
#define PARSER_HPP

#include <queue>
#include "message.hpp"
#include <string>

namespace Ghoti::Wave {
class Parser {
  public:
  enum Type {
    REQUEST,
    RESPONSE,
  };
  Parser(Type type);
  void processChunk(const char * buffer, size_t len);
  void parseMessageTarget(const std::string & target);
  std::queue<Message> messages;

  private:
  enum ReadStateMajor {
    NEW_HEADER,
    FIELD_LINE,
    MESSAGE_BODY,
  };
  enum ReadStateMinor {
    BEGINNING_OF_REQUEST_LINE,
    BEGINNING_OF_STATUS_LINE,
    BEGINNING_OF_FIELD_LINE,
    CRLF,
    AFTER_CRLF,
    BEGINNING_OF_REQUEST,
    METHOD,
    AFTER_METHOD,
    REQUEST_TARGET,
    AFTER_REQUEST_TARGET,
    HTTP_VERSION,
    AFTER_HTTP_VERSION,
    FIELD_NAME,
    AFTER_FIELD_NAME,
    BEFORE_FIELD_VALUE,
    FIELD_VALUE,
    SINGLETON_FIELD_VALUE,
    LIST_FIELD_VALUE,
    UNQUOTED_FIELD_VALUE,
    QUOTED_FIELD_VALUE_OPEN,
    QUOTED_FIELD_VALUE_PROCESS,
    QUOTED_FIELD_VALUE_ESCAPE,
    QUOTED_FIELD_VALUE_CLOSE,
    AFTER_FIELD_VALUE,
    FIELD_VALUE_COMMA,
    AFTER_FIELD_VALUE_COMMA,
    AFTER_HEADER_FIELDS,
    MESSAGE_START,
  };
  Type type;
  size_t cursor;
  ReadStateMajor readStateMajor;
  ReadStateMinor readStateMinor;
  size_t majorStart;
  size_t minorStart;
  std::string input;
  std::string errorMessage;
  std::string tempFieldName;
  std::string tempFieldValue;
  Message currentMessage;
};

}

#endif // PARSER_HPP

