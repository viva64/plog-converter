//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2023 (c) PVS-Studio LLC

#pragma once

#include "ioutput.h"

namespace PlogConverter
{

class JsonOutput : public BasicFormatOutput<JsonOutput>
{
public:
  explicit JsonOutput(const ProgramOptions&);
  ~JsonOutput() override = default;

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
    return "json";
  }

  [[nodiscard]]
  static std::string_view OutputSuffix() noexcept
  {
    return FormatName();
  }

  [[nodiscard]] 
  static bool SupportsSourceRootMarker() noexcept
  {
    return true;
  }

private:
  static constexpr size_t m_version{ 2u };
  nlohmann::json m_jsonOutput;
};

}
