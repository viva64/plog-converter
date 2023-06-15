//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2023 (c) PVS-Studio LLC

#include <iomanip>

#include "json.hpp"
#include "defectdojooutput.h"

namespace PlogConverter
{

  DefectDojoOutput::DefectDojoOutput(const ProgramOptions& options)
    : BasicFormatOutput{ options }
  {
  }

  void DefectDojoOutput::Start()
  {
    m_defectDojoOutput["type"] = "PVS-Studio";
    m_defectDojoOutput["findings"] = std::vector<nlohmann::json>{};
  }

  constexpr std::string_view GetDefectDojoSeverityString(unsigned level)
  {
    switch (level)
    {
    case 1: return "High";
    case 2: return "Medium";
    case 3: return "Low";
    default: return "Critical";
    }
  }

  bool DefectDojoOutput::Write(const Warning& msg)
  {
    if (msg.GetErrorCode() == 0)
    {
      return false;
    }

    nlohmann::json msgDefectDojo;

    msgDefectDojo["title"] = msg.code;
    msgDefectDojo["description"] = msg.code + ": " + msg.message;
    msgDefectDojo["severity"] = GetDefectDojoSeverityString(msg.level);
    msgDefectDojo["cwe"] = msg.cwe;

    auto position = msg.positions.front();

    ANSItoUTF8(position.file);

    ReplaceRelativeRoot(position.file, "");

    auto checkLine = std::to_string(position.navigation.currentLine) + 
                     position.file + 
                     msg.code + 
                     msg.message;

    msgDefectDojo["unique_id_from_tool"] = PvsStudio::PvsHash(checkLine);

    msgDefectDojo["file_path"] = position.file;
    msgDefectDojo["line"] = position.line;
    msgDefectDojo["references"] = msg.GetVivaUrl();
    msgDefectDojo["active"] = false;

    if (msg.falseAlarm) 
    {
      msgDefectDojo["false_p"] = msg.falseAlarm;
    }

    m_defectDojoOutput["findings"].emplace_back(std::move(msgDefectDojo));
    
    return true;
  }

  void DefectDojoOutput::Finish()
  {
    m_ostream << std::setw(2) << m_defectDojoOutput << std::endl;
    BasicFormatOutput<DefectDojoOutput>::Finish();
  }

  [[nodiscard]] bool DefectDojoOutput::SupportsRelativePath() noexcept
  {
    return true;
  }

  [[nodiscard]] bool DefectDojoOutput::OutputIsFile() noexcept
  {
    return true;
  }

  [[nodiscard]] std::string_view DefectDojoOutput::FormatName() noexcept
  {
    return "defectdojo";
  }

  [[nodiscard]] std::string_view DefectDojoOutput::OutputSuffix() noexcept
  {
    static std::string suffix{ std::string{ FormatName() }.append(".json") };
    return suffix;
  }

}

