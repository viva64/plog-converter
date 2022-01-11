//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2022 (c) PVS-Studio LLC

#ifndef IWORKER
#define IWORKER
#include "configs.h"

namespace PlogConverter
{

class IWorker
{
public:
  virtual void Run(const ProgramOptions &opt) = 0;
  virtual ~IWorker();
};

}

#endif // IWORKER

