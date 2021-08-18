//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2021 (c) PVS-Studio LLC

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
  bool Write(const Warning& msg) override;
};

}

#endif // CSVOUTPUT_H
