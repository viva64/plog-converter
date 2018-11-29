//  2006-2008 (c) Viva64.com Team
//  2008-2018 (c) OOO "Program Verification Systems"

#include <algorithm>
#include "simplehtml.h"

namespace PlogConverter
{

using namespace std;

SimpleHTMLOutput::SimpleHTMLOutput(const ProgramOptions &opt) : IOutput(opt, "html")
{

}

SimpleHTMLOutput::~SimpleHTMLOutput() = default;

const int SimpleHTMLOutput::DefaultCweColumnWidth = 6;  //%
const int SimpleHTMLOutput::DefaultMISRAColumnWidth = 9;  //%
const int SimpleHTMLOutput::DefaultMessageColumnWidth = 65; //%

static char HtmlHead[] = R"(
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" lang="en">
<head>
  <title>Messages</title>
  <meta http-equiv="content-type" content="text/html; charset=windows-1251" />
  <style type="text/css">
    td {
      padding: 0;
      text-align: left;
      vertical-align: top;
    }

    legend {
      color: blue;
      font: 1.2em bold Comic Sans MS Verdana;
      text-align: center;
    }
  </style>
</head>
<body>
  <table style="width: 100%; font: 12pt normal Century Gothic;">
)";

static char HtmlEnd[] = R"(
  </table>
</body>
</html>
)";

void SimpleHTMLOutput::PrintHtmlStart()
{
  m_ostream << HtmlHead << std::endl;
}

void SimpleHTMLOutput::PrintHtmlEnd()
{
  m_ostream << HtmlEnd << std::endl;
}

void SimpleHTMLOutput::PrintHeading(const std::string &text)
{
  m_ostream << R"(    <tr style='background: lightcyan;'>)" << endl;
  m_ostream << R"(      <td colspan='5' style='color: red; text-align: center; font-size: 1.2em;'>)" << text << R"(</td>)" << endl;
  m_ostream << R"(    </tr>)" << endl;
}

int SimpleHTMLOutput::GetMessageColumnWidth() const
{
  int width = DefaultMessageColumnWidth;

  for (const auto& security : m_errorCodeMappings)
  {
    if (security == SecurityCodeMapping::CWE)
      width -= GetCweColumnWidth();

    if (security == SecurityCodeMapping::MISRA)
      width -= GetMISRAColumnWidth();
  }

  return width;
}

int SimpleHTMLOutput::GetCweColumnWidth() const
{
  return DefaultCweColumnWidth;
}

int SimpleHTMLOutput::GetMISRAColumnWidth() const
{
  return DefaultMISRAColumnWidth;
}

void SimpleHTMLOutput::PrintTableCaption()
{
#ifdef WIN32
  m_ostream << R"(    <caption style="font-weight: bold;background: #fff;color: #000;border: none !important;">MESSAGES</caption>)" << endl;
  m_ostream << R"(    <tr style="background: black; color: white;">)" << endl;
  m_ostream << R"(      <th style="width: 10%;">Project</th>)" << endl;
  m_ostream << R"(      <th style="width: 20%;">File</th>)" << endl;
  m_ostream << R"(      <th style="width: 5%;">Code</th>)" << endl;
  m_ostream << R"(      <th style="width: 45%;">Message</th>)" << endl;
  m_ostream << R"(      <th style="width: 20%;">Analyzed Source File(s)</th>)" << endl;
  m_ostream << R"(    </tr>)" << endl;
#else
  m_ostream << R"(    <caption style="font-weight: bold;background: #fff;color: #000;border: none !important;">MESSAGES</caption>)" << endl;
  m_ostream << R"(    <tr style="background: black; color: white;">)" << endl;
  m_ostream << R"(      <th style="width: 30%;">Location</th>)" << endl;
  m_ostream << R"(      <th style="width: 5%;">Code</th>)" << endl;

  for (const auto& security : m_errorCodeMappings)
  {
    if (security == SecurityCodeMapping::CWE)
      m_ostream << R"(      <th style="width: )" << GetCweColumnWidth() << R"(%;">CWE</th>)" << endl;
    
    if (security == SecurityCodeMapping::MISRA)
      m_ostream << R"(      <th style="width: )" << GetMISRAColumnWidth() << R"(%;">MISRA</th>)" << endl;
  }

  m_ostream << R"(      <th style="width: 65%;">Message</th>)" << endl;
  m_ostream << R"(    </tr>)" << endl;
#endif
}

void SimpleHTMLOutput::PrintMessages(const std::vector<Warning> &messages, const std::string &caption)
{
  if (messages.empty())
  {
    return;
  }

  PrintHeading(caption);
  for (const auto &err : messages)
  {
#ifdef WIN32
    m_ostream << R"(    <tr>)" << endl;
    m_ostream << R"(      <td style='width: 10%;'></td>)" << endl;
    m_ostream << R"(      <td style='width: 20%;'><div title=")" << EscapeHtml(err.GetFile()) << R"(">)" << FileBaseName(err.GetFile())
              << " (" << err.GetLine() << R"()</div></td>)" << endl;
    m_ostream << R"(      <td style='width: 5%;'><a target="_blank" href=')" << err.GetVivaUrl() << R"('>)"
              << err.code << R"(</a></td>)" << endl;
    m_ostream << R"(      <td style='width: 45%;'>)" << EscapeHtml(err.message) << R"(</td>)" << endl;
    m_ostream << R"(      <td style='width: 20%;'></td>)" << endl;
    m_ostream << R"(    </tr>)" << endl;
#else
    m_ostream << R"(    <tr>)" << endl;
    m_ostream << R"(      <td style='width: 30%;'><div title=")"
              << EscapeHtml(err.GetFile())
              << R"(">)"
              << FileBaseName(err.GetFile())
              << " (" << err.GetLine()
              << R"()</div></td>)"
              << endl;
    m_ostream << R"(      <td style='width: 5%;'><a target="_blank" href=')"
              << err.GetVivaUrl()
              << R"('>)"
              << err.code
              << R"(</a></td>)"
              << endl;

    for (const auto& security : m_errorCodeMappings)
    {
      if (security == SecurityCodeMapping::CWE)
      {
        if (err.HasCWE())
          m_ostream << R"(      <td style='width: )" << GetCweColumnWidth() << R"(%;'><a target="_blank" href=')"
                    << err.GetCWEUrl() << R"('>)" << err.GetCWEString() << R"(</a></td>)" << endl;
        else
          m_ostream << R"(      <th style="width: )" << GetCweColumnWidth() << R"(%;"></th>)" << endl;
      }

      if (security == SecurityCodeMapping::MISRA)
      {
        if (err.HasMISRA())
          m_ostream << R"(      <td style='width: )" << GetMISRAColumnWidth() << R"(%;'><a target="_blank" href=')"
                    << err.GetVivaUrl() << R"('>)" << err.GetMISRAString() << R"(</a></td>)" << endl;
        else
          m_ostream << R"(      <th style="width: )" << GetMISRAColumnWidth() << R"(%;"></th>)" << endl;
      }
    }

    m_ostream << R"(      <td style='width: )" << GetMessageColumnWidth() << R"(%;'>)" << EscapeHtml(err.message) << R"(</td>)" << endl;
    m_ostream << R"(    </tr>)" << endl;
#endif
  }
}

void SimpleHTMLOutput::PrintTableBody()
{
  PrintMessages(m_info, "Fails/Info");
  PrintMessages(m_ga, "General Analysis (GA)");
  PrintMessages(m_op, "Micro-optimizations (OP)");
  PrintMessages(m_64, "64-bit errors (64)");
  PrintMessages(m_cs, "Customers Specific (CS)");
  PrintMessages(m_misra, "MISRA");
}

void SimpleHTMLOutput::Start()
{
  PrintHtmlStart();
}

void SimpleHTMLOutput::Write(const Warning& msg)
{
  if (msg.IsDocumentationLinkMessage())
  {
    return;
  }

  AnalyzerType analyzerType = msg.GetType();
  if (analyzerType == AnalyzerType::General)
    m_ga.push_back(msg);
  else if (analyzerType == AnalyzerType::Optimization)
    m_op.push_back(msg);
  else if (analyzerType == AnalyzerType::Viva64)
    m_64.push_back(msg);
  else if (analyzerType == AnalyzerType::CustomerSpecific)
    m_cs.push_back(msg);
  else if (analyzerType == AnalyzerType::Misra)
    m_misra.push_back(msg);
  else
    m_info.push_back(msg);
}

void SimpleHTMLOutput::Finish()
{
  if (m_ga.empty() && m_op.empty() && m_64.empty() && m_cs.empty() && m_misra.empty() && m_info.empty())
  {
    m_ostream << "No messages generated" << std::endl;
  }
  else
  {
    PrintTableCaption();
    PrintTableBody();
  }

  PrintHtmlEnd();
}

}