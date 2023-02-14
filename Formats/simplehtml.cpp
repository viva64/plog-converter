//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2023 (c) PVS-Studio LLC

#include <algorithm>

#include "simplehtml.h"

namespace PlogConverter
{

SimpleHTMLOutput::SimpleHTMLOutput(const ProgramOptions &opt) : BasicFormatOutput{ opt }
{
}

const int SimpleHTMLOutput::DefaultCweColumnWidth = 6;  //%
const int SimpleHTMLOutput::DefaultSASTColumnWidth = 9;  //%
const int SimpleHTMLOutput::DefaultMessageColumnWidth = 65; //%

static constexpr std::string_view SimpleHtmlHead =
R"(<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" lang="en">
<head>
  <title>Messages</title>
  <meta http-equiv="content-type" content="text/html; charset=utf-8" />
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

static constexpr std::string_view SimpleHtmlEnd =
R"(  </table>
</body>
</html>)";

void SimpleHTMLOutput::PrintHtmlStart()
{
  m_ostream << SimpleHtmlHead;
}

void SimpleHTMLOutput::PrintHtmlEnd()
{
  m_ostream << SimpleHtmlEnd << std::endl;
}

void SimpleHTMLOutput::PrintHeading(const std::string &text)
{
  m_ostream << R"(    <tr style='background: lightcyan;'>)" << '\n';
  m_ostream << R"(      <td colspan='5' style='color: red; text-align: center; font-size: 1.2em;'>)" << text << R"(</td>)" << '\n';
  m_ostream << R"(    </tr>)" << std::endl;
}

int SimpleHTMLOutput::GetMessageColumnWidth() const
{
  int width = DefaultMessageColumnWidth;

  bool showSAST = false;
  bool showCWE = false;

  DetectShowTags(showCWE, showSAST);

  if (showCWE)
  {
    width -= GetCweColumnWidth();
  }

  if (showSAST)
  {
    width -= GetSASTColumnWidth();
  }

  return width;
}

int SimpleHTMLOutput::GetCweColumnWidth() const
{
  return DefaultCweColumnWidth;
}

int SimpleHTMLOutput::GetSASTColumnWidth() const
{
  return DefaultSASTColumnWidth;
}

void SimpleHTMLOutput::PrintTableCaption()
{
  m_ostream << R"(    <caption style="font-weight: bold;background: #fff;color: #000;border: none !important;">MESSAGES</caption>)" << '\n';
  m_ostream << R"(    <tr style="background: black; color: white;">)" << '\n';
#ifdef WIN32
  m_ostream << R"(      <th style="width: 10%;">Project</th>)" << '\n';
  m_ostream << R"(      <th style="width: 20%;">File</th>)" << '\n';
  m_ostream << R"(      <th style="width: 5%;">Code</th>)" << '\n';
  m_ostream << R"(      <th style="width: 45%;">Message</th>)" << '\n';
  m_ostream << R"(      <th style="width: 20%;">Analyzed Source File(s)</th>)" << '\n';
#else
  m_ostream << R"(      <th style="width: 30%;">Location</th>)" << '\n';
  m_ostream << R"(      <th style="width: 5%;">Code</th>)" << '\n';

  bool showCwe = false;
  bool showSast = false;

  DetectShowTags(showCwe, showSast);

  if (showCwe)
  {
    m_ostream << R"(      <th style="width: )" << GetCweColumnWidth() << R"(%;">CWE</th>)" << '\n';
  }
  if (showSast)
  {
    m_ostream << R"(      <th style="width: )" << GetSASTColumnWidth() << R"(%;">SAST</th>)" << '\n';
  }

  m_ostream << R"(      <th style="width: 65%;">Message</th>)" << '\n';
#endif
  m_ostream << R"(    </tr>)" << std::endl;
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
    std::string fileUTF8 = err.GetFileUTF8();
    m_ostream << R"(    <tr>)" << '\n';
    m_ostream << R"(      <td style='width: 10%;'></td>)" << '\n';
    m_ostream << R"(      <td style='width: 20%;'><div title=")" << EscapeHtml(fileUTF8) << R"(">)" << FileBaseName(fileUTF8)
              << " (" << err.GetLine() << R"()</div></td>)" << '\n';
    m_ostream << R"(      <td style='width: 5%;'><a target="_blank" href=')" << err.GetVivaUrl() << R"('>)"
              << err.code << R"(</a></td>)" << '\n';
    m_ostream << R"(      <td style='width: 45%;'>)" << EscapeHtml(err.message) << R"(</td>)" << '\n';
    m_ostream << R"(      <td style='width: 20%;'></td>)" << '\n';
    m_ostream << R"(    </tr>)" << std::endl;
#else
    m_ostream << R"(    <tr>)" << '\n';
    m_ostream << R"(      <td style='width: 30%;'><div title=")" << EscapeHtml(err.GetFile()) << R"(">)" << FileBaseName(err.GetFile())
              << " (" << err.GetLine() << R"()</div></td>)" << '\n';
    m_ostream << R"(      <td style='width: 5%;'><a target="_blank" href=')" << err.GetVivaUrl() << R"('>)"
              << err.code << R"(</a></td>)" << '\n';

    bool showCwe = false;
    bool showSast = false;

    DetectShowTags(showCwe, showSast);

    if (showCwe)
    {
      if (err.HasCWE())
      {
        m_ostream << R"(      <td style='width: )" << GetCweColumnWidth() << R"(%;'><a target="_blank" href=')"
                  << err.GetCWEUrl() << R"('>)" << err.GetCWEString() << R"(</a></td>)" << '\n';
      }
      else
      {
        m_ostream << R"(      <th style="width: )" << GetCweColumnWidth() << R"(%;"></th>)" << '\n';
      }
    }

    if (showSast)
    {
      if (err.HasSAST())
      {
        m_ostream << R"(      <td style='width: )" << GetSASTColumnWidth() << R"(%;'>)"
                  << err.sastId << R"(</td>)" << '\n';
      }
      else
      {
        m_ostream << R"(      <th style="width: )" << GetSASTColumnWidth() << R"(%;"></th>)" << '\n';
      }
    }

    m_ostream << R"(      <td style='width: )" << GetMessageColumnWidth() << R"(%;'>)" << EscapeHtml(err.message) << R"(</td>)" << '\n';
    m_ostream << R"(    </tr>)" << std::endl;
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
  PrintMessages(m_autosar, "AUTOSAR");
  PrintMessages(m_owasp, "OWASP");
}

void SimpleHTMLOutput::Start()
{
  PrintHtmlStart();
}

bool SimpleHTMLOutput::Write(const Warning& msg)
{
  switch (msg.GetType())
  {
    case (AnalyzerType::General):
    {
      m_ga.push_back(msg);
      break;
    }
    case (AnalyzerType::Optimization):
    {
      m_op.push_back(msg);
      break;
    }
    case (AnalyzerType::Viva64):
    {
      m_64.push_back(msg);
      break;
    }
    case (AnalyzerType::CustomerSpecific):
    {
      m_cs.push_back(msg);
      break;
    }
    case (AnalyzerType::Misra):
    {
      m_misra.push_back(msg);
      break;
    }
    case (AnalyzerType::Autosar):
    {
      m_autosar.push_back(msg);
      break;
    }
    case (AnalyzerType::Owasp):
    {
      m_owasp.push_back(msg);
      break;
    }
    default:
    {
      m_info.push_back(msg);
    }
  }

  return true;
}

void SimpleHTMLOutput::Finish()
{
  if (   m_ga.empty() && m_op.empty() && m_64.empty() && m_cs.empty()
      && m_misra.empty() && m_info.empty() && m_owasp.empty() && m_autosar.empty())
  {
    m_ostream << "No messages generated" << std::endl;
  }
  else
  {
    PrintTableCaption();
    PrintTableBody();
  }

  PrintHtmlEnd();
  BasicFormatOutput<SimpleHTMLOutput>::Finish();
}

}
