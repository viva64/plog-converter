//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2022 (c) PVS-Studio LLC

#include <iomanip>

#include "json.hpp"
#include "jsonoutput.h"

namespace PlogConverter
{

JsonOutput::JsonOutput(const ProgramOptions& options) : BasicFormatOutput{ options }
{
}

void JsonOutput::Start()
{
  m_jsonOutput["version"] = m_version;
  m_jsonOutput["warnings"] = std::vector<nlohmann::json>{};
}

bool CheckCode(const Warning& msg)
{
  if (msg.GetErrorCode() == 0)
  {
    return false;
  }

  return true;
}

bool JsonOutput::Write(const Warning& msg)
{
  if (!CheckCode(msg))
  {
    return false;
  }

  nlohmann::json msgJson;

  msgJson["code"]  = msg.code;
  msgJson["cwe"]   = msg.cwe;
  msgJson["sastId"] = msg.sastId;
  msgJson["level"] = msg.level;

  std::vector<nlohmann::json> positionsJsons;

  bool isFirstIteration = true;
  for (auto position : msg.positions)
  {
    auto &positionsJson = positionsJsons.emplace_back();

    ANSItoUTF8(position.file);
    positionsJson["file"] = position.file;
    positionsJson["line"] = position.line;
    positionsJson["endLine"] = position.endLine;
    positionsJson["column"] = position.column;
    positionsJson["endColumn"] = position.endColumn;

    if (isFirstIteration)
    {
      nlohmann::json navigationJson;
      const auto& nav = position.navigation;
      navigationJson["previousLine"] = static_cast<int>(PvsStudio::PvsHash(nav.previousLineString));
      navigationJson["currentLine"] = static_cast<int>(PvsStudio::PvsHash(nav.currentLineString));
      navigationJson["nextLine"] = static_cast<int>(PvsStudio::PvsHash(nav.nextLineString));
      navigationJson["columns"] = nav.columns;

      positionsJson["navigation"] = navigationJson;

      isFirstIteration = false;
    }
  }

  msgJson["positions"] = positionsJsons;

  msgJson["projects"]   = msg.projects;

  msgJson["message"] = msg.message;
  msgJson["falseAlarm"] = msg.falseAlarm;
  msgJson["favorite"] = msg.favorite;

  m_jsonOutput["warnings"].emplace_back(msgJson);

  return true;
}

void JsonOutput::Finish()
{
  m_ostream << std::setw(2) << m_jsonOutput << std::endl;
  BasicFormatOutput<JsonOutput>::Finish();
}

}
