//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2021 (c) PVS-Studio LLC

#include "messagefilter.h"
#include "warning.h"
#include <cstring>
#include <cctype>
#include <algorithm>
#include <functional>
#include <cassert>

namespace PlogConverter
{

static size_t AnalyzerLevelIndex(AnalyzerType type, int level)
{
  assert(level >= 1 && level <= Analyzer::LevelsCount);
  assert(type != AnalyzerType::Unknown);
  return static_cast<size_t>(type) * Analyzer::LevelsCount + (level - 1);
}

MessageFilter::MessageFilter(IOutput* output, const ProgramOptions &options)
  : IFilter(output)
  , m_enabledAnalyzerLevels(Analyzer::AnalyzersCount * Analyzer::LevelsCount, options.enabledAnalyzers.empty() ? 1 : 0)
  , m_disabledKeywords(options.disabledKeywords)
  , m_disabledPaths(options.disabledPaths)
  , m_disabledWarnings(options.disabledWarnings)
{
  for (const Analyzer& it : options.enabledAnalyzers)
  {
    if (it.levels.empty())
    {
      for (int l = 1; l <= Analyzer::LevelsCount; ++l)
      {
        m_enabledAnalyzerLevels[AnalyzerLevelIndex(it.type, l)] = 1;
      }
    }

    for (int l : it.levels)
    {
      m_enabledAnalyzerLevels[AnalyzerLevelIndex(it.type, l)] = 1;
    }
  }
}

MessageFilter::~MessageFilter() = default;

bool MessageFilter::Check(const Warning &message) const
{
  return CheckCode(message)
      && CheckFalseAlarm(message)
      && CheckKeywords(message)
      && CheckLevel(message)
      && CheckPath(message);
}

bool MessageFilter::CheckCode(const Warning &message) const
{
  return m_disabledWarnings.find(message.code) == m_disabledWarnings.end();
}

bool MessageFilter::CheckFalseAlarm(const Warning &message) const
{
  return !message.falseAlarm;
}

bool MessageFilter::CheckKeywords(const Warning &message) const
{
  if (m_disabledKeywords.empty())
  {
    return true;
  }

  auto isAlNum = [](char c) { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); };

  const std::string& errorString = message.GetNavigationInfo().currentLineString;
  const size_t errorStringSize = errorString.size();
  for (size_t tokenBegin = 0; tokenBegin < errorStringSize;)
  {
    size_t tokenEnd = 0;
    while (tokenBegin < errorStringSize && !(isAlNum(errorString[tokenBegin]) || errorString[tokenBegin] == '_'))
    {
      tokenBegin++;
    }

    tokenEnd = tokenBegin;
    while (tokenEnd < errorStringSize && (isAlNum(errorString[tokenEnd]) || errorString[tokenEnd] == '_'))
    {
      tokenEnd++;
    }

    if (m_disabledKeywords.find(errorString.substr(tokenBegin, tokenEnd - tokenBegin)) != m_disabledKeywords.end())
    {
      return false;
    }

    tokenBegin = tokenEnd;
  }

  return true;
}

bool MessageFilter::CheckLevel(const Warning &message) const
{
  const AnalyzerType type = message.GetType();
  return message.level <= 0
      || message.level > Analyzer::LevelsCount
      || type == AnalyzerType::Unknown
      || m_enabledAnalyzerLevels[AnalyzerLevelIndex(type, message.level)];
}

bool MessageFilter::CheckPath(const Warning& message) const
{
  return std::none_of(m_disabledPaths.begin(), m_disabledPaths.end(),
                      [&](const std::string &path)
                      {
                      	return message.GetFile().find(path) != std::string::npos;
                      });
}

}
