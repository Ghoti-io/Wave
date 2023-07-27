/**
 * @file
 *
 * Header file for declaring the Session class.
 */

#ifndef GHOTI_WAVE_PARSER_HPP
#define GHOTI_WAVE_PARSER_HPP

#include <queue>
#include <ghoti.io/util/shared_string_view.hpp>
#include "wave/blob.hpp"
#include "wave/hasClientParameters.hpp"
#include "wave/hasServerParameters.hpp"
#include "wave/message.hpp"

namespace Ghoti::Wave {

/**
 * Parses a HTTP/1.1 data stream into discrete messages.
 */
class Parser {
  public:
  /**
   * Represents the type of parsing being performed.
   */
  enum Type {
    REQUEST,  ///< This is a Request stream.
    RESPONSE, ///< This is a Response stream.
  };

  /**
   * The constructor.
   *
   * HTTP/1.1 streams do not have an interchangeable syntax, so the stream type
   * must be declared.
   *
   * The stream will accept an array of bytes, and it will remember its
   * previous parsing position.
   *
   * @param type The Parser::Type of the message stream.
   */
  Parser(Type type);

  /**
   * Process a block of data.
   *
   * @param buffer The buffer to be processed.
   * @param len The length of the buffer in bytes.
   */
  void processBlock(const char * buffer, size_t len);
  void parseMessageTarget(const Ghoti::shared_string_view & target);

  /**
   * Use the provided Message as the recipient of parsing for the Message's id.
   *
   * If a Message with the target ID already exists, then the provided message
   * will adopt the contents of the existing data.
   *
   * @param message The object that should receive the desired messages.
   */
  void registerMessage(std::shared_ptr<Message> message);

  /**
   * A queue of messages that have been parsed so far.
   *
   * The calling session manager may pop messages from the queue as needed.
   */
  std::queue<std::shared_ptr<Message>> messages;

  private:

  /**
   * Return the parameter value for MEMCHUNKSIZELIMIT.
   *
   * @return The parameter value.
   */
  virtual uint32_t getMEMCHUNKSIZELIMIT() = 0;

  /**
   * Primary state tracking values.
   *
   * These values are used to indicate which major stage the parser is in while
   * parsing the message stream.
   *
   * The parser uses two stages, to make the parser switch cases easier to
   * follow and to reuse common stages in different contexts (e.g., CRLF).
   */
  enum ReadStateMajor {
    NEW_HEADER,    ///< Expect a new message header.
    FIELD_LINE,    ///< Expect a new header field.
    MESSAGE_BODY,  ///< Expect the message body.
    CHUNKED_BODY,  ///< Expect a chunked body.
    TRAILER,       ///< Look for an (optional) trailer section.
    FINISHED,      ///< The message is finished.
  };

  /**
   * Secondary state tracking values.
   *
   * These values are used to indicate which "part" of the primary state is
   * being tracked.
   */
  enum ReadStateMinor {
    BEGINNING_OF_REQUEST_LINE, ///< A request line is starting.
    BEGINNING_OF_STATUS_LINE,  ///< A status line is starting.
    BEGINNING_OF_FIELD_LINE,   ///< A header field line is starting.
    CRLF,                      ///< Expect a CRLF.
    AFTER_CRLF,                ///< A CRLF has been identified.
    BEGINNING_OF_REQUEST,      ///< Optional whitespace parsed, request line
                               ///<   is now starting.
    BEGINNING_OF_STATUS,       ///< Optional whitespace parsed, status line is
                               ///<   now starting.
    METHOD,                    ///< Method expected.
    AFTER_METHOD,              ///< Method successfully parsed.
    REQUEST_TARGET,            ///< Expect request target.
    AFTER_REQUEST_TARGET,      ///< Request target successfully parsed.
    HTTP_VERSION,              ///< HTTP version expected.
    AFTER_HTTP_VERSION,        ///< HTTP version successfully parsed.
    RESPONSE_CODE,             ///< Response Code Expected.
    REASON_PHRASE,             ///< Reason Phrase Expected.
    FIELD_NAME,                ///< Header field name expected.
    AFTER_FIELD_NAME,          ///< Header field name successfully parsed.
    BEFORE_FIELD_VALUE,        ///< Header field value about to be processed.
    FIELD_VALUE,               ///< Header field value expected.
    SINGLETON_FIELD_VALUE,     ///< Singleton header field value expected.
    LIST_FIELD_VALUE,          ///< List of header fields expected.
    UNQUOTED_FIELD_VALUE,      ///< Unquoted field value expected.
    QUOTED_FIELD_VALUE_OPEN,   ///< Quoted field value begin.
    QUOTED_FIELD_VALUE_PROCESS,///< Quoted field value is being processed.
    QUOTED_FIELD_VALUE_ESCAPE, ///< Quoted field value char is being escaped.
    QUOTED_FIELD_VALUE_CLOSE,  ///< Quoted field value is being closed.
    AFTER_FIELD_VALUE,         ///< Field value processed.
    FIELD_VALUE_COMMA,         ///< Field value comma expected.
    AFTER_FIELD_VALUE_COMMA,   ///< Field value comma processed.
    AFTER_HEADER_FIELDS,       ///< Header fields processed.
    MESSAGE_START,             ///< Message started.
    MESSAGE_READ,              ///< Message being read.
    CHUNK_START,               ///< The beginning of a new chunk.
    CHUNK_SIZE,                ///< Chunk header being read.
    AFTER_CHUNK_SIZE,          ///< Chunk size is read, extensions may follow.
    CHUNK_EXTENSIONS_IDENTIFIER, ///< Identifies that a chunk extension will
                                 ///<   follow.
    CHUNK_EXTENSIONS,          ///< Chunk Extensions being read.
    AFTER_CHUNK_EXTENSIONS,    ///< Newline after chunk extensions.
    CHUNK_BODY,                ///< Chunk body being read.
    AFTER_CHUNK_BODY,          ///< Chunk body is complete.
    TRAILER_FINISHED,          ///< The trailer section is finished.
    MESSAGE_FINISHED,          ///< The message is finished.
  };

  /**
   * The Parser::Type of HTTP/1.1 stream that will be processed.
   */
  Type type;

  /**
   * An internal counter that indicates the character currently being
   * processed.
   */
  size_t cursor;

  /**
   * Tracks the primary state for the parsing state machine.
   */
  ReadStateMajor readStateMajor;

  /**
   * Tracks the secondary state for the parsing state machine.
   */
  ReadStateMinor readStateMinor;

  /**
   * Indicates the cursor position at which the major state was last updated.
   */
  size_t majorStart;

  /**
   * Indicates the cursor position at which the minor state was last updated.
   */
  size_t minorStart;

  /**
   * The input string, stored internally so that the stream will be processed
   * correctly, even if it is split across multiple buffered reads.
   */
  Ghoti::shared_string_view input;

  /**
   * An error message to communicate a parsing issue.
   */
  Ghoti::shared_string_view errorMessage;

  /**
   * The field name currently being processed.
   */
  Ghoti::shared_string_view tempFieldName;

  /**
   * The field value currently being processed.
   */
  Ghoti::shared_string_view tempFieldValue;

  /**
   * The extensions that accompany the chunk.
   */
  Ghoti::shared_string_view extensions;

  /**
   * A map to store a Message associated with a sequence.
   *
   * This approach is used so that the parser can be informed of the existence
   * of an expected message.  This way, the supplied Message object can act as
   * the recipient of the message as it is parsed.
   *
   * The registered message should be the same message that was provided to the
   * caller of the Client::sendRequest() function.
   *
   * `messageRegister[ID] = message`
   */
  std::unordered_map<uint32_t, std::shared_ptr<Message>> messageRegister;

  /**
   * The current message being parsed.
   */
  std::shared_ptr<Message> currentMessage;

  /**
   * Create a new message whose Message::Type matches the Parser::Type of this
   * parser.
   *
   * This function should really only be used by Parser::Type::Request parsing,
   * since all Parser::Type::Response streams should have already registered a
   * Message object to receive the parsed message.
   *
   * @return A properly typed message.
   */
  std::shared_ptr<Message> createNewMessage() const;

  /**
   * The content length that was encountered when parsing the header.
   */
  size_t contentLength;

  /**
   * The size of the chunk currently being read (in bytes).
   */
  size_t chunkSize;

  /**
   * The current chunk being collected.
   */
  Ghoti::Wave::Blob currentChunk;
};

/**
 * Specialized class for handling the request parser, to make it easier to
 * pass in ServerParameters.
 */
class RequestParser : public Parser, public HasServerParameters {
  public:
  /**
   * Default constructor.
   */
  RequestParser();

  private:
  /**
   * Return the parameter value for MEMCHUNKSIZELIMIT.
   *
   * @return The parameter value.
   */
  virtual uint32_t getMEMCHUNKSIZELIMIT() override;
};

/**
 * Specialized class for handling the response parser, to make it easier to
 * pass in ClientParameters.
 */
class ResponseParser : public Parser, public HasClientParameters {
  public:
  /**
   * Default constructor.
   */
  ResponseParser();

  private:
  /**
   * Return the parameter value for MEMCHUNKSIZELIMIT.
   *
   * @return The parameter value.
   */
  virtual uint32_t getMEMCHUNKSIZELIMIT() override;
};
}

#endif // GHOTI_WAVE_PARSER_HPP

