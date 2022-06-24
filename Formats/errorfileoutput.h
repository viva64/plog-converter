//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2022 (c) PVS-Studio LLC

#ifndef ERRORFILEOUTPUT_H
#define ERRORFILEOUTPUT_H
#include "ioutput.h"

namespace PlogConverter
{

class ErrorFileOutput;
template<>
constexpr std::string_view GetFormatName<ErrorFileOutput>() noexcept
{
  return "errorfile";
}

class ErrorFileOutput : public IOutput
{
public:
  explicit ErrorFileOutput(const ProgramOptions&);
  ~ErrorFileOutput() override;
  bool Write(const Warning& msg) override;

  [[nodiscard]]
  std::string_view GetFormatName() const noexcept override
  {
    return ::PlogConverter::GetFormatName<ErrorFileOutput>();
  }
};

}

#endif // ERRORFILEOUTPUT_H
