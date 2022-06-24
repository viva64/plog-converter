//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2022 (c) PVS-Studio LLC

#ifndef ERRORFILEVERBOSEOUTPUT_H
#define ERRORFILEVERBOSEOUTPUT_H
#include "errorfileoutput.h"

namespace PlogConverter
{

class ErrorFileVerboseOutput;
template<>
constexpr std::string_view GetFormatName<ErrorFileVerboseOutput>() noexcept
{
  return "errorfile-verbose";
}

class ErrorFileVerboseOutput : public ErrorFileOutput
{
public:
  explicit ErrorFileVerboseOutput(const ProgramOptions&);
  ~ErrorFileVerboseOutput() override;
  bool Write(const Warning& msg) override;

  [[nodiscard]]
  std::string_view GetFormatName() const noexcept override
  {
    return ::PlogConverter::GetFormatName<ErrorFileVerboseOutput>();
  }
};

}

#endif // ERRORFILEVERBOSEOUTPUT_H
