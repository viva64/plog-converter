//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2023 (c) PVS-Studio LLC

#include <cstddef>
#include <cstring>
#include <fstream>
#include <iostream>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
// for WideCharToMultiByte, MultiByteToWideChar
#include <stringapiset.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fnmatch.h>
#endif
#include <unordered_map>
#include <algorithm>

#include "utils.h"


namespace PlogConverter
{
  bool MatchPath(const char* pszFile, const char* pszSpec)
  {
#ifdef _WIN32
    return PathMatchSpecA(pszFile, pszSpec);
#elif defined (__GNUC__)
    return fnmatch(pszSpec, pszFile, 0) == 0;
#else
#error no definition of PathMatchSpec(const char*, const char*) or fnmatch(const char*, const char*) function.
#endif
  }

  bool ContainsSubPath(const std::filesystem::path &haystack, const std::filesystem::path &needle)
  {
    std::filesystem::path wcHaystack = std::filesystem::weakly_canonical(haystack);
    std::filesystem::path wcNeedle   = std::filesystem::weakly_canonical(needle);
    
    if (   !wcHaystack.has_root_directory() 
        || !wcNeedle.has_root_directory())
    {
      return false;
    }

    auto hasystackStart = std::begin(wcHaystack);
    auto needleStart = std::begin(wcNeedle);

    for (; hasystackStart != std::end(wcHaystack) 
        && needleStart != std::end(wcNeedle); 
        ++hasystackStart, ++needleStart)
    {
      if (*hasystackStart != *needleStart)
      {
        return false;
      }
    }

    if (   hasystackStart == std::end(wcHaystack) 
        && needleStart != std::end(wcNeedle))
    {
      return false;
    }
    return true;
  }
 
  std::string GetAbsolutePath(const std::string& pathStr)
  {
    namespace stdfs = std::filesystem;
    std::string expandPath { Expand(Trim(pathStr)) };
    
    if (!IsGlobPath(expandPath))
    {
      try
      {         
        return stdfs::absolute(expandPath).string();
      }
      catch (const std::exception& e)
      {
        std::cout << "Path invalid syntax: " << pathStr << std::endl;
        std::cout << "Exeption: " << e.what() << std::endl;
      }
    }

    return expandPath;
  }

  bool ContainsSubstring(std::string_view haystack, std::string_view needle) noexcept
  {
    if (haystack.empty() || needle.empty())
    {
      return false;
    }

    return haystack.find(needle) != std::string_view::npos;
  }

constexpr static auto SpaceChars = std::string_view{ " \t\n\r" };

std::string_view Trim(std::string_view src)
{
  auto begin = src.find_first_not_of(SpaceChars);
  if (begin == std::string_view::npos) { return {}; }

  auto end = src.find_last_not_of(SpaceChars);
  if (end == std::string_view::npos) { return {}; }

  return src.substr(begin, end - begin + 1);
}

std::pair<std::string, std::string> SplitKeyValue(std::string_view srcRaw)
{
  if (srcRaw.empty()) { return {}; }

  auto src = Trim(srcRaw);

  size_t tailBound;
  auto firstWordBound = src.find('=');

  if (firstWordBound != std::string::npos)
  {
    tailBound = firstWordBound + 1;
    while (tailBound < src.size() && SpaceChars.find(src[tailBound]) != std::string::npos)
      ++tailBound;
    while (firstWordBound != 1 && SpaceChars.find(src[firstWordBound - 1]) != std::string::npos)
      --firstWordBound;
  }
  else
  {
    tailBound = src.size();
  }

  return { std::string{ src.substr(0, firstWordBound) }, std::string{ src.substr(tailBound) } };
}

bool StartsWith(std::string_view src, std::string_view prefix)
{
  return prefix.empty() || (prefix.size() <= src.size() && src.compare(0, prefix.size(), prefix) == 0);
}

bool EndsWith(std::string_view src, std::string_view suffix)
{
  return suffix.empty() || (suffix.size() <= src.size() && src.compare(src.size() - suffix.size(), suffix.size(), suffix) == 0);
}

void Replace(std::string &src, std::string_view toReplace, std::string_view replacer)
{
  std::string::size_type pos = src.find(toReplace);
  if(pos != std::string::npos)
    src.replace(pos, toReplace.size(), replacer);
}

void ReplaceAll(std::string& src, std::string_view toReplace, std::string_view replacer)
{
  std::string::size_type pos = src.find(toReplace);
  while(pos != std::string::npos)
  {
    src.replace(pos, toReplace.size(), replacer);
    pos = src.find(toReplace, pos + replacer.size());
  }
}

std::ifstream OpenFile(const std::filesystem::path& path)
{
  std::ifstream stream(path);
  if (!stream.is_open())
  {
    throw FilesystemException("File doesn't exist: " + path.string());
  }

  const int BOMSize = 3;
  char header[BOMSize];
  const unsigned char UTF8Bom[3] = { 0xEF, 0xBB, 0xBF };
  stream.read(header, BOMSize);

  if (!memcmp(header, UTF8Bom, BOMSize))
  {
    stream.seekg(BOMSize);
  }
  else
  {
    stream.seekg(0);
  }

  return stream;
}

std::string Expand(std::string_view path)
{
  std::string res{ path };

  if (StartsWith(res, "~/"))
  {
#ifdef __GNUC__
    const char *home = getenv("HOME");
#else
    const char *home = getenv("userprofile");
#endif
    if (home != nullptr && *home != '\0')
    {
      res.erase(0, 1);
      res.insert(0, home);
    }
  }

  if (!res.empty() && (res.back() == '/' || res.back() == '\\'))
  {
    res.pop_back();
  }

  return res;
}

std::string EscapeHtml(const std::string &src)
{
  std::string str = src;
  ReplaceAll(str, "&", "&amp;");
  ReplaceAll(str, "\"", "&quot;");
  ReplaceAll(str, "<", "&lt;");
  ReplaceAll(str, ">", "&gt;");
  return str;
}

std::string ToLower(std::string_view src)
{
  std::string str;
  str.resize(src.size());
  for (auto c : src)
  {
    str.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
  }

  return str;
}

std::string FileBaseName(const std::string &filePath)
{
  size_t i = filePath.find_last_of("/\\");
  return i != std::string::npos ? filePath.substr(i + 1) : filePath;
}

std::string FileStem(const std::string &filePath)
{
  return filePath.substr(0, filePath.find_last_of('.'));
}

std::string FileExtension(const std::string &filePath)
{
  const size_t dotPos = filePath.find_last_of('.');
  return (dotPos != std::string::npos) ? filePath.substr(dotPos + 1) : "";
}

bool IsXmlFile(std::ifstream &fs)
{
  if (!fs.is_open())
  {
    return false;
  }

  std::string header = {};

  bool isXml = false;

  while (std::getline(fs, header, '\n'))
  {
    header = Trim(header);

    if (header.empty() || StartsWith(header, "<!--"))
    {
      continue;
    }

    if (StartsWith(header, "<?xml"))
    {
      isXml = true;
    }

    break;
  }

  fs.seekg(0, std::ios_base::beg);
  return isXml;
}

bool IsDirectory(const std::string &path)
{
#ifdef _WIN32
  DWORD ftyp = GetFileAttributesA(path.c_str());
  if (ftyp == INVALID_FILE_ATTRIBUTES)
    return false;

  if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
    return true;

  return false;
#else
  DIR *dir = opendir(path.c_str());
  if (dir != nullptr)
  {
    closedir(dir);
    return true;
  }

  return false;
#endif
}

bool MakeDirectory(const std::string &path)
{
#ifdef _WIN32
  return _mkdir(path.c_str()) == 0;
#else
  return mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0;
#endif
}

bool Exists(const std::string &path)
{
#ifdef _WIN32
  return _access_s(path.c_str(), 0) == 0;
#else
  return access(path.c_str(), F_OK) == 0;
#endif
}

unsigned ParseUint(const std::string &str)
{
  return static_cast<unsigned>(std::stoi(str));
}

std::string LeftPad(const std::string &str, size_t size, char ch)
{
  std::string res;
  res.reserve(size > str.length() ? size : str.length());

  if (str.length() < size)
  {
    res.append(size - str.length(), ch);
  }

  res.append(str.data(), str.length());
  return res;
}

#ifdef _WIN32
static void ConvertWithCodePage(std::string& source,
                                unsigned fromCodePage,
                                unsigned toCodePage,
                                std::unordered_map<std::string, std::string>& cache)
{
  auto [newEntry, ok] = cache.try_emplace(source);
  if (!ok)
  {
    source = newEntry->second;
    return;
  }

  const auto sourceSize = static_cast<int>(source.size()); //-V202
  auto sourceBuf = source.c_str();
  auto size_needed = MultiByteToWideChar(fromCodePage, 0, sourceBuf, sourceSize, nullptr, 0);
  std::wstring intermediateStr(size_needed, 0);
  MultiByteToWideChar(fromCodePage, 0, sourceBuf, sourceSize, intermediateStr.data(), size_needed);

  const auto intermediateSize = static_cast<int>(intermediateStr.size()); //-V202
  auto intermediateBuf = intermediateStr.c_str();
  size_needed = WideCharToMultiByte(toCodePage, 0, intermediateBuf, intermediateSize, nullptr, 0, nullptr, nullptr);
  source = std::string(static_cast<size_t>(size_needed), 0); //-V201
  WideCharToMultiByte(toCodePage, 0, intermediateBuf, intermediateSize, source.data(), size_needed, nullptr, nullptr);

  newEntry->second = source;

}
#endif

void UTF8toANSI([[maybe_unused]] std::string& source)
{
#ifdef _WIN32
  static std::unordered_map<std::string, std::string> cache;
  ConvertWithCodePage(source, CP_UTF8, CP_ACP, cache);
#endif
}

void ANSItoUTF8([[maybe_unused]] std::string& source)
{
#ifdef _WIN32
  static std::unordered_map<std::string, std::string> cache;
  ConvertWithCodePage(source, CP_ACP, CP_UTF8, cache);
#endif
}

bool EqualPaths(std::string_view lhs, std::string_view rhs) noexcept
{
  using PathComparator = bool (*)(std::string_view, std::string_view);

#ifdef _WIN32
  constexpr PathComparator pathCmp = [](std::string_view lhs, std::string_view rhs)
  {
    return lhs.size() == rhs.size()
        && std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
                      [](char a, char b) { return tolower(a) == tolower(b); });
  };
#else
  constexpr PathComparator pathCmp = &std::operator==;
#endif

  return pathCmp(lhs, rhs);
}

bool LexicallyLesserPath(std::string_view lhs, std::string_view rhs) noexcept
{
  using PathComparator = bool (*)(std::string_view, std::string_view);

#ifdef _WIN32
  constexpr PathComparator pathCmp = [](std::string_view lhs, std::string_view rhs)
  {
    if (lhs.size() < rhs.size())
    {
      return true;
    }

    if (rhs.size() < lhs.size())
    {
      return false;
    }

    for (size_t i = 0; i < lhs.size(); ++i)
    {
      const auto ll = tolower(lhs[i]);
      const auto rl = tolower(rhs[i]);
      if (ll < rl)
      {
        return true;
      }

      if (rl < ll)
      {
        return false;
      }
    }

    return false;
  };
#else
  constexpr PathComparator pathCmp = [](std::string_view lhs, std::string_view rhs)
  {
    return lhs < rhs;
  };
#endif

  return pathCmp(lhs, rhs);
}

  std::string to_hex(unsigned char x)
  {
    char buf[3];
    snprintf(buf, sizeof(buf), "%2X", static_cast<uint32_t>(x));
    return std::string{ "%" } + (x < 0x10 ? "0" : "") + std::string{buf};
  }

  std::string GetSourceTreeRootMarker()
  {
    using namespace std::string_literals;

    static auto ret = "|?|"s;
    return ret;
  }

  const std::string &GetPathSeparator()
  {
    using namespace std::string_literals;
#ifdef _WIN32
    static auto sep { "\\"s };
#else
    static auto sep { "/"s  };
#endif

    return sep;
  }

  void ReplacePathPrefix(std::string &toReplace, std::string_view replacer)
  {
    std::error_code rc;
    auto relative = std::filesystem::relative(toReplace, replacer, rc); //-V821

    if (!rc && !relative.empty())
    {
      toReplace = GetSourceTreeRootMarker() + GetPathSeparator() + relative.string();
    }
  }

  void ReplaceRelativeRoot(std::string& str, const std::string& root)
  {
    Replace(str, GetSourceTreeRootMarker(), root);
  }

  void ReplaceAbsolutePrefix(std::string& str, const std::string& root)
  {
    ReplacePathPrefix(str, root);
  }

} //namespace PlogConverter

unsigned GetHashCodePVS(std::string_view msg) noexcept
{
    unsigned sum = 0;
    for (char ch : msg)
    {
        if (ch >= 0 && ch != ' ' && ch != '\t')
        {
            bool hiBit = (sum & 0x80000000u) != 0;
            sum <<= 1;
            sum ^= ch;
            if (hiBit)
                sum ^= 0x00000001u;
        }
    }
    return sum;
}

static constexpr bool IsDigit(char c) noexcept
{
  return '0' <= c && c <= '9';
}

static constexpr bool IsSpace(char c) noexcept
{
  return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\v' || c == '\f';
}

static constexpr std::string_view TrimRight(std::string_view str) noexcept
{
  while (!str.empty() && IsSpace(str.back()))
  {
    str.remove_suffix(1);
  }

  return str;
}

namespace PvsStudio
{
  std::string FixErrorString(std::string line)
  {
    char prev = '\0';
    char cur;

    for (auto it = line.begin(); it != line.end(); prev = cur)
    {
      cur = *it;
      if (IsDigit(cur))
      {
        if (IsDigit(prev))
        {
          it = line.erase(it);
        }
        else
        {
          *it++ = '_';
        }
      }
      else if (IsSpace(cur))
      {
        if (IsSpace(prev))
        {
          it = line.erase(it);
        }
        else
        {
          *it++ = ' ';
        }
      }
      else
      {
        ++it;
      }
    }

    while (!line.empty() && IsSpace(line.front()))
    {
      line.erase(line.begin());
    }

    while (!line.empty() && IsSpace(line.back()))
    {
      line.pop_back();
    }

    return line;
  }

  static bool IsAnalyzerErrorCode(std::string_view errStr)
  {
    if (errStr.size() != 4 && errStr.size() != 5)
    {
      return false;
    }

    if (errStr[0] != 'V' && errStr[0] != 'v')
    {
      return false;
    }

    for (auto i = 1; i <= 3; i++)
    {
      if (!IsDigit(errStr[i]))
      {
        return false;
      }
    }

    if (errStr.size() == 5)
    {
      if (!IsDigit(errStr[4]))
      {
        return false;
      }
    }

    return true;
  }

  static std::string_view DeleteMinusComments(std::string_view msg) noexcept
  {
    bool continueSearch;
    do
    {
      continueSearch = false;
      if (const auto index = msg.rfind("//-"); index != std::string_view::npos)
      {
        std::string_view v_marker = TrimRight(msg.substr(index + 3));
        if (continueSearch = IsAnalyzerErrorCode(v_marker); continueSearch)
        {
          msg = msg.substr(0, index);
        }
      }
    } while (continueSearch);

    return msg;
  }

  uint32_t PvsHash(std::string_view line, unsigned version) noexcept
  {
    if (line.empty()) { return 0; }

    if (version < 2)
    {
      auto lineFixed = FixErrorString(std::string{ line });
      return GetHashCodePVS(lineFixed);
    }

    return GetHashCodePVS(DeleteMinusComments(line));
  }
}
