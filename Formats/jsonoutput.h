#ifndef JSONOUTPUT_H
#define JSONOUTPUT_H

#include "ioutput.h"

namespace PlogConverter
{

class JsonOutput : public IOutput
{
public:
  explicit JsonOutput(const ProgramOptions&);
  ~JsonOutput() override = default;
  void Start() override;
  bool Write(const Warning& msg) override;
  void Finish() override;

private:
  static constexpr size_t m_version{ 2u };
  nlohmann::json m_jsonOutput;
};

}

#endif // JSONOUTPUT_H
