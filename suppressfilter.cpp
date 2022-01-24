//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2022 (c) PVS-Studio LLC

#include "suppressfilter.h"
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
  return static_cast<size_t>(type) * Analyzer::LevelsCount + (level - 1); //-V104
}

SuppressFilter::SuppressFilter(IOutput* output, const ProgramOptions &options)
  : IFilter(output)
  , m_disabledWarnings(options.disabledWarnings)
  , m_enabledAnalyzerLevels(Analyzer::AnalyzersCount * Analyzer::LevelsCount, options.enabledAnalyzers.empty() ? 1 : 0)
  , m_enabledFiles(options.enabledFiles.size())
  , m_enabledWarnings(options.enabledWarnings)
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

  for (size_t count = 0; count < options.enabledFiles.size(); ++count)
  {
    auto &file = options.enabledFiles[count];

    if (auto linePos = file.rfind(':');
        linePos != std::string::npos)
    {
      auto name = file.substr(0, linePos - 1);
      auto line = file.substr(linePos + 1);

      try
      {
        m_enabledFiles[count].line = std::stoul(line);
      }
      catch (std::invalid_argument &)
      {
        name = file;
      }

      m_enabledFiles[count].name = name;
    }
    else
    {
      m_enabledFiles[count].name = file;
    }
  }
}

SuppressFilter::~SuppressFilter() = default;

bool SuppressFilter::Check(const Warning &message) const
{
  return CheckCode(message)
      && CheckFiles(message)
      && CheckLevel(message)
      && CheckWarnings(message);
}

bool SuppressFilter::CheckCode(const Warning &message) const
{
  return m_disabledWarnings.find(message.code) == m_disabledWarnings.end();
}

bool SuppressFilter::CheckFiles(const Warning &message) const
{
  if (m_enabledFiles.empty())
  {
    return true;
  }

  for (auto file : m_enabledFiles)
  {
    if (   File::CheckName(message, file)
        && File::CheckLine(message, file))
    {
      return true;
    }
  }

  return false;
}

bool SuppressFilter::CheckLevel(const Warning &message) const
{
  const AnalyzerType type = message.GetType();
  return message.level <= 0
      || message.level > Analyzer::LevelsCount
      || type == AnalyzerType::Unknown
      || m_enabledAnalyzerLevels[AnalyzerLevelIndex(type, message.level)];
}

bool SuppressFilter::CheckWarnings(const Warning &message) const
{
  if (m_enabledWarnings.empty())
  {
    return true;
  }

  for (auto warning : m_enabledWarnings)
  {
    if (warning == message.GetErrorCode())
    {
      return true;
    }
  }

  return false;
}

}
