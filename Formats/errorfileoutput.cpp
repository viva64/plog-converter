//  2006-2008 (c) Viva64.com Team
//  2008-2018 (c) OOO "Program Verification Systems"

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
      securityPrefix += '[' + msg.GetCWEString() + ']';

    if (security == SecurityCodeMapping::MISRA && msg.HasMISRA())
    {
      if (!securityPrefix.empty())
        securityPrefix += " ";

      securityPrefix += '[' + msg.GetMISRAString() + ']';
    }
  }

  if (!securityPrefix.empty())
    securityPrefix += " ";

  m_ostream << msg.GetFile() << ":" << msg.GetLine() << ":1: "
            << msg.GetLevelString() << ": "
            << msg.code << " "
            << securityPrefix << msg.message << std::endl;
}

}