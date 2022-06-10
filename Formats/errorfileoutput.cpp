//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2022 (c) PVS-Studio LLC

#include "errorfileoutput.h"

namespace PlogConverter
{

ErrorFileOutput::ErrorFileOutput(const ProgramOptions &opt) : IOutput(opt, "err")
{
}

ErrorFileOutput::~ErrorFileOutput() = default;

bool ErrorFileOutput::Write(const Warning& msg)
{
  std::string securityPrefix;

  bool showSAST = false;
  bool showCWE = false;

  DetectShowTags(showCWE, showSAST);

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

  #if defined (_WIN32)
    const std::string column { ": " };
  #else
    const std::string column { ":1: " };
  #endif

  m_ostream << msg.GetFileUTF8() << ":" << msg.GetLine() << column
            << msg.GetLevelString() << ": "
            << msg.code << " "
            << securityPrefix << msg.message << std::endl;

  return true;
}

}
