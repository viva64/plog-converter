//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2021 (c) PVS-Studio LLC

#include "misracomplianceoutput.h"
#include <resources.h>
#include <algorithm>
#include "../utils.h"

namespace PlogConverter
{

static char HtmlHead[] = R"(
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
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

static char HtmlEnd[] = R"(
  </table>
</body>
</html>
)";

MisraComplianceOutput::MisraComplianceOutput(const ProgramOptions& opt)
  : m_directory { opt.output }
  , m_grpFile{ opt.grp }
  , m_customDiviations { opt.misraDivations }
{ 
  if (m_directory.empty())
  {
    throw std::logic_error("No output directory for html report");
  }

  if (!m_grpFile.empty() && !Exists(m_grpFile))
  {
    throw std::runtime_error("File not found: " + m_grpFile);
  }

  if (m_directory.back() == '/' || m_directory.back() == '\\')
  {
    m_directory.pop_back();
  }

  if (Exists(m_directory))
  {
    m_directory += "/misracompliance";
  }

  if (Exists(m_directory))
  {
    throw std::runtime_error("Output directory already exists: " + m_directory);
  }

  if (!MakeDirectory(m_directory))
  {
    throw std::runtime_error("Couldn't create directory for HTML report: " + m_directory);
  }

  m_ofstream.open(m_directory + "/misracompliance.html");
  if (!m_ofstream.is_open())
  {
    throw FilesystemException("Couldn't open " + m_directory + "/misracompliance.html");
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

  std::string misraCCode = GetMisraCCode(msg.sastId);
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
  PrintFileExtra("logomisra.png", PlogConverter::Resources::LogoMisra(), std::ios_base::binary);
}

void MisraComplianceOutput::PrintHtmlStart()
{
  m_ofstream << HtmlHead << std::endl;
}

void MisraComplianceOutput::PrintHtmlEnd()
{
  m_ofstream << HtmlEnd << std::endl;
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

      const auto& guideline = tokens.front();
      const auto& categoryLine = *std::next(tokens.begin());

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

  m_ofstream << R"(  <table style="width: 100%; font: 14pt normal Century Gothic;">)" << std::endl;
  m_ofstream << R"(    <caption style="font-weight: bold;background: #fff;color: #000;border: none !important;"></caption>)" << std::endl;
  m_ofstream << R"(    <tr style="background: #454545; color: white;">)" << std::endl;
  m_ofstream << R"(      <th style="width: 25%;">Guideline</th>)" << std::endl;
  m_ofstream << R"(      <th style="width: 25%;">Category</th>)" << std::endl;
  m_ofstream << R"(      <th style="width: 25%;">Recategorication</th>)" << std::endl;
  m_ofstream << R"(      <th style="width: 25%;">Compliance</th>)" << std::endl;
  m_ofstream << R"(    </tr>)" << std::endl;

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

  m_ofstream << R"(<p><img src="logoMisra.png" width="115" height="111" class="leftimg"/>
  <h2>MISRA Guideline Compliance Summary</h2>
  <p style="font: 13pt normal Century Gothic;">Guidelines: <b>MISRA C 2012</b></p>
  <p style="font: 13pt normal Century Gothic;">Checking tool: <b>PVS-Studio</b></p></p>)" << std::endl;
  m_ofstream << R"(<hr align="left" width="545" size="1" color="#999999" />)" << std::endl;

  if (resultCompliant)
  {
    m_ofstream << R"( <p style="font: 13pt normal Century Gothic;">Result: <b style='color:green !important;'>Compliant</b></p>)" << std::endl;
  }
  else
  {
    m_ofstream << R"( <p style="font: 13pt normal Century Gothic;">Result: <b style='color:red !important;'>Not compliant</b></p>)" << std::endl;
  }

  m_ofstream << R"( <p style="font: 13pt normal Century Gothic;">Summary: )" + summary + R"(</p>)" << std::endl;
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
    m_ofstream << R"(    <tr style='background: white;'>)" << std::endl;
  }
  else
  {
    m_ofstream << R"(    <tr style='background: #F4F4F4;'>)" << std::endl;
  }
  colorFlipFlop = !colorFlipFlop;

  m_ofstream << R"(      <td colspan='0' style='color: black; text-align: center; font-size: 1.0em;'>)" << cd.guideline << R"(</td>)" << std::endl;
  m_ofstream << R"(      <td colspan='1' style='color: black; text-align: center; font-size: 1.0em;'>)" << ToString(cd.defaultCategory) << R"(</td>)" << std::endl;
  m_ofstream << R"(      <td colspan='1' style='color: black; text-align: center; font-size: 1.0em;'>)" << ToString(cd.recategorization) << R"(</td>)" << std::endl;

  std::string bgcolor = "";
  std::string compliance = ToString(cd.compliance, cd.deviationsCount, cd.violationsCount);
  if (   ((cd.recategorization == Category::Mandatory || cd.defaultCategory == Category::Mandatory) && cd.violationsCount > 0)
      || ((cd.recategorization == Category::Required  || cd.defaultCategory == Category::Required)  && cd.violationsCount > 0)
      || ((cd.recategorization == Category::Mandatory || cd.defaultCategory == Category::Mandatory) && cd.deviationsCount > 0))
  {
    bgcolor = "bgcolor=\"#FADBD8\"";
  }

  m_ofstream << R"(      <td colspan='2' )" + bgcolor + R"( style='color: black; text-align: center; font-size: 1.0em;'>)" << compliance << R"(</td>)" << std::endl;
  m_ofstream << R"(    </tr>)" << std::endl;
}

Category MisraComplianceOutput::ToCategory(const std::string& category)
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
      return "Mandatory";
    
    case (Category::Required):
      return "Required";

    case (Category::Advisory):
      return "Advisory";
 
    case (Category::Disapplied):
      return "Disapplied";

    default:
      return "";
  }
}

std::string MisraComplianceOutput::ToString(Compliance compliance, int deviationsCount, int violationsCount)
{
  switch (compliance)
  {
  case (Compliance::Compliant):
    return "Compliant";

  case (Compliance::Deviations):
    return "Deviations (" + std::to_string(deviationsCount) + ")";

  case (Compliance::Violations):
    return "Violations (" + std::to_string(violationsCount) + ")";

  case (Compliance::ViolationsDeviations):
    return "Violations (" + std::to_string(violationsCount) + "), "
         + "Deviations (" + std::to_string(violationsCount) + ")";

  case (Compliance::Disapplied):
    return "Disapplied";

  case (Compliance::NotSupported):
    return "Not Supported";

  default:
    return "";
  }
}

void MisraComplianceOutput::PrintFileExtra(const std::string& fileName, const std::string& data, std::ios_base::openmode mode)
{
  std::ofstream file(m_directory + '/' + fileName, mode);
  file.write(data.c_str(), data.length());
}

MisraComplianceOutput::CategoriesMap &MisraComplianceOutput::Categories()
{
  static MisraComplianceOutput::CategoriesMap misra_c =
  {
    { "Rule 1.1", { Category::Required, Compliance::NotSupported } },
    { "Rule 1.2", { Category::Advisory, Compliance::NotSupported } },
    { "Rule 1.3", { Category::Required, Compliance::NotSupported } },

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
    { "Rule 5.2", { Category::Required, Compliance::NotSupported } },
    { "Rule 5.3", Category::Required },
    { "Rule 5.4", { Category::Required, Compliance::NotSupported } },
    { "Rule 5.5", { Category::Required, Compliance::NotSupported } },
    { "Rule 5.6", { Category::Required, Compliance::NotSupported } },
    { "Rule 5.7", { Category::Required, Compliance::NotSupported } },
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
    { "Rule 8.8", { Category::Required, Compliance::NotSupported } },
    { "Rule 8.9", { Category::Advisory, Compliance::NotSupported } },
    { "Rule 8.10", { Category::Required, Compliance::NotSupported } },
    { "Rule 8.11", Category::Advisory },
    { "Rule 8.12", Category::Required },
    { "Rule 8.13", { Category::Advisory, Compliance::NotSupported } },
    { "Rule 8.14", Category::Required },

    { "Rule 9.1", Category::Mandatory },
    { "Rule 9.2", { Category::Required, Compliance::NotSupported } },
    { "Rule 9.3", Category::Required },
    { "Rule 9.4", { Category::Required, Compliance::NotSupported } },
    { "Rule 9.5", Category::Required },

    { "Rule 10.1", Category::Required },
    { "Rule 10.2", Category::Required },
    { "Rule 10.3", Category::Required },
    { "Rule 10.4", Category::Required },
    { "Rule 10.5", Category::Advisory },
    { "Rule 10.6", Category::Required },
    { "Rule 10.7", { Category::Required, Compliance::NotSupported } },
    { "Rule 10.8", { Category::Required, Compliance::NotSupported } },

    { "Rule 11.1", Category::Required },
    { "Rule 11.2", { Category::Required, Compliance::NotSupported } },
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

    { "Rule 17.1", { Category::Required, Compliance::NotSupported } },
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
    { "Rule 20.2", { Category::Required, Compliance::NotSupported } },
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
    { "Rule 21.11", { Category::Required, Compliance::NotSupported } },
    { "Rule 21.12", { Category::Advisory, Compliance::NotSupported } },

    { "Rule 22.1", ComplianceData(Category::Required)  },
    { "Rule 22.2", Category::Mandatory },
    { "Rule 22.3", { Category::Required, Compliance::NotSupported } },
    { "Rule 22.4", ComplianceData(Category::Mandatory, Compliance::NotSupported) },
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
