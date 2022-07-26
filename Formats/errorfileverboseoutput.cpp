//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2022 (c) PVS-Studio LLC

#include "errorfileverboseoutput.h"

namespace PlogConverter
{

ErrorFileVerboseOutput::ErrorFileVerboseOutput(const ProgramOptions &opt) : ErrorFileOutputImpl<ErrorFileVerboseOutput>{ opt }
{
}

constexpr std::string_view GetColumnDelimeter()
{
  using namespace std::literals::string_view_literals;
#if defined (_WIN32)
  return ": "sv;
#else
  return ":1: "sv;
#endif
}

constexpr std::string_view GetAdditionalPrefix() noexcept
{
  using namespace std::literals::string_view_literals;
  return "|--"sv;
}

bool ErrorFileVerboseOutput::Write(const Warning& msg)
{
  using namespace std::literals::string_view_literals;

  if (ErrorFileOutputImpl::Write(msg))
  {
    if (msg.positions.size() > 1u)
    {
      auto warningLevel = msg.GetLevelString();
      static constexpr auto delimeter = GetColumnDelimeter();
      static constexpr auto prefix = GetAdditionalPrefix();

      for (auto it = std::next(msg.positions.begin(), 1u); it != msg.positions.end(); ++it)
      {
        m_ostream << it->file << ':' << it->line << delimeter
                  << warningLevel << ": "sv << prefix
                  << " Additional position : "sv << msg.code << '\n';
      }

      m_ostream.flush();
    }
    return true;
  }

  return false;
}

}
