//  2006-2008 (c) Viva64.com Team
//  2008-2018 (c) OOO "Program Verification Systems"

#ifndef CONFIGS
#define CONFIGS
#include <vector>
#include <string>
#include <set>
#include <functional>
#include <memory>

namespace PlogConverter
{

enum class SecurityCodeMapping
{
  CWE
};

struct Analyzer;
class IOutput;

struct ProgramOptions
{
  using AllocFunction = std::function<std::unique_ptr<IOutput>(const ProgramOptions&)>;

  std::string                                 cmdLine;
  std::vector<std::string>                    inputFiles;
  std::string                                 output;
  std::string                                 configFile;
  std::vector<AllocFunction>                  formats;
  std::vector<SecurityCodeMapping>            codeMappings;
  std::set<std::string>                       disabledWarnings;
  std::vector<std::string>                    disabledPaths;
  std::set<std::string>                       disabledKeywords;
  std::vector<Analyzer>                       enabledAnalyzers;
  std::string                                 projectRoot = ".";
  std::string                                 outputName;
  std::string                                 projectName;
  std::string                                 projectVersion;
  bool                                        useStderr = false;
};


}

#endif // CONFIGS

