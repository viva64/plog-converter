#pragma once

#include <cstdint>
#include <cstddef>
#include <algorithm>
#include <functional>
#include <unordered_set>

namespace PlogConverter
{
  struct Warning;

  namespace detail
  {
    struct PlogWarningHasher
    {
      template <typename ...Hashes, std::enable_if_t< std::conjunction_v< std::is_unsigned<Hashes>... >, int> = 0 >
      static constexpr uintmax_t hash_combine(uintmax_t hash1, Hashes ...hashN)
      {
        if constexpr (sizeof...(hashN) == 0)
        {
          return hash1;
        }
        else
        {
          std::array<std::common_type_t<Hashes...>, sizeof...(hashN)> hashes{ hashN... };
          size_t res = hash1;
          for (auto hash : hashes)
          {
            res = hash_combine(res, hash);
          }

          return res;
        }
      }

      static constexpr inline uintmax_t hash_combine(uintmax_t seed, uintmax_t hash)
      {
        return seed ^ (hash + 0x9e3779b9 + (seed << 6) + (seed >> 2));
      }

      std::size_t operator()(const Warning &w) const;
    };

    struct PlogWarningEqual
    {
      bool operator()(const Warning &lhs, const Warning &rhs) const;
    };
  }

  class SameWarningsFilter
  {
  private:
    using UniquesCollection = std::unordered_set<Warning, detail::PlogWarningHasher, detail::PlogWarningEqual>;

  public:
    /*!
     * @brief Filter container depends on the data in the capatitor
     * @param container Warnings to filter
     * @return Container with only unique entries
    */
    template <typename Container>
    [[nodiscard]] Container FilterContainer(Container container)
    {
      static_assert(!std::is_reference_v<Container>,
                    "Function takes container only by value");

      static_assert(std::is_convertible_v<std::remove_extent_t<Container>, Warning>,
                    "Container element should be convertible to PlogConverter::Warning");

      if (container.empty())
      {
        return {};
      }


      Container uniques{};

      using namespace std::placeholders;
      std::copy_if(std::move_iterator{ std::begin(container) }, std::move_iterator{ std::end(container) },
                   std::inserter(uniques, std::end(uniques)),
                   std::bind(&SameWarningsFilter::IsUnique, this, _1));

      return uniques;
    }

    /*!
     * @brief Checks uniqueness of single warning depends on the data in the capatitor
     * @param warning Warning to check
     * @return Bool value showing is warning unique or not
    */
    [[nodiscard]] bool IsUnique(const Warning &warning);

    /*!
     * @brief Reset entries in capacitor
    */
    void Reset() noexcept;

  private:
    UniquesCollection m_unique; //!< Unique entries capacitor
  };
}