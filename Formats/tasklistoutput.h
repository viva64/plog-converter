//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2023 (c) PVS-Studio LLC

#pragma once

#include <string_view>

#include "ioutput.h"

namespace PlogConverter
{

template <class Derived = void>
class TaskListOutputImpl : public BasicFormatOutput<std::conditional_t<std::is_void_v<Derived>, TaskListOutputImpl<>, Derived>>
{
  static std::string Escape(std::string_view str)
  {
    auto res = std::string{ str };
    ReplaceAll(res, "\\", "\\\\");
    return res;
  }

  using Base = BasicFormatOutput<std::conditional_t<std::is_void_v<Derived>, TaskListOutputImpl<>, Derived>>;

public:
  explicit TaskListOutputImpl(const ProgramOptions& opt) : Base{ opt }
  {
  }
  ~TaskListOutputImpl() override = default;

  bool Write(const Warning &msg) override
  {
    using namespace std::literals;

    std::string securityPrefix;

    bool showSAST = false;
    bool showCWE = false;

    Base::DetectShowTags(showCWE, showSAST);

    if (showCWE && msg.HasCWE())
    {
      securityPrefix = msg.GetCWEString();
    }

    if (showSAST && msg.HasSAST())
    {
      static constexpr auto separator = ", "sv;
      auto maxPossibleSize = securityPrefix.size() + msg.sastId.size() + separator.size();
      securityPrefix.reserve(maxPossibleSize);
      
      if (!securityPrefix.empty())
      {
        securityPrefix += separator;
      }

      securityPrefix += msg.sastId;
    }

    Base::m_ostream << msg.GetFileUTF8() << "\t" << msg.GetLine() << "\t"
                    << msg.GetLevelString("err", "warn", "note") << "\t"
                    << msg.code << " ";

    if (!securityPrefix.empty())
    {
      Base::m_ostream << '[' << securityPrefix << "] "sv;
    }

    Base::m_ostream << Escape(msg.message) << std::endl;

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
    return "tasklist";
  }

  [[nodiscard]]
  static std::string_view OutputSuffix() noexcept
  {
    return "tasks";
  }
};

using TaskListOutput = TaskListOutputImpl<>;

}
