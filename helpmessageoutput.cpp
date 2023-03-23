//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2023 (c) PVS-Studio LLC

#include "helpmessageoutput.h"

namespace PlogConverter
{
  HelpMessageOutput::HelpMessageOutput(NextOutputPtr output)
    : m_nextOutput(std::move(output)), m_helpMsgPrinted(false)
  {
  }

  void HelpMessageOutput::Start()
  {
    if (m_nextOutput)
    {
      m_nextOutput->Start();
    }
  }

  bool HelpMessageOutput::Write(const Warning& message)
  {
    if (!m_nextOutput)
    {
      return false;
    }

    if (!m_helpMsgPrinted)
    {
      m_nextOutput->Write(Warning::GetDocumentationLinkMessage());
      m_helpMsgPrinted = true;
    }

    return m_nextOutput->Write(message);
  }

  void HelpMessageOutput::Finish()
  {
    if (m_nextOutput)
    {
      m_nextOutput->Finish();
    }
  }

  [[nodiscard]] std::string_view HelpMessageOutput::FormatName_() const noexcept
  {
    if (auto base = dynamic_cast<INameable *>(m_nextOutput.get()))
    {
      return base->FormatName_();
    }
    return std::string_view("");
  }

  void HelpMessageOutput::ClearOutput(bool removeEmptyFile) noexcept
  {
    if (auto base = dynamic_cast<IFileClearable *>(m_nextOutput.get()))
    {
      return base->ClearOutput(removeEmptyFile);
    }
  }

  void HelpMessageOutput::HardClearOutput() noexcept
  {
    if (auto base = dynamic_cast<IFileClearable *>(m_nextOutput.get()))
    {
      return base->HardClearOutput();
    }
  }

  [[nodiscard]] bool HelpMessageOutput::SupportsRelativePath_() const noexcept
  {
    if (auto base = dynamic_cast<ISupportsRelativePath *>(m_nextOutput.get()))
    {
      return base->SupportsRelativePath_();
    }
  }
}