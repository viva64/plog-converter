//  2006-2008 (c) Viva64.com Team
//  2008-2018 (c) OOO "Program Verification Systems"

#ifndef CHAR_MAP
#define CHAR_MAP
#include <cstddef>
#include <string>

namespace CharMap
{
  char Encode(char ch);
  char Decode(char ch);
  void Encode(std::string &str);
  void Decode(std::string &str);
  void EncodeForPlatform(std::string &str);
  bool IsStartEncodedMarker(const std::string &str);
}

#endif
