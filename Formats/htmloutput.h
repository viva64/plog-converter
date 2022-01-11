//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2022 (c) PVS-Studio LLC

#ifndef HTMLOUTPUT_H
#define HTMLOUTPUT_H
#include <unordered_map>
#include <map>
#include <vector>
#include <string>
#include "ioutput.h"

namespace PlogConverter
{

class HTMLOutput : public IOutput
{
private:
  std::vector<Warning> m_messages;

  size_t m_ga = 0;
  size_t m_op = 0;
  size_t m_64 = 0;
  size_t m_cs = 0;
  size_t m_misra = 0;
  size_t m_autosar = 0;
  size_t m_owasp = 0;
  size_t m_fails = 0;

  std::map<AnalyzerType, std::string> m_desc;
  std::unordered_map<std::string, size_t> m_map;

  std::string m_directory;
  std::string m_cmdline;
  std::string m_projectName;
  std::string m_projectVersion;
  std::size_t m_currentId = 0;

  bool m_hasAnyCWE = false;
  bool m_showSASTColumn = false;
  bool m_hasAnyProjects = false;
  void CheckProjectsAndCWEAndSAST();

  void PrintFileExtra(const std::string &fileName, const std::string &data, std::ios_base::openmode mode = std::ios_base::out);
  void PrintFileSources();
  void PrintHtmlStart();
  void PrintHtmlEnd();
  void PrintTableInfo();
  void PrintTableCaption();
  void PrintTableBody();
  std::string GenerateSourceHtml(std::stringstream &sourceFile);

public:
  explicit HTMLOutput(const ProgramOptions &);
  ~HTMLOutput() override;
  bool Write(const Warning& msg) override;
  void Finish() override;
};

}

#endif // HTMLOUTPUT_H