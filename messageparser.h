//  2006-2008 (c) Viva64.com Team
//  2008-2018 (c) OOO "Program Verification Systems"

#ifndef MESSAGEPARSER_H
#define MESSAGEPARSER_H
#include <string>
#include "warning.h"

namespace PlogConverter
{

class MessageParser
{
public:
  MessageParser();
  void Parse(const std::string& line, Warning& msg);

  static const std::string delimiter;
  static void StringFromMessage(const Warning &msg, std::string &res);

protected:
  bool ParseMessage(const std::string &line, Warning& msg);
  unsigned ParseSecurityId(const std::vector<std::string> &alternativeNames, const std::string &idTypePrefix) const;

private:
  std::vector<std::string> m_fields;
};

}

#endif // MESSAGEPARSER_H
