//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"

#ifndef XMLOUTPUT_H
#define XMLOUTPUT_H
#include "ioutput.h"

namespace PlogConverter
{

class XMLOutput : public IOutput
{
public:
  explicit XMLOutput(const ProgramOptions &);
  void Start() override;
  void Write(const Warning& msg) override;
  void Finish() override;
  ~XMLOutput() override;
};

}

#endif // XMLOUTPUT_H
