#ifndef JSONOUTPUT_H
#define JSONOUTPUT_H

#include "ioutput.h"

namespace PlogConverter
{

class JsonOutput;
template<>
constexpr std::string_view GetFormatName<JsonOutput>() noexcept
{
  return "json";
}

class JsonOutput : public IOutput
{
public:
  explicit JsonOutput(const ProgramOptions&);
  ~JsonOutput() override = default;
  void Start() override;
  bool Write(const Warning& msg) override;
  void Finish() override;

  [[nodiscard]]
  std::string_view GetFormatName() const noexcept override
  {
    return ::PlogConverter::GetFormatName<JsonOutput>();
  }

private:
  static constexpr size_t m_version{ 2u };
  nlohmann::json m_jsonOutput;
};

}

#endif // JSONOUTPUT_H
