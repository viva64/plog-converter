#include "samewarningsfilter.h"
#include "warning.h"

namespace PlogConverter
{
  namespace detail
  {
    std::size_t PlogWarningHasher::operator()(const PlogConverter::Warning &w) const
    {
      auto hash = hash_combine(std::hash<std::decay_t<decltype(w.level)>>          {}(w.level),
                               std::hash<std::decay_t<decltype(w.GetErrorCode())>> {}(w.GetErrorCode()),
                               std::hash<std::decay_t<decltype(w.message)>>        {}(w.message),
                               std::hash<std::decay_t<decltype(w.GetFile())>>      {}(w.GetFile()),
                               std::hash<std::decay_t<decltype(w.GetLine())>>      {}(w.GetLine()));
      return hash;
    }

    bool PlogWarningEqual::operator()(const Warning &lhs, const Warning &rhs) const
    {
      return    std::forward_as_tuple(lhs.level, lhs.GetErrorCode(), lhs.message, lhs.GetLine(), lhs.GetFile())
             == std::forward_as_tuple(rhs.level, rhs.GetErrorCode(), rhs.message, rhs.GetLine(), rhs.GetFile());
    }
  }

  bool SameWarningsFilter::IsUnique(const Warning &warning)
  {
    return m_unique.emplace(warning).second;
  }

  void SameWarningsFilter::Reset() noexcept
  {
    m_unique.clear();
  };
}