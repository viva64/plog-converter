//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2023 (c) PVS-Studio LLC

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
  m_jsonOutput["version"]  = m_version;
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

  msgJson["code"]   = msg.code;
  msgJson["cwe"]    = msg.cwe;
  msgJson["sastId"] = msg.sastId;
  msgJson["level"]  = msg.level;

  std::vector<nlohmann::json> positionsJsons;

  bool isFirstPosition = true;
  for (auto &position : msg.positions)
  {
    auto &positionsJson = positionsJsons.emplace_back();

    auto utf8file = position.file;
    ANSItoUTF8(utf8file);

    positionsJson["file"]      = std::move(utf8file);
    positionsJson["line"]      = position.line;
    positionsJson["endLine"]   = position.endLine;
    positionsJson["column"]    = position.column;
    positionsJson["endColumn"] = position.endColumn;

    if (isFirstPosition)
    {
      nlohmann::json navigationJson;

      const auto& nav                = position.navigation;
      navigationJson["previousLine"] = static_cast<std::int32_t>(nav.previousLine);
      navigationJson["currentLine"]  = static_cast<std::int32_t>(nav.currentLine);
      navigationJson["nextLine"]     = static_cast<std::int32_t>(nav.nextLine);
      navigationJson["columns"]      = nav.columns;
      positionsJson["navigation"]    = std::move(navigationJson);

      isFirstPosition                = false;
    }
  }

  msgJson["positions"]  = std::move(positionsJsons);
  msgJson["projects"]   = msg.projects;
  msgJson["message"]    = msg.message;
  msgJson["falseAlarm"] = msg.falseAlarm;
  msgJson["favorite"]   = msg.favorite;

  m_jsonOutput["warnings"].push_back(std::move(msgJson));

  return true;
}

void JsonOutput::Finish()
{
  m_ostream << std::setw(2) << m_jsonOutput << std::endl;
  BasicFormatOutput<JsonOutput>::Finish();
}

}
