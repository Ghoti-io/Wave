/**
 * @file
 *
 * Define the text parsing functions.
 */

#include "client.hpp"
#include <cstdint>
#include <ctype.h>
#include <set>
#include <string>

using namespace std;
using namespace Ghoti::Wave;

namespace Ghoti::Wave {

static set<string> listFields{
  "ACCEPT",
  "ACCEPT-CHARSET",
  "ACCEPT-ENCODING",
  "ACCEPT-LANGUAGE",
  "ACCEPT-RANGES",
  "CONNECTION",
  "CONTENT-ENCODING",
  "CONTENT-LANGUAGE",
  "EXPECT",
  "IF-MATCH",
  "IF-NONE-MATCH",
  "TE",
  "TRAILER",
  "UPGRADE",
  "VARY",
  "VIA",
};

bool isListField(const string & name) {
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

}

