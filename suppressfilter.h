//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2022 (c) PVS-Studio LLC

#pragma once

#include <optional>

#include "configs.h"
#include "ioutput.h"
#include "messagefilter.h"
#include "utils.h"
#include "warning.h"

namespace PlogConverter
{

class SuppressFilter : public IFilter<Warning>
{
private:
  struct File
  {
    std::string name;
    unsigned int line = 0;

    static inline bool CheckLine(const Warning& message, const File& file)
    {
      return file.line == 0 || message.GetLine() == file.line;
    }

    static inline bool CheckName(const Warning& message, const File& file)
    {
      return message.GetFile().find(file.name) != std::string::npos;
    }
  };

  std::set<std::string> m_disabledWarnings;
  std::vector<int8_t>   m_enabledAnalyzerLevels;
  std::vector<File>     m_enabledFiles;
  std::vector<uint32_t> m_enabledWarnings;

public:
  explicit SuppressFilter(PlogConverter::IOutput<PlogConverter::Warning>* output, const ProgramOptions& options);
  ~SuppressFilter() override;

  bool Check(const Warning& message) const override;

private:
  bool CheckCode(const Warning& message) const;
  bool CheckFiles(const Warning& message) const;
  bool CheckLevel(const Warning& message) const;
  bool CheckWarnings(const Warning& message) const;
};

}
