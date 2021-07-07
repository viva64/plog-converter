//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2021 (c) PVS-Studio LLC

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

  bool showSAST = false;
  bool showCWE = false;

  DetectShowTags(&showCWE, &showSAST);

  if (showCWE && msg.HasCWE())
    securityPrefix += msg.GetCWEString();

  if (showSAST && msg.HasSAST())
  {
    if (!securityPrefix.empty())
      securityPrefix += ", ";

    securityPrefix += msg.sastId;
  }

  if (!securityPrefix.empty())
  {
    securityPrefix = '[' + securityPrefix + "] ";
  }

  m_ostream << msg.GetFileUTF8() << "\t" << msg.GetLine() << "\t"
            << msg.GetLevelString("err", "warn", "note") << "\t"
            << msg.code << " "
            << securityPrefix << Escape(msg.message) << std::endl;
}

}
