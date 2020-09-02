//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020 (c) PVS-Studio LLC

#ifndef ANALYZERMESSAGE
#define ANALYZERMESSAGE
#include <vector>
#include <string>
#include <algorithm>
#include "utils.h"
#include "JsonUtils.h"

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
  Unknown          = 6
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

  WarningPosition(std::string file_, int line_)
    : file(std::move(file_)), line(line_), endLine(line_)
  {}

  template <typename T>
  void Serialize(T& stream)
  {
    stream.Optional("file", file)
          .Optional("line", line)
          .Optional("column", column)
          .Optional("endColumn", endColumn, std::numeric_limits<int>::max())
          .Optional("navigation", navigation);

    stream.Optional("endLine", endLine, line);
  }
};

struct Warning
{
  constexpr static const char*      CWEPrefix = "CWE-";
  
  constexpr static const char*      MISRACorePrefix = "MISRA: ";
  constexpr static const char*      MISRAPrefixC = "MISRA C ";
  constexpr static const char*      MISRAPrefixCPlusPlus = "MISRA C++ ";
  
  std::string                       code;
  std::string                       message;
  std::vector<WarningPosition>      positions;
  std::vector<unsigned>             additionalLines;
  std::vector<std::string>          stacktrace;
  std::vector<std::string>          projects;
  unsigned                          cwe = 0;
  std::string                       misra;
  unsigned                          level = 0;
  bool                              favorite = false;
  bool                              falseAlarm = false;
  bool                              trialMode = false;

  template <typename T>
  void Serialize(T& stream)
  {
    stream.Required("code", code)
          .Required("message", message)
          .Optional("stacktrace", stacktrace)
          .Optional("positions", positions)
          .Optional("projects", projects)
          .Optional("cwe", cwe)
          .Optional("misra", misra)
          .Optional("level", level)
          .Optional("favorite", favorite)
          .Optional("falseAlarm", falseAlarm)
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
  bool                              HasMISRA() const;

  AnalyzerType                      GetType() const;
  unsigned                          GetErrorCode() const;
  unsigned                          GetLine() const;
  const std::string &               GetFile() const;
  const NavigationInfo &            GetNavigationInfo() const;
  std::string                       GetVivaUrl() const;
  std::string                       GetCWEUrl() const;
  std::string                       GetCWEString() const;
  std::string                       GetMISRAString() const;
  std::string                       GetMISRAStringWithLanguagePrefix() const;
  std::vector<unsigned>             GetExtendedLines() const;

  std::string                       GetLevelString() const;
  std::string                       GetLevelString(const std::string &l01, const std::string &l2, const std::string &l3) const;
  std::string                       GetLevelString(const std::string &l0, const std::string &l1, const std::string &l2, const std::string &l3) const;

  void                              Clear();
};

}

#endif // ANALYZERMESSAGE