//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2022 (c) PVS-Studio LLC

#include "sarifvscodeoutput.h"

namespace PlogConverter
{

class SarifVSCodeOutputProcessor : public SarifOutputProcessor
{
public:
  std::vector<nlohmann::ordered_json> GetLocations(const Warning &warning) const override;
  std::vector<nlohmann::json> GetRelatedLocations(const Warning &warning) const override;
};

SarifVSCodeOutput::SarifVSCodeOutput(const ProgramOptions& opt) : SarifOutputImpl<SarifVSCodeOutput>{ opt }
{
}

void SarifVSCodeOutput::Finish()
{
  SarifOutputImpl::Finish(SarifVSCodeOutputProcessor{});
}

std::vector<nlohmann::ordered_json> SarifVSCodeOutputProcessor::GetLocations(const Warning &warning) const
{
  std::vector<nlohmann::ordered_json> locations;
  for (auto &position : warning.positions)
  {
    auto positionFile = position.file;
    ANSItoUTF8(positionFile);

    locations.emplace_back(MakeLocationJson(UriFileEscape(std::move(positionFile)),
      {
        position.line,
        position.endLine,
        position.column,
        position.endColumn
      }));
  }
  return locations;
}

std::vector<nlohmann::json> SarifVSCodeOutputProcessor::GetRelatedLocations(const Warning &) const
{
  return {};
}

}
