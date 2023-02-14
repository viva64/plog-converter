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
}