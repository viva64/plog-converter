//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2022 (c) PVS-Studio LLC

#include "ioutput.h"
#include "utils.h"
#include <filesystem>

namespace PlogConverter
{

void IOutput::Start() {}
void IOutput::Finish() {}

static std::basic_ostream<char> &OfstreamOrStdOut(std::ofstream &of, bool useStdErr, const std::string &path)
{
  if (path.empty())
  {
    return useStdErr ? std::cerr : std::cout;
  }

  return of;
}

IOutput::IOutput(const ProgramOptions &options, const std::string &extension)
  : m_ostream(OfstreamOrStdOut(m_ofstream, options.useStderr, options.output))
{
  if (!options.output.empty())
  {
    std::string output = options.output;
    if (options.outputIsDirectory)
    {
      if (   !std::filesystem::exists(output)
          && !std::filesystem::create_directory(output))
      {
        throw std::runtime_error("Couldn't create directory: " + output);
      }

      output += '/';
      output += options.outputName.empty() ? FileStem(FileBaseName(options.inputFiles.at(0)))
                                           : options.outputName;
      if (output.back() != '.')
      {
        output += '.';
      }

      output += extension;
    }
    else if (options.formats.size() > 1)
    {
      if (output.back() != '.')
      {
        output += '.';
      }

      output += extension;
    }

    m_ofstream.open(output);
    if (!m_ofstream.is_open())
    {
      throw FilesystemException("Can't write to file: " + output);
    }

    m_output = output;
  }
  m_errorCodeMappings = options.codeMappings;
}

IOutput::IOutput() : m_ostream(std::cout)
{
}

IOutput::IOutput(const std::string &path) : m_ostream(m_ofstream)
{
  if (path.empty())
  {
    throw FilesystemException("Empty filepath");
  }

  m_ofstream.open(path);
  if (!m_ofstream.is_open())
  {
    throw FilesystemException("Can't write to file: " + path);
  }
}

void IOutput::DetectShowTags(bool &showCWE, bool &showSAST) const
{
  showSAST = false;
  showCWE = false;

  for (const auto& security : m_errorCodeMappings)
  {
    if (security == SecurityCodeMapping::MISRA || security == SecurityCodeMapping::AUTOSAR || security == SecurityCodeMapping::OWASP)
    {
      showSAST = true;
    }
    if (security == SecurityCodeMapping::CWE)
    {
      showCWE = true;
    }
  }
}

[[nodiscard]] bool IOutput::IsSupportRelativePath() const noexcept
{
  return m_isSupportRelativePath;
}

[[nodiscard]] std::string_view IOutput::GetFormatName() const noexcept
{
  return ::PlogConverter::GetFormatName<IOutput>();
}

void IOutput::ClearOutput() &&
{
  if (m_ofstream.is_open())
  {
    m_ofstream.close();
  }

  if (std::filesystem::is_empty(m_output))
  {
    std::filesystem::remove(m_output);
  }
}

}
