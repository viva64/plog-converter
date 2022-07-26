//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2022 (c) PVS-Studio LLC

#pragma once

#include <string>
#include <vector>

#include "configs.h"
#include "iworker.h"
#include "outputfactory.h"

namespace PlogConverter
{

Analyzer ParseEnabledAnalyzer(const std::string &str);
void ParseEnabledAnalyzers(std::string str, std::vector<Analyzer>& analyzers);

enum class ConverterRunState : int
{
  Success = 0,                      // Conversion finished successfully
  GenericException = 1,             // Handled exception
  OutputLogNotEmpty = 2,            // Output contains non-suppressed warnings after filtration. 
                                    //   This exit code will be generated only when using converter
                                    //   with --indicateWarnings (-w) flag;
                                    // 3 - skipped of Windows PlogConverter compatibility
                                    // 4 - skipped of Windows PlogConverter compatibility
  UnsopportedPathTransofrmation = 5 // Some render formats doesn't support relative root
};

class Application
{
public:
  static const std::string AppName_Default;
  static const std::string AppName_Win;
  static const std::string AboutPVSStudio;

  static const char CmdAnalyzerFlagName_Short;
  static const std::string CmdAnalyzerFlagName_Full;

  int Exec(int argc, const char **argv);
  void AddWorker(std::unique_ptr<IWorker> worker);

  OutputFactory outputFactory;

private:
  void SetCmdOptions(int argc, const char** argv);
  void SetConfigOptions(const std::string& path);

  ProgramOptions m_options;
  std::vector<std::unique_ptr<IWorker>> m_workers;
};

}
