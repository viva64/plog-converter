#include "samewarningsfilter.h"
#include "warning.h"

namespace PlogConverter
{
  namespace detail
  {
    std::size_t PlogWarningHasher::operator()(const PlogConverter::Warning &w) const
    {
      auto hash = hash_combine(std::hash<int>        {}(w.level),
                               std::hash<int>        {}(w.GetErrorCode()),
                               std::hash<std::string>{}(w.message));
    
      for (auto &pos : w.positions)
      {
        hash = hash_combine(hash,
                            std::hash<std::string>{}(pos.file),
                            std::hash<int>        {}(pos.line),
                            std::hash<unsigned>   {}(pos.navigation.currentLine),
                            std::hash<unsigned>   {}(pos.navigation.previousLine),
                            std::hash<unsigned>   {}(pos.navigation.nextLine));
      }
    
      return hash;
    }

    bool PlogWarningEqual::operator()(const Warning &lhs, const Warning &rhs) const
    {
      return    lhs.level == rhs.level
             && lhs.GetErrorCode() == rhs.GetErrorCode()
             && lhs.message == rhs.message
             && !(lhs.positions < rhs.positions || rhs.positions < lhs.positions);
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