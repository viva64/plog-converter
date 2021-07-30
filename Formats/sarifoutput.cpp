//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2021 (c) PVS-Studio LLC

#include <filesystem>
#include <optional>

#include "sarifoutput.h"
#include "PVS-StudioVersionInfo.h"

namespace stdfs = std::filesystem;

namespace PlogConverter
{

inline std::string to_hex(unsigned char x)
{
  std::string hex;
  hex.resize(2);
  snprintf(hex.data(), hex.size() + 1, "%2X", static_cast<uint32_t>(x));
  return std::string{"%"} + (x < 0x10 ? "0" : "") + hex;
}

std::string UriFileEscape(const std::string &filePath)
{
  using UriCache_t = std::unordered_map<stdfs::path::string_type, std::string>;
  static UriCache_t uriCache;

  auto path = stdfs::path{ filePath }.make_preferred();
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
    std::string str = element.string();
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
  cacheIter->second = output + Join(path, escapeElement, "");

  return cacheIter->second;
}

void SarifOutput::Write(const Warning &msg)
{
  if (msg.IsDocumentationLinkMessage())
  {
    return;
  }

  m_warnings.push_back(msg);
}

nlohmann::ordered_json MessageJson(const std::string &message)
{
  return
  { "message",
    { { "text", message } }
  };
}

struct Region
{
  unsigned int startLine;
  unsigned int endLine;
  unsigned int startColumn;
  unsigned int endColumn;
};

nlohmann::ordered_json LocationJson(const std::string &uri, const Region &region,
                                    const std::optional<std::string> &message = {})
{
  nlohmann::ordered_json location;

  if (message)
  {
    location = nlohmann::ordered_json::object({ MessageJson(*message) });
  }

  location["physicalLocation"] =
  {
    { "artifactLocation",
      nlohmann::ordered_json::object({ { "uri", uri } })
    },
    { "region",
      {
        { "startLine", region.startLine },
        { "endLine", region.endLine },
        { "startColumn", region.startColumn },
        { "endColumn", region.endColumn }
      }
    }
  };

  return location;
}

struct PvsStudioSarifRun
{
  nlohmann::ordered_json rules;
  nlohmann::ordered_json results;

  PvsStudioSarifRun(const std::list<Warning> &warnings) //-V826
  {
    for (auto&& warning : warnings)
    {
      // Add "rules" section

      if (ruleIds.insert(warning.code).second)
      {
        nlohmann::ordered_json rule
        {
          { "id", warning.code },
          { "name", "Rule" + warning.code },
          { "help",
            { { "text", warning.GetVivaUrl() } }
          },
          { "helpUri", warning.GetVivaUrl() }
        };

        if (warning.HasCWE())
        {
          rule["properties"] = {
            { "tags",
              { "security", "external/cwe/cwe-" + std::to_string(warning.cwe) }
            }
          };
        }

        rules.push_back(rule);
      }

      // Add "results" section

      nlohmann::ordered_json locations
      {
        LocationJson(UriFileEscape(warning.GetFileUTF8()),
                     { warning.GetLine(), warning.GetEndLine(), warning.GetStartColumn(), warning.GetEndColumn() }),
      };

      nlohmann::ordered_json result
      {
        { "ruleId", warning.code },
        MessageJson(warning.message),
        { "level", warning.GetLevelString() },
        { "locations", std::vector{ std::move(locations) } }
      };

      if (auto size = warning.positions.size(); size > 1)
      {
        std::vector<nlohmann::json> relatedLocations;

        // todo [c++20]: ranges
        for (size_t i = 1; i < size; ++i)
        {
          auto&& position = warning.positions[i];
          auto positionFile = position.file;
          ANSItoUTF8(positionFile);

          relatedLocations.push_back(LocationJson(UriFileEscape(positionFile), //-V823
                                                  { position.line, position.endLine, position.column, position.endColumn },
                                                  warning.message));
        }

        result["relatedLocations"] = std::move(relatedLocations);
      }

      results.push_back(std::move(result));
    }
  }

  nlohmann::ordered_json GetDriverJson()
  {
    nlohmann::ordered_json driver
    {
      { "name", "PVS-Studio" },
      { "semanticVersion", IDS_APP_VERSION },
      { "informationUri", "https://pvs-studio.com" },
      { "rules", std::move(rules) }
    };

    return
    {
      { "driver", std::move(driver) }
    };
  }

private:
  std::set<std::string> ruleIds;
};

void SarifOutput::Finish()
{
  PvsStudioSarifRun pvsStudioRun { m_warnings };

  nlohmann::ordered_json run
  {
    { "tool", pvsStudioRun.GetDriverJson() },
    { "results", std::move(pvsStudioRun.results) }
  };

  nlohmann::ordered_json sarif
  {
    { "version", "2.1.0" },
    { "$schema", "https://raw.githubusercontent.com/oasis-tcs/sarif-spec/master/Schemata/sarif-schema-2.1.0.json" },
    { "runs", std::vector { std::move(run) } }
  };

  m_ofstream << sarif.dump(2) << std::endl;
}

}
