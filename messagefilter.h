//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2021 (c) PVS-Studio LLC

#ifndef MESSAGEFILTER_H
#define MESSAGEFILTER_H
#include "configs.h"
#include "warning.h"
#include "utils.h"
#include "ioutput.h"

namespace PlogConverter
{

class MessageFilter : public IFilter
{
private:
  std::vector<int8_t> m_enabledAnalyzerLevels;
  std::set<std::string> m_disabledKeywords;
  std::vector<std::string> m_disabledPaths;
  std::set<std::string> m_disabledWarnings;

public:
  explicit MessageFilter(IOutput* output, const ProgramOptions& options);
  ~MessageFilter() override;
  bool Check(const Warning& message) const override;

private:
  bool CheckCode(const Warning& message) const;
  bool CheckFalseAlarm(const Warning& message) const;
  bool CheckKeywords(const Warning& message) const;
  bool CheckLevel(const Warning& message) const;
  bool CheckPath(const Warning& message) const;
};

}

#endif // MESSAGEFILTER_H
