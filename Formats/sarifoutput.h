//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2021 (c) PVS-Studio LLC

#ifndef SARIFOUTPUT_H
#define SARIFOUTPUT_H
#include "ioutput.h"
#include <list>

namespace PlogConverter
{

class SarifOutput : public IOutput
{
  std::list<Warning> m_warnings;
public:
  explicit SarifOutput(const ProgramOptions& opt) : IOutput(opt, "sarif") {}
  void Start() override {}
  void Write(const Warning& msg) override;
  void Finish() override;
  ~SarifOutput() override = default;
};

}

#endif // SARIFOUTPUT_H
