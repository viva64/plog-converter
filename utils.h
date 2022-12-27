//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2022 (c) PVS-Studio LLC

#pragma once

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "json.hpp"

namespace PlogConverter
{

class FilesystemException : public std::runtime_error
{
public:
  explicit FilesystemException(const std::string& msg) : std::runtime_error(msg) {}
};

std::pair<std::string, std::string> SplitKeyValue(const std::string& srcRaw);
std::string Trim(const std::string& src);
void Replace(std::string& src, const std::string& toReplace, const std::string& replacer);
void ReplaceAll(std::string& src, const std::string& toReplace, const std::string& replacer);
std::string EscapeHtml(const std::string& src);
std::string ToLower(std::string_view src);

std::string LeftPad(const std::string &str, size_t size, char ch = ' ');

void UTF8toANSI(std::string& source);
void ANSItoUTF8(std::string& source);

bool StartsWith(const std::string& src, const std::string& prefix);
bool EndsWith(const std::string& src, const std::string& suffix);

template <typename OutIt, typename Fn>
void Split(const std::string& src, const std::string& delim, OutIt it, Fn fn)
{
  if (src.empty() || delim.empty())
  {
    return;
  }

  std::string::size_type pos = 0;
  std::string::size_type next_pos;
  do
  {
    next_pos = src.find(delim, pos);
    std::string::size_type pieceLength = next_pos - pos;
    *it = fn(src.substr(pos, pieceLength));
    ++it;
    pos = next_pos + delim.size();
  } while (next_pos != std::string::npos);
}

template <typename OutIt>
void Split(const std::string& src, const std::string& delim, OutIt it)
{
  Split(src, delim, it, [](std::string &&s)->std::string&& { return std::move(s); });
}

inline bool IsGlobPath(std::string_view path)
 {
     return path.find_first_of("*?") != std::string_view::npos;
 }

inline std::vector<std::string> Split(std::string text, const std::string &separator)
{
  size_t pos = 0;
  std::vector<std::string> tokens;
  while ((pos = text.find(separator)) != std::string::npos)
  {
    tokens.push_back(Trim(text.substr(0, pos))); //-V823
    text.erase(0, pos + separator.length());
  }
  tokens.push_back(Trim(text)); //-V823
  return tokens;
}

template <typename Range, typename Map>
std::string Join(Range &&range, Map fn, const std::string &delimiter)
{
  std::string res;
  auto it = std::begin(range);
  auto end = std::end(range);

  if (it == end)
  {
    return res;
  }

  res = fn(*it);

  for (++it; it != end; ++it)
  {
    res.append(delimiter.begin(), delimiter.end());
    res += fn(*it);
  }

  return res;
}

template <typename Range>
std::string Join(Range &&range, const std::string &delimiter = " ")
{
  return Join(range, [](auto v) { return v; }, delimiter);
}

unsigned ParseUint(const std::string &str);

std::ifstream OpenFile(const std::filesystem::path &path);
std::string Expand(const std::string &path);
std::string FileBaseName(const std::string &filePath);
std::string FileStem(const std::string &path);
std::string FileExtension(const std::string &path);

bool IsXmlFile(std::ifstream &fs);
bool IsDirectory(const std::string &path);
bool MakeDirectory(const std::string &path);
bool Exists(const std::string &path);


template <typename To, typename From, typename Deleter>
bool IsA(const std::unique_ptr<From, Deleter> &p) noexcept
{
  if (auto cast = dynamic_cast<To*>(p.get()))
  {
    return true;
  }

  return false;
}

template <typename ResultType, typename From, typename Deleter>
std::unique_ptr<ResultType, Deleter> To(std::unique_ptr<From, Deleter> &&p) noexcept
{
  if (auto cast = dynamic_cast<ResultType*>(p.get()))
  {
    std::unique_ptr<ResultType, Deleter> result { cast, std::move(p.get_deleter()) };
    p.release();
    return result;
  }

  return {};
}

template <typename ResultType, typename From, typename Deleter>
std::unique_ptr<ResultType, Deleter> UnsafeTo(std::unique_ptr<From, Deleter>&& p) noexcept
{
  std::unique_ptr<ResultType, Deleter> result{ static_cast<ResultType*>(p.get()), std::move(p.get_deleter()) };
  p.release();
  return result;
}

template <typename Key, typename Value>
constexpr auto WriteOption(nlohmann::json& j, Key&& fieldName, Value&& value)
{
  if (!std::empty(value))
  {
    value.erase(std::remove_if(std::begin(value), std::end(value),
      [](unsigned char symb) { return symb >= 0x80; }), std::end(value));
    j.emplace(std::forward<decltype(fieldName)>(fieldName), std::forward<decltype(value)>(value));
  }
}

bool          EqualPaths(std::string_view lhs, std::string_view rhs) noexcept;
bool LexicallyLesserPath(std::string_view lhs, std::string_view rhs) noexcept;


void ReplaceRelativeRoot(std::string& str, const std::string& root);
const std::string &GetSourceTreeRootMarker();
bool MatchPath(const char* pszFile, const char* pszSpec);
bool ContainsSubPath(const std::filesystem::path &haystack, const std::filesystem::path &needle);
bool ContainsSubstring(std::string_view, std::string_view) noexcept;
std::string GetAbsolutePath(const std::string& path);
}

unsigned GetHashCodePVS(std::string_view msg) noexcept;

namespace PvsStudio
{
  std::string FixErrorString(std::string line);
  uint32_t PvsHash(std::string_view line, unsigned version = ~0u /* latest version by default */) noexcept;
}
