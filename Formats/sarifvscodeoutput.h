//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2022 (c) PVS-Studio LLC

#ifndef SARIFVSCODEOUTPUT_H
#define SARIFVSCODEOUTPUT_H
#include "sarifoutput.h"

namespace PlogConverter
{

class SarifVSCodeOutput : public SarifOutput
{
public:
  explicit SarifVSCodeOutput(const ProgramOptions &opt);
  void Finish() override;
};

}

#endif // SARIFVSCODEOUTPUT_H
