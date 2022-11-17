//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2022 (c) PVS-Studio LLC

#pragma once
#include "configs.h"
#include "ioutput.h"
#include "utils.h"
#include "warning.h"
#include <string>
#include <vector>



namespace PlogConverter
{
class PathFilter : public IFilter<Warning>
{
  std::vector<std::string> m_disabledPaths;
  std::vector<std::string> m_enabledPaths;
  bool mutable m_sourceTreeRootFound = false;

public:
  explicit PathFilter(IOutput<Warning>* output, const ProgramOptions& option);
  ~PathFilter() override;
  bool Check(const Warning& warning) const override;

private:
  bool IsExcludedPath(const std::string &srcPath) const;
  bool IsIncludedPath(const std::string &srcPath) const;
 };
}