//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2021 (c) PVS-Studio LLC

#ifndef TASKLISTOUTPUT_H
#define TASKLISTOUTPUT_H
#include "ioutput.h"

namespace PlogConverter
{

class TaskListOutput : public IOutput
{
public:
  explicit TaskListOutput(const ProgramOptions &);
  ~TaskListOutput() override;
  bool Write(const Warning &msg) override;
};

}

#endif // TASKLISTOUTPUT_H
