//  2006-2008 (c) Viva64.com Team
//  2008-2019 (c) OOO "Program Verification Systems"

#ifndef MULTIOUTPUT
#define MULTIOUTPUT
#include "ioutput.h"

namespace PlogConverter
{

class MultipleOutput : public IOutput
{
public:
  MultipleOutput();
  ~MultipleOutput() override;

  void Start() override;
  void Write(const Warning& msg) override;
  void Finish() override;

  void Add(std::unique_ptr<IOutput> output);

private:
  std::vector<std::unique_ptr<IOutput>> m_outputs;
};

}

#endif // MULTIOUTPUT

