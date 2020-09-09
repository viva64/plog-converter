//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020 (c) PVS-Studio LLC

#include "messageparser.h"
#include "utils.h"
#include "../ThirdParty/json.hpp"
#include <string>
#include <stdexcept>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <array>

namespace PlogConverter
{

class JsonParseException : public std::exception
{
};

std::string getFromJson(const nlohmann::json& j, std::string_view key, bool requiered = false)
{
  if (!j.contains(key))
  {
    if (!requiered)
      return std::string{};

    throw JsonParseException{};
  }

  auto data = j.at(key.data());

  if (data.is_string())
  {
    return j.at(key.data()).get<std::string>();
  }
  else if (data.is_number_unsigned())
  {
    return std::to_string(j.at(key.data()).get<unsigned>());
  }
  else
  {
    return j.at(key.data()).dump();
  }
};

struct Position
{
  std::string file;
  std::vector<size_t> lines;
};

void from_json(const nlohmann::json& j, Position& p)
{
  p.file = getFromJson(j, "file", true);
  auto lines = nlohmann::json::parse(getFromJson(j, "lines", true));
  p.lines = lines.get<std::vector<size_t>>();
}

void from_json(const nlohmann::json &j, MessageParser &mp)
{
  auto emplaceFromJson = [&j, &mp](std::string_view key, bool requiered = false)
  {
    mp.m_fields.emplace_back(getFromJson(j, key, requiered));
  };

  auto getPositions = [&j]()
  {
    auto positions = nlohmann::json::parse(getFromJson(j, "positions", true));

    if (!positions.is_array())
    {
      throw JsonParseException{};
    }

    return positions.get<std::vector<Position>>();
  };

  auto position = getPositions().front();

  mp.m_fields.reserve(14);
  mp.m_fields.emplace_back("Viva64-EM");
  mp.m_fields.emplace_back("full");
  mp.m_fields.emplace_back(std::to_string(position.lines.front()));
  mp.m_fields.emplace_back(position.file);
  mp.m_fields.emplace_back("error"); // old format compatibility
  emplaceFromJson("code", true);
  emplaceFromJson("message", true);
  emplaceFromJson("falseAlarm", true);
  emplaceFromJson("level", true);
  emplaceFromJson("prevLine");
  emplaceFromJson("currLine");
  emplaceFromJson("nextLine");
  mp.m_fields.emplace_back(Join(position.lines, [](auto v) { return std::to_string(v); }, ","));
  emplaceFromJson("alternativeNames");
}

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

void MessageParser::Parse(
  const std::string& file,
  const std::string& line,
  const std::string& level,
  const std::string& text, Warning& msg)
{
  try
  {
    msg.trialMode = false;
    msg.code = "External";
    msg.message = text;
    msg.falseAlarm = false;
    msg.positions.clear();
    msg.positions.emplace_back(file, ParseUint(line));
    msg.cwe = 0;
    msg.misra.clear();
    
    if (level == "error")
    {
      msg.level = 1;
    }
    else if (level == "warn" || level == "warning")
    {
      msg.level = 2;
    }
    else if (level == "note")
    {
      msg.level = 3;
    }
    else
    {
      msg.level = 0;
    }
  }
  catch (std::exception&)
  {
    msg.Clear();
    msg.message = text;
  }
}

const std::string MessageParser::delimiter = "<#~>";

bool MessageParser::ParseMessage(const std::string& line, Warning& msg)
{
  m_fields.clear();

  if (StartsWith(line, "{") && EndsWith(line, "}"))
  {
    try
    {
      auto j = nlohmann::json::parse(line);
      auto mp = j.get<MessageParser>();
      m_fields = std::move(mp.m_fields);
    }
    catch ([[maybe_unused]]const JsonParseException& e)
    {
      std::cerr << "Wrong JSON format" << std::endl;
      return false;
    }
  }
  else
  {
    Split(line, delimiter, std::back_inserter(m_fields));
  }

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
    msg.misra.clear();
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