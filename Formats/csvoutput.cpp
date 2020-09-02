//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020 (c) PVS-Studio LLC

#include <algorithm>
#include "csvoutput.h"

namespace PlogConverter
{

CSVOutput::CSVOutput(const ProgramOptions &opt) : IOutput(opt, "csv")
{

}

static std::string Escape(const std::string &src)
{
  std::string str = src;
  ReplaceAll(str, "\"", "\"\"");
  ReplaceAll(str, ",", "\\,");
  return str;
}

void CSVOutput::Start()
{
  m_ostream << "Mark" << ','
            << "ErrorType" << ','
            << "ErrorCode" << ',';

  for (const auto& security : m_errorCodeMappings)
  {
    if (security == SecurityCodeMapping::CWE)
      m_ostream << "CWE" << ',';
    
    if (security == SecurityCodeMapping::MISRA)
      m_ostream << "MISRA" << ',';
  }

  m_ostream << "Message" << ','
            << "FileLink" << ','
            << "Line" << ','
            << "FilePath" << std::endl;
}

void CSVOutput::Write(const Warning& msg)
{
  m_ostream << ","
            << msg.GetLevelString() << ","
            << "\"=HYPERLINK(\"\"" << msg.GetVivaUrl() << "\"\", \"\"" << msg.code << "\"\")\"" << ',';

  for (const auto& security : m_errorCodeMappings)
  {
    if (security == SecurityCodeMapping::CWE)
    {
      if (msg.HasCWE())
        m_ostream << "\"=HYPERLINK(\"\"" << msg.GetCWEUrl() << "\"\", \"\"" << msg.GetCWEString() << "\"\")\"" << ',';
      else
        m_ostream << " ,";
    }
    
    if (security == SecurityCodeMapping::MISRA)
    {
      if (msg.HasMISRA())
        m_ostream << "\"=HYPERLINK(\"\"" << msg.GetVivaUrl() << "\"\", \"\"" << msg.GetMISRAStringWithLanguagePrefix() << "\"\")\"" << ',';
      else
        m_ostream << " ,";
    }
  }

  m_ostream << '"' << Escape(msg.message) << '"' << ','
            << "\"=HYPERLINK(\"\"file://" << msg.GetFile() << "\"\", \"\" Open file\"\")\"" << ','
            << '"' << msg.GetLine() << '"' << ','
            << Escape(msg.GetFile()) << std::endl;
}

}