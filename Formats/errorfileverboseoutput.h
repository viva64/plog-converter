//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2023 (c) PVS-Studio LLC

#pragma once

#include "errorfileoutput.h"

namespace PlogConverter
{

class ErrorFileVerboseOutput : public ErrorFileOutputImpl<ErrorFileVerboseOutput>
{
public:
  explicit ErrorFileVerboseOutput(const ProgramOptions&);
  ~ErrorFileVerboseOutput() override = default;

  bool Write(const Warning& msg) override;

  [[nodiscard]]
  static std::string_view FormatName() noexcept
  {
    return "errorfile-verbose";
  }

  [[nodiscard]]
  static std::string_view OutputSuffix() noexcept
  {
    using namespace std::literals;
    static auto suffix = "verbose."s + std::string { ErrorFileOutput::OutputSuffix() };
    return suffix;
  }
};

}
