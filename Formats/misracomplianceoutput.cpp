//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2023 (c) PVS-Studio LLC

#include <algorithm>
#include <resources.h>

#include "misracomplianceoutput.h"
#include "../utils.h"

namespace PlogConverter
{

static constexpr std::string_view MisraHtmlHead =
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

    .leftimg {
    float:left;
    margin: 0 15px 0 0;
    }

    legend {
      color: blue;
      font: 1.2em bold Comic Sans MS Verdana;
      text-align: center;
    }
  </style>
</head>
<body>
)";

static constexpr std::string_view MisraHtmlEnd =
R"(  </table>
</body>
</html>)";

MisraComplianceOutput::MisraComplianceOutput(const ProgramOptions& opt)
  : BasicFormatOutput(opt)
  , m_grpFile{ opt.grp }
  , m_customDiviations{ opt.misraDivations }
{
  if (!m_grpFile.empty() && !std::filesystem::exists(m_grpFile))
  {
    throw std::runtime_error("File not found: " + m_grpFile.string());
  }
}

void MisraComplianceOutput::Start()
{
  for (auto& complianceData : Categories())
  {
    complianceData.second.guideline = complianceData.first;
  }

  RecategoriesByGRP();
}

bool MisraComplianceOutput::Write(const Warning& msg)
{
  if (msg.GetType() != AnalyzerType::Misra)
  {
    return false;
  }

  auto misraCCode = GetMisraCCode(msg.sastId);
  if (misraCCode.empty())
  {
    return false;
  }

  auto& m_misra_c = Categories();
  std::string code = "Rule " + misraCCode;
  if (auto it = m_misra_c.find(code); it == m_misra_c.end())
  {
    std::cerr << "Unrecognized MISRA code: " << code << std::endl;
  }
  else
  {
    auto isInExceptions = std::find(m_customDiviations.begin(), m_customDiviations.end(), code) != m_customDiviations.end();

    if (msg.falseAlarm || isInExceptions)
    {
      it->second.deviationsCount++;
    }
    else
    {
      it->second.violationsCount++;
    }
  }

  return true;
}

void MisraComplianceOutput::Finish()
{
  for (auto& complianceData : Categories())
  {
    SetComplianceContent(complianceData.second);
  }

  PrintHtmlStart();
  PrintHtmpComplianceReport();
  PrintHtmlEnd();

  m_ostream.flush();
}

void MisraComplianceOutput::PrintHtmlStart()
{
  m_ostream << MisraHtmlHead;
}

void MisraComplianceOutput::PrintHtmlEnd()
{
  m_ostream << MisraHtmlEnd;
}

namespace
{
  bool HasOnlySpaces(std::string_view line)
  {
    return line.find_first_not_of(" \t\v\f") == std::string::npos;
  }

  std::string CreateReservedString(std::string_view prefix, size_t size)
  {
    if (size < prefix.size())
    {
      return {};
    }

    std::string result;
    result.reserve(size);
    result += std::string(prefix);

    return result;
  }

  constexpr size_t MaxLineNumberLenght = 256;

  void DumpError(const std::string& message, size_t lineNumber)
  {
    static constexpr std::string_view lineNumberPrefix{ "In line number " };
    static auto errorPrefix = CreateReservedString(lineNumberPrefix, lineNumberPrefix.size() + message.size() + MaxLineNumberLenght);
    errorPrefix += std::to_string(lineNumber);
    errorPrefix += " - ";
    errorPrefix += message;
    throw std::runtime_error(errorPrefix);
  }
}

void MisraComplianceOutput::RecategoriesByGRP()
{
  std::ifstream in(m_grpFile);
  if (in.is_open())
  {
    std::string line;
    auto& m_misra_c = Categories();

    for (size_t lineNumber = 1; getline(in, line); ++lineNumber)
    {
      if (HasOnlySpaces(line))
      {
        continue;
      }

      auto tokens = Split(line, "=");

      if (tokens.size() != 2)
      {
        DumpError("Incorrect GRP line: \"" + line + "\". Expected \"Rule <ID> = <Category>\".", lineNumber);
      }

      auto& guideline = tokens.front();
      auto& categoryLine = *std::next(tokens.begin());

      auto category = ToCategory(categoryLine);

      if (category == Category::None)
      {
        DumpError("Unknown GRP category: " + categoryLine, lineNumber);
      }

      auto it = m_misra_c.find(guideline);
      if (it == m_misra_c.end())
      {
        DumpError("Unknown GRP guideline: "+ guideline + ". Expected \"Rule <Number>.<Number>\"", lineNumber);
      }

      auto& element = it->second;
      if (category < element.defaultCategory && element.defaultCategory != Category::Advisory)
      {
        DumpError("You cannot downgrade the guideline from " + ToString(element.defaultCategory) + " to " + ToString(category) + " for " + element.guideline, lineNumber);
      }

      element.recategorization = category;
    }

    in.close();
  }
}

std::string MisraComplianceOutput::GetMisraCCode(const std::string& sastId)
{
  if (sastId.find(misraPrefix) != 0)
  {
    return {};
  }

  return sastId.substr(misraPrefix.length(), sastId.length());
}

void MisraComplianceOutput::PrintHtmpComplianceReport()
{
  PrintHtmlComplianceHeader();

  m_ostream << R"(  <table style="width: 100%; font: 14pt normal Century Gothic;">)" << '\n'
            << R"(    <caption style="font-weight: bold;background: #fff;color: #000;border: none !important;"></caption>)" << '\n'
            << R"(    <tr style="background: #454545; color: white;">)" << '\n'
            << R"(      <th style="width: 25%;">Guideline</th>)" << '\n'
            << R"(      <th style="width: 25%;">Category</th>)" << '\n'
            << R"(      <th style="width: 25%;">Recategorication</th>)" << '\n'
            << R"(      <th style="width: 25%;">Compliance</th>)" << '\n'
            << R"(    </tr>)" << std::endl;

  bool colorFlipFlop = true;
  for (const auto& [_, cd] : Categories())
  {
    PrintTableRow(cd, colorFlipFlop);
    colorFlipFlop = !colorFlipFlop;
  }
}

void MisraComplianceOutput::PrintHtmlComplianceHeader()
{
  auto&& [resultCompliant, summary] = GetComplianceResult();

  m_ostream
  << "<p><img src=\"data:image/png;base64,"
  << "iVBORw0KGgoAAAANSUhEUgAAAPUAAAD0CAMAAAB0M5DyAAAABGdBTUEAALGPC/xhBQAAAAFzUkdCAK7OHOkAAAAJcEhZcwAAFxIA"
     "ABcSAWef0lIAAAMAUExURXkAGXkAGXgAGHkAGXkAGXkAGXkAGUxpcXoAGnwAGXgAGXkAGXkAGXoAGXkAGXkAGXgAGHkAGXkAG3sA"
     "GXoAGXcAF1UAAHoAGXkAGXkAGHkAGnoAGXgAGHkAGXoAGXkAGngAIngAGXgAGHgAGXkAGXkAGHkAGHkAGXkAGnkAGXwAG3YAGXoA"
     "GnoAGnkAGIAMAIAAI3oAGH0AGXgAFnkAGAAAG3gAGXsAAHkAGWYAG3kAGHYAGXkAG3gAG3gAGf8AUXoAGHkAGXkAGXkAGnkAF3oA"
     "GXkAGXkAGXkAGXkAGXgAGHkAGXgAGHkAGXkAGHkAGHkAGXgAJHgAGHkAGFUADHkAGXkAGXkAG3kAGXgAFnkAGHoAGXoAGngAGXkA"
     "GXcAEXoAGHkAGXkAGXgAGXsAG3kAGHkAGnkAGXgAF3gAGIUAHngAGHkAG3kAGXkAGXsAGnkAGnkAGHkAGHkAGXkAGHoAGHkAGXkA"
     "GnkAGYgAQHkAGXgAF3kAGXkAF3cAFXkAGnkAGXkAGIAAHXEAD3gAGHkAGXkAGXgAGHkAGHwAHHoAGXgAGpkALXkAGIAAgHoAGXkA"
     "GXgAGXkAGXkAGnkAGHkAGHkAGXkAGHkAGHkAGHkAGXoAGXkAGHkAGXkAGHkAGXkAGnkAGXkAGHkAGKoAPXkAGXgAGHgAGXkAGXgA"
     "GHgAF3kAGHgAGHgAGHkAGHkAGXkAGHkAGXkAGG0AEnkAGnkAGXkAGXoAGXoAGXwAGH0AHXkAGnkAGXgAGHgAGHkAG3kAGXoAGHoA"
     "GXYAGnkAGHoAGHkAGXgAGHoAGnoAGnkAGXoAGXkAF3gAGHgAGHkAGXkAGXkAGnkAGXgAGHcAGHkAGXgAGHwAF3kAGXkAGngAGXkA"
     "GHkAGXoAGXgAGXoAGnUAGXgAGHkAGncAF3kAGHkAGXgAGXkAGX0AG3gAGHkAGHoAGm0AAHEAEngAG3gAGHkAGY4AJnkAGHoAGXoA"
     "GnsAGnkAGXkAGXUAGHkAGHkAGngAGXkAG3kAGngAGXgAGNYk0uQAAAEAdFJOU+rs8PTd8/IA7ynr7vHr6e228Hg+n2UG8e/y7/Py"
     "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"
  << R"(" width="115" height="111" class="leftimg"/>
  <h2>MISRA Guideline Compliance Summary</h2>
  <p style="font: 13pt normal Century Gothic;">Guidelines: <b>MISRA C 2012</b></p>
  <p style="font: 13pt normal Century Gothic;">Checking tool: <b>PVS-Studio</b></p></p>)" << '\n'
  << R"(<hr align="left" width="545" size="1" color="#999999" />)" << '\n';

  if (resultCompliant)
  {
    m_ostream << R"( <p style="font: 13pt normal Century Gothic;">Result: <b style='color:green !important;'>Compliant</b></p>)" << '\n';
  }
  else
  {
    m_ostream << R"( <p style="font: 13pt normal Century Gothic;">Result: <b style='color:red !important;'>Not compliant</b></p>)" << '\n';
  }

  m_ostream << R"( <p style="font: 13pt normal Century Gothic;">Summary: )" + summary + R"(</p>)" << std::endl;
}

std::pair<bool, std::string> MisraComplianceOutput::GetComplianceResult()
{
  int mandatoryViolationsCount = 0;
  int mandatoryDeviationsCount = 0;

  int requiredViolationsCount = 0;
  int requiredDeviationsCount = 0;

  int advisoryViolationsCount = 0;
  int advisoryDeviationsCount = 0;

  for (const auto& [_, cd] : Categories())
  {
    bool isMandatory = cd.recategorization == Category::Mandatory || (cd.recategorization == Category::None && cd.defaultCategory == Category::Mandatory);
    bool isRequired  = cd.recategorization == Category::Required  || (cd.recategorization == Category::None && cd.defaultCategory == Category::Required);
    bool isAdvisory  = cd.recategorization == Category::Advisory  || (cd.recategorization == Category::None && cd.defaultCategory == Category::Advisory);

    if (isMandatory)
    {
      if (cd.compliance == Compliance::Violations)
      {
        ++mandatoryViolationsCount;
      }
      else if (cd.compliance == Compliance::Deviations)
      {
        ++mandatoryDeviationsCount;
      }
      else if (cd.compliance == Compliance::ViolationsDeviations)
      {
        ++mandatoryViolationsCount;
        ++mandatoryDeviationsCount;
      }
    }
    else if (isRequired)
    {
      if (cd.compliance == Compliance::Violations)
      {
        ++requiredViolationsCount;
      }
      else if (cd.compliance == Compliance::Deviations)
      {
        ++requiredDeviationsCount;
      }
      else if (cd.compliance == Compliance::ViolationsDeviations)
      {
        ++requiredViolationsCount;
        ++requiredDeviationsCount;
      }
    }
    else if (isAdvisory)
    {
      if (cd.compliance == Compliance::Violations)
      {
        ++advisoryViolationsCount;
      }
      else if (cd.compliance == Compliance::Deviations)
      {
        ++advisoryDeviationsCount;
      }
      else if (cd.compliance == Compliance::ViolationsDeviations)
      {
        ++advisoryViolationsCount;
        ++advisoryDeviationsCount;
      }
    }
  }

  std::string summary;
  if (   mandatoryViolationsCount == 0 && mandatoryDeviationsCount == 0
      && requiredViolationsCount  == 0 && requiredDeviationsCount  == 0
      && advisoryViolationsCount  == 0 && advisoryDeviationsCount  == 0)
  {
    summary = "No deviations or violations were found in your project.";
  }
  else
  {
    summary = GetSummaryGuidelines(Compliance::Violations, mandatoryViolationsCount, requiredViolationsCount, advisoryViolationsCount);
    summary += GetSummaryGuidelines(Compliance::Deviations, mandatoryDeviationsCount, requiredDeviationsCount, advisoryDeviationsCount);
  }

  if (   mandatoryViolationsCount == 0
      && mandatoryDeviationsCount == 0
      && requiredViolationsCount == 0)
  {
    
    return { true, summary };
  }
  else
  {
   
    return { false, summary };
  }
}

std::string MisraComplianceOutput::GetSummaryGuidelines(Compliance compliance, int mandatoryCount, int requiredCount, int advisoryCount)
{
  if (mandatoryCount == 0 && requiredCount == 0 && advisoryCount == 0)
  {
    return {};
  }

  std::vector<std::string> content;
  content.reserve(3);

  if (mandatoryCount > 0)
  {
    auto str = std::to_string(mandatoryCount) + " mandatory guideline" + (mandatoryCount > 1 ? "s" : "");
    content.push_back(std::move(str));
  }

  if (requiredCount > 0)
  {
    auto str = std::to_string(requiredCount) + " required guideline" + (requiredCount > 1 ? "s" : "");
    content.push_back(std::move(str));
  }

  if (advisoryCount > 0)
  {
    auto str = std::to_string(advisoryCount) + " advisory guideline" + (advisoryCount > 1 ? "s" : "");
    content.push_back(std::move(str));
  }

  std::string summary;

  if (compliance == Compliance::Violations)
  {
    summary = "There were violations of ";
  }
  else if (compliance == Compliance::Deviations)
  {
    summary = "There were deviations of ";
  }

  for (size_t i = 0; i < content.size(); ++i)
  {
    summary += content[i];
    summary += (i == content.size() - 1 ? ".\n" : ", ");
  }

  return summary;
}

void MisraComplianceOutput::SetComplianceContent(ComplianceData &cd)
{
  if (cd.compliance == Compliance::NotSupported)
  {
    return;
  }

  if (cd.recategorization == Category::Disapplied)
  {
    cd.compliance = Compliance::Disapplied;
    return;
  }

  if (cd.deviationsCount == 0 && cd.violationsCount == 0)
  {
    cd.compliance = Compliance::Compliant;
    return;
  }

  if (cd.deviationsCount > 0 && cd.violationsCount > 0)
  {
    cd.compliance = Compliance::ViolationsDeviations;
    return;
  }

  if (cd.deviationsCount > 0)
  {
    cd.compliance = Compliance::Deviations;
    return;
  }

  if (cd.violationsCount > 0)
  {
    cd.compliance = Compliance::Violations;
    return;
  }
}

void MisraComplianceOutput::PrintTableRow(const ComplianceData& cd, bool colorFlipFlop)
{
  if (colorFlipFlop)
  {
    m_ostream << R"(    <tr style='background: white;'>)" << '\n';
  }
  else
  {
    m_ostream << R"(    <tr style='background: #F4F4F4;'>)" << '\n';
  }
  colorFlipFlop = !colorFlipFlop;

  m_ostream << R"(      <td colspan='0' style='color: black; text-align: center; font-size: 1.0em;'>)" << cd.guideline << R"(</td>)" << '\n'
            << R"(      <td colspan='1' style='color: black; text-align: center; font-size: 1.0em;'>)" << ToString(cd.defaultCategory) << R"(</td>)" << '\n'
            << R"(      <td colspan='1' style='color: black; text-align: center; font-size: 1.0em;'>)" << ToString(cd.recategorization) << R"(</td>)" << '\n';

  std::string bgcolor = "";
  std::string compliance = ToString(cd.compliance, cd.deviationsCount, cd.violationsCount);
  if (   ((cd.recategorization == Category::Mandatory || cd.defaultCategory == Category::Mandatory) && cd.violationsCount > 0)
      || ((cd.recategorization == Category::Required  || cd.defaultCategory == Category::Required)  && cd.violationsCount > 0)
      || ((cd.recategorization == Category::Mandatory || cd.defaultCategory == Category::Mandatory) && cd.deviationsCount > 0))
  {
    bgcolor = "bgcolor=\"#FADBD8\"";
  }

  m_ostream << R"(      <td colspan='2' )" + bgcolor + R"( style='color: black; text-align: center; font-size: 1.0em;'>)" << compliance << R"(</td>)" << '\n'
            << R"(    </tr>)" << std::endl;
}

Category MisraComplianceOutput::ToCategory(std::string_view category)
{
  auto lower = ToLower(category);
  if (lower == "mandatory")
  {
    return Category::Mandatory;
  }
  else if (lower == "required")
  {
    return Category::Required;
  }
  else if (lower == "advisory")
  {
    return Category::Advisory;
  }
  else if (lower == "disapplied")
  {
    return Category::Disapplied;
  }
  else
  {
    return Category::None;
  }
}

std::string MisraComplianceOutput::ToString(Category category)
{
  switch (category)
  {
    case (Category::Mandatory):
    {
      return "Mandatory";
    }
    case (Category::Required):
    {
      return "Required";
    }
    case (Category::Advisory):
    {
      return "Advisory";
    }
    case (Category::Disapplied):
    {
      return "Disapplied";
    }
    default:
    {
      return "";
    }
  }
}

std::string MisraComplianceOutput::ToString(Compliance compliance, int deviationsCount, int violationsCount)
{
  switch (compliance)
  {
    case (Compliance::Compliant):
    {
      return "Compliant";
    }
    case (Compliance::Deviations):
    {
      return "Deviations (" + std::to_string(deviationsCount) + ")";
    }
    case (Compliance::Violations):
    {
      return "Violations (" + std::to_string(violationsCount) + ")";
    }
    case (Compliance::ViolationsDeviations):
    {
      return "Violations (" + std::to_string(violationsCount) + "), "
           + "Deviations (" + std::to_string(violationsCount) + ")";
    }
    case (Compliance::Disapplied):
    {
      return "Disapplied";
    }
    case (Compliance::NotSupported):
    {
      return "Not Supported";
    }
    default:
    {
      return "";
    }
  }
}

MisraComplianceOutput::CategoriesMap &MisraComplianceOutput::Categories()
{
  static MisraComplianceOutput::CategoriesMap misra_c =
  {
    { "Rule 1.1", { Category::Required, Compliance::NotSupported } },
    { "Rule 1.2", { Category::Advisory, Compliance::NotSupported } },
    { "Rule 1.3", Category::Required },

    { "Rule 2.1", Category::Required },
    { "Rule 2.2", { Category::Required, Compliance::NotSupported } },
    { "Rule 2.3", { Category::Advisory, Compliance::NotSupported } },
    { "Rule 2.4", { Category::Advisory, Compliance::NotSupported } },
    { "Rule 2.5", { Category::Advisory, Compliance::NotSupported } },
    { "Rule 2.6", Category::Advisory },
    { "Rule 2.7", Category::Advisory },

    { "Rule 3.1", Category::Required },
    { "Rule 3.2", Category::Required },

    { "Rule 4.1", Category::Required },
    { "Rule 4.2", { Category::Advisory, Compliance::NotSupported } },

    { "Rule 5.1", { Category::Required, Compliance::NotSupported } },
    { "Rule 5.2", Category::Required },
    { "Rule 5.3", Category::Required },
    { "Rule 5.4", { Category::Required, Compliance::NotSupported } },
    { "Rule 5.5", { Category::Required, Compliance::NotSupported } },
    { "Rule 5.6", Category::Required },
    { "Rule 5.7", Category::Required },
    { "Rule 5.8", { Category::Required, Compliance::NotSupported } },
    { "Rule 5.9", { Category::Advisory, Compliance::NotSupported } },

    { "Rule 6.1", Category::Required },
    { "Rule 6.2", Category::Required },

    { "Rule 7.1", Category::Required },
    { "Rule 7.2", Category::Required },
    { "Rule 7.3", Category::Required },
    { "Rule 7.4", Category::Required },

    { "Rule 8.1", { Category::Required, Compliance::NotSupported } },
    { "Rule 8.2", Category::Required },
    { "Rule 8.3", { Category::Required, Compliance::NotSupported } },
    { "Rule 8.4", { Category::Required, Compliance::NotSupported } },
    { "Rule 8.5", { Category::Required, Compliance::NotSupported } },
    { "Rule 8.6", { Category::Required, Compliance::NotSupported } },
    { "Rule 8.7", { Category::Advisory, Compliance::NotSupported } },
    { "Rule 8.8", Category::Required },
    { "Rule 8.9", { Category::Advisory, Compliance::NotSupported } },
    { "Rule 8.10", Category::Required },
    { "Rule 8.11", Category::Advisory },
    { "Rule 8.12", Category::Required },
    { "Rule 8.13", { Category::Advisory, Compliance::NotSupported } },
    { "Rule 8.14", Category::Required },

    { "Rule 9.1", Category::Mandatory },
    { "Rule 9.2", { Category::Required, Compliance::NotSupported } },
    { "Rule 9.3", Category::Required },
    { "Rule 9.4", Category::Required },
    { "Rule 9.5", Category::Required },

    { "Rule 10.1", Category::Required },
    { "Rule 10.2", Category::Required },
    { "Rule 10.3", Category::Required },
    { "Rule 10.4", Category::Required },
    { "Rule 10.5", Category::Advisory },
    { "Rule 10.6", Category::Required },
    { "Rule 10.7", Category::Required },
    { "Rule 10.8", { Category::Required, Compliance::NotSupported } },

    { "Rule 11.1", Category::Required },
    { "Rule 11.2", Category::Required },
    { "Rule 11.3", Category::Required },
    { "Rule 11.4", Category::Advisory },
    { "Rule 11.5", Category::Advisory },
    { "Rule 11.6", Category::Required },
    { "Rule 11.7", Category::Required },
    { "Rule 11.8", Category::Required },
    { "Rule 11.9", { Category::Required, Compliance::NotSupported } },

    { "Rule 12.1", { Category::Advisory, Compliance::NotSupported } },
    { "Rule 12.2", Category::Required },
    { "Rule 12.3", Category::Advisory },
    { "Rule 12.4", Category::Advisory },

    { "Rule 13.1", { Category::Required, Compliance::NotSupported } },
    { "Rule 13.2", { Category::Required, Compliance::NotSupported } },
    { "Rule 13.3", Category::Advisory },
    { "Rule 13.4", Category::Advisory },
    { "Rule 13.5", { Category::Required, Compliance::NotSupported } },
    { "Rule 13.6", Category::Mandatory },

    { "Rule 14.1", Category::Required },
    { "Rule 14.2", { Category::Required, Compliance::NotSupported } },
    { "Rule 14.3", Category::Required },
    { "Rule 14.4", Category::Required },

    { "Rule 15.1", Category::Advisory },
    { "Rule 15.2", Category::Required },
    { "Rule 15.3", Category::Required },
    { "Rule 15.4", Category::Advisory },
    { "Rule 15.5", Category::Advisory },
    { "Rule 15.6", Category::Required },
    { "Rule 15.7", Category::Required },

    { "Rule 16.1", { Category::Required, Compliance::NotSupported } },
    { "Rule 16.2", Category::Required },
    { "Rule 16.3", Category::Required },
    { "Rule 16.4", Category::Required },
    { "Rule 16.5", Category::Required },
    { "Rule 16.6", Category::Required },
    { "Rule 16.7", Category::Required },

    { "Rule 17.1", Category::Required },
    { "Rule 17.2", Category::Required },
    { "Rule 17.3", Category::Mandatory },
    { "Rule 17.4", Category::Mandatory },
    { "Rule 17.5", Category::Advisory },
    { "Rule 17.6", Category::Mandatory },
    { "Rule 17.7", { Category::Required, Compliance::NotSupported } },
    { "Rule 17.8", { Category::Advisory, Compliance::NotSupported } },

    { "Rule 18.1", { Category::Required, Compliance::NotSupported } },
    { "Rule 18.2", { Category::Required, Compliance::NotSupported } },
    { "Rule 18.3", { Category::Required, Compliance::NotSupported } },
    { "Rule 18.4", Category::Advisory },
    { "Rule 18.5", { Category::Advisory, Compliance::NotSupported } },
    { "Rule 18.6", Category::Required },
    { "Rule 18.7", Category::Required },
    { "Rule 18.8", Category::Required },

    { "Rule 19.1", ComplianceData(Category::Mandatory, Compliance::NotSupported) },
    { "Rule 19.2", ComplianceData(Category::Advisory)  },

    { "Rule 20.1", { Category::Advisory, Compliance::NotSupported } },
    { "Rule 20.2", Category::Required },
    { "Rule 20.3", { Category::Required, Compliance::NotSupported } },
    { "Rule 20.4", Category::Required },
    { "Rule 20.5", { Category::Advisory, Compliance::NotSupported } },
    { "Rule 20.6", { Category::Required, Compliance::NotSupported } },
    { "Rule 20.7", Category::Required },
    { "Rule 20.8", { Category::Required, Compliance::NotSupported } },
    { "Rule 20.9", { Category::Required, Compliance::NotSupported } },
    { "Rule 20.10", { Category::Advisory, Compliance::NotSupported } },
    { "Rule 20.11", { Category::Required, Compliance::NotSupported } },
    { "Rule 20.12", { Category::Required, Compliance::NotSupported } },
    { "Rule 20.13", Category::Required },
    { "Rule 20.14", { Category::Required, Compliance::NotSupported } },

    { "Rule 21.1", Category::Required },
    { "Rule 21.2", Category::Required },
    { "Rule 21.3", Category::Required },
    { "Rule 21.4", Category::Required },
    { "Rule 21.5", Category::Required },
    { "Rule 21.6", Category::Required },
    { "Rule 21.7", Category::Required },
    { "Rule 21.8", Category::Required },
    { "Rule 21.9", Category::Required },
    { "Rule 21.10", Category::Required },
    { "Rule 21.11", Category::Required },
    { "Rule 21.12", { Category::Advisory, Compliance::NotSupported } },

    { "Rule 22.1", ComplianceData(Category::Required)  },
    { "Rule 22.2", Category::Mandatory },
    { "Rule 22.3", { Category::Required, Compliance::NotSupported } },
    { "Rule 22.4", ComplianceData(Category::Mandatory) },
    { "Rule 22.5", Category::Mandatory },
    { "Rule 22.6", Category::Mandatory },

    { "Directive 1.1", { Category::Required, Compliance::NotSupported } },
    { "Directive 2.1", { Category::Required, Compliance::NotSupported } },
    { "Directive 3.1", { Category::Required, Compliance::NotSupported } },
    { "Directive 4.1", { Category::Required, Compliance::NotSupported } },
    { "Directive 4.2", { Category::Advisory, Compliance::NotSupported } },
    { "Directive 4.3", { Category::Required, Compliance::NotSupported } },
    { "Directive 4.4", { Category::Advisory, Compliance::NotSupported } },
    { "Directive 4.5", { Category::Advisory, Compliance::NotSupported } },
    { "Directive 4.6", { Category::Advisory, Compliance::NotSupported } },
    { "Directive 4.7", { Category::Required, Compliance::NotSupported } },
    { "Directive 4.8", { Category::Advisory, Compliance::NotSupported } },
    { "Directive 4.9", { Category::Advisory, Compliance::NotSupported } },
    { "Directive 4.10", { Category::Required, Compliance::NotSupported } },
    { "Directive 4.11", { Category::Required, Compliance::NotSupported } },
    { "Directive 4.12", { Category::Required, Compliance::NotSupported } },
    { "Directive 4.13", { Category::Advisory, Compliance::NotSupported } }
  };

  return misra_c;
}

}
