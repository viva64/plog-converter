//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2023 (c) PVS-Studio LLC

#include <filesystem>
#include <sys/types.h>
#include <sys/stat.h>

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <iterator>
#include <string>
#include <unordered_set>
#include <regex>

#include "PathFilter.h"
#include "application.h"
#include "charmap.h"
#include "helpmessageoutput.h"
#include "logparserworker.h"
#include "messagefilter.h"
#include "multipleoutput.h"
#include "outputfactory.h"
#include "sourceroottransformer.h"
#include "utils.h"

#include "Formats/jsonoutput.h"
#include "Formats/misracomplianceoutput.h"
#include "Formats/errorfileoutput.h"
#include "Formats/errorfileverboseoutput.h"
#include "Formats/tasklistoutput.h"
#include "Formats/tasklistverboseoutput.h"
#include "Formats/gitlaboutput.h"

namespace PlogConverter
{

  InputFile::InputFile(std::string path_)
    : path(std::move(path_)), stream(OpenFile(path))
  {
  }

  LogParserWorker::~LogParserWorker() = default;

  void LogParserWorker::OnWarning(Warning &warning)
  {
    for (auto &position : warning.positions)
    {
      UTF8toANSI(position.file);
    }

    if (m_output && m_output->Write(warning))
    {
      ++m_countSuccess;
    }

    if (warning.IsRenewMessage())
    {
      ++m_countNonError;
    }

    ++m_countTotal;
}

void LogParserWorker::ParseLog(std::vector<InputFile> &inputFiles,
                               IOutput<Warning> &output,
                               const std::string &root)
{
  m_output = &output;
  m_root = root;

  output.Start();

  for (auto &inputFile : inputFiles)
  {
    if (EndsWith(inputFile.path, ".json"))
    {
      ParseJsonLog(inputFile);
    }
    else if (EndsWith(inputFile.path, ".cerr"))
    {
      ParseCerrLog(inputFile);
    }
    else
    {
      ParseRawLog(inputFile);
    }
  }

  output.Finish();
}

void LogParserWorker::ParseRawLog(InputFile &file)
{
  if (IsXmlFile(file.stream))
  {
    using namespace std::literals;
    static constexpr auto message =
#ifdef _WIN32
      "This tool does not support analyzer reports in XML format. Please use the PlogConverter.exe tool instead."sv;
#else
      "This tool does not support analyzer reports in XML format. Please save analyzer report in JSON format."sv;
#endif
    
    std::cout << message << std::endl;
    m_warning.Clear();
    m_warning.message = message;
    OnWarning(m_warning);
    return;
  }

  bool decode = false;

  while (std::getline(file.stream, m_line))
  {
    if (CharMap::IsStartEncodedMarker(m_line))
    {
      decode = true;
      continue;
    }

    if (m_line.empty() || m_line[0] == '#')
    {
      continue;
    }

    if (decode)
    {
      CharMap::Decode(m_line);
    }

    auto it = m_hashTable.emplace(std::move(m_line));
    if (it.second)
    {
      m_messageParser.Parse(*it.first, m_warning);
      OnWarning(m_warning);
    }
  }
}

void LogParserWorker::ParseCerrLog(InputFile& file)
{
  std::smatch match;
  std::regex re("(.+):([0-9]+):([0-9]+): *(note|warn|warning|error) *: *(.*) *");

  while (std::getline(file.stream, m_line))
  {
    if (std::regex_search(m_line, match, re) && match.size() == 6)
    {
      auto it = m_hashTable.emplace(std::move(m_line));
      if (it.second)
      {
        m_messageParser.Parse(
          match.str(1),
          match.str(2),
          match.str(4),
          match.str(5),
          m_warning);
        OnWarning(m_warning);
      }
    }
  }
}

struct JsonDocument
{
  std::vector<Warning> warnings;

  template <typename Stream>
  void Serialize(Stream &stream)
  {
    stream.Required("warnings", warnings);
  }
};

void LogParserWorker::ParseJsonLog(InputFile &file)
{
  nlohmann::json j;
  JsonDocument doc;

  file.stream >> j;
  doc = j;

  for (auto &warning : doc.warnings)
  {
    OnWarning(warning);
  }
}

void LogParserWorker::Run(const ProgramOptions &optionsSrc)
{
  auto options = optionsSrc;

  std::vector<InputFile> inputFiles;
  for (const auto &path : options.inputFiles)
  {
    inputFiles.emplace_back(path);
  }

  auto &formats = options.formats;
  if (formats.empty())
  {
    throw std::logic_error("Render type is required");
  }
  else if (formats.size() > 1)
  {
    std::filesystem::path outDir = options.output.empty() ? "." : options.output;

    if (   !std::filesystem::exists(outDir)
        && !std::filesystem::create_directory(outDir))
    {
      throw std::runtime_error("Couldn't create directory: " + outDir.string());
    }
  }

  std::unique_ptr<MisraComplianceOutput, std::default_delete<BaseFormatOutput>> misraCompliance;
  std::unique_ptr<JsonOutput,   std::default_delete<BaseFormatOutput>> jsonOutput;
  std::unique_ptr<GitLabOutput, std::default_delete<BaseFormatOutput>> gitlabOutput;

  MultipleOutput<Warning> pathFilterPipeline;
  for (const auto &format : formats)
  {
    auto f = format(options);

    if (!CheckUnsupporterdTransformation(f, options))
    {
      std::move(*f).ClearOutput(true);
      continue;
    }

    if (IsA<MisraComplianceOutput>(f))
    {
      misraCompliance = UnsafeTo<MisraComplianceOutput>(std::move(f));
    }
    else if (optionsSrc.projectRoot.empty() && IsA<JsonOutput>(f))
    {
      jsonOutput = UnsafeTo<JsonOutput>(std::move(f));
    }
    else if (optionsSrc.projectRoot.empty() && IsA<GitLabOutput>(f))
    {
      gitlabOutput = UnsafeTo<GitLabOutput>(std::move(f));
    }
    else if (!options.noHelp && (   IsA<ErrorFileOutput>(f) || IsA<ErrorFileVerboseOutput>(f) 
                                 || IsA<TaskListOutput> (f) || IsA<TaskListVerboseOutput> (f)))
    {
      pathFilterPipeline.Add(std::make_unique<HelpMessageOutput>(std::move(f)));
    }
    else
    {
      pathFilterPipeline.Add(std::move(f));
    }
  }

  MultipleOutput<Warning> transformPipeline;
  if (!pathFilterPipeline.empty())
  {
    transformPipeline.Add(std::make_unique<PathFilter>(&pathFilterPipeline, options));
  }
  
  MultipleOutput<Warning> noTransformPipeline;
  if (jsonOutput)
  {
    noTransformPipeline.Add(std::move(jsonOutput));
  }
  if (gitlabOutput)
  {
    noTransformPipeline.Add(std::move(gitlabOutput));
  }

  MultipleOutput<Warning> filterPipeline;
  if (!transformPipeline.empty())
  {
    filterPipeline.Add(std::make_unique<SourceRootTransformer>(&transformPipeline, options));
  }

  if (!noTransformPipeline.empty())
  {
    filterPipeline.Add(std::make_unique<PathFilter>(&noTransformPipeline, options));
  }

  MultipleOutput<Warning> output;
  if (!filterPipeline.empty())
  {
    output.Add(std::make_unique<MessageFilter>( &filterPipeline, options ));
  }

  MultipleOutput<Warning> misraPathFilterPipline;
  if (misraCompliance)
  {
    misraPathFilterPipline.Add(std::make_unique<PathFilter>(misraCompliance.get(), options));
    output.Add(std::make_unique<SourceRootTransformer>(&misraPathFilterPipline, options));
  }
  else
  {
    if (!options.grp.empty())
    {
      std::cout << "The use of the 'grp' flag is valid only for the 'misra' format. Otherwise, it will be ignored." << std::endl;
    }

    if (!options.misraDivations.empty())
    {
      std::cout << "The use of the 'misra_deviations' flag is valid only for the 'misra' format. Otherwise, it will be ignored." << std::endl;
    }
  }

  ParseLog(inputFiles, output, options.projectRoot);

  std::cout << "Total messages: " << m_countTotal << '\n'
            << "Filtered messages: " << m_countSuccess << std::endl;
}

size_t LogParserWorker::GetTotalWarnings() const
{
  return m_countTotal;
}

size_t LogParserWorker::GetPrintedWarnings() const
{
  return std::max(m_countSuccess, m_countNonError) - m_countNonError;
}

[[nodiscard]]
bool LogParserWorker::IsErrorHappend() const noexcept
{
  return m_isUnsopporterdTransformationErrorHappend;
}

}
