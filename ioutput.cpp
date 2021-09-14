//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2021 (c) PVS-Studio LLC

#include "ioutput.h"
#include "utils.h"

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

IOutput::IOutput(const ProgramOptions &options, const std::string &extension) : m_ostream(OfstreamOrStdOut(m_ofstream, options.useStderr, options.output))
{
  if (!options.output.empty())
  {
    std::string output = options.output;
    if (IsDirectory(output))
    {
      output += '/';
      output += options.outputName.empty() ? FileStem(FileBaseName(options.inputFiles.at(0))) : options.outputName;
      output += '.';
      output += extension;
    }

    m_ofstream.open(output);
    if (!m_ofstream.is_open())
    {
      throw FilesystemException("Can't write to file: " + output);
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

void IOutput::DetectShowTags(bool* showCWE, bool* showSAST) const
{
  *showSAST = false;
  *showCWE = false;

  for (const auto& security : m_errorCodeMappings)
  {
    if (security == SecurityCodeMapping::MISRA || security == SecurityCodeMapping::AUTOSAR || security == SecurityCodeMapping::OWASP)
    {
      *showSAST = true;
    }
    if (security == SecurityCodeMapping::CWE)
    {
      *showCWE = true;
    }
  }
}

}
