/**
 * @file
 *
 * Define the Ghoti::Wave::Parser class.
 */

#include <arpa/inet.h>
#include <ghoti.io/pool.hpp>
#include <iostream>
#include "parser.hpp"
#include "parsing.hpp"
#include <set>
#include <string.h>

using namespace std;
using namespace Ghoti::Pool;
using namespace Ghoti::Wave;

#define SET_MINOR_STATE(nextState) \
  this->readStateMinor = nextState; \
  this->minorStart = cursor;

#define SET_MAJOR_STATE(nextState) \
  this->readStateMajor = nextState; \
  this->majorStart = cursor; \
  SET_MINOR_STATE(BEGINNING_OF_LINE);

#define READ_WHITESPACE_OPTIONAL(nextState) \
  while ((cursor < input_length) && ( \
      isspace(this->input[cursor]) \
      && (this->input[cursor] != '\n') \
      && (this->input[cursor] != '\r'))) { \
    ++cursor; \
  } \
  if ((cursor < input_length) && ( \
      !isspace(this->input[cursor]) \
      || (this->input[cursor] == '\n') \
      || (this->input[cursor] == '\r'))) { \
    SET_MINOR_STATE(nextState); \
  }

#define READ_WHITESPACE_REQUIRED(nextState, statusCode, errorMessage) \
  while ((cursor < input_length) && ( \
      isspace(this->input[cursor]) \
      && (this->input[cursor] != '\n') \
      && (this->input[cursor] != '\r'))) { \
    ++cursor; \
  } \
  if (cursor < input_length) { \
    if (cursor > this->minorStart) { \
      SET_MINOR_STATE(nextState); \
    } \
    else { \
      this->currentMessage.setStatusCode(statusCode).setErrorMessage(errorMessage); \
    } \
  }

// CR `MAY` be ignored, so look for either CRLF or just LF.
// https://datatracker.ietf.org/doc/html/rfc9112#section-2.2-3
#define READ_CRLF_OPTIONAL(nextState) \
  while (cursor < input_length) { \
    if ((this->input[cursor] != '\r') && (this->input[cursor] != '\n')) { \
      SET_MINOR_STATE(nextState); \
      break; \
    } \
    ++cursor; \
  }

// CR `MAY` be ignored, so look for either CRLF or just LF.
// https://datatracker.ietf.org/doc/html/rfc9112#section-2.2-3
#define READ_CRLF_REQUIRED(nextState, statusCode, errorMessage) \
  size_t len = cursor - this->minorStart; \
  while ((cursor < input_length) && (len < 2)) { \
    if (((len == 0) && !((this->input[cursor] == '\r') || (this->input[cursor] == '\n'))) \
      || ((len == 1) && (this->input[cursor] != '\n'))) { \
      this->currentMessage.setStatusCode(statusCode).setErrorMessage(errorMessage); \
    } \
    if (!this->currentMessage.hasError() && (this->input[cursor] == '\n')) { \
      SET_MINOR_STATE(nextState); \
      ++cursor; \
      break; \
    } \
    ++cursor; \
    ++len; \
  }


// https://www.rfc-editor.org/rfc/rfc9110#name-overview
// PATCH - https://www.rfc-editor.org/rfc/rfc5789
static set<string> messageMethods{"GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE", "PATCH"};

Parser::Parser() :
  readStateMajor{NEW_HEADER},
  readStateMinor{BEGINNING_OF_LINE},
  majorStart{0},
  minorStart{0},
  input{} {}

void Parser::parseMessageTarget([[maybe_unused]]const std::string & target) {
  // Parse origin-form
  // https://datatracker.ietf.org/doc/html/rfc9112#name-origin-form
  //
  // Parse absolute-form
  // https://datatracker.ietf.org/doc/html/rfc9112#name-absolute-form
  //
  // Parse authority-form
  // https://datatracker.ietf.org/doc/html/rfc9112#name-authority-form
  //
  // Parse asterisk-form
  // https://datatracker.ietf.org/doc/html/rfc9112#name-asterisk-form
}

void Parser::processChunk(const char * buffer, size_t len) {
  //cout << "Processing (" << len << "): " << string(buffer, len) << endl;
  size_t cursor = this->input.length();
  this->input += string(buffer, len);
  size_t input_length = this->input.length();
  while (!this->currentMessage.hasError() && (cursor < input_length)) {
    switch (this->readStateMajor) {
      case NEW_HEADER:
        // https://datatracker.ietf.org/doc/html/rfc9112#name-request-line
        // request-line   = method SP request-target SP HTTP-version
        switch (this->readStateMinor) {
          case BEGINNING_OF_LINE: {
            // https://datatracker.ietf.org/doc/html/rfc9112#section-2.2-6
            READ_CRLF_OPTIONAL(BEGINNING_OF_REQUEST);
            break;
          }
          case BEGINNING_OF_REQUEST: {
            READ_WHITESPACE_OPTIONAL(METHOD);
            break;
          }
          case METHOD: {
            while ((cursor < input_length) && isgraph(this->input[cursor])) {
              ++cursor;
            }
            if (cursor < input_length) {
              // Finished reading Method.
              string method = this->input.substr(this->minorStart, cursor - this->minorStart);
              if (messageMethods.contains(method)) {
                // Finished reading a valid method.
                this->currentMessage.setMethod(method);
                SET_MINOR_STATE(AFTER_METHOD);
              }
              else {
                // https://www.rfc-editor.org/rfc/rfc9110#section-9.1-10
                this->currentMessage.setStatusCode(501).setErrorMessage("Unrecognized method");
              }
            }
            break;
          }
          case AFTER_METHOD: {
            READ_WHITESPACE_REQUIRED(REQUEST_TARGET, 400, "Error reading request line.");
            break;
          }
          case REQUEST_TARGET: {
            while ((cursor < input_length) && isgraph(this->input[cursor])) {
              ++cursor;
            }
            if (cursor < input_length) {
              // Finished reading request target.
              string target = this->input.substr(this->minorStart, cursor - this->minorStart);
              this->parseMessageTarget(target);
              this->currentMessage.setTarget(target);
              SET_MINOR_STATE(AFTER_REQUEST_TARGET);
            }
            break;
          }
          case AFTER_REQUEST_TARGET: {
            READ_WHITESPACE_REQUIRED(HTTP_VERSION, 400, "Error reading request line.");
            break;
          }
          case HTTP_VERSION: {
            while ((cursor < input_length) && isgraph(this->input[cursor])) {
              ++cursor;
            }
            if (cursor < input_length) {
              // Finished reading message target.
              string version = this->input.substr(this->minorStart, cursor - this->minorStart);
              this->currentMessage.setVersion(version);
              SET_MINOR_STATE(AFTER_HTTP_VERSION);
            }
            break;
          }
          case AFTER_HTTP_VERSION: {
            READ_WHITESPACE_OPTIONAL(CRLF);
            break;
          }
          case CRLF: {
            READ_CRLF_REQUIRED(AFTER_CRLF, 400, "Error reading request line.");
            break;
          }
          case AFTER_CRLF: {
            SET_MAJOR_STATE(FIELD_LINE);
            this->tempFieldName = "";
            break;
          }
          default: {
            this->currentMessage.setStatusCode(400).setErrorMessage("Error reading message line.");
          }
        }
      break;
      case FIELD_LINE:
        // https://datatracker.ietf.org/doc/html/rfc9110#section-5.2
        switch (this->readStateMinor) {
          case BEGINNING_OF_LINE: {
            // Field lines must not begin with whitespace, unless packaged
            // within the "message/http" media type.
            // https://datatracker.ietf.org/doc/html/rfc9112#name-obsolete-line-folding
            //
            // Identify the first character of a field name.
            // https://datatracker.ietf.org/doc/html/rfc9110#section-16.3.1-6.2
            // Note that the specification makes a "SHOULD" recommendation, but
            // does not actually disallow the token characters.
            while ((cursor < input_length) && isTokenChar(this->input[cursor])) {
              ++cursor;
            }
            if (cursor < input_length) {
              // Finished reading request target.
              string name = this->input.substr(this->minorStart, cursor - this->minorStart);
              transform(name.begin(), name.end(), name.begin(), ::toupper);
              this->tempFieldName = name;
              SET_MINOR_STATE(AFTER_FIELD_NAME);
            }
            break;
          }
          case AFTER_FIELD_NAME: {
            // https://datatracker.ietf.org/doc/html/rfc9112#section-5-1
            if (this->input[cursor] == ':') {
              SET_MINOR_STATE(BEFORE_FIELD_VALUE);
              ++cursor;
            }
            else {
              // https://datatracker.ietf.org/doc/html/rfc9112#section-5.1-2
              this->currentMessage.setStatusCode(400).setErrorMessage("Illegal character between field name and colon");
            }
            break;
          }
          case BEFORE_FIELD_VALUE: {
            // Remove leading whitespace.
            // https://datatracker.ietf.org/doc/html/rfc9112#section-5-1
            // https://datatracker.ietf.org/doc/html/rfc9110#section-5.5-3
            READ_WHITESPACE_OPTIONAL(FIELD_VALUE);
            break;
          }
          case FIELD_VALUE: {
            if (isListField(this->tempFieldName)) {
              SET_MINOR_STATE(LIST_FIELD_VALUE);
            }
            else {
              SET_MINOR_STATE(SINGLETON_FIELD_VALUE);
            }
            break;
          }
          case SINGLETON_FIELD_VALUE: {
            while((cursor < input_length) && (this->input[cursor] != '\n')) {
              ++cursor;
            }
            if (cursor < input_length) {
              // Back up tempCursor to be before the CRLF.  CR is optional.
              // https://datatracker.ietf.org/doc/html/rfc9112#section-2.2-3
              size_t tempCursor = cursor - 1;
              if ((tempCursor >= this->minorStart) && (this->input[tempCursor] == '\r')) {
                --tempCursor;
              }
              // Eliminate trailing whitespace.
              // https://datatracker.ietf.org/doc/html/rfc9110#section-5.5-3
              while ((tempCursor >= this->minorStart) && isWhitespaceChar(this->input[tempCursor])) {
                --tempCursor;
              }
              // Verify that there are no illegal characters.
              for (size_t i = this->minorStart; i <= tempCursor; ++i) {
                if (!isFieldContentChar(this->input[i])) {
                  this->currentMessage.setStatusCode(400).setErrorMessage("Illegal character in singleton field value");
                  break;
                }
              }
              // If anything remains, then it is the field value.
              if (tempCursor >= this->minorStart) {
                this->currentMessage.addFieldValue(this->tempFieldName, this->input.substr(this->minorStart, tempCursor - this->minorStart + 1));
                SET_MINOR_STATE(CRLF);
              }
              else {
                this->currentMessage.setStatusCode(400).setErrorMessage("Singleton field value is blank/empty");
              }
            }
            break;
          }
          case LIST_FIELD_VALUE: {
            if (this->input[cursor] == '"') {
              SET_MINOR_STATE(QUOTED_FIELD_VALUE_OPEN);
              ++cursor;
            }
            else if (isTokenChar(this->input[cursor])) {
              // Intentionally not advancing the cursor.
              SET_MINOR_STATE(UNQUOTED_FIELD_VALUE);
            }
            else {
              this->currentMessage.setStatusCode(400).setErrorMessage("Illegal character in field value");
            }
            break;
          }
          case UNQUOTED_FIELD_VALUE: {
            while((cursor < input_length) && (this->input[cursor] != ',') && (this->input[cursor] != '\n')) {
              ++cursor;
            }
            if (cursor < input_length) {
              // We found either a comma or a \n.

              size_t tempCursor = cursor - 1;
              if (this->input[cursor] == '\n') {
                // Back up tempCursor to be before the CRLF, if present.
                // CR is optional.
                // https://datatracker.ietf.org/doc/html/rfc9112#section-2.2-3
                if ((tempCursor >= this->minorStart) && (this->input[tempCursor] == '\r')) {
                  --tempCursor;
                }
              }
              // Eliminate trailing whitespace.
              // https://datatracker.ietf.org/doc/html/rfc9110#section-5.5-3
              while ((tempCursor >= this->minorStart) && isWhitespaceChar(this->input[tempCursor])) {
                --tempCursor;
              }
              // Verify that there are no illegal characters.
              for (size_t i = this->minorStart; i <= tempCursor; ++i) {
                if (!isFieldContentChar(this->input[i])) {
                  this->currentMessage.setStatusCode(400).setErrorMessage("Illegal character in singleton field value");
                  break;
                }
              }
              // If anything remains, then it is the field value.
              if (tempCursor >= this->minorStart) {
                this->currentMessage.addFieldValue(this->tempFieldName, this->input.substr(this->minorStart, tempCursor - this->minorStart + 1));
                if (this->input[cursor] == ',') {
                  SET_MINOR_STATE(FIELD_VALUE_COMMA);
                }
                else {
                  SET_MINOR_STATE(CRLF);
                }
              }
              else {
                this->currentMessage.setStatusCode(400).setErrorMessage("Singleton field value is blank/empty");
              }
            }
            break;
          }
          case QUOTED_FIELD_VALUE_OPEN: {
            this->tempFieldValue = "";
            SET_MINOR_STATE(QUOTED_FIELD_VALUE_PROCESS);
            break;
          }
          case QUOTED_FIELD_VALUE_PROCESS: {
            while((cursor < input_length) && isQuotedChar(this->input[cursor])) {
              ++cursor;
            }
            if (cursor < input_length) {
              // Input scanning hit either an escaped character, a double
              // quote, or an illegal character.
              if (this->input[cursor] == '\\') {
                this->tempFieldValue += this->input.substr(this->minorStart, cursor - this->minorStart);
                ++cursor;
                SET_MINOR_STATE(QUOTED_FIELD_VALUE_ESCAPE);
              }
              else if (this->input[cursor] == '"') {
                this->tempFieldValue += this->input.substr(this->minorStart, cursor - this->minorStart);
                ++cursor;
                SET_MINOR_STATE(QUOTED_FIELD_VALUE_CLOSE);
              }
              else {
                this->currentMessage.setStatusCode(400).setErrorMessage("Quoted field value is malformed");
              }
            }
            break;
          }
          case QUOTED_FIELD_VALUE_ESCAPE: {
            this->tempFieldValue += this->input[cursor];
            ++cursor;
            SET_MINOR_STATE(QUOTED_FIELD_VALUE_PROCESS);
            break;
          }
          case QUOTED_FIELD_VALUE_CLOSE: {
            SET_MINOR_STATE(AFTER_FIELD_VALUE);
            this->currentMessage.addFieldValue(this->tempFieldName, this->tempFieldValue);
            break;
          }
          case AFTER_FIELD_VALUE: {
            READ_WHITESPACE_OPTIONAL(FIELD_VALUE_COMMA);
            break;
          }
          case FIELD_VALUE_COMMA: {
            if (this->input[cursor] == ',') {
              ++cursor;
              SET_MINOR_STATE(AFTER_FIELD_VALUE_COMMA);
            }
            else if (isCRLFChar(this->input[cursor])) {
              SET_MINOR_STATE(CRLF);
            }
            else {
              READ_CRLF_REQUIRED(AFTER_CRLF, 400, "Error reading field line.");
            }

            break;
          }
          case AFTER_FIELD_VALUE_COMMA: {
            READ_WHITESPACE_OPTIONAL(LIST_FIELD_VALUE);
            break;
          }
          case CRLF: {
            READ_CRLF_REQUIRED(AFTER_CRLF, 400, "Error reading field line.");
            break;
          }
          case AFTER_CRLF: {
            //cout << this->currentMessage;
            SET_MAJOR_STATE(FIELD_LINE);
            this->tempFieldName = "";
            break;
          }
          default: {
            this->currentMessage.setErrorMessage("foo");
          }
        }
      break;
      case MESSAGE_BODY:
        switch (this->readStateMinor) {
          default: {
            this->currentMessage.setErrorMessage("foo");
          }
        }
      break;
      default: {
        this->currentMessage.setErrorMessage("foo");
      }
    }
  }
}

