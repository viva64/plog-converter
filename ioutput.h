//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2021 (c) PVS-Studio LLC

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
  virtual bool Write(const Warning& message) = 0;
  virtual void Finish();
  virtual ~IOutput() = default;

protected:
  IOutput(const ProgramOptions &options, const std::string &extension);
  explicit IOutput(const std::string &path);
  IOutput();

  void DetectShowTags(bool* showCWE, bool* showSAST) const;

  std::basic_ostream<char> &m_ostream;
  std::ofstream m_ofstream;
  std::vector<SecurityCodeMapping> m_errorCodeMappings;
};

class IFilter : public IOutput
{
public:
  IFilter(IOutput* output) : m_output(output) { };

  void Start() override
  {
    if (m_output)
      m_output->Start();
  }

  bool Write(const Warning& message) override
  {
    if (!Check(message))
      return false;

    if (m_output)
      return m_output->Write(message);

    return true;
  }

  void Finish() override
  {
    if (m_output)
      m_output->Finish();
  }

  virtual ~IFilter() = default;

protected:
  virtual bool Check(const Warning& message) const = 0;

  IOutput* m_output = nullptr;
};

class ITransform : public IOutput
{
public:
  ITransform(IOutput* output) : m_output(output) { };

  void Start() override
  {
    if (m_output)
      m_output->Start();
  }

  bool Write(const Warning& message) override
  {
    return m_output->Write(Transform(message));
  }

  void Finish() override
  {
    if (m_output)
      m_output->Finish();
  }

  virtual ~ITransform() = default;

protected:
  virtual Warning Transform(const Warning& message) const = 0;

  IOutput* m_output = nullptr;
};

}

#endif // IOUTPUT
