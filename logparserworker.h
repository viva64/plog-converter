//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2022 (c) PVS-Studio LLC

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

struct InputFile
{
  const std::string path;
  std::ifstream stream;

  InputFile(std::string path_);
};

class LogParserWorker : public IWorker
{
public:
  ~LogParserWorker() override;

  void Run(const ProgramOptions &options) override;

  void ParseLog(
    std::vector<InputFile> &inputFiles,
    IOutput<Warning> &output,
    const std::string &root
  );

  size_t GetTotalWarnings() const;
  size_t GetPrintedWarnings() const;

  [[nodiscard]]
  bool IsErrorHappend() const noexcept override;

private:
  void ParseRawLog(InputFile &file);
  void ParseJsonLog(InputFile &file);
  void ParseCerrLog(InputFile& file);
  void OnWarning(Warning &warning);

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
  std::unordered_set<std::string> m_hashTable {4096};

  bool m_isUnsopporterdTransformationErrorHappend = false;

  Warning m_warning;
};

}
