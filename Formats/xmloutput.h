//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2022 (c) PVS-Studio LLC

#pragma once

#include "ioutput.h"

namespace PlogConverter
{

class XMLOutput : public BasicFormatOutput<XMLOutput>
{
public:
  explicit XMLOutput(const ProgramOptions &);
  ~XMLOutput() override = default;

  void Start() override;
  bool Write(const Warning& msg) override;
  void Finish() override;

  [[nodiscard]]
  static bool SupportsRelativePath() noexcept
  {
    return false;
  }

  [[nodiscard]]
  static bool OutputIsFile() noexcept
  {
    return true;
  }

  [[nodiscard]]
  static std::string_view FormatName() noexcept
  {
    return "xml";
  }

  [[nodiscard]]
  static std::string_view OutputSuffix() noexcept
  {
    return FormatName();
  }
};

}
