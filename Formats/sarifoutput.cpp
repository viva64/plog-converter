//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2023 (c) PVS-Studio LLC

#include <filesystem>
#include <optional>

#include "sarifoutput.h"
#include "PVS-StudioVersionInfo.h"

namespace stdfs = std::filesystem;
using namespace std::string_literals;
using namespace std::string_view_literals;

namespace
{
  inline std::string to_hex(unsigned char x)
  {
    std::string hex;
    hex.resize(2);
    snprintf(hex.data(), hex.size() + 1, "%2X", static_cast<uint32_t>(x));
    return std::string{ "%" } + (x < 0x10 ? "0" : "") + hex;
  }
}

namespace PlogConverter
{

nlohmann::ordered_json SarifOutputProcessor::operator()(const WarningList &warnings) const
{
  nlohmann::ordered_json rules;
  nlohmann::ordered_json results;
  std::set<std::string> ruleIDs;

  for (const auto &warning : warnings)
  {
    results.emplace_back(ProcessRule(warning, rules, ruleIDs));
  }

  nlohmann::ordered_json run
  {
    { "tool"sv, GetDriverJson(std::move(rules)) },
    { "results"sv, std::move(results) }
  };

  nlohmann::ordered_json sarif
  {
    { "version"sv, "2.1.0"sv },
    { "$schema"sv, "https://raw.githubusercontent.com/oasis-tcs/sarif-spec/master/Schemata/sarif-schema-2.1.0.json"sv },
    { "runs"sv, std::vector { std::move(run) } }
  };

  return sarif;
}

std::string SarifOutputProcessor::UriFileEscape(std::string filePath)
{
  using UriCache_t = std::unordered_map<stdfs::path::string_type, std::string>;
  static UriCache_t uriCache;

  auto path = stdfs::path{ std::move(filePath) }.make_preferred();
  auto [cacheIter, isNew] = uriCache.try_emplace(path.native(), "");
  if (!isNew)
  {
    return cacheIter->second;
  }

  constexpr auto isUnreserved = [](const char c)
  {
    // RFC 3986 section 2.3 Unreserved Characters(January 2005)
    // A B C D E F G H I J K L M N O P Q R S T U V W X Y Z
    // a b c d e f g h i j k l m n o p q r s t u v w x y z
    // 0 1 2 3 4 5 6 7 8 9 - _ . ~

    return (c >= '0' && c <= '9')
      || (c >= 'A' && c <= 'Z')
      || (c >= 'a' && c <= 'z')
      || c == '-'
      || c == '.'
      || c == '_'
      || c == '~';
  };

  // Works similar to the curl_easy_escape function from curl
  // https://curl.se/libcurl/c/curl_easy_escape.html
  // > ...encodes the data byte-by-byte into the URL encoded version
  // > without knowledge or care for what particular character encoding
  // > the application or the receiving server may assume that the data uses
  const auto escapeElement = [&isUnreserved](const stdfs::path &element) -> std::string
  {
    auto str = element.string();
    if (str.empty() || stdfs::path::preferred_separator == element.native().front())
    {
      return {};
    }

    std::string output;
    output.reserve(str.size() + 1);
    output.push_back('/');
    for (auto ch : str)
    {
      if (isUnreserved(ch))
      {
        output.push_back(ch);
      }
      else
      {
        output.append(to_hex(ch));
      }
    }

    return output;
  };

  std::string output;
  output.reserve(10);       // reserved for scheme and Windows drive letter
  output.append("file://");
  cacheIter->second = output + PlogConverter::Join(path, escapeElement, "");

  return cacheIter->second;
}

nlohmann::ordered_json SarifOutputProcessor::MakeMessageJson(const std::string &message)
{
  return
  { "message",
    { { "text", message } }
  };
}

nlohmann::ordered_json SarifOutputProcessor::MakeLocationJson(const std::string &uri, Region region, const std::string &message)
{
  nlohmann::ordered_json location;

  if (!message.empty())
  {
    location = nlohmann::ordered_json::object({ MakeMessageJson(message) });
  }

  location["physicalLocation"] =
  {
    { "artifactLocation",
      nlohmann::ordered_json::object({ { "uri", uri } })
    },
    { "region",
      {
        { "startLine",   region.startLine },
        { "endLine",     region.endLine },
        { "startColumn", region.startColumn },
        { "endColumn",   region.endColumn }
      }
    }
  };

  return location;
}

nlohmann::ordered_json SarifOutputProcessor::ProcessRule(const Warning &warning, nlohmann::ordered_json &rules, RulesIDs &rulesIDs) const
{
  // Add "rules" section
  if (rulesIDs.insert(warning.code).second)
  {
    const auto vivaUrl { warning.GetVivaUrl() };
    nlohmann::ordered_json rule
    {
      { "id"sv, warning.code },
      { "name"sv, std::string{ "Rule" } + warning.code },
      { "help"sv,
        { { "text"sv, vivaUrl } }
      },
      { "helpUri"sv, vivaUrl }
    };

    if (warning.HasCWE())
    {
      rule["properties"] = {
        { "tags"sv,
          { "security"sv, std::string{ "external/cwe/cwe-" } + std::to_string(warning.cwe) }
        }
      };
    }

    rules.emplace_back(std::move(rule));
  }

  // Add "results" section
  nlohmann::ordered_json result
  {
    { "ruleId"sv, warning.code },
    MakeMessageJson(warning.message),
    { "level"sv, warning.GetLevelString() },
    { "locations"sv, GetLocations(warning) }
  };

  if (warning.positions.size() > 1)
  {
    auto relatedLocations = GetRelatedLocations(warning);
    if (!relatedLocations.empty())
    {
      result["relatedLocations"s] = std::move(relatedLocations);
    }
  }
  return result;
}

nlohmann::ordered_json SarifOutputProcessor::GetDriverJson(nlohmann::ordered_json rules) const
{
  nlohmann::ordered_json driver
  {
    { "name"sv, "PVS-Studio"sv },
    { "semanticVersion"sv, IDS_APP_VERSION },
    { "informationUri"sv, "https://pvs-studio.com"sv },
    { "rules"sv, std::move(rules) }
  };

  return
  {
    { "driver"sv, std::move(driver) }
  };
}

std::vector<nlohmann::ordered_json> SarifOutputProcessor::GetLocations(const Warning &warning) const
{
  return
  {
    MakeLocationJson(UriFileEscape(warning.GetFileUTF8()),
                    {
                      warning.GetLine(),
                      warning.GetEndLine(),
                      warning.GetStartColumn(),
                      warning.GetEndColumn()
                    })
  };
}

std::vector<nlohmann::json> SarifOutputProcessor::GetRelatedLocations(const Warning &warning) const
{
  std::vector<nlohmann::json> relatedLocations;
  std::set<WarningPosition> uniquePositions;

  // TODO Check error. Standart says that 'relatedLocations' represents a locations relevant to understanding the result. But we pass all of them.
  uniquePositions.insert(warning.positions.begin(), warning.positions.end());

  for (const auto &position : uniquePositions)
  {
    auto positionFile = position.file;
    ANSItoUTF8(positionFile);

    relatedLocations.emplace_back(MakeLocationJson(UriFileEscape(std::move(positionFile)),
      {
        position.line,
        position.endLine,
        position.column,
        position.endColumn
      },
      warning.message));
  }
  return relatedLocations;
}

}
