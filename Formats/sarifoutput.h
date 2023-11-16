//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2023 (c) PVS-Studio LLC

#pragma once

#include <list>

#include "ioutput.h"
#include "utils.h"

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

  static const std::string relativePathPrefixWithDot;

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

template <class Derived = void>
class SarifOutputImpl : public BasicFormatOutput<std::conditional_t<std::is_void_v<Derived>, SarifOutputImpl<>, Derived>>
{
  using Base = BasicFormatOutput<std::conditional_t<std::is_void_v<Derived>, SarifOutputImpl<>, Derived>>;
  std::list<Warning> m_warnings;

public:
  explicit SarifOutputImpl(const ProgramOptions &opt) : Base{ opt }
  {
  }
  ~SarifOutputImpl() override = default;

  void Start() override {}
  bool Write(const Warning& msg) override
  {
    m_warnings.push_back(msg);

    return true;
  }
  void Finish() override
  {
    Finish(SarifOutputProcessor{});
  }
  void Finish(const SarifOutputProcessor &proc)
  {
    Base::m_ostream << proc(m_warnings).dump(2) << std::endl;
    BasicFormatOutput<std::conditional_t<std::is_void_v<Derived>, SarifOutputImpl<>, Derived>>::Finish();
  }

  [[nodiscard]]
  static bool SupportsRelativePath() noexcept
  {
    return true;
  }

  [[nodiscard]]
  static bool OutputIsFile() noexcept
  {
    return true;
  }

  [[nodiscard]]
  static std::string_view FormatName() noexcept
  {
    return "sarif";
  }

  [[nodiscard]]
  static std::string_view OutputSuffix() noexcept
  {
    return FormatName();
  }
};

using SarifOutput = SarifOutputImpl<>;

}
