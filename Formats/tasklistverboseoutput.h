//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2023 (c) PVS-Studio LLC

#pragma once

#include "tasklistoutput.h"

namespace PlogConverter
{

class TaskListVerboseOutput : public TaskListOutputImpl<TaskListVerboseOutput>
{
public:
  explicit TaskListVerboseOutput(const ProgramOptions &);
  ~TaskListVerboseOutput() override = default;

  bool Write(const Warning &msg) override;

  [[nodiscard]]
  static std::string_view FormatName() noexcept
  {
    return "tasklist-verbose";
  }

  [[nodiscard]]
  static std::string_view OutputSuffix() noexcept
  {
    static auto suffix = std::string{ "verbose." }.append(TaskListOutput::OutputSuffix());
    return suffix;
  }
};

}
