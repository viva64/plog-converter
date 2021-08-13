//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2021 (c) PVS-Studio LLC

#ifndef MESSAGEPARSER_H
#define MESSAGEPARSER_H
#include <string>
#include "warning.h"

namespace PlogConverter
{

class MessageParser;
void from_json(const nlohmann::json& j, MessageParser& mp);

class MessageParser
{
public:
  MessageParser();
  void Parse(
    const std::string& line,
    Warning& msg);
  void Parse(
    const std::string& file,
    const std::string& line,
    const std::string& level,
    const std::string& text,
    Warning& msg);

  static const std::string delimiter;
  static void StringFromMessage(const Warning &msg, std::string &res);

protected:
  bool ParseMessage(const std::string &line, Warning& msg);
  std::string ParseSecurityId(const std::vector<std::string> &alternativeNames, const std::string &idTypePrefix) const;

private:
  std::vector<std::string> m_fields;
  friend void from_json(const nlohmann::json& j, MessageParser& mp);
};

}

#endif // MESSAGEPARSER_H
