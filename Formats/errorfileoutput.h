//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2023 (c) PVS-Studio LLC

#pragma once

#include "ioutput.h"

namespace PlogConverter
{

template <class Derived = void>
class ErrorFileOutputImpl : public BasicFormatOutput<std::conditional_t<std::is_void_v<Derived>, ErrorFileOutputImpl<>, Derived>>
{
  using Base = BasicFormatOutput<std::conditional_t<std::is_void_v<Derived>, ErrorFileOutputImpl<>, Derived>>;
public:
  explicit ErrorFileOutputImpl(const ProgramOptions& opt) : Base{ opt }
  {
  }
  ~ErrorFileOutputImpl() override = default;

  bool Write(const Warning& msg) override
  {
    std::string securityPrefix;

    bool showSAST = false;
    bool showCWE = false;

    Base::DetectShowTags(showCWE, showSAST);

    if (showCWE && msg.HasCWE())
      securityPrefix += msg.GetCWEString();

    if (showSAST && msg.HasSAST())
    {
      if (!securityPrefix.empty())
        securityPrefix += ", ";

      securityPrefix += msg.sastId;
    }

    if (!securityPrefix.empty())
    {
      securityPrefix = '[' + securityPrefix + "] ";
    }

#if defined (_WIN32)
    const std::string column{ ": " };
#else
    const std::string column{ ":1: " };
#endif

    Base::m_ostream << msg.GetFileUTF8() << ":" << msg.GetLine() << column
              << msg.GetLevelString() << ": "
              << msg.code << " "
              << securityPrefix << msg.message << std::endl;

    return true;
  }

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
    return "errorfile";
  }

  [[nodiscard]]
  static std::string_view OutputSuffix() noexcept
  {
    return "err";
  }
};

using ErrorFileOutput = ErrorFileOutputImpl<>;

}
