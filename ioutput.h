//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2022 (c) PVS-Studio LLC

#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>

#include "configs.h"
#include "warning.h"
#include "utils.h"

namespace PlogConverter
{

template <typename T>
class IOutput
{
public:
  virtual ~IOutput() = default;

  virtual void Start() = 0;
  virtual bool Write(const T& message) = 0;
  virtual void Finish() = 0;
};

class BaseFormatOutput : public IOutput<Warning>
{
public:
  virtual ~BaseFormatOutput() = default;
  void ClearOutput() noexcept
  {
    m_buf.pubsync();
    if (auto& fstream = m_buf.m_ofstream)
    {
      if (fstream->is_open())
      {
        fstream->close();
      }

      std::error_code ignored;
      if (std::filesystem::is_empty(m_output, ignored))
      {
        std::filesystem::remove(m_output, ignored);
      }
    }
    if (m_buf.m_ostream)
    {
      m_buf.m_ostream->flush();
    }
  }

  virtual void Start() override {};
  virtual bool Write(const Warning& message) override = 0;
  virtual void Finish() override
  {
    ClearOutput();
  };

  [[nodiscard]] virtual bool SupportsRelativePath_() const noexcept = 0;
  [[nodiscard]] virtual bool OutputIsFile_() const noexcept = 0;
  [[nodiscard]] virtual std::string_view FormatName_() const noexcept = 0;
  [[nodiscard]] virtual std::string_view OutputSuffix_() const noexcept = 0;

protected:
  friend class OutputFactory;
  BaseFormatOutput() = default;

  void DetectShowTags(bool& showCWE, bool& showSAST) const
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

  template <size_t BufSize = BUFSIZ>
  struct streamsbuf : public std::streambuf
  {
    std::unique_ptr<char[]> obuf = std::make_unique<char[]>(BufSize);
    std::ostream* m_ostream = nullptr;
    std::unique_ptr<std::ofstream> m_ofstream{};

    streamsbuf()
    {
      setp(obuf.get(), obuf.get() + BufSize);
    }
    std::ofstream* AddOfstream(const std::filesystem::path& path)
    {
      m_ofstream = std::make_unique<std::ofstream>(path.string());
      return m_ofstream.get();
    }
    void SetOstream(std::ostream& stream)
    {
      m_ostream = &stream;
    }

  protected:
    virtual int overflow(int c) override
    {
      auto begin = pbase();
      auto size = pptr() - begin;

      int ret = traits_type::not_eof(c);

      auto overflowStreams = [&begin, &size, &c](std::ostream* s)
      {
        if (!s)
        {
          return true;
        }

        s->write(begin, size);
        if (c != traits_type::eof())
        {
          s->put(traits_type::to_char_type(c));
        }
        if (!s->good())
        {
          return false;
        }

        return true;
      };

      if (!overflowStreams(m_ostream) || !overflowStreams(m_ofstream.get()))
      {
        ret = traits_type::eof();
      }

      pbump(static_cast<int>(-size));
      return ret;
    }

    virtual int sync() override
    {
      auto begin = pbase();
      auto size = pptr() - begin;
      int ret = 0;

      auto syncStreams = [&begin, &size](std::ostream* s)
      {
        if (!s)
        {
          return true;
        }

        s->write(begin, size);
        s->flush();
        if (!s->good())
        {
          return false;
        }

        return true;
      };

      if (!syncStreams(m_ostream) || !syncStreams(m_ofstream.get()))
      {
        ret = -1;
      }

      pbump(static_cast<int>(-size));
      return ret;
    }
  };

  std::vector<SecurityCodeMapping> m_errorCodeMappings;
  std::filesystem::path m_output;
  streamsbuf<> m_buf;
  std::ostream m_ostream{ &m_buf };
};

template <typename FormatOutput>
class BasicFormatOutput : public BaseFormatOutput
{
public:
  BasicFormatOutput(const ProgramOptions& options) : BaseFormatOutput()
  {
    using namespace std::literals;

    auto useFile = !options.output.empty() || !options.outputName.empty();

    if (!useFile || options.useStdout)
    {
      if (!FormatOutput::OutputIsFile())
      {
        throw FilesystemException(std::string{ "Format '" }.append(FormatOutput::FormatName()) + "' can't be used for standard output.");
      }

      m_buf.SetOstream(options.useStderr ? std::cerr : std::cout);
    }

    if (useFile)
    {
      m_output = options.output;
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

        fileName.replace_extension(OutputSuffix_());
        m_output /= fileName;
      }
      else if (!options.outputName.empty())
      {
        m_output /= options.outputName + "."s.append(OutputSuffix_());
      }

      if (FormatOutput::OutputIsFile())
      {
        if (!m_output.empty())
        {
          auto fileOutput = m_buf.AddOfstream(m_output);
          if (!fileOutput->is_open())
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
    }

    m_errorCodeMappings = options.codeMappings;
  }

  [[nodiscard]] virtual bool SupportsRelativePath_() const noexcept override
  {
    return FormatOutput::SupportsRelativePath();
  }
  [[nodiscard]] virtual bool OutputIsFile_() const noexcept override
  {
    return FormatOutput::OutputIsFile();
  }
  [[nodiscard]] virtual std::string_view FormatName_() const noexcept override
  {
    return FormatOutput::FormatName();
  }
  [[nodiscard]] virtual std::string_view OutputSuffix_() const noexcept override
  {
    return FormatOutput::OutputSuffix();
  }
};

template <typename T>
class IFilter : public IOutput<T>
{
public:
  IFilter(IOutput<T>* output) : m_output(output) { };
  virtual ~IFilter() = default;

  void Start() override
  {
    if (m_output)
    {
      m_output->Start();
    }
  }

  bool Write(const T& message) override
  {
    if (!Check(message))
    {
      return false;
    }

    if (m_output)
    {
      return m_output->Write(message);
    }

    return true;
  }

  void Finish() override
  {
    if (m_output)
    {
      m_output->Finish();
    }
  }

protected:
  virtual bool Check(const T& message) const = 0;

  IOutput<T>* m_output = nullptr;
};

template <typename T>
class ITransform : public IOutput<T>
{
public:
  ITransform(IOutput<T>* output) : m_output(output) { };
  virtual ~ITransform() = default;

  void Start() override
  {
    if (m_output)
    {
      m_output->Start();
    }
  }

  bool Write(const T& message) override
  {
    return m_output->Write(Transform(message));
  }

  void Finish() override
  {
    if (m_output)
    {
      m_output->Finish();
    }
  }

protected:
  virtual T Transform(T message) const = 0;

  IOutput<T>* m_output = nullptr;
};

}
