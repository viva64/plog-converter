//  2006-2008 (c) Viva64.com Team
//  2008-2018 (c) OOO "Program Verification Systems"

#ifndef ERRORFILEOUTPUT_H
#define ERRORFILEOUTPUT_H
#include "ioutput.h"

namespace PlogConverter
{

class ErrorFileOutput : public IOutput
{
public:
  explicit ErrorFileOutput(const ProgramOptions&);
  ~ErrorFileOutput() override;
  void Write(const Warning& msg) override;
};

}

#endif // ERRORFILEOUTPUT_H
