//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2022 (c) PVS-Studio LLC

#pragma once

#include "ioutput.h"

namespace PlogConverter
{

class GitLabOutput : public BasicFormatOutput<GitLabOutput>
{
public:
  explicit GitLabOutput(const ProgramOptions&);
  ~GitLabOutput() noexcept override = default;

  void Start() override;
  bool Write(const Warning& msg) override;
  void Finish() override;

  [[nodiscard]]
  static bool SupportsRelativePath() noexcept
  {
    return true;
  }

  [[nodiscard]]
  static bool OutputIsFile() noexcept
  {
    return true;
  }

  [[nodiscard]]
  static std::string_view FormatName() noexcept
  {
    return "gitlab";
  }

  [[nodiscard]]
  static std::string_view OutputSuffix() noexcept
  {
    static std::string suffix{ std::string{ FormatName() }.append(".json") };
    return suffix;
  }

private:
  nlohmann::json m_gitLabOutput;
};

}
