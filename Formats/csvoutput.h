//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2022 (c) PVS-Studio LLC

#ifndef CSVOUTPUT_H
#define CSVOUTPUT_H
#include "ioutput.h"

namespace PlogConverter
{

class CSVOutput;
template <>
constexpr std::string_view GetFormatName<CSVOutput>() noexcept
{
  return "csv";
}

class CSVOutput : public IOutput
{
public:
  explicit CSVOutput(const ProgramOptions&);
  void Start() override;
  bool Write(const Warning& msg) override;

  [[nodiscard]]
  std::string_view GetFormatName() const noexcept override
  {
    return ::PlogConverter::GetFormatName<CSVOutput>();
  }
};

}

#endif // CSVOUTPUT_H
