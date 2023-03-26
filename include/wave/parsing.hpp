/**
 * @file
 * Header file for declaring text parsing functions.
 */

#ifndef CLIENT_HPP
#define CLIENT_HPP

namespace Ghoti::Wave {

/**
 * Identify a field name as accepting a list-based set of values.
 *
 * @param name The field name.
 * @result Whether or not the field name is recognized as a list-based field.
 */
bool isListField(const std::string & name);

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

};

#endif // CLIENT_HPP

