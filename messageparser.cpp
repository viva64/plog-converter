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

bool MessageParser::ParseMessage(const std::string& srcLine, Warning& msg)
{
  try
  {
    msg = Warning::Parse(srcLine);
  }
  catch (ParseException &)
  {
    return false;
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
  switch (msg.format)
  {
  case Warning::Format::OldStyle:
  {
    res = msg.GetOldstyleOutput();
    break;
  }

  case Warning::Format::RawJson:
  {
    res = msg.GetJsonOutput();
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