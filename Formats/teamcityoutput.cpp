//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020 (c) PVS-Studio LLC

#include "teamcityoutput.h"

namespace PlogConverter
{
  TeamCityOutput::TeamCityOutput(const ProgramOptions& opt) : IOutput(opt, "TeamCity.txt")
  {

  }

  std::string TeamCityOutput::EscapeMessage(const std::string& messageToEscape)
  {
    const char teamCityEscape = '|';
    std::string result;
    result.reserve(messageToEscape.size() + messageToEscape.size() / 2);
    for (const auto symbol : messageToEscape)
    {      
      switch (symbol)
      {
        case '|':
        case '\'':
        case '[':
        case ']':
          result += teamCityEscape;
          result += symbol;
          break;
        case '\r':
          result += teamCityEscape;
          result += 'r';
          break;
        case '\n':
          result += teamCityEscape;
          result += 'n';
          break;
        default:
          result += symbol;
          break;
        }
    }
    return result;
  }

  void TeamCityOutput::Write(const Warning& msg) 
  {   
    std::string securityPrefix;

    for (const auto& security : m_errorCodeMappings)
    {
      if (security == SecurityCodeMapping::CWE && msg.HasCWE())
        securityPrefix += msg.GetCWEString();

      if (security == SecurityCodeMapping::MISRA && msg.HasMISRA())
      {
        if (!securityPrefix.empty())
          securityPrefix += ", ";

        securityPrefix += msg.GetMISRAStringWithLanguagePrefix();
      }
    }

    if (!securityPrefix.empty())
    {
      securityPrefix = '[' + securityPrefix + "] ";
    }
     
    if(m_inspectionsIDs.find(msg.code) == m_inspectionsIDs.end())
    {    
      m_ostream << "##teamcity[inspectionType id='" << msg.code << "'"
                << "name = '" << msg.code << "'"
                << "description = '" << msg.GetVivaUrl() << "'"
                << "category = '" << msg.GetLevelString("High" , "Medium" , "Low") << "']"
                << std::endl;     
      m_inspectionsIDs.emplace(msg.code);
    }
    m_ostream << "##teamcity[inspection typeId='" << msg.code << "'"
              << "message = '" << EscapeMessage(securityPrefix + msg.message) << "'"
              << "file = '" << msg.GetFile() << "'"
              << "line = '" << msg.GetLine() << "'"
              << "SEVERITY = 'ERROR']"
              << std::endl;    
  }

  TeamCityOutput::~TeamCityOutput() = default;
}