//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2023 (c) PVS-Studio LLC

#include <stdexcept>

#include "Formats/xmloutput.h"
#include "Formats/csvoutput.h"
#include "Formats/errorfileoutput.h"
#include "Formats/errorfileverboseoutput.h"
#include "Formats/tasklistoutput.h"
#include "Formats/tasklistverboseoutput.h"
#include "Formats/htmloutput.h"
#include "Formats/simplehtml.h"
#include "Formats/teamcityoutput.h"
#include "Formats/sarifoutput.h"
#include "Formats/sarifvscodeoutput.h"
#include "Formats/misracomplianceoutput.h"
#include "Formats/jsonoutput.h"
#include "Formats/totalsoutput.h"
#include "Formats/gitlaboutput.h"
#include "Formats/defectdojooutput.h"
#include "outputfactory.h"

namespace PlogConverter
{

template <typename T>
static OutputFactory::AllocFunction FactoryFunction()
{
  return [](const ProgramOptions &opt) { return std::make_unique<T>(opt); };
}

template<typename T>
std::pair<std::string_view, OutputFactory::AllocFunction> CreateOutputFormatRecord()
{
  return { T::FormatName(), FactoryFunction<T>() };
}

OutputFactory::OutputFactory()
  : m_formats {
      CreateOutputFormatRecord<CSVOutput>(),
      CreateOutputFormatRecord<XMLOutput>(),
      CreateOutputFormatRecord<HTMLOutput>(),
      CreateOutputFormatRecord<ErrorFileOutput>(),
      CreateOutputFormatRecord<ErrorFileVerboseOutput>(),
      CreateOutputFormatRecord<TaskListOutput>(),
      CreateOutputFormatRecord<TaskListVerboseOutput>(),
      CreateOutputFormatRecord<SimpleHTMLOutput>(),
      CreateOutputFormatRecord<TeamCityOutput>(),
      CreateOutputFormatRecord<SarifOutput>(),
      CreateOutputFormatRecord<SarifVSCodeOutput>(),
      CreateOutputFormatRecord<MisraComplianceOutput>(),
      CreateOutputFormatRecord<JsonOutput>(),
      CreateOutputFormatRecord<TotalsOutput>(),
      CreateOutputFormatRecord<GitLabOutput>(),
      CreateOutputFormatRecord<DefectDojoOutput>()
    }
{
}

std::unique_ptr<BaseFormatOutput> OutputFactory::createOutput(const ProgramOptions &os, const std::string &format)
{
  auto it = m_formats.find(format);
  if (it == m_formats.end())
  {
    throw std::runtime_error("Incorrect format given");
  }

  return std::unique_ptr<BaseFormatOutput>(it->second(os));
}

void OutputFactory::registerOutput(std::string_view format, AllocFunction f)
{
  if (!f)
  {
    throw std::invalid_argument("Factory function is nullptr");
  }

  auto &fn = m_formats[format];
  if (fn)
  {
    throw std::runtime_error("Format with such name already registered");
  }

  fn = std::move(f);
}

}
