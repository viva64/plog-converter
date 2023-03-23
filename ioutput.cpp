//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2023 (c) PVS-Studio LLC

#include "ioutput.h"

namespace PlogConverter
{
  void BaseFormatOutput::ClearOutput(bool removeEmptyFile) noexcept
  {
    m_buf.pubsync();
    if (auto &fstream = m_buf.m_ofstream)
    {
      if (fstream->is_open())
      {
        fstream->close();
      }

      std::error_code ignored;
      if (removeEmptyFile && std::filesystem::is_empty(m_output, ignored))
      {
        std::filesystem::remove(m_output, ignored);
      }
    }
    if (m_buf.m_ostream)
    {
      m_buf.m_ostream->flush();
    }
  }

  void BaseFormatOutput::HardClearOutput() noexcept
  {
    m_buf.pubsync();
    if (auto &fstream = m_buf.m_ofstream)
    {
      if (fstream->is_open())
      {
        fstream->close();
      }
      std::error_code ignored;
      std::filesystem::remove(m_output, ignored);
    }
    if (m_buf.m_ostream)
    {
      m_buf.m_ostream->flush();
    }
  }

  void BaseFormatOutput::Finish()
  {
    ClearOutput(false);
  };

  void BaseFormatOutput::DetectShowTags(bool &showCWE, bool &showSAST) const
  {
    showSAST = false;
    showCWE = false;

    for (const auto &security : m_errorCodeMappings)
    {
      if (security == SecurityCodeMapping::MISRA || security == SecurityCodeMapping::AUTOSAR || security == SecurityCodeMapping::OWASP)
      {
        showSAST = true;
      }
      if (security == SecurityCodeMapping::CWE)
      {
        showCWE = true;
      }
    }
  }
}