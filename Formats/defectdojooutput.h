//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2023 (c) PVS-Studio LLC

#pragma once

#include "ioutput.h"

namespace PlogConverter
{

  class DefectDojoOutput : public BasicFormatOutput<DefectDojoOutput>
  {
  public:
    explicit DefectDojoOutput(const ProgramOptions&);
    ~DefectDojoOutput() noexcept override = default;

    void Start() override;
    bool Write(const Warning& msg) override;
    void Finish() override;

    [[nodiscard]] static bool SupportsRelativePath() noexcept;
    [[nodiscard]] static bool OutputIsFile() noexcept;
    [[nodiscard]] static std::string_view FormatName() noexcept;
    [[nodiscard]] static std::string_view OutputSuffix() noexcept;

  private:
    nlohmann::json m_defectDojoOutput;
  };

}
