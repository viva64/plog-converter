//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"

#include "errorfileoutput.h"

namespace PlogConverter
{

ErrorFileOutput::ErrorFileOutput(const ProgramOptions &opt) : IOutput(opt, "err")
{

}

ErrorFileOutput::~ErrorFileOutput() = default;

void ErrorFileOutput::Write(const Warning& msg)
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

  #if defined (_WIN32)
    std::string_view column = ": ";
  #else
    std::string_view column = ":1: ";
  #endif

  m_ostream << msg.GetFile() << ":" << msg.GetLine() << column
            << msg.GetLevelString() << ": "
            << msg.code << " "
            << securityPrefix << msg.message << std::endl;
}

}
