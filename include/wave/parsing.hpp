/**
 * @file
 * Header file for declaring text parsing functions.
 */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <ghoti.io/util/shared_string_view.hpp>

namespace Ghoti::Wave {

/**
 * Identify a field name as accepting a list-based set of values.
 *
 * @param name The field name.  The field name must be uppercase.
 * @result Whether or not the field name is recognized as a list-based field.
 */
bool isListField(const Ghoti::shared_string_view & name);

/**
 * Identify valid Token characters.
 *
 * @param c The character to test.
 * @result Whether or not the character is a valid token character.
 */
bool isTokenChar(uint8_t c);

/**
 * Identify valid whitespace characters.
 *
 * @param c The character to test.
 * @result Whether or not the character is a valid visible character.
 */
bool isWhitespaceChar(uint8_t c);

/**
 * Identify valid Visible (printing) characters.
 *
 * @param c The character to test.
 * @result Whether or not the character is a valid visible character.
 */
bool isVisibleChar(uint8_t c);

/**
 * Identify valid obs-text characters.
 *
 * @param c The character to test.
 * @result Whether or not the character is a valid obs-text character.
 */
bool isObsoleteTextChar(uint8_t c);

/**
 * Identify valid field-name characters.
 *
 * @param c The character to test.
 * @result Whether or not the character is a valid field-name character.
 */
bool isFieldNameChar(uint8_t c);

/**
 * Identify valid quoted characters.
 *
 * @param c The character to test.
 * @result Whether or not the character is a valid quoted character.
 */
bool isQuotedChar(uint8_t c);

/**
 * Identify valid field-content characters.
 *
 * @param c The character to test.
 * @result Whether or not the character is a valid field-content character.
 */
bool isFieldContentChar(uint8_t c);

/**
 * Identify CRLF characters.
 *
 * @param c The character to test.
 * @result Whether or not the character is a valid CRLF character.
 */
bool isCRLFChar(uint8_t c);

/**
 * Indicate whether or not the string contains a character which makes it
 * necessary to wrap the string in double quotes.
 *
 * @param str The string in question.
 * @result Whether or not the string needs to be wrapped in double quotes.
 */
bool fieldValueQuotesNeeded(const Ghoti::shared_string_view & str);

/**
 * Escape a field value.
 *
 * @param str The field value to be escaped.
 * @result The escaped field value.
 */
std::string fieldValueEscape(const Ghoti::shared_string_view & str);

};

#endif // CLIENT_HPP

