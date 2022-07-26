//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2022 (c) PVS-Studio LLC

#pragma once

#include <string_view>

#include "ioutput.h"

namespace PlogConverter
{

class TeamCityOutput : public BasicFormatOutput<TeamCityOutput>
{
private:
  static std::string EscapeMessage(std::string_view);
  std::set<std::string> m_inspectionsIDs;

public:
  explicit TeamCityOutput(const ProgramOptions&);
  ~TeamCityOutput() override = default;

  bool Write(const Warning &msg) override;

  [[nodiscard]]
  static bool SupportsRelativePath() noexcept
  {
    return false;
  }

  [[nodiscard]]
  static bool OutputIsFile() noexcept
  {
    return true;
  }

  [[nodiscard]]
  static std::string_view FormatName() noexcept
  {
    return "teamcity";
  }

  [[nodiscard]]
  static std::string_view OutputSuffix() noexcept
  {
    return "TeamCity.txt";
  }
};

}
