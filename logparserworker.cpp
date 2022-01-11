//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2022 (c) PVS-Studio LLC

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

#include "application.h"
#include "charmap.h"
#include "logparserworker.h"
#include "messagefilter.h"
#include "multipleoutput.h"
#include "outputfactory.h"
#include "sourceroottransformer.h"
#include "utils.h"

#include "Formats/jsonoutput.h"
#include "Formats/misracomplianceoutput.h"

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
                               IOutput &output,
                               const std::string &root,
                               const ProgramOptions *options /* = nullptr */)
{
  m_output = &output;
  m_root = root;

  output.Start();
  if (!options || !options->noHelp)
  {
    output.Write(Warning::GetDocumentationLinkMessage());
  }

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
    if (!options.output.empty())
    {
      options.output = ".";
    }

    if (!Exists(options.output) && !MakeDirectory(options.output))
    {
      throw std::runtime_error("Couldn't create directory: " + options.output);
    }
  }

  std::unique_ptr<MisraComplianceOutput, std::default_delete<IOutput>> misraCompliance;
  std::unique_ptr<JsonOutput, std::default_delete<IOutput>> jsonOutput;

  MultipleOutput transformPipeline;
  for (const auto &format : formats)
  {
    auto f = format(options);
    if (IsA<MisraComplianceOutput>(f))
    {
      misraCompliance = UnsafeTo<MisraComplianceOutput>(std::move(f));
    }
    else if (optionsSrc.projectRoot.empty() && IsA<JsonOutput>(f))
    {
      jsonOutput = UnsafeTo<JsonOutput>(std::move(f));
    }
    else
    {
      transformPipeline.Add(std::move(f));
    }
  }

  MultipleOutput filterPipeline;

  if (!transformPipeline.empty())
  {
    filterPipeline.Add(std::make_unique<SourceRootTransformer>( &transformPipeline, options ));
  }

  if (jsonOutput)
  {
    filterPipeline.Add(std::move(jsonOutput));
  }

  MultipleOutput output;
  if (!filterPipeline.empty())
  {
    output.Add(std::make_unique<MessageFilter>( &filterPipeline, options ));
  }

  if (misraCompliance)
  {
    output.Add(std::make_unique<SourceRootTransformer>(misraCompliance.get(), options));
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

  ParseLog(inputFiles, output, options.projectRoot, &options);

  std::cout << "Total messages: " << m_countTotal << std::endl
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

}
