#include <iomanip>
#include "jsonoutput.h"
#include "json.hpp"

namespace PlogConverter
{

JsonOutput::JsonOutput(const ProgramOptions& options)
  : IOutput { options, "json" }
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

static unsigned GetHashCodePVS(std::string_view msg)
{
  unsigned sum = 0;
  for (char ch : msg)
  {
    if (ch != ' ' && ch != '\t')
    {
      bool hiBit = (sum & 0x80000000u) != 0;
      sum <<= 1;
      sum ^= ch;
      if (hiBit)
        sum ^= 0x00000001u;
    }
  }
  return sum;
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
      navigationJson["previousLine"] = static_cast<int>(GetHashCodePVS(nav.previousLineString));
      navigationJson["currentLine"] = static_cast<int>(GetHashCodePVS(nav.currentLineString));
      navigationJson["nextLine"] = static_cast<int>(GetHashCodePVS(nav.nextLineString));
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
  m_ostream << std::setw(2) << m_jsonOutput;
}

}
