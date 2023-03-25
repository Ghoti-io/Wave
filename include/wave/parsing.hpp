/**
 * @file
 * Header file for declaring text parsing functions.
 */

#ifndef CLIENT_HPP
#define CLIENT_HPP

namespace Ghoti::Wave {

/**
 * Identify valid Token characters.
 *
 * @param c The character to test.
 * @result Whether or not the character is a valid token character.
 */
bool isTChar(uint8_t c);

/**
 * Identify valid Visible (printing) characters.
 *
 * @param c The character to test.
 * @result Whether or not the character is a valid visible character.
 */
bool isVChar(uint8_t c);

/**
 * Identify valid obs-text characters.
 *
 * @param c The character to test.
 * @result Whether or not the character is a valid obs-text character.
 */
bool isObsTextChar(uint8_t c);

/**
 * Identify valid field-content characters.
 *
 * @param c The character to test.
 * @result Whether or not the character is a valid field-content character.
 */
bool isFieldContentChar(uint8_t c);

};

#endif // CLIENT_HPP

