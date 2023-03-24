//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2023 (c) PVS-Studio LLC

#include "tasklistverboseoutput.h"

using namespace std::literals::string_view_literals;

namespace PlogConverter
{

TaskListVerboseOutput::TaskListVerboseOutput(const ProgramOptions &opt) : TaskListOutputImpl<TaskListVerboseOutput>{ opt }
{
}

constexpr std::string_view GetAdditionalPrefix() noexcept
{
  return "    |--"sv;
}

bool TaskListVerboseOutput::Write(const Warning &msg)
{
  if (TaskListOutputImpl::Write(msg))
  {
    if (msg.positions.size() > 1u)
    {
      auto warningLevel = msg.GetLevelString("err"sv, "warn"sv, "note"sv);

      for (auto it = std::next(msg.positions.begin(), 1u); it != msg.positions.end(); ++it)
      {
        m_ostream << it->file << '\t' << it->line << '\t'
                  << warningLevel << '\t';
        m_ostream << GetAdditionalPrefix()
                  << " Additional position : "sv << msg.code << '\n';
      }

      m_ostream.flush();
    }

    return true;
  }

  return false;
}

[[nodiscard]] std::string_view TaskListVerboseOutput::FormatName() noexcept
{
  return "tasklist-verbose";
}

[[nodiscard]] std::string_view TaskListVerboseOutput::OutputSuffix() noexcept
{
  using namespace std::literals;
  static auto suffix = "verbose."s + std::string { TaskListOutput::OutputSuffix() };
  return suffix;
}

}
