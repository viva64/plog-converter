// 2006-2008 (c) Viva64.com Team
// 2008-2020 (c) OOO "Program Verification Systems"
// 2020-2021 (c) PVS-Studio LLC

#include "warning.h"
#include <JsonUtils.h>

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
  docsMessage.positions = { {"www.viva64.com/en/w", 1u } };
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
  return cwe == 0 ? std::string {} : CWEPrefix + std::to_string(cwe);
}

std::string Warning::GetSASTString() const
{
  return sastId;
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

static std::string ConvertToString(const std::vector<WarningPosition> &positions)
{
  using PathComparator = bool (*)(std::string_view, std::string_view);

#ifdef _WIN32
  constexpr PathComparator pathCmp = [](std::string_view lhs, std::string_view rhs)
  {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
                      [](char a, char b) { return tolower(a) == tolower(b);});
  };
#else
  constexpr PathComparator pathCmp = &std::operator==;
#endif

  auto posIt = positions.begin();
  std::string mainFile = posIt->file;
  std::string result = std::to_string(positions.front().line);

  for (const auto &position: positions)
  {
    if (!pathCmp(position.file, mainFile))
    {
      return {};
    }
  }

  for (++posIt; posIt != positions.end(); ++posIt)
  {
    result += ", " + std::to_string(posIt->line);
  }

  return result;
}

std::string Warning::GetOldstyleOutput() const &
{
  std::vector<std::string> alternativeNames;

  if (HasCWE())
  {
    alternativeNames.emplace_back(GetCWEString());
  }

  if (HasSAST())
  {
    alternativeNames.emplace_back(GetSASTString());
  }

  // 18 symbols for keywords + 13 delimiters with 4 symbols
  constexpr auto minMessageSize = 18 + 13 * 4;

  std::string result;
  result.reserve(minMessageSize);

  const auto &mainPosition = positions.front();
  const auto &navigation = mainPosition.navigation;

  result += "Viva64-EM<#~>full<#~>";
  result += std::to_string(mainPosition.line);
  result += "<#~>";
  result += mainPosition.file;
  result += "<#~>error<#~>";
  result += code;
  result += "<#~>";
  result += message;
  result += "<#~>";
  result += (falseAlarm ? "true" : "false");
  result += "<#~>";
  result += std::to_string(level);
  result += "<#~>";
  result += navigation.previousLineString;
  result += "<#~>";
  result += navigation.currentLineString;
  result += "<#~>";
  result += navigation.nextLineString;
  result += "<#~>";
  result += ConvertToString(positions);
  result += "<#~>";
  result += Join(std::move(alternativeNames), ",");

  return result;
}

std::string Warning::GetOldstyleOutput() &&
{
  return static_cast<const Warning &>(*this).GetOldstyleOutput();
}

std::string Warning::GetFormattedOutput() const &
{
  return format == Format::RawJson ? GetJsonOutput() : GetOldstyleOutput();
}

std::string Warning::GetFormattedOutput() &&
{
  return format == Format::RawJson ? std::move(*this).GetJsonOutput()
                                   : std::move(*this).GetOldstyleOutput();
}

std::string Warning::GetJsonOutput() const &
{
  auto j = ConvertToJson(*this);
  return j.dump(-1, '\0', false, nlohmann::json::error_handler_t::ignore);
}

std::string Warning::GetJsonOutput() && //-V659
{
  auto j = ConvertToJson(std::move(*this));
  return j.dump(-1, '\0', false, nlohmann::json::error_handler_t::ignore);
}

struct SourceFilePosition
{
  std::string file;  // Полный путь к файлу
  std::vector<size_t> lines; // Набор номеров строк

  SourceFilePosition()  noexcept {}

  SourceFilePosition(std::string file, std::vector<size_t> lines) noexcept
    : file { std::move(file) }
    , lines { std::move(lines) }
  {
  }
};

[[maybe_unused]]
static void to_json(nlohmann::json &j, const SourceFilePosition &p)
{
  j = nlohmann::json {
    { "file",  p.file },
    { "lines", p.lines }
  };
}

[[maybe_unused]]
static void from_json(const nlohmann::json &j, SourceFilePosition &p)
{
  j.at("file").get_to(p.file);
  j.at("lines").get_to(p.lines);
}

nlohmann::json Warning::ConvertToJson(Warning w)
{
  // Поля, которые могут отсутствовать
  constexpr auto writeOption = [](nlohmann::json &j, auto &&fieldName, auto &&value)
  {
    if (!std::empty(value))
    {
      // Строка из исходника может прийти не в utf-8 кодировке (nlohmann не дает сделать нестандартный json)
      // Вырезаем не ASCII символы
      value.erase(std::remove_if(std::begin(value), std::end(value),
                                 [](unsigned char symb) { return symb >= 0x80; }),
                  std::end(value));
      j.emplace(std::forward<decltype(fieldName)>(fieldName), std::forward<decltype(value)>(value));
    }
  };

  using PathComparator = bool (*)(std::string_view, std::string_view);

#ifdef _WIN32
  constexpr PathComparator pathCmp = [](std::string_view lhs, std::string_view rhs)
    {
      return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
                        [](char a, char b) { return tolower(a) == tolower(b);});
    };
#else
  constexpr PathComparator pathCmp = &std::operator==;
#endif

  nlohmann::json j {
    { "falseAlarm", w.falseAlarm },
    { "level",      w.level },
    { "code",       std::move(w.code) },
    { "message",    std::move(w.message) }
  };

  {
    std::vector<SourceFilePosition> joinedPositions;
    joinedPositions.reserve(w.positions.size());
    std::string_view currentFileName;

    for (auto &position : w.positions)
    {
      if (!pathCmp(currentFileName, position.file))
      {
        currentFileName = joinedPositions.emplace_back(std::move(position.file),
                                                       std::vector { static_cast<size_t>(position.line) })
          .file;
      }
      else
      {
        joinedPositions.back().lines.push_back(position.line);
      }
    }

    j.emplace("positions", std::move(joinedPositions));

    auto navigation = w.positions.front().navigation;

    if (w.HasCWE())
    {
      j.emplace("cwe", w.cwe);
    }

    if (w.HasSAST())
    {
      writeOption(j, "sastId",   std::move(w.sastId));
    }

    writeOption(j, "prevLine", std::move(navigation.previousLineString));
    writeOption(j, "currLine", std::move(navigation.currentLineString));
    writeOption(j, "nextLine", std::move(navigation.nextLineString));
  }

  return j;
}

Warning Warning::Parse(const std::string& srcLine)
{
  Warning warning;

  std::string line = Trim(srcLine);
  if (StartsWith(line, "{") && EndsWith(line, "}"))
  {
    auto j = nlohmann::json::parse(line);

    j["falseAlarm"].get_to(warning.falseAlarm);
    j["level"].get_to(warning.level);
    j["code"].get_to(warning.code);
    j["message"].get_to(warning.message);

    std::vector<SourceFilePosition> sourcePositions;
    j["positions"].get_to(sourcePositions);

    for (auto&& p : sourcePositions)
    {
      for (auto l : p.lines)
      {
        warning.positions.emplace_back(p.file, l);
      }
    }

    constexpr auto readOption = [](const nlohmann::json &j, auto &&fieldName, auto &value)
    {
      auto emplaceTo = j.find(fieldName);
      if (emplaceTo != j.end())
      {
        emplaceTo->get_to(value);
      }
    };

    auto &navigation = warning.positions.front().navigation;
    readOption(j, "cwe", warning.cwe);
    readOption(j, "sastId", warning.sastId);
    readOption(j, "prevLine", navigation.previousLineString);
    readOption(j, "currLine", navigation.currentLineString);
    readOption(j, "nextLine", navigation.nextLineString);

    warning.format = Warning::Format::RawJson;
  }
  else
  {
    const std::string delimiter = "<#~>";
    std::vector<std::string> fields;
    fields.reserve(14);
    Split(line, delimiter, std::back_inserter(fields));

    if ((fields.size() != 13 && fields.size() != 14) || fields.front() != "Viva64-EM")
    {
      throw ParseException("error parsing old format message");
    }

    warning.trialMode = fields[1] == "trial";
    const auto lineNo = ParseUint(fields[2]);
    const auto file = std::move(fields[3]);
    //msg.errorType = std::move(m_fields[4]); (deprecated)
    warning.code = std::move(fields[5]);
    warning.message = std::move(fields[6]);
    warning.falseAlarm = fields[7] == "true";
    warning.level = ParseUint(fields[8]);

    warning.positions.emplace_back(file, lineNo);

    auto &navigation = warning.positions.front().navigation;
    navigation.previousLineString = std::move(fields[9]);
    navigation.currentLineString = std::move(fields[10]);
    navigation.nextLineString = std::move(fields[11]);

    std::vector<size_t> lines;
    Split(fields[12], ",", std::back_inserter(lines), ParseUint);

    if (lines.size() > 1)
    {
      for (auto it = lines.begin() + 1; it != lines.end(); ++it)
      {
        warning.positions.emplace_back(file, *it);
      }
    }

    if (fields.size() > 13)
    {
      auto commaPos = fields[13].find(',');
      auto CWE_SubStr = fields[13].substr(0, commaPos);
      std::string CWEPrefixStr { Warning::CWEPrefix };
      if (StartsWith(CWE_SubStr, CWEPrefixStr))
      {
        warning.cwe = ParseUint(CWE_SubStr.substr(CWEPrefixStr.length()));
        if (commaPos != std::string::npos)
        {
          warning.sastId = fields[13].substr(commaPos + 1);
        }
      }
      else
      {
        warning.sastId = std::move(fields[13]);
      }
    }

    warning.format = Format::OldStyle;
  }

  return warning;
}
}