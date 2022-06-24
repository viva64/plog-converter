//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2022 (c) PVS-Studio LLC

#ifndef TEAMCITYOUTPUT_H
#define TEAMCITYOUTPUT_H

#include "ioutput.h"

namespace PlogConverter
{

  class TeamCityOutput;
  template<>
  constexpr std::string_view GetFormatName<TeamCityOutput>() noexcept
  {
    return "teamcity";
  }
  
  class TeamCityOutput : public IOutput
  {
  private:
    std::string EscapeMessage(const std::string&);
    std::set<std::string> m_inspectionsIDs;
  public:
    explicit TeamCityOutput(const ProgramOptions&);
    bool Write(const Warning &msg) override;
    ~TeamCityOutput() override;

    [[nodiscard]]
    std::string_view GetFormatName() const noexcept override
    {
      return ::PlogConverter::GetFormatName<TeamCityOutput>();
    }
  };
}

#endif
