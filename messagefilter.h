//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2022 (c) PVS-Studio LLC

#pragma once

#include "configs.h"
#include "ioutput.h"
#include "utils.h"
#include "warning.h"

namespace PlogConverter
{

class MessageFilter : public IFilter<Warning>
{
  std::vector<int8_t> m_enabledAnalyzerLevels;
  std::set<std::string> m_disabledKeywords;
  std::vector<std::string> m_disabledPaths;
  std::set<std::string> m_disabledWarnings;

public:
  explicit MessageFilter(IOutput<Warning>* output, const ProgramOptions& options);
  ~MessageFilter() override;

  bool Check(const Warning& message) const override;

private:
  bool CheckCode(const Warning& message) const;
  bool CheckFalseAlarm(const Warning& message) const;
  bool CheckKeywords(const Warning& message) const;
  bool CheckLevel(const Warning& message) const;
};

}
