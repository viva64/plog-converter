//  2006-2008 (c) Viva64.com Team
//  2008-2018 (c) OOO "Program Verification Systems"

#include "messageparser.h"
#include "utils.h"
#include <string>
#include <stdexcept>
#include <algorithm>
#include <cstring>
#include <array>

namespace PlogConverter
{

MessageParser::MessageParser() = default;

void MessageParser::Parse(const std::string& line, Warning& msg)
{
  try
  {
    if (!ParseMessage(line, msg))
    {
      msg.Clear();
      msg.message = line;
    }
  }
  catch (std::exception&)
  {
    msg.Clear();
    msg.message = line;
  }
}

const std::string MessageParser::delimiter = "<#~>";

bool MessageParser::ParseMessage(const std::string& line, Warning& msg)
{
  m_fields.clear();
  Split(line, delimiter, std::back_inserter(m_fields));

  if ((m_fields.size() != 13 && m_fields.size() != 14) || m_fields[0] != "Viva64-EM")
  {
    return false;
  }

  msg.trialMode = m_fields[1] == "trial";
  const auto lineNo = ParseUint(m_fields[2]);
  const auto file = std::move(m_fields[3]);
  //msg.errorType = std::move(m_fields[4]); (deprecated)
  msg.code = std::move(m_fields[5]);
  msg.message = std::move(m_fields[6]);
  msg.falseAlarm = m_fields[7] == "true";
  msg.level = ParseUint(m_fields[8]);

  msg.positions.clear();
  msg.positions.emplace_back(file, lineNo);

  auto &navigation = msg.positions.front().navigation;
  navigation.previousLineString = std::move(m_fields[9]);
  navigation.currentLineString = std::move(m_fields[10]);
  navigation.nextLineString = std::move(m_fields[11]);

  msg.additionalLines.clear();
  Split(m_fields[12], ",", std::back_inserter(msg.additionalLines), ParseUint);
  for (unsigned pos : msg.additionalLines)
  {
    if (pos != lineNo)
    {
      msg.positions.emplace_back(file, pos);
    }
  }

  if (m_fields.size() > 13)
  {
    std::vector<std::string> alternativeNames;
    Split(m_fields[13], ",", std::back_inserter(alternativeNames));
    
    //cwe
    auto parseId = ParseSecurityId(alternativeNames, Warning::CWEPrefix);
    if (!parseId.empty())
      msg.cwe = static_cast<unsigned>(std::stoi(parseId));
    else
      msg.cwe = 0;
    
    //misra
    msg.misra = ParseSecurityId(alternativeNames, Warning::MISRACorePrefix);
  }
  else
  {
    msg.cwe = 0;
    msg.misra = "";
  }

  return true;
}

std::string MessageParser::ParseSecurityId(const std::vector<std::string> &alternativeNames, const std::string &idTypePrefix) const
{
  for (const auto& security : alternativeNames)
    if (StartsWith(security, idTypePrefix))
      return security.substr(idTypePrefix.length());

  return "";
}

void MessageParser::StringFromMessage(const Warning &msg, std::string &res)
{
  auto join = [&res](const auto &v, const std::string &delimiter, auto fn)
  {
    if (!v.empty())
    {
      for (auto &&u : v)
      {
        res += fn(u);
        res += delimiter;
      }

      res.resize(res.length() - delimiter.size());
    }
  };

  res.clear();
  res += "Viva64-EM";
  res += delimiter;
  res += msg.trialMode ? "trial" : "full";
  res += delimiter;
  res += std::to_string(msg.GetLine());
  res += delimiter;
  res += msg.GetFile();
  res += delimiter;
  res += "error";
  res += delimiter;
  res += msg.code;
  res += delimiter;
  res += msg.message;
  res += delimiter;
  res += msg.falseAlarm ? "true" : "false";
  res += delimiter;
  res += std::to_string(msg.level);
  res += delimiter;
  res += msg.GetNavigationInfo().previousLineString;
  res += delimiter;
  res += msg.GetNavigationInfo().currentLineString;
  res += delimiter;
  res += msg.GetNavigationInfo().nextLineString;
  res += delimiter;
  join(msg.GetExtendedLines(), ",", [](auto &s) { return std::to_string(s); });
  res += delimiter;
  
  if(msg.HasCWE())
  {
    res += msg.GetCWEString();
  }

  if (msg.HasMISRA())
  {
    if (msg.HasCWE())
    {
      res += ',';
    }

    res += msg.GetMISRAString();
  }
}

}