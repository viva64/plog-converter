#ifndef TOTALSOUTPUT_H
#define TOTALSOUTPUT_H

#include "ioutput.h"

namespace PlogConverter
{
  class TotalsOutput;
  template<>
  constexpr std::string_view GetFormatName<TotalsOutput>() noexcept
  {
    return "totals";
  }

  class TotalsOutput : public IOutput
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
    std::string_view GetFormatName() const noexcept override
    {
      return ::PlogConverter::GetFormatName<TotalsOutput>();
    }

  private:
    static void AddByLevel(Staticstic::WarningsCount &value, size_t level);
    static std::string ToLevelTriplet(const Staticstic::WarningsCount &value);

    Staticstic m_stats{};
  };

}

#endif // TOTALSOUTPUT_H
