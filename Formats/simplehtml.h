//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2023 (c) PVS-Studio LLC

#pragma once

#include "ioutput.h"

#include <vector>

namespace PlogConverter
{

class SimpleHTMLOutput : public BasicFormatOutput<SimpleHTMLOutput>
{
public:
  explicit SimpleHTMLOutput(const ProgramOptions &);
  ~SimpleHTMLOutput() override = default;

  void Start() override;
  bool Write(const Warning& msg) override;
  void Finish() override;

  [[nodiscard]]
  static bool SupportsRelativePath() noexcept
  {
    return false;
  }

  [[nodiscard]]
  static bool OutputIsFile() noexcept
  {
    return true;
  }

  [[nodiscard]]
  static std::string_view FormatName() noexcept
  {
    return "html";
  }

  [[nodiscard]]
  static std::string_view OutputSuffix() noexcept
  {
    return FormatName();
  }

  static const int DefaultCweColumnWidth;
  static const int DefaultSASTColumnWidth;
  static const int DefaultMessageColumnWidth;

private:
  std::vector<Warning> m_ga;
  std::vector<Warning> m_op;
  std::vector<Warning> m_64;
  std::vector<Warning> m_cs;
  std::vector<Warning> m_misra;
  std::vector<Warning> m_info;
  std::vector<Warning> m_autosar;
  std::vector<Warning> m_owasp;

  int GetMessageColumnWidth() const;
  int GetCweColumnWidth() const;
  int GetSASTColumnWidth() const;

  void PrintHtmlStart();
  void PrintHtmlEnd();
  void PrintTableCaption();
  void PrintTableBody();
  void PrintHeading(const std::string &);
  void PrintMessages(const std::vector<Warning>&, const std::string &);
};

}
