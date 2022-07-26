//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2022 (c) PVS-Studio LLC

#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include "JsonUtils.h"
#include "utils.h"

namespace PlogConverter
{

using PvsStudio::from_json;
using PvsStudio::to_json;

enum class AnalyzerType
{
  Fail             = 0,
  General          = 1,
  Viva64           = 2,
  Optimization     = 3,
  CustomerSpecific = 4,
  Misra            = 5,
  Owasp            = 6,
  Autosar          = 7,
  Unknown          = 8
};

struct Analyzer
{
  constexpr static const int LevelsCount = 3;
  constexpr static const int AnalyzersCount = static_cast<size_t>(AnalyzerType::Unknown);

  std::vector<int>    levels;
  AnalyzerType        type;
};

struct NavigationInfo
{
  std::string         previousLineString;
  std::string         currentLineString;
  std::string         nextLineString;
  unsigned            previousLine = 0;
  unsigned            currentLine = 0;
  unsigned            nextLine = 0;
  unsigned            columns = 0;

  NavigationInfo() = default;

  NavigationInfo(std::string previousLineString,
                 std::string currentLineString,
                 std::string nextLineString,
                 unsigned    previousLine = 0,
                 unsigned    currentLine = 0,
                 unsigned    nextLine = 0,
                 unsigned    columns = 0) noexcept
    : previousLineString { RemoveNonAscii(std::move(previousLineString)) }
    , currentLineString  { RemoveNonAscii(std::move(currentLineString)) }
    , nextLineString     { RemoveNonAscii(std::move(nextLineString)) }
    , previousLine { previousLine }
    , currentLine  { currentLine }
    , nextLine     { nextLine }
    , columns      { columns }
  {
  }

  std::string RemoveNonAscii(std::string value)
  {
    value.erase(std::remove_if(std::begin(value), std::end(value),
                               [](unsigned char symb) { return symb >= 0x80; }),
                std::end(value));

    return value;
  };

  template <typename T>
  void Serialize(T& stream)
  {
    stream.Optional("previousLine", previousLine)
          .Optional("currentLine", currentLine)
          .Optional("nextLine", nextLine)
          .Optional("columns", columns);
  }
};

struct WarningPosition
{
  std::string         file;
  unsigned            line = 1;
  unsigned            endLine = 1;
  unsigned            column = 1;
  unsigned            endColumn = std::numeric_limits<int>::max();
  NavigationInfo      navigation;

  WarningPosition() = default;

  WarningPosition(std::string file_, unsigned line_)
    : file    { std::move(file_) }
    , line    { line_ }
    , endLine { line_ }
  {}

  WarningPosition(std::string file_, size_t line_)
    : file    { std::move(file_) }
    , line    { static_cast<unsigned>(line_) }
    , endLine { static_cast<unsigned>(line_) }
  {}

  bool operator<(const WarningPosition& other) const noexcept;

  template <typename T>
  void Serialize(T& stream)
  {
    stream.Required("file", file)
          .Required("line", line)
          .Optional("column", column)
          .Optional("endColumn", endColumn, std::numeric_limits<int>::max())
          .Optional("navigation", navigation)
          .Optional("endLine", endLine, line);
  }
};

class ParseException : public std::runtime_error
{
public:
  using runtime_error::runtime_error;
};

struct Warning
{
  enum class Format
  {
    Unknown,
    OldStyle,
    RawJson
  };

  constexpr static const char*      CWEPrefix = "CWE-";

  std::string                       code;
  std::string                       message;
  std::string                       sastId;
  std::vector<WarningPosition>      positions;
  std::vector<std::string>          stacktrace;
  std::vector<std::string>          projects;
  unsigned                          cwe = 0;
  unsigned                          level = 0;
  bool                              favorite = false;
  bool                              falseAlarm = false;
  bool                              trialMode = false;

  Format                            format = Format::OldStyle;

  Warning() = default;

  Warning(std::string code, //-V688
          std::string message,
          std::string file,
          Format format,
          size_t line = 1,
          unsigned level = 0)
    : code { std::move(code) }
    , message { std::move(message) }
    , level { level }
    , format { format }
  {
    positions.emplace_back(std::move(file), line);
  }

  Warning(unsigned code, //-V688
          const std::string &message,
          const std::string &file,
          Format format,
          size_t line = 1,
          unsigned level = 0)
    : message { std::move(message) }
    , level { level }
    , format { format }
  {
    // todo: (c++ 20) use fmt
    if (code < 10)
    {
      this->code = "V00" + std::to_string(code);
    }
    else if (code < 100)
    {
      this->code = "V0" + std::to_string(code);
    }
    else
    {
      this->code = "V" + std::to_string(code);
    }

    positions.emplace_back(file, line);
  }

  Warning(std::string code,
          std::string message,
          std::vector<WarningPosition> positions,
          Format format,
          unsigned cwe,
          std::string sast,
          unsigned level,
          bool falseAlarm
  )
    : code { std::move(code) }
    , message { std::move(message) }
    , sastId { std::move(sast) }
    , positions { std::move(positions) }
    , cwe { cwe }
    , level { level }
    , falseAlarm { falseAlarm }
    , format { format }
  {
  }

  template <typename T>
  void Serialize(T& stream)
  {
    stream.Required("code", code)
          .Required("message", message)
          .Required("level", level)
          .Required("positions", positions)
          .Optional("cwe", cwe, 0)
          .Optional("sastId", sastId)
          .Optional("favorite", favorite)
          .Optional("falseAlarm", falseAlarm)
          .Optional("stacktrace", stacktrace)
          .Optional("projects", projects)
          .Optional("trialMode", trialMode);
  }

  static Warning                    GetDocumentationLinkMessage();

  bool                              IsDocumentationLinkMessage() const;
  bool                              IsRenewMessage() const;
  bool                              IsExternalMessage() const;
  bool                              IsUpdateMessage() const;
  bool                              IsTrialMessage() const;
  bool                              HasProjects() const;
  bool                              HasCWE() const;
  bool                              HasSAST() const;

  AnalyzerType                      GetType() const;
  unsigned                          GetErrorCode() const;
  unsigned                          GetLine() const;
  unsigned                          GetEndLine() const;
  unsigned                          GetStartColumn() const;
  unsigned                          GetEndColumn() const;
  const std::string &               GetFile() const;
  const std::string                 GetFileUTF8() const;
  const NavigationInfo &            GetNavigationInfo() const;
  std::string                       GetVivaUrl() const;
  std::string                       GetCWEUrl() const;
  std::string                       GetCWEString() const;
  std::string                       GetSASTString() const;
  std::vector<unsigned>             GetExtendedLines() const;

  std::string_view                  GetLevelString() const noexcept;
  std::string_view                  GetLevelString(std::string_view l01, std::string_view l2, std::string_view l3) const noexcept;
  std::string_view                  GetLevelString(std::string_view l0, std::string_view l1, std::string_view l2, std::string_view l3) const noexcept;

  static Warning Parse(const std::string& str);

  std::string                       GetOldstyleOutput() const &;
  std::string                       GetOldstyleOutput() &&;

  std::string                       GetFormattedOutput() const &;
  std::string                       GetFormattedOutput() &&;

  std::string                       GetJsonOutput() const &;
  std::string                       GetJsonOutput() &&;


  void                              Clear();

private:
  static nlohmann::json ConvertToJson(Warning w);
};

}
