//  2006-2008 (c) Viva64.com Team
//  2008-2018 (c) OOO "Program Verification Systems"

#ifndef CSVOUTPUT_H
#define CSVOUTPUT_H
#include "ioutput.h"

namespace PlogConverter
{

class CSVOutput : public IOutput
{
public:
  explicit CSVOutput(const ProgramOptions&);
  void Start() override;
  void Write(const Warning& msg) override;
};

}

#endif // CSVOUTPUT_H
