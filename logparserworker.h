//  2006-2008 (c) Viva64.com Team
//  2008-2019 (c) OOO "Program Verification Systems"

#ifndef LOGPARSERWORKER_H
#define LOGPARSERWORKER_H
#include <memory>
#include <unordered_set>

#include "application.h"
#include "ioutput.h"
#include "iworker.h"
#include "messagefilter.h"
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
    IMessageFilter &filter,
    IOutput &output,
    const std::string &root
  );

  size_t GetTotalWarnings() const;
  size_t GetPrintedWarnings() const;

private:
  void ParseRawLog(InputFile &file);
  void ParseJsonLog(InputFile &file);
  void OnWarning(Warning &warning);

  MessageParser m_messageParser;
  size_t m_countSuccess = 0;
  size_t m_countTotal = 0;
  IMessageFilter *m_filter = nullptr;
  IOutput *m_output = nullptr;
  std::string m_root;

  std::string m_line;
  std::unordered_set<std::string> m_hashTable {4096};
  Warning m_warning;
};

}

#endif // LOGPARSERWORKER_H
