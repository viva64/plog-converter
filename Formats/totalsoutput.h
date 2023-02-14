//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2023 (c) PVS-Studio LLC

#pragma once

#include "ioutput.h"

namespace PlogConverter
{

class TotalsOutput : public BasicFormatOutput<TotalsOutput>
{
  struct Staticstic
  {
    using WarningsCount = std::tuple<size_t, size_t, size_t>;
    WarningsCount generalAnalysis { };
    WarningsCount optimization { };
    WarningsCount x64BitIssues { };
    WarningsCount customerSpecific { };
    WarningsCount misra { };
    WarningsCount autosar { };
    WarningsCount owasp { };
    size_t fails { };
    WarningsCount total { };
  };

public:
  explicit TotalsOutput(const ProgramOptions&);
  ~TotalsOutput() override = default;

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
    return "totals";
  }

  [[nodiscard]]
  static std::string_view OutputSuffix() noexcept
  {
    return FormatName();
  }

private:
  static void AddByLevel(Staticstic::WarningsCount &value, size_t level);
  static std::string ToLevelTriplet(const Staticstic::WarningsCount &value);

  Staticstic m_stats{};
};

}
