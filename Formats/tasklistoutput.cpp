//  2006-2008 (c) Viva64.com Team
//  2008-2018 (c) OOO "Program Verification Systems"

#include "tasklistoutput.h"

namespace PlogConverter
{

TaskListOutput::TaskListOutput(const ProgramOptions &opt) : IOutput(opt, "tasks")
{

}

TaskListOutput::~TaskListOutput() = default;

static std::string Escape(const std::string &str)
{
  std::string res = str;
  ReplaceAll(res, "\\", "\\\\");
  return res;
}

void TaskListOutput::Write(const Warning& msg)
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

      securityPrefix += '[' + msg.GetMISRAStringWithLanguagePrefix() + ']';
    }
  }

  if (!securityPrefix.empty())
    securityPrefix += " ";

  m_ostream << msg.GetFile() << "\t" << msg.GetLine() << "\t"
            << msg.GetLevelString("err", "warn", "note") << "\t"
            << msg.code << " "
            << securityPrefix << Escape(msg.message) << std::endl;
}

}