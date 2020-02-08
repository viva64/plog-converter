//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"

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
#include "utils.h"

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
    Replace(position.file, "|?|", m_root);
  }

  if (m_filter == nullptr || m_filter->Check(warning))
  {
    if (m_output != nullptr)
    {
      m_output->Write(warning);
    }
    
    ++m_countSuccess;

    if (warning.IsRenewMessage())
    {
      ++m_countNonError;
    }
  }

  ++m_countTotal;
}

void LogParserWorker::ParseLog(std::vector<InputFile> &inputFiles,
                               IMessageFilter& filter,
                               IOutput &output,
                               const std::string &root)
{
  m_filter = &filter;
  m_output = &output;
  m_root = root;

  output.Start();
  output.Write(Warning::GetDocumentationLinkMessage());

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

  MessageFilter filter(options);

  MultipleOutput output;

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

  for (const auto &format : formats)
  {
    output.Add(format(options));
  }

  ParseLog(inputFiles, filter, output, options.projectRoot);

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
