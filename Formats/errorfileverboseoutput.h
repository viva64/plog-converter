//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2022 (c) PVS-Studio LLC

#ifndef ERRORFILEVERBOSEOUTPUT_H
#define ERRORFILEVERBOSEOUTPUT_H
#include "errorfileoutput.h"

namespace PlogConverter
{

class ErrorFileVerboseOutput : public ErrorFileOutput
{
public:
  explicit ErrorFileVerboseOutput(const ProgramOptions&);
  ~ErrorFileVerboseOutput() override;
  bool Write(const Warning& msg) override;
};

}

#endif // ERRORFILEVERBOSEOUTPUT_H
