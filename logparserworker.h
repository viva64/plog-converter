//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2023 (c) PVS-Studio LLC

#pragma once

#include <memory>
#include <unordered_set>

#include "application.h"
#include "ioutput.h"
#include "iworker.h"
#include "messagefilter.h"
#include "suppressfilter.h"
#include "messageparser.h"
#include "warning.h"

namespace PlogConverter
{

class InputFile
{
public:
  InputFile()                            = delete;
  InputFile(const InputFile&)            = delete;
  InputFile& operator=(const InputFile&) = delete;

  InputFile(InputFile&&)                 = default;
  InputFile& operator=(InputFile&&)      = default;
  ~InputFile()                           = default;

  InputFile(std::string path_);

  bool operator<(const InputFile &other) const noexcept;

  std::string_view Path() const noexcept { return m_path;   }
  std::ifstream&   Stream()     noexcept { return m_stream; }

private:
  std::ifstream m_stream;
  std::string   m_path;
};

class LogParserWorker : public IWorker
{
private:
  using WarningsLogContent = WarningJsonExtractor::Container;

public:
  ~LogParserWorker() override;

  void Run(const ProgramOptions &options) override;

  void ParseLog(std::vector<InputFile> &inputFiles,
                IOutput<Warning> &output,
                const std::string &root);

  size_t GetTotalWarnings() const;
  size_t GetPrintedWarnings() const;

  [[nodiscard]]
  bool IsErrorHappend() const noexcept override;

private:
  void ParseRawLog(InputFile &file,  WarningsLogContent &warnings);
  void ParseJsonLog(InputFile &file, WarningsLogContent &warnings);
  void ParseCerrLog(InputFile& file, WarningsLogContent &warnings);
  void OnWarning(const Warning &warning);

  template<typename Format>
  bool CheckUnsupporterdTransformation(const Format &fmt,
                                       const ProgramOptions &opt) noexcept
  {
    if (fmt && !opt.projectRoot.empty()
            &&  opt.pathTransformationMode == PathTransformationMode::ToRelative
            && !fmt->SupportsRelativePath_())
    {
      std::cout << "Error: the \'" << fmt->FormatName_() << "\' format doesn't support relative root\n";
      m_isUnsopporterdTransformationErrorHappend = true;
      return false;
    }

    return true;
  }

  MessageParser m_messageParser;
  size_t m_countSuccess = 0;
  size_t m_countNonError = 0;
  size_t m_countTotal = 0;
  IOutput<Warning>* m_output = nullptr;
  std::string m_root;
  std::string m_line;

  bool m_isUnsopporterdTransformationErrorHappend = false;
};

}
