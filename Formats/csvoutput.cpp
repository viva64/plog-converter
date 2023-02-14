//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2023 (c) PVS-Studio LLC

#include <algorithm>
#include <string_view>

#include "csvoutput.h"

namespace PlogConverter
{

CSVOutput::CSVOutput(const ProgramOptions &opt) : BasicFormatOutput(opt)
{
}

static std::string Escape(std::string_view src)
{
  auto str = std::string{ src };
  ReplaceAll(str, "\"", "\"\"");
  ReplaceAll(str, ",", "\\,");
  return str;
}

void CSVOutput::Start()
{
  m_ostream << "Mark" << ','
            << "ErrorType" << ','
            << "ErrorCode" << ',';

  bool showCwe = false;
  bool showSast = false;

  DetectShowTags(showCwe, showSast);

  if (showCwe)
    m_ostream << "CWE" << ',';

  if (showSast)
    m_ostream << "SAST" << ',';

  m_ostream << "Message" << ','
            << "FileLink" << ','
            << "Line" << ','
            << "FilePath" << std::endl;
}

bool CSVOutput::Write(const Warning& msg)
{
  m_ostream << ","
            << msg.GetLevelString() << ","
            << "\"=HYPERLINK(\"\"" << msg.GetVivaUrl() << "\"\", \"\"" << msg.code << "\"\")\"" << ',';

  bool showSAST = false;
  bool showCWE = false;

  DetectShowTags(showCWE, showSAST);

  if (showCWE)
  {
    if (msg.HasCWE())
      m_ostream << "\"=HYPERLINK(\"\"" << msg.GetCWEUrl() << "\"\", \"\"" << msg.GetCWEString() << "\"\")\"" << ',';
    else
      m_ostream << " ,";
  }

  if (showSAST)
  {
    if (msg.HasSAST())
      m_ostream << '"' << msg.sastId << '"' << ',';
    else
      m_ostream << " ,";
  }

  std::string fileUTF8 = msg.GetFileUTF8();
  m_ostream << '"' << Escape(msg.message) << '"' << ','
            << "\"=HYPERLINK(\"\"file://" << fileUTF8 << "\"\", \"\" Open file\"\")\"" << ','
            << '"' << msg.GetLine() << '"' << ','
            << Escape(fileUTF8) << std::endl;

  return true;
}

}