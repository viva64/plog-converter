//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2022 (c) PVS-Studio LLC

#include "args.hxx"

using namespace std::string_literals;
using namespace std::string_view_literals;

class StringSplitter
{
  // todo [c++20]: use forward iterator concept
  
  template <typename It>
  It SkipSeparator(It strIt, It last, std::string_view sep) const noexcept;

  template<typename It>
  bool IsAnySeparator(It strIt, It last) const noexcept;

  template <typename It>
  It SkipAnySeparator(It strIt, It last) const noexcept;

  template <typename It>
  It FindSeparator(It first, It last) const noexcept;

public:

  StringSplitter(std::vector<std::string> separators);

  std::vector<std::string> operator()(const std::string &str) const noexcept;

private:
  std::vector<std::string> m_separators;
};

class DefaultSplitter final : public StringSplitter
{
public:
  DefaultSplitter();
};

template <typename K, typename AllocFuntion,
          typename Splitter = DefaultSplitter,
          template <typename...> class List = std::vector,
          typename Reader = args::ValueReader,
          template <typename...> class Map = std::unordered_map>
class MapFlagListMulti : public args::MapFlagList<K, AllocFuntion, List, Reader, Map>
{
  using Base = args::MapFlagList<K, AllocFuntion>;
  using args::MapFlagList<K, AllocFuntion>::MapFlagList;

  void ParseValue(const std::vector<std::string> &values_) override
  {
    const std::string &value = values_.at(0);
    std::vector<K> keys = Splitter{}(value);

    for (const auto &rawKey : keys)
    {
      Base::ParseValue({ rawKey });
    }
  }
};
