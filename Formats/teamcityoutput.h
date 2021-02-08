//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2021 (c) PVS-Studio LLC

#ifndef TEAMCITYOUTPUT_H
#define TEAMCITYOUTPUT_H

#include "ioutput.h"

namespace PlogConverter
{
  class TeamCityOutput : public IOutput
  {
  private:
    std::string EscapeMessage(const std::string&);
    std::set<std::string> m_inspectionsIDs;
  public:
    explicit TeamCityOutput(const ProgramOptions&);
    void Write(const Warning&) override; 
    ~TeamCityOutput() override; 
  };
}  

#endif
