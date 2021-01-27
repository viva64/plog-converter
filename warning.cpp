// 2006-2008 (c) Viva64.com Team
// 2008-2020 (c) OOO "Program Verification Systems"
// 2020 (c) PVS-Studio LLC

#include "warning.h"

namespace PlogConverter
{

AnalyzerType Warning::GetType() const
{
  const auto errorCode = GetErrorCode();
  if ((errorCode >= 100 && errorCode <= 499) || errorCode == 4)
  {
    return AnalyzerType::Viva64;
  }
  else if (errorCode >= 1 && errorCode <= 99)
  {
    return AnalyzerType::Fail;
  }
  else if (errorCode >= 500 && errorCode <= 799)
  {
    return AnalyzerType::General;
  }
  else if (errorCode >= 800 && errorCode <= 999)
  {
    return AnalyzerType::Optimization;
  }
  else if (errorCode >= 1000 && errorCode <= 1999)
  {
    return AnalyzerType::General;
  }
  else if (errorCode >= 2000 && errorCode <= 2499)
  {
    return AnalyzerType::CustomerSpecific;
  }
  else if (errorCode >= 2500 && errorCode <= 2999)
  {
    return AnalyzerType::Misra;
  }
  else if (errorCode >= 3000 && errorCode <= 3499)
  {
    return AnalyzerType::General; // C#
  }
  else if (errorCode >= 3500 && errorCode <= 3999)
  {
    return AnalyzerType::Autosar;
  }
  else if (errorCode >= 5000 && errorCode <= 5299)
  {
    return AnalyzerType::Owasp;
  }
  else if (errorCode >= 5300 && errorCode <= 5599)
  {
    return AnalyzerType::Owasp; // Java
  }
  else if (errorCode >= 5600 && errorCode <= 5999)
  {
    return AnalyzerType::Owasp; // C#
  }
  else if (errorCode >= 6000 && errorCode <= 6999)
  {
    return AnalyzerType::General; // Java
  }
  else if (IsExternalMessage())
  {
    return AnalyzerType::General; // External tools
  }

  return AnalyzerType::Unknown;
}

Warning Warning::GetDocumentationLinkMessage()
{
  Warning docsMessage;
  docsMessage.code = "Help:";
  docsMessage.level = 1;
  docsMessage.positions = { {"www.viva64.com/en/w", 1 } };
  docsMessage.message = "The documentation for all analyzer warnings is available here: https://www.viva64.com/en/w/.";
  return docsMessage;
}

bool Warning::IsDocumentationLinkMessage() const
{
  return code == "Help:";
}

bool Warning::IsRenewMessage() const
{
  return code == "Renew";
}

bool Warning::IsExternalMessage() const
{
  return code == "External";
}

bool Warning::IsUpdateMessage() const
{
  return code == "Update";
}

bool Warning::IsTrialMessage() const
{
  return code == "Trial";
}

unsigned Warning::GetErrorCode() const
{
  if (   code.empty()
      || code.front() != 'V'
      || !std::all_of(code.begin() + 1, code.end(), [](char c) { return isdigit(c); }))
  {
    return 0;
  }

  return static_cast<unsigned>(atoi(code.c_str() + 1));
}

std::string Warning::GetVivaUrl() const
{
  if (IsRenewMessage())
  {
    return "https://www.viva64.com/en/renewal/";
  }

  if (IsExternalMessage())
  {
    return "https://www.viva64.com/en/w/";
  }

  if (IsUpdateMessage())
  {
    return "https://www.viva64.com/en/pvs-studio-download/";
  }

  if (IsTrialMessage())
  {
    return "https://www.viva64.com/en/pvs-studio-download/#trial_form";
  }

  const auto errorCode = GetErrorCode();
  if (errorCode != 0)
  {
    return "https://www.viva64.com/en/w/v" + LeftPad(std::to_string(errorCode), 3, '0') + '/';
  }

  return {};
}

std::string Warning::GetCWEUrl() const
{
  if (!HasCWE())
  {
    return {};
  }

  return "https://cwe.mitre.org/data/definitions/" + std::to_string(cwe) + ".html";
}

std::string Warning::GetCWEString() const
{
  return cwe == 0 ? "" : CWEPrefix + std::to_string(cwe);
}

std::string Warning::GetMISRAString() const
{
  return std::string(Warning::MISRACorePrefix) + sastId;
}

std::string Warning::GetMISRAStringWithLanguagePrefix() const
{
  if (sastId.find('-') != std::string::npos)
    return MISRAPrefixCPlusPlus + sastId;
  else if (sastId.find('.') != std::string::npos)
    return MISRAPrefixC + sastId;
  else
    return "";
}

bool Warning::HasCWE() const
{
  return cwe != 0;
}

bool Warning::HasSAST() const
{
  return !sastId.empty();
}

bool Warning::HasProjects() const
{
  return !projects.empty();
}

std::string Warning::GetLevelString() const
{
  return GetLevelString("error", "warning", "note");
}

std::string Warning::GetLevelString(const std::string &l01, const std::string &l2, const std::string &l3) const
{
  return GetLevelString(l01, l01, l2, l3);
}

std::string Warning::GetLevelString(const std::string &l0, const std::string &l1, const std::string &l2, const std::string &l3) const
{
  switch (level)
  {
  case 3:   return l3;
  case 2:   return l2;
  case 1:   return l1;
  default:  return l0;
  }
}

void Warning::Clear()
{
  //TODO:
  *this = {};
}

std::vector<unsigned> Warning::GetExtendedLines() const
{
  if (!additionalLines.empty())
  {
    return additionalLines;
  }

  std::vector<unsigned> res;

  if (positions.size() <= 1)
  {
    return res;
  }

  for (const auto &pos : positions)
  {
    if (pos.file == positions.front().file)
    {
      res.push_back(pos.line);
    }
  }

  return res;
}

const NavigationInfo &Warning::GetNavigationInfo() const
{
  static const NavigationInfo empty;
  return positions.empty() ? empty : positions.front().navigation;
}

const std::string &Warning::GetFile() const
{
  static const std::string empty;
  return positions.empty() ? empty : positions.front().file;
}

unsigned Warning::GetLine() const
{
  return positions.empty() ? 0 : positions.front().line;
}

unsigned Warning::GetEndLine() const
{
  return positions.empty() ? 0 : positions.front().endLine;
}

unsigned Warning::GetStartColumn() const
{
  return positions.empty() ? 0 : positions.front().column;
}

unsigned Warning::GetEndColumn() const
{
  return positions.empty() ? 0 : positions.front().endColumn;
}

}
