//  2006-2008 (c) Viva64.com Team
//  2008-2018 (c) OOO "Program Verification Systems"

#ifndef IOUTPUT
#define IOUTPUT
#include <iostream>
#include <fstream>
#include "warning.h"
#include "configs.h"
#include "utils.h"

namespace PlogConverter
{

class IOutput
{
public:
  virtual void Start();
  virtual void Write(const Warning& msg) = 0;
  virtual void Finish();
  virtual ~IOutput();

protected:
  IOutput(const ProgramOptions &options, const std::string &extension);
  explicit IOutput(const std::string &path);
  IOutput();

  std::basic_ostream<char> &m_ostream;
  std::ofstream m_ofstream;
  std::vector<SecurityCodeMapping> m_errorCodeMappings;
};

}

#endif // IOUTPUT

