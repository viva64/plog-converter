//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2022 (c) PVS-Studio LLC

#include "xmloutput.h"

namespace PlogConverter
{

XMLOutput::XMLOutput(const ProgramOptions &opt) : BasicFormatOutput{ opt }
{
}

void XMLOutput::Start()
{
  m_ostream << "<?xml version=\"1.0\"?>" << '\n'
            << "<NewDataSet>" << std::endl;
}

bool XMLOutput::Write(const Warning& msg)
{
  m_ostream << "  <PVS-Studio_Analysis_Log>" << '\n'
            << "    <Level>" << msg.level << "</Level>" << '\n'
            << "    <ErrorType>" << msg.GetLevelString() << "</ErrorType>" << '\n'
            << "    <ErrorCode>" << msg.code << "</ErrorCode>" << '\n'
            << "    <Message>" << EscapeHtml(msg.message) << "</Message>" << '\n'
            << "    <Line>" << msg.GetLine() << "</Line>" << '\n'
            << "    <File>" << msg.GetFileUTF8() << "</File>" << '\n';

  auto extendedLines = msg.GetExtendedLines();
  if(extendedLines.size() > 1)
  {
    m_ostream << "    <Positions>" << '\n';
    m_ostream << "      <Position lines=\"" << Join(extendedLines, [](auto v) { return std::to_string(v); }, ",") << "\">" << msg.GetFile() << "</Position>" << '\n';
    m_ostream << "    </Positions>" << '\n';
  }

  if (msg.HasCWE())
  {
    m_ostream << "    <CWECode>" << msg.GetCWEString() << "</CWECode>" << '\n';
  }

  if (msg.HasSAST())
  {
    m_ostream << "    <SAST>" << msg.sastId << "</SAST>" << '\n';
  }

  m_ostream << "  </PVS-Studio_Analysis_Log>" << std::endl;

  return true;
}

void XMLOutput::Finish()
{
  m_ostream << "</NewDataSet>" << std::endl;
  BasicFormatOutput<XMLOutput>::Finish();
}

}
