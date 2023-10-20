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
#include <string_view>
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
#include "sourcetreerootremover.h"
#include "leveltransform.h"
#include "utils.h"

#include "Formats/jsonoutput.h"
#include "Formats/htmloutput.h"
#include "Formats/misracomplianceoutput.h"
#include "Formats/errorfileoutput.h"
#include "Formats/errorfileverboseoutput.h"
#include "Formats/tasklistoutput.h"
#include "Formats/tasklistverboseoutput.h"
#include "Formats/gitlaboutput.h"

namespace PlogConverter
{

  InputFile::InputFile(std::string path_)
    : m_stream(OpenFile(path_))
    , m_path(std::move(path_))
  {}

  bool InputFile::operator<(const InputFile &other) const noexcept
  {
    return m_path < other.m_path;
  }

  LogParserWorker::~LogParserWorker() = default;

  void LogParserWorker::OnWarning(const Warning &warning)
  {
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
  m_root   = root;

  output.Start();
  WarningsLogContent outputContent{};

  for (auto &inputFile : inputFiles)
  {
    if (EndsWith(inputFile.Path(), ".json"))
    {
      ParseJsonLog(inputFile, outputContent);
    }
    else if (EndsWith(inputFile.Path(), ".cerr"))
    {
      ParseCerrLog(inputFile, outputContent);
    }
    else
    {
      ParseRawLog(inputFile, outputContent);
    }
  }

  for (auto &warning : outputContent)
  {
    OnWarning(warning);
  }

  output.Finish();
}

void LogParserWorker::ParseRawLog(InputFile &file, WarningsLogContent &warnings)
{
  if (IsXmlFile(file.Stream()))
  {
    using namespace std::literals;
    static constexpr auto message =
#ifdef _WIN32
      "This tool does not support analyzer reports in XML format. Please use the PlogConverter.exe tool instead."sv;
#else
      "This tool does not support analyzer reports in XML format. Please save analyzer report in JSON format."sv;
#endif
    
    std::cout << message << std::endl;
    Warning warning{};
    warning.message = message;
    warnings.emplace(std::move(warning));
    return;
  }

  bool decode = false;

  while (std::getline(file.Stream(), m_line))
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

    Warning parsedWarning{};
    m_messageParser.Parse(m_line, parsedWarning);

    for (auto &position : parsedWarning.positions)
    {
      UTF8toANSI(position.file);
    }

    if (auto notLess = warnings.lower_bound(parsedWarning);
        notLess == warnings.end() || warnings.key_comp()(parsedWarning, *notLess))
    {
      warnings.emplace_hint(notLess, std::move(parsedWarning));
    }
    else if (parsedWarning < *notLess)
    {
      auto pos     = std::next(notLess);
      auto node    = warnings.extract(notLess);
      node.value() = std::move(parsedWarning);
      warnings.insert(pos, std::move(node));
    }
  }
}

void LogParserWorker::ParseCerrLog(InputFile& file, WarningsLogContent &warnings)
{
  std::smatch match;
  std::regex re("(.+):([0-9]+):([0-9]+): *(note|warn|warning|error) *: *(.*) *");

  while (std::getline(file.Stream(), m_line))
  {
    if (std::regex_search(m_line, match, re) && match.size() == 6)
    {
      Warning parsedWarning{};
      m_messageParser.Parse(match.str(1),
                            match.str(2),
                            match.str(4),
                            match.str(5),
                            parsedWarning);

      for (auto &position : parsedWarning.positions)
      {
        UTF8toANSI(position.file);
      }

      if (auto notLess = warnings.lower_bound(parsedWarning);
          notLess == warnings.end() || warnings.key_comp()(parsedWarning, *notLess))
      {
        warnings.emplace_hint(notLess, std::move(parsedWarning));
      }
      else if (parsedWarning < *notLess)
      {
        auto pos = std::next(notLess);
        auto node = warnings.extract(notLess);
        node.value() = std::move(parsedWarning);
        warnings.insert(pos, std::move(node));
      }
    }
  }
}

struct JsonDocument
{
  std::deque<Warning> warnings;

  template <typename Stream>
  void Serialize(Stream &stream)
  {
    stream.Required("warnings", warnings);
  }
};

void LogParserWorker::ParseJsonLog(InputFile &file, WarningsLogContent &warnings)
{
  WarningJsonExtractor warningJsonExtractor{ warnings };
  
  auto j = nlohmann::json::parse(file.Stream(), warningJsonExtractor);

  // TODO: Remove JsonDocument
  JsonDocument doc;
  doc = j; //-V1001
}

void LogParserWorker::Run(const ProgramOptions &optionsSrc)
{
  auto options = optionsSrc;

  std::vector<InputFile> inputFiles;
  for (const auto &path : options.inputFiles)
  {
    inputFiles.emplace_back(path);
  }

  std::sort(inputFiles.begin(), inputFiles.end());

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
  std::unique_ptr<HTMLOutput, std::default_delete<BaseFormatOutput>> fullHtmlOutput;

  auto generateOutput = [&options](auto ptr) -> IOutput<PlogConverter::Warning>*
  {
    if (auto base = dynamic_cast<ISupportsRelativePath *>(ptr.get()))
    {
      if (base->SupportsRelativePath_())
      {
        return new SourceRootRemover(std::move(ptr), options);
      }
      else
      {
        return new SourceRootChecker(std::move(ptr), options);
      }
    }
    return nullptr;
  };

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
    else
    {
      if (!options.noHelp && (   IsA<ErrorFileOutput>(f) || IsA<ErrorFileVerboseOutput>(f)
                              || IsA<TaskListOutput>(f)  || IsA<TaskListVerboseOutput>(f)))
      {  
        auto help = std::make_unique<HelpMessageOutput>(std::move(f));
        pathFilterPipeline.Add(std::unique_ptr<IOutput<Warning>>(generateOutput(std::move(help))));
      }
      else if (IsA<HTMLOutput>(f))
      {
        pathFilterPipeline.Add(std::make_unique<LevelTransform>(std::unique_ptr<IOutput<Warning>>(generateOutput(std::move(f))), options));
      }
      else
      {
        pathFilterPipeline.Add(std::unique_ptr<IOutput<Warning>>(generateOutput(std::move(f))));
      }
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
    noTransformPipeline.Add(std::make_unique<SourceRootRemover>(std::move(gitlabOutput), options));
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
    output.Add(std::make_unique<MessageFilter>(&filterPipeline, options));
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
