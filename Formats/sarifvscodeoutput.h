//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2022 (c) PVS-Studio LLC

#ifndef SARIFVSCODEOUTPUT_H
#define SARIFVSCODEOUTPUT_H
#include "sarifoutput.h"

namespace PlogConverter
{

class SarifVSCodeOutput;
template<>
constexpr std::string_view GetFormatName<SarifVSCodeOutput>() noexcept
{
  return "sarif-vscode";
}

class SarifVSCodeOutput : public SarifOutput
{
public:
  explicit SarifVSCodeOutput(const ProgramOptions &opt);
  void Finish() override;

  [[nodiscard]]
  std::string_view GetFormatName() const noexcept override
  {
    return ::PlogConverter::GetFormatName<SarifVSCodeOutput>();
  }
};

}

#endif // SARIFVSCODEOUTPUT_H
