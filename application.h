//  2006-2008 (c) Viva64.com Team
//  2008-2018 (c) OOO "Program Verification Systems"

#ifndef APPLICATION_H
#define APPLICATION_H
#include "configs.h"
#include <string>
#include <vector>
#include "iworker.h"
#include "outputfactory.h"

namespace PlogConverter
{

Analyzer ParseEnabledAnalyzer(const std::string &str);
void ParseEnabledAnalyzers(std::string str, std::vector<Analyzer>& analyzers);

class Application
{
public:
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

#endif // APPLICATION_H
