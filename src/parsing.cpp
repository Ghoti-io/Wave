/**
 * @file
 *
 * Define the text parsing functions.
 */

#include <cstdint>
#include <ctype.h>
#include <set>
#include "parsing.hpp"

using namespace std;
using namespace Ghoti;
using namespace Ghoti::Wave;

namespace Ghoti::Wave {

/**
 * Fields identified as having values that are expected to be a list.
 * https://datatracker.ietf.org/doc/html/rfc9110
 */
static set<shared_string_view> listFields{
  "ACCEPT",
  "ACCEPT-CHARSET",
  "ACCEPT-ENCODING",
  "ACCEPT-LANGUAGE",
  "ACCEPT-RANGES",
  "ALLOW",
  "AUTHENTICATION-INFO",
  "CONNECTION",
  "CONTENT-ENCODING",
  "CONTENT-LANGUAGE",
  "EXPECT",
  "IF-MATCH",
  "IF-NONE-MATCH",
  "PROXY-AUTHENTICATE",
  "PROXY-AUTHENTICATION-INFO",
  "TE",
  "TRAILER",
  "UPGRADE",
  "VARY",
  "VIA",
  "WWW-AUTHENTICATE",
};

bool isListField(const shared_string_view & name) {
  return listFields.contains(name);
}

bool isTokenChar(uint8_t c) {
  // https://datatracker.ietf.org/doc/html/rfc9110#section-5.6.2-2
  static bool map[] = {
    //Nul                  BelBs Tb Lf Vt Ff Cr
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    //                                 Esc
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    //SP !  "  #  $  %  &  '  (  )  *  +  ,  -  .  /
      0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0,
    //0  1  2  3  4  5  6  7  8  9  :  ;  <  =  >  ?
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
    //@  A  B  C  D  E  F  G  H  I  J  K  L  M  N  O
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    //P  Q  R  S  T  U  V  W  X  Y  Z  [  \  ]  ^  _
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1,
    //`  a  b  c  d  e  f  g  h  i  j  k  l  m  n  o
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    //p  q  r  s  t  u  v  w  x  y  z  {  |  }  ~  Del
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0,
    //Extended ascii
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  };
  return map[c];
}

bool isWhitespaceChar(uint8_t c) {
  // https://datatracker.ietf.org/doc/html/rfc9110#section-5.6.3-7
  static bool map[] = {
    //Nul                  BelBs Tb Lf Vt Ff Cr
      0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
    //                                 Esc
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    //SP !  "  #  $  %  &  '  (  )  *  +  ,  -  .  /
      1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    //0  1  2  3  4  5  6  7  8  9  :  ;  <  =  >  ?
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    //@  A  B  C  D  E  F  G  H  I  J  K  L  M  N  O
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    //P  Q  R  S  T  U  V  W  X  Y  Z  [  \  ]  ^  _
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    //`  a  b  c  d  e  f  g  h  i  j  k  l  m  n  o
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    //p  q  r  s  t  u  v  w  x  y  z  {  |  }  ~  Del
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    //Extended ascii
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  };
  return map[c];
}

bool isVisibleChar(uint8_t c) {
  // https://datatracker.ietf.org/doc/html/rfc5234#autoid-25
  // https://en.cppreference.com/w/c/string/byte/isgraph
  // The specification matches.
  return isgraph(c);
}

bool isObsoleteTextChar(uint8_t c) {
  // https://datatracker.ietf.org/doc/html/rfc9110#section-5.5-2
  return c >= 0x80;
}

bool isFieldNameChar(uint8_t c) {
  // https://datatracker.ietf.org/doc/html/rfc9110#section-5.5-2
  static bool map[] = {
    //Nul                  BelBs Tb Lf Vt Ff Cr
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    //                                 Esc
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    //SP !  "  #  $  %  &  '  (  )  *  +  ,  -  .  /
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
    //0  1  2  3  4  5  6  7  8  9  :  ;  <  =  >  ?
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
    //@  A  B  C  D  E  F  G  H  I  J  K  L  M  N  O
      0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    //P  Q  R  S  T  U  V  W  X  Y  Z  [  \  ]  ^  _
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    //`  a  b  c  d  e  f  g  h  i  j  k  l  m  n  o
      0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    //p  q  r  s  t  u  v  w  x  y  z  {  |  }  ~  Del
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    //Extended ascii
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  };
  return map[c];
}

bool isQuotedChar(uint8_t c) {
  // https://datatracker.ietf.org/doc/html/rfc9110#section-5.5-2
  static bool map[] = {
    //Nul                  BelBs Tb Lf Vt Ff Cr
      0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
    //                                 Esc
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    //SP !  "  #  $  %  &  '  (  )  *  +  ,  -  .  /
      1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    //0  1  2  3  4  5  6  7  8  9  :  ;  <  =  >  ?
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    //@  A  B  C  D  E  F  G  H  I  J  K  L  M  N  O
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    //P  Q  R  S  T  U  V  W  X  Y  Z  [  \  ]  ^  _
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1,
    //`  a  b  c  d  e  f  g  h  i  j  k  l  m  n  o
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    //p  q  r  s  t  u  v  w  x  y  z  {  |  }  ~  Del
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    //Extended ascii
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  };
  return map[c];
}

bool isFieldContentChar(uint8_t c) {
  // https://datatracker.ietf.org/doc/html/rfc9110#section-5.5-2
  static bool map[] = {
    //Nul                  BelBs Tb Lf Vt Ff Cr
      0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
    //                                 Esc
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    //SP !  "  #  $  %  &  '  (  )  *  +  ,  -  .  /
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    //0  1  2  3  4  5  6  7  8  9  :  ;  <  =  >  ?
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    //@  A  B  C  D  E  F  G  H  I  J  K  L  M  N  O
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    //P  Q  R  S  T  U  V  W  X  Y  Z  [  \  ]  ^  _
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    //`  a  b  c  d  e  f  g  h  i  j  k  l  m  n  o
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    //p  q  r  s  t  u  v  w  x  y  z  {  |  }  ~  Del
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    //Extended ascii
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  };
  return map[c];
}

bool isCRLFChar(uint8_t c) {
  static bool map[] = {
    //Nul                  BelBs Tb Lf Vt Ff Cr
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0,
    //                                 Esc
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    //SP !  "  #  $  %  &  '  (  )  *  +  ,  -  .  /
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    //0  0  2  3  4  5  6  7  8  9  :  ;  <  =  >  ?
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    //@  A  B  C  D  E  F  G  H  I  J  K  L  M  N  O
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    //P  Q  R  S  T  U  V  W  X  Y  Z  [  \  ]  ^  _
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    //`  a  b  c  d  e  f  g  h  i  j  k  l  m  n  o
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    //p  q  r  s  t  u  v  w  x  y  z  {  |  }  ~  Del
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    //Extended ascii
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  };
  return map[c];
}

bool fieldValueQuotesNeeded(const shared_string_view & str) {
  // The presence of any character that is not a token also requires the value
  // to be double-quoted.
  // https://www.rfc-editor.org/rfc/rfc9110.html#section-5.6.2
  // The presence of a comma necessitates the field being double-quoted.
  //   Note: This requirement is met by the isTokenChar() check, but is listed
  //   here for completeness.
  // https://www.rfc-editor.org/rfc/rfc9110.html#section-5.5-8
  for (auto & ch : str) {
    if (!isTokenChar(ch)) {
      return true;
    }
  }
  return false;
}

string fieldValueEscape(const shared_string_view & str) {
  // Safe characters, identified as "qdtext" in the spec.
  // https://www.rfc-editor.org/rfc/rfc9110.html#section-5.6.4-2
  static bool safe[] = {
    //Nul                  BelBs Tb Lf Vt Ff Cr
      0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
    //                                 Esc
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    //SP !  "  #  $  %  &  '  (  )  *  +  ,  -  .  /
      1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    //0  0  2  3  4  5  6  7  8  9  :  ;  <  =  >  ?
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    //@  A  B  C  D  E  F  G  H  I  J  K  L  M  N  O
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    //P  Q  R  S  T  U  V  W  X  Y  Z  [  \  ]  ^  _
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1,
    //`  a  b  c  d  e  f  g  h  i  j  k  l  m  n  o
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    //p  q  r  s  t  u  v  w  x  y  z  {  |  }  ~  Del
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
    //Extended ascii
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  };
  string temp{};
  for (auto & ch : str) {
    if (!safe[(uint8_t) ch]) {
      temp += '\\';
    }
    temp += ch;
  }
  return temp;
}

}

