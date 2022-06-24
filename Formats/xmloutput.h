//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2022 (c) PVS-Studio LLC

#ifndef XMLOUTPUT_H
#define XMLOUTPUT_H
#include "ioutput.h"

namespace PlogConverter
{

class XMLOutput;
template <>
constexpr std::string_view GetFormatName<XMLOutput>() noexcept
{
  return "xml";
}


class XMLOutput : public IOutput
{
public:
  explicit XMLOutput(const ProgramOptions &);
  void Start() override;
  bool Write(const Warning& msg) override;
  void Finish() override;
  ~XMLOutput() override;

  [[nodiscard]]
  std::string_view GetFormatName() const noexcept override
  {
    return ::PlogConverter::GetFormatName<XMLOutput>();
  }
};

}

#endif // XMLOUTPUT_H
