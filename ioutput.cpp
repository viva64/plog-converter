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

static std::basic_ostream<char> &OfstreamOrStdOut(std::ofstream &of, bool useStdStream, bool useStdErr)
{
  if (useStdStream)
  {
    return useStdErr ? std::cerr : std::cout;
  }

  return of;
}

IOutput::IOutput(const ProgramOptions &options, const std::string &extension, bool isFile)
  : m_ostream(OfstreamOrStdOut(m_ofstream, options.output.empty() && options.outputName.empty(), options.useStderr))
  , m_output(options.output)
{
  using namespace std::literals;

  if (options.formats.size() > 1)
  {
    std::filesystem::path fileName;
    if (options.outputName.empty())
    {
      if (options.inputFiles.size() == 1)
      {
        fileName = std::filesystem::path{ options.inputFiles.front() }.filename();
      }
      else
      {
        fileName = "MergedReport."s;
      }
    }
    else
    {
      fileName = options.outputName.back() == '.' ? options.outputName : options.outputName + "."s;
    }
    fileName.replace_extension(extension);
    m_output /= fileName;
  }
  else if (!options.outputName.empty())
  {
    m_output /= options.outputName + "."s + extension;
  }

  if (isFile)
  {
    if (!m_output.empty())
    {
      m_ofstream.open(m_output);
      if (!m_ofstream.is_open())
      {
        throw FilesystemException("Can't write to file: "s + m_output.string());
      }
    }
  }
  else
  {
    if (m_output.empty())
    {
      throw FilesystemException("No output directory for report");
    }

    if (std::filesystem::exists(m_output))
    {
      throw FilesystemException("Output directory already exists: " + m_output.string());
    }

    if (!std::filesystem::create_directory(m_output))
    {
      throw FilesystemException("Couldn't create directory for report: " + m_output.string());
    }
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
