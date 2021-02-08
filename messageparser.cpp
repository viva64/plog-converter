//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2021 (c) PVS-Studio LLC

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
}

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
  emplaceFromJson("sastId");
  emplaceFromJson("cwe");
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
    msg.sastId.clear();
    
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

bool MessageParser::ParseMessage(const std::string& srcLine, Warning& msg)
{
  msg.format = Warning::MessageFormat::Unknown;

  m_fields.clear();
  std::string line = Trim(srcLine);
  if (StartsWith(line, "{") && EndsWith(line, "}"))
  {
    try
    {
      auto j = nlohmann::json::parse(line);
      auto mp = j.get<MessageParser>();
      m_fields = std::move(mp.m_fields);
      msg.format = Warning::MessageFormat::RawJson;
    }
    catch (const JsonParseException &)
    {
      std::cerr << "Wrong JSON format" << std::endl;
      return false;
    }
  }
  else
  {
    Split(line, delimiter, std::back_inserter(m_fields));
    msg.format = Warning::MessageFormat::OldStyle;
  }

  if ((m_fields.size() != 13 && m_fields.size() != 15) || m_fields[0] != "Viva64-EM")
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

  msg.sastId = m_fields.size() > 13 ? m_fields[13] : std::string();
  msg.cwe = m_fields.size() > 14 && !m_fields[14].empty() ? static_cast<unsigned>(std::stoi(m_fields[14])) : 0;

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
  res.clear();

  std::string alternativeNames;
  if (msg.HasCWE())
  {
    alternativeNames += msg.GetCWEString();
  }

  if (msg.HasSAST())
  {
    if (!alternativeNames.empty())
    {
      alternativeNames += ',';
    }

    alternativeNames += msg.GetMISRAString();
  }

  switch (msg.format)
  {
  case Warning::MessageFormat::OldStyle:
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
    res += alternativeNames;
    break;
  }

  case Warning::MessageFormat::RawJson:
  {
    auto lines = msg.GetExtendedLines();
    if (lines.empty())
    {
      lines.push_back(msg.GetLine());
    }

    auto j = nlohmann::json {
      { "falseAlarm", msg.falseAlarm },
      { "level"     , msg.level },
      { "code"      , msg.code },
      { "message"   , msg.message },
      {
        "positions" , std::vector<nlohmann::json> {
                      {
                        { "file",  msg.GetFile() },
                        { "lines", lines }
                      } }
      }
    };

    constexpr auto putOptionalStringView = [](nlohmann::json &j, std::string_view fieldName, std::string value)
    {
      if (!value.empty())
      {
        j.emplace(fieldName, std::move(value));
      }
    };

    putOptionalStringView(j, "alternativeNames", alternativeNames);

    putOptionalStringView(j, "prevLine", msg.GetNavigationInfo().previousLineString);
    putOptionalStringView(j, "currLine", msg.GetNavigationInfo().currentLineString);
    putOptionalStringView(j, "nextLine", msg.GetNavigationInfo().nextLineString);

    res = j.dump(-1, '\0', false);
    break;
  }

  default:
  {
    std::cerr << "Wrong warning format" << std::endl;
    break;
  }
  }
}

}