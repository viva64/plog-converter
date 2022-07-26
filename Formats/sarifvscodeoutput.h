//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2022 (c) PVS-Studio LLC

#pragma once

#include "sarifoutput.h"

namespace PlogConverter
{

class SarifVSCodeOutput : public SarifOutputImpl<SarifVSCodeOutput>
{
public:
  explicit SarifVSCodeOutput(const ProgramOptions &opt);
  ~SarifVSCodeOutput() override = default;

  void Finish() override;

  [[nodiscard]]
  static std::string_view FormatName() noexcept
  {
    return "sarif-vscode";
  }

  [[nodiscard]]
  static std::string_view OutputSuffix() noexcept
  {
    static auto suffix = std::string{ "vscode." }.append(SarifOutput::OutputSuffix());
    return suffix;
  }
};

}
