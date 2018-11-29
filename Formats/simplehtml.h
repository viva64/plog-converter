//  2006-2008 (c) Viva64.com Team
//  2008-2018 (c) OOO "Program Verification Systems"

#ifndef SIMPLEHTMLOUTPUT_H
#define SIMPLEHTMLOUTPUT_H
#include <vector>
#include "ioutput.h"

namespace PlogConverter
{

class SimpleHTMLOutput : public IOutput
{
public:
  explicit SimpleHTMLOutput(const ProgramOptions &);
  ~SimpleHTMLOutput() override;
  void Start() override;
  void Write(const Warning& msg) override;
  void Finish() override;

  static const int DefaultCweColumnWidth;
  static const int DefaultMISRAColumnWidth;
  static const int DefaultMessageColumnWidth;

private:
  std::vector<Warning> m_ga;
  std::vector<Warning> m_op;
  std::vector<Warning> m_64;
  std::vector<Warning> m_cs;
  std::vector<Warning> m_misra;
  std::vector<Warning> m_info;

  int GetMessageColumnWidth() const;
  int GetCweColumnWidth() const;
  int GetMISRAColumnWidth() const;

  void PrintHtmlStart();
  void PrintHtmlEnd();
  void PrintTableCaption();
  void PrintTableBody();
  void PrintHeading(const std::string &);
  void PrintMessages(const std::vector<Warning>&, const std::string &);
};

}

#endif // SIMPLEHTMLOUTPUT_H
