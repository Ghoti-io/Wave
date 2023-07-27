/**
 * @file
 *
 * Define the Ghoti::Wave::Parser class.
 */

#include <arpa/inet.h>
#include <cassert>
#include <ghoti.io/pool.hpp>
#include <iostream>
#include <set>
#include <string.h>
#include "wave/parser.hpp"
#include "wave/parsing.hpp"

using namespace std;
using namespace Ghoti;
using namespace Ghoti::Pool;
using namespace Ghoti::Wave;

#define START_NEW_INPUT \
  this->input = string{this->input.substr(this->cursor, this->input.length())}; \
  this->cursor = 0; \
  input_length = this->input.length();

#define SET_NEW_HEADER \
  this->readStateMajor = NEW_HEADER; \
  this->readStateMinor = this->type == REQUEST \
    ? BEGINNING_OF_REQUEST_LINE \
    : BEGINNING_OF_STATUS_LINE; \
  this->majorStart = this->cursor; \
  this->minorStart = this->cursor; \
  this->contentLength = 0;

#define SET_MINOR_STATE(nextState) \
  this->readStateMinor = nextState; \
  this->minorStart = this->cursor;

#define SET_MAJOR_STATE(nextMajorState, nextMinorState) \
  this->readStateMajor = nextMajorState; \
  this->majorStart = this->cursor; \
  SET_MINOR_STATE(nextMinorState);

#define READ_WHITESPACE_OPTIONAL(nextState) \
  while ((this->cursor < input_length) && ( \
      isspace(this->input[this->cursor]) \
      && (this->input[this->cursor] != '\n') \
      && (this->input[this->cursor] != '\r'))) { \
    ++this->cursor; \
  } \
  if ((this->cursor < input_length) && ( \
      !isspace(this->input[this->cursor]) \
      || (this->input[this->cursor] == '\n') \
      || (this->input[this->cursor] == '\r'))) { \
    SET_MINOR_STATE(nextState); \
  }

#define READ_WHITESPACE_REQUIRED(nextState, statusCode, errorMessage) \
  while ((this->cursor < input_length) && ( \
      isspace(this->input[this->cursor]) \
      && (this->input[this->cursor] != '\n') \
      && (this->input[this->cursor] != '\r'))) { \
    ++this->cursor; \
  } \
  if (this->cursor < input_length) { \
    if (this->cursor > this->minorStart) { \
      SET_MINOR_STATE(nextState); \
    } \
    else { \
      this->currentMessage->setStatusCode(statusCode).setErrorMessage(errorMessage); \
    } \
  }

// CR `MAY` be ignored, so look for either CRLF or just LF.
// https://datatracker.ietf.org/doc/html/rfc9112#section-2.2-3
#define READ_CRLF_OPTIONAL(nextState) \
  while (this->cursor < input_length) { \
    if ((this->input[this->cursor] != '\r') && (this->input[this->cursor] != '\n')) { \
      SET_MINOR_STATE(nextState); \
      break; \
    } \
    ++this->cursor; \
  }

// CR `MAY` be ignored, so look for either CRLF or just LF.
// https://datatracker.ietf.org/doc/html/rfc9112#section-2.2-3
#define READ_CRLF_REQUIRED(nextState, statusCode, errorMessage) \
  size_t len = this->cursor - this->minorStart; \
  while ((this->cursor < input_length) && (len < 2)) { \
    if (((len == 0) && !((this->input[this->cursor] == '\r') || (this->input[this->cursor] == '\n'))) \
      || ((len == 1) && (this->input[this->cursor] != '\n'))) { \
      this->currentMessage->setStatusCode(statusCode).setErrorMessage(errorMessage); \
    } \
    if (!this->currentMessage->hasError() && (this->input[this->cursor] == '\n')) { \
      SET_MINOR_STATE(nextState); \
      ++this->cursor; \
      break; \
    } \
    ++this->cursor; \
    ++len; \
  }

#define REQUEST_STATUS_ERROR (this->type == REQUEST ? "Error reading request line." : "Error reading status line.")

// https://www.rfc-editor.org/rfc/rfc9110#name-overview
// PATCH - https://www.rfc-editor.org/rfc/rfc5789
static set<shared_string_view> messageMethods{"GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE", "PATCH"};

Parser::Parser(Type type) :
  type{type},
  cursor{0},
  input{},
  currentMessage{make_shared<Message>(type == REQUEST ? Message::Type::REQUEST : Message::Type::RESPONSE)},
  contentLength{0},
  currentChunk{} {
    SET_NEW_HEADER;
  }

void Parser::parseMessageTarget([[maybe_unused]]const shared_string_view & target) {
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

void Parser::processBlock(const char * buffer, size_t len) {
  //cout << "Processing (" << len << "): " << string(buffer, len) << endl;
  this->input += string(buffer, len);
  size_t input_length = this->input.length();
  while (!this->currentMessage->hasError() && (this->cursor < input_length)) {
    switch (this->readStateMajor) {
      case NEW_HEADER: {
        // https://datatracker.ietf.org/doc/html/rfc9112#name-request-line
        // request-line   = method SP request-target SP HTTP-version
        switch (this->readStateMinor) {
          case BEGINNING_OF_REQUEST_LINE: {
            // https://datatracker.ietf.org/doc/html/rfc9112#section-2.2-6
            READ_CRLF_OPTIONAL(BEGINNING_OF_REQUEST);
            break;
          }
          case BEGINNING_OF_REQUEST: {
            READ_WHITESPACE_OPTIONAL(METHOD);
            break;
          }
          case BEGINNING_OF_STATUS_LINE: {
            // https://datatracker.ietf.org/doc/html/rfc9112#section-4-1
            READ_CRLF_OPTIONAL(BEGINNING_OF_STATUS);
            break;
          }
          case BEGINNING_OF_STATUS: {
            READ_WHITESPACE_OPTIONAL(HTTP_VERSION);
            break;
          }
          case METHOD: {
            while ((this->cursor < input_length) && isgraph(this->input[this->cursor])) {
              ++this->cursor;
            }
            if (this->cursor < input_length) {
              // Finished reading Method.
              shared_string_view method = this->input.substr(this->minorStart, this->cursor - this->minorStart);
              if (messageMethods.contains(method)) {
                // Finished reading a valid method.
                this->currentMessage->setMethod(method);
                SET_MINOR_STATE(AFTER_METHOD);
              }
              else {
                // https://www.rfc-editor.org/rfc/rfc9110#section-9.1-10
                this->currentMessage->setStatusCode(501).setErrorMessage("Unrecognized method");
              }
            }
            break;
          }
          case AFTER_METHOD: {
            READ_WHITESPACE_REQUIRED(REQUEST_TARGET, 400, REQUEST_STATUS_ERROR);
            break;
          }
          case REQUEST_TARGET: {
            while ((this->cursor < input_length) && isgraph(this->input[this->cursor])) {
              ++this->cursor;
            }
            if (this->cursor < input_length) {
              // Finished reading request target.
              shared_string_view target = this->input.substr(this->minorStart, this->cursor - this->minorStart);
              this->parseMessageTarget(target);
              this->currentMessage->setTarget(target);
              SET_MINOR_STATE(AFTER_REQUEST_TARGET);
            }
            break;
          }
          case AFTER_REQUEST_TARGET: {
            READ_WHITESPACE_REQUIRED(HTTP_VERSION, 400, REQUEST_STATUS_ERROR);
            break;
          }
          case HTTP_VERSION: {
            while ((this->cursor < input_length) && isgraph(this->input[this->cursor])) {
              ++this->cursor;
            }
            if (this->cursor < input_length) {
              // Finished reading message target.
              shared_string_view version = this->input.substr(this->minorStart, this->cursor - this->minorStart);
              this->currentMessage->setVersion(version);
              SET_MINOR_STATE(AFTER_HTTP_VERSION);
            }
            break;
          }
          case AFTER_HTTP_VERSION: {
            if (this->type == REQUEST) {
              READ_WHITESPACE_OPTIONAL(CRLF);
            }
            else {
              READ_WHITESPACE_REQUIRED(RESPONSE_CODE, 400, REQUEST_STATUS_ERROR);
            }
            break;
          }
          case RESPONSE_CODE: {
            // https://datatracker.ietf.org/doc/html/rfc9112#section-4-4
            // Must be 3 digits.
            while ((this->cursor < input_length) && isdigit(this->input[this->cursor]) && ((this->cursor - this->minorStart) < 3)) {
              ++this->cursor;
            }
            if ((this->cursor - this->minorStart) == 3) {
              this->currentMessage->setStatusCode(
                ((this->input[this->minorStart] - '0') * 100)
                + ((this->input[this->minorStart + 1] - '0') * 10)
                + ((this->input[this->minorStart + 2] - '0')));
              READ_WHITESPACE_REQUIRED(REASON_PHRASE, 400, REQUEST_STATUS_ERROR);
            }
            break;
          }
          case REASON_PHRASE: {
            // https://datatracker.ietf.org/doc/html/rfc9112#section-4-7
            while ((this->cursor < input_length) && !isCRLFChar(this->input[this->cursor])) {
              ++this->cursor;
            }
            if (this->cursor < input_length) {
              SET_MINOR_STATE(CRLF);
            }
            break;
          }
          case CRLF: {
            READ_CRLF_REQUIRED(AFTER_CRLF, 400, REQUEST_STATUS_ERROR);
            break;
          }
          case AFTER_CRLF: {
            SET_MAJOR_STATE(FIELD_LINE, BEGINNING_OF_FIELD_LINE);
            this->tempFieldName = "";
            break;
          }
          default: {
            this->currentMessage->setStatusCode(400).setErrorMessage(REQUEST_STATUS_ERROR);
          }
        }
        break;
      }
      case FIELD_LINE:
      case TRAILER: {
        // https://datatracker.ietf.org/doc/html/rfc9110#section-5.2
        // https://datatracker.ietf.org/doc/html/rfc9112#name-chunked-trailer-section
        switch (this->readStateMinor) {
          case BEGINNING_OF_FIELD_LINE: {
            // Intentionally not advancing the cursor in this step.
            if ((this->input[this->cursor] == '\r') || (this->input[this->cursor] == '\n')) {
              SET_MINOR_STATE(this->readStateMajor == FIELD_LINE
                ? AFTER_HEADER_FIELDS
                : TRAILER_FINISHED);
            }
            else {
              SET_MINOR_STATE(FIELD_NAME);
            }
            break;
          }
          case FIELD_NAME: {
            // Field lines must not begin with whitespace, unless packaged
            // within the "message/http" media type.
            // https://datatracker.ietf.org/doc/html/rfc9112#name-obsolete-line-folding
            //
            // Identify the first character of a field name.
            // https://datatracker.ietf.org/doc/html/rfc9110#section-16.3.1-6.2
            // Note that the specification makes a "SHOULD" recommendation, but
            // does not actually disallow the token characters.
            while ((this->cursor < input_length) && isTokenChar(this->input[this->cursor])) {
              ++this->cursor;
            }
            if (this->cursor < input_length) {
              // Finished reading request target.
              auto name = string{this->input.substr(this->minorStart, this->cursor - this->minorStart)};
              transform(name.begin(), name.end(), name.begin(), ::toupper);
              this->tempFieldName = name;
              SET_MINOR_STATE(AFTER_FIELD_NAME);
            }
            break;
          }
          case AFTER_FIELD_NAME: {
            // https://datatracker.ietf.org/doc/html/rfc9112#section-5-1
            if (this->input[this->cursor] == ':') {
              SET_MINOR_STATE(BEFORE_FIELD_VALUE);
              ++this->cursor;
            }
            else {
              // https://datatracker.ietf.org/doc/html/rfc9112#section-5.1-2
              this->currentMessage->setStatusCode(400).setErrorMessage("Illegal character between field name and colon");
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
            while((this->cursor < input_length) && (this->input[this->cursor] != '\n')) {
              ++this->cursor;
            }
            if (this->cursor < input_length) {
              // Back up tempCursor to be before the CRLF.  CR is optional.
              // https://datatracker.ietf.org/doc/html/rfc9112#section-2.2-3
              size_t tempCursor = this->cursor - 1;
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
                  this->currentMessage->setStatusCode(400).setErrorMessage("Illegal character in singleton field value");
                  break;
                }
              }
              // If anything remains, then it is the field value.
              if (tempCursor >= this->minorStart) {
                auto value = this->input.substr(this->minorStart, tempCursor - this->minorStart + 1);
                if (this->readStateMajor == FIELD_LINE) {
                  this->currentMessage->addFieldValue(this->tempFieldName, value);
                }
                else {
                  this->currentMessage->addTrailerFieldValue(this->tempFieldName, value);
                }
                if (this->tempFieldName == "CONTENT-LENGTH") {
                  // https://datatracker.ietf.org/doc/html/rfc9112#name-content-length
                  int32_t contentLength{0};
                  for (auto ch : value) {
                    if ((contentLength >= 0) && !isdigit(ch)) {
                      contentLength = -1;
                      this->currentMessage->setStatusCode(400).setErrorMessage("Invalid Content-Length");
                      break;
                    }
                    else {
                      // Converting ASCII numbers to an integer, one character
                      // at a time.
                      contentLength *= 10;
                      contentLength += ch - '0';
                    }
                  }
                  this->contentLength = contentLength;
                  if (this->readStateMajor == FIELD_LINE) {
                    this->currentMessage->setTransport(Message::Transport::FIXED);
                  }
                }
                SET_MINOR_STATE(CRLF);
              }
              else {
                this->currentMessage->setStatusCode(400).setErrorMessage("Singleton field value is blank/empty");
              }
            }
            break;
          }
          case LIST_FIELD_VALUE: {
            if (this->input[this->cursor] == '"') {
              SET_MINOR_STATE(QUOTED_FIELD_VALUE_OPEN);
              ++this->cursor;
            }
            else if (isTokenChar(this->input[this->cursor])) {
              // Intentionally not advancing the this->cursor.
              SET_MINOR_STATE(UNQUOTED_FIELD_VALUE);
            }
            else {
              this->currentMessage->setStatusCode(400).setErrorMessage("Illegal character in field value");
            }
            break;
          }
          case UNQUOTED_FIELD_VALUE: {
            while((this->cursor < input_length) && (this->input[this->cursor] != ',') && (this->input[this->cursor] != '\n')) {
              ++this->cursor;
            }
            if (this->cursor < input_length) {
              // We found either a comma or a \n.

              size_t tempCursor = this->cursor - 1;
              if (this->input[this->cursor] == '\n') {
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
                  this->currentMessage->setStatusCode(400).setErrorMessage("Illegal character in singleton field value");
                  break;
                }
              }
              // If anything remains, then it is the field value.
              if (tempCursor >= this->minorStart) {
                if (this->readStateMajor == FIELD_LINE) {
                  this->currentMessage->addFieldValue(this->tempFieldName, this->input.substr(this->minorStart, tempCursor - this->minorStart + 1));
                }
                else {
                  this->currentMessage->addTrailerFieldValue(this->tempFieldName, this->input.substr(this->minorStart, tempCursor - this->minorStart + 1));
                }
                if (this->input[this->cursor] == ',') {
                  SET_MINOR_STATE(FIELD_VALUE_COMMA);
                }
                else {
                  SET_MINOR_STATE(CRLF);
                }
              }
              else {
                this->currentMessage->setStatusCode(400).setErrorMessage("Singleton field value is blank/empty");
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
            while((this->cursor < input_length) && isQuotedChar(this->input[this->cursor])) {
              ++this->cursor;
            }
            if (this->cursor < input_length) {
              // Input scanning hit either an escaped character, a double
              // quote, or an illegal character.
              if (this->input[this->cursor] == '\\') {
                this->tempFieldValue += this->input.substr(this->minorStart, this->cursor - this->minorStart);
                ++this->cursor;
                SET_MINOR_STATE(QUOTED_FIELD_VALUE_ESCAPE);
              }
              else if (this->input[this->cursor] == '"') {
                this->tempFieldValue += this->input.substr(this->minorStart, this->cursor - this->minorStart);
                ++this->cursor;
                SET_MINOR_STATE(QUOTED_FIELD_VALUE_CLOSE);
              }
              else {
                this->currentMessage->setStatusCode(400).setErrorMessage("Quoted field value is malformed");
              }
            }
            break;
          }
          case QUOTED_FIELD_VALUE_ESCAPE: {
            this->tempFieldValue += this->input[this->cursor];
            ++this->cursor;
            SET_MINOR_STATE(QUOTED_FIELD_VALUE_PROCESS);
            break;
          }
          case QUOTED_FIELD_VALUE_CLOSE: {
            SET_MINOR_STATE(AFTER_FIELD_VALUE);
            if (this->readStateMajor == FIELD_LINE) {
              this->currentMessage->addFieldValue(this->tempFieldName, this->tempFieldValue);
            }
            else {
              this->currentMessage->addTrailerFieldValue(this->tempFieldName, this->tempFieldValue);
            }
            break;
          }
          case AFTER_FIELD_VALUE: {
            READ_WHITESPACE_OPTIONAL(FIELD_VALUE_COMMA);
            break;
          }
          case FIELD_VALUE_COMMA: {
            if (this->input[this->cursor] == ',') {
              ++this->cursor;
              SET_MINOR_STATE(AFTER_FIELD_VALUE_COMMA);
            }
            else if (isCRLFChar(this->input[this->cursor])) {
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
            SET_MAJOR_STATE(FIELD_LINE, BEGINNING_OF_FIELD_LINE);
            this->tempFieldName = "";
            break;
          }
          case AFTER_HEADER_FIELDS:
          case TRAILER_FINISHED: {
            // CR `MAY` be ignored, so look for either CRLF or just LF.
            // https://datatracker.ietf.org/doc/html/rfc9112#section-2.2-3
            size_t len = this->cursor - this->minorStart;
            while ((this->cursor < input_length) && (len < 2)) {
              ++this->cursor;
              ++len;
              if (((len == 1) && !((this->input[this->cursor] == '\r') || (this->input[this->cursor] == '\n')))
                || ((len == 2) && (this->input[this->cursor] != '\n'))) {
                this->currentMessage->setStatusCode(400).setErrorMessage("Error reading field line.");
              }
              if (!this->currentMessage->hasError() && (this->input[this->cursor] == '\n')) {
                if (this->readStateMajor == FIELD_LINE) {
                  // Note: We are un-incrementing the cursor here.  That way,
                  // in the event that the message ends at this point (e.g.,
                  // there is no body message), the not-yet-incremented cursor
                  // allows us to move execution to the next phase.  If we need
                  // to end processing at that point, then so be it.
                  this->cursor -= len;
                  SET_MAJOR_STATE(MESSAGE_BODY, MESSAGE_START);
                }
                else {
                  SET_MAJOR_STATE(FINISHED, MESSAGE_FINISHED);
                }
                break;
              }
            }
            break;
          }
          default: {
            assert(false);
            this->currentMessage->setErrorMessage("foo");
          }
        }
        break;
      }
      case MESSAGE_BODY: {
        switch (this->readStateMinor) {
          case MESSAGE_START: {
            // Increment the cursor, which was not done at the end of the
            // previous step, AFTER_HEADER_FIELDS.
            ++this->cursor;

            // Determine whether or not there is a message body.
            // https://datatracker.ietf.org/doc/html/rfc9112#section-6-4
            //auto fields = this->currentMessage->getFields();
            //if (fields.contains("CONTENT-LENGTH") || fields.contains("TRANSFER-ENCODING")) {
            if (this->contentLength > 0) {
              SET_MINOR_STATE(MESSAGE_READ);
            }
            else {
              // This is the end of the message.
              this->messages.emplace(move(this->currentMessage));
              this->currentMessage = this->createNewMessage();
              SET_NEW_HEADER;
            }
            break;
          }
          case MESSAGE_READ: {
            auto cursorStart = this->cursor;

            // Read in as much as possible, until the fixed contentLength is
            // reached, whichever is first.
            while ((this->cursor < input_length) && ((this->cursor - this->minorStart) < this->contentLength)) {
              ++this->cursor;
            }

            // Move the processed part into a chunk..
            if (this->currentChunk.append(this->input.substr(cursorStart, this->cursor - cursorStart))) {
              // The append failed.  We can't do anything else.
              // Insufficient Storage
              // https://datatracker.ietf.org/doc/html/rfc4918#section-11.5
              this->currentMessage->setStatusCode(507).setErrorMessage("Insufficient Storage");
              break;
            }

            // If the chunk is too big in memory, convert it to a file.
            if ((this->currentChunk.getType() == Blob::Type::TEXT) && (this->currentChunk.getText().length() > this->getMEMCHUNKSIZELIMIT())) {
              if (this->currentChunk.convertToFile()) {
                // Insufficient Storage
                // https://datatracker.ietf.org/doc/html/rfc4918#section-11.5
                this->currentMessage->setStatusCode(507).setErrorMessage("Insufficient Storage");
                break;
              }
            }

            // If there is no more to read, then finalize the message.
            if ((this->cursor - this->minorStart) == this->contentLength) {
              this->currentMessage->setMessageBody(move(this->currentChunk));
              this->currentChunk = {};
              SET_MAJOR_STATE(FINISHED, MESSAGE_FINISHED);
            }
            break;
          }
          default: {
            assert(false);
            this->currentMessage->setErrorMessage("foo");
          }
        }
        break;
      }
      case CHUNKED_BODY: {
        switch (this->readStateMinor) {
          case CHUNK_START: {
            this->chunkSize = 0;
            this->extensions = "";
            this->currentChunk = {};
            SET_MINOR_STATE(CHUNK_SIZE);
            break;
          }
          case CHUNK_SIZE: {
            // https://datatracker.ietf.org/doc/html/rfc9112#section-7.1-3
            // overflowProtect is used to make sure that calculating chunkSize
            // will not overflow when reading in a new digit.
            // The approach is assume that size_t is 16 bits.  overflowProtect
            // should then be the maximum value that can fit into 8 bits,
            // calculated as ((1 << (16 - 8)) - 1).
            // If size_t is 64 bits, then overflowProtect should be the maximum
            // value that can fit into 64 - 8, or 56 bits, calculated as
            // ((1 << (64 - 8)) - 1).
            size_t overflowProtect = (1 << (sizeof(size_t) - 8)) - 1;
            while((this->cursor < input_length) && isxdigit(this->input[this->cursor])) {
              if (this->chunkSize > overflowProtect) {
                // This next digit will cause an overflow, so error instead.
                this->currentMessage->setStatusCode(400).setErrorMessage("Chunk size too large.");
                break;
              }
              char ch = this->input[this->cursor];
              this->chunkSize <<= 8;
              this->chunkSize += isdigit(ch)
                ? ch - '0'
                : isupper(ch)
                  ? ch - 'A' + 10
                  : ch - 'a' + 10;
              ++this->cursor;
            }
            if (this->cursor < input_length) {
              SET_MINOR_STATE(AFTER_CHUNK_SIZE);
            }
            break;
          }
          case AFTER_CHUNK_SIZE: {
            READ_WHITESPACE_OPTIONAL(CHUNK_EXTENSIONS_IDENTIFIER);
            break;
          }
          case CHUNK_EXTENSIONS_IDENTIFIER: {
            if (this->input[this->cursor] == ';') {
              ++this->cursor;
              SET_MINOR_STATE(CHUNK_EXTENSIONS);
            }
            else {
              SET_MINOR_STATE(CRLF);
            }
            break;
          }
          case CHUNK_EXTENSIONS: {
            // https://datatracker.ietf.org/doc/html/rfc9112#name-chunk-extensions
            // TODO: parse these into a list of extension name/value pairs.
            //   I'm not doing it at the moment because the use of these
            //   extension values are implementation specific, and it's just a
            //   lot of complexity for a feature that's not being used at the
            //   moment.
            while ((this->cursor < input_length) && !isCRLFChar(this->input[this->cursor])) {
              ++this->cursor;
            }
            if (this->cursor < input_length) {
              this->extensions = this->input.substr(this->minorStart, this->cursor - this->minorStart);
              SET_MINOR_STATE(AFTER_CHUNK_EXTENSIONS);
            }
            break;
          }
          case AFTER_CHUNK_EXTENSIONS: {
            READ_CRLF_REQUIRED(CHUNK_BODY, 400, "Error reading chunk size/extensions.");
            break;
          }
          case CHUNK_BODY: {
            auto cursorStart = this->cursor;

            // Read in as much as possible, until the fixed contentLength is
            // reached, whichever is first.
            while ((this->cursor < input_length) && ((this->cursor - this->minorStart) < this->chunkSize)) {
              ++this->cursor;
            }

            // Move the processed part into a chunk..
            if (this->currentChunk.append(this->input.substr(cursorStart, this->cursor - cursorStart))) {
              // The append failed.  We can't do anything else.
              // Insufficient Storage
              // https://datatracker.ietf.org/doc/html/rfc4918#section-11.5
              this->currentMessage->setStatusCode(507).setErrorMessage("Insufficient Storage");
              break;
            }

            // If the chunk is too big in memory, convert it to a file.
            if ((this->currentChunk.getType() == Blob::Type::TEXT) && (this->currentChunk.getText().length() > this->getMEMCHUNKSIZELIMIT())) {
              if (this->currentChunk.convertToFile()) {
                // Insufficient Storage
                // https://datatracker.ietf.org/doc/html/rfc4918#section-11.5
                this->currentMessage->setStatusCode(507).setErrorMessage("Insufficient Storage");
                break;
              }
            }

            // If there is no more to read, then finalize the message.
            if ((this->cursor - this->minorStart) == this->chunkSize) {
              this->currentMessage->addChunk(move(this->currentChunk));
              this->currentChunk = {};
              this->currentMessage->setReady(false);

              // If this was a 0-length chunk, then it was the last chunk and
              // we should move on to the Trailer section.  Else, try to read
              // the next chunk.
              if (this->chunkSize) {
                SET_MINOR_STATE(CHUNK_START);
                START_NEW_INPUT;
              }
              else {
                SET_MAJOR_STATE(TRAILER, BEGINNING_OF_FIELD_LINE);
              }
            }
            break;
          }
          default: {
            assert(false);
          }
        }
        break;
      }
      case FINISHED: {
        // This is the end of the message.
        this->currentMessage->setReady(true);
        this->messages.emplace(move(this->currentMessage));
        this->currentMessage = this->createNewMessage();

        // Break the input so that each message has its own shared_string_view.
        START_NEW_INPUT;

        SET_NEW_HEADER;
        break;
      }
      default: {
        assert(false);
        this->currentMessage->setErrorMessage("foo");
      }
    }
  }
  if (this->currentMessage->hasError()) {
    this->messages.emplace(move(this->currentMessage));
    this->currentMessage = this->createNewMessage();
  }
}

void Parser::registerMessage(shared_ptr<Message> message) {
  auto id = message->getId();

  if (this->messageRegister.contains(id)) {
    // There is already a message with this id.
    auto source = this->messageRegister[id];
    this->messageRegister[id] = message;
    message->adoptContents(*source);
  }
  else {
    // There is not a message with this id.
    this->messageRegister[message->getId()] = message;
  }
}

shared_ptr<Message> Parser::createNewMessage() const {
  switch (this->type) {
    case REQUEST:
      return make_shared<Message>(Message::Type::REQUEST);
    case RESPONSE:
    default:
      return make_shared<Message>(Message::Type::RESPONSE);
  }
}

RequestParser::RequestParser() : Parser(REQUEST) {}

uint32_t RequestParser::getMEMCHUNKSIZELIMIT() {
  auto result = this->getParameter<uint32_t>(ServerParameter::MEMCHUNKSIZELIMIT);
  return result ? *result : 0;
}

ResponseParser::ResponseParser() : Parser(RESPONSE) {}

uint32_t ResponseParser::getMEMCHUNKSIZELIMIT() {
  auto result = this->getParameter<uint32_t>(ClientParameter::MEMCHUNKSIZELIMIT);
  return result ? *result : 0;
}

