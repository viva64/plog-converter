//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2022 (c) PVS-Studio LLC

#ifndef TASKLISTVERBOSEOUTPUT_H
#define TASKLISTVERBOSEOUTPUT_H
#include "tasklistoutput.h"

namespace PlogConverter
{

class TaskListVerboseOutput : public TaskListOutput
{
public:
  explicit TaskListVerboseOutput(const ProgramOptions &);
  ~TaskListVerboseOutput() override;
  bool Write(const Warning &msg) override;
};

}

#endif // TASKLISTVERBOSEOUTPUT_H
