//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2022 (c) PVS-Studio LLC

#ifndef SARIFOUTPUT_H
#define SARIFOUTPUT_H
#include "ioutput.h"
#include <list>

namespace PlogConverter
{
  class SarifOutputProcessor
  {
  public:
    using WarningList = std::list<Warning>;
    using RulesIDs = std::set<std::string>;

    virtual ~SarifOutputProcessor() = default;
    nlohmann::ordered_json operator()(const WarningList &warnings) const;

  protected:
    struct Region
    {
      unsigned int startLine;
      unsigned int endLine;
      unsigned int startColumn;
      unsigned int endColumn;
    };
    
    static std::string UriFileEscape(std::string filePath);
    static nlohmann::ordered_json MakeMessageJson(const std::string &message);
    static nlohmann::ordered_json MakeLocationJson(const std::string &uri,
                                                   Region region,
                                                   const std::string &message = {});

    virtual nlohmann::ordered_json ProcessRule(const Warning &warning, nlohmann::ordered_json &rules, RulesIDs &rulesIDs) const;
    virtual nlohmann::ordered_json GetDriverJson(nlohmann::ordered_json rules) const;
    virtual std::vector<nlohmann::ordered_json> GetLocations(const Warning &warning) const;
    virtual std::vector<nlohmann::json> GetRelatedLocations(const Warning &warning) const;
  };

class SarifOutput;
template<>
constexpr std::string_view GetFormatName<SarifOutput>() noexcept
{
  return "sarif";
}

class SarifOutput : public IOutput
{
  std::list<Warning> m_warnings;
public:
  explicit SarifOutput(const ProgramOptions &opt);
  void Start() override {}
  bool Write(const Warning& msg) override;
  void Finish() override;
  void Finish(const SarifOutputProcessor &proc);
  ~SarifOutput() override = default;

  [[nodiscard]]
  std::string_view GetFormatName() const noexcept override
  {
    return ::PlogConverter::GetFormatName<SarifOutput>();
  }
};

}

#endif // SARIFOUTPUT_H
