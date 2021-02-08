//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2021 (c) PVS-Studio LLC

#include "utils.h"
#include <cstring>
#include <fstream>

#include <cstddef>

#ifdef _WIN32
#include <io.h>
#include <Shlwapi.h>
#include <direct.h>
#else
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#endif


namespace PlogConverter
{

std::string Trim(const std::string& src)
{
  const static std::string spaceChars(" \t\n");
  std::string::size_type begin = src.find_first_not_of(spaceChars);
  if(begin == std::string::npos)
    begin = 0;
  std::string::size_type end = src.find_last_not_of(spaceChars + '\r');
  if(end != std::string::npos)
    end -= (begin - 1);
  else
    end = 0;
  return src.substr(begin, end);
}

std::pair<std::string, std::string> SplitKeyValue(const std::string& srcRaw)
{
  if (srcRaw.empty())
    return std::pair<std::string, std::string>();
  const std::string src = Trim(srcRaw);
  const static std::string spaceChars(" \t\n");
  size_t firstWordBound = src.find('=');
  size_t tailBound;
  if (firstWordBound != std::string::npos)
  {
    tailBound = firstWordBound + 1;
    while (tailBound < src.size() && spaceChars.find(src[tailBound]) != std::string::npos)
      ++tailBound;
    while (firstWordBound != 1 && spaceChars.find(src[firstWordBound - 1]) != std::string::npos)
      --firstWordBound;
  }
  else
    tailBound = src.size();
  return std::make_pair(src.substr(0, firstWordBound), src.substr(tailBound));
}

bool StartsWith(const std::string &src, const std::string &prefix)
{
  return prefix.empty() || (prefix.size() <= src.size() && src.compare(0, prefix.size(), prefix) == 0);
}

bool EndsWith(const std::string &src, const std::string &suffix)
{
  return suffix.empty() || (suffix.size() <= src.size() && src.compare(src.size() - suffix.size(), suffix.size(), suffix) == 0);
}

void Replace(std::string &src, const std::string& toReplace, const std::string &replacer)
{
  std::string::size_type pos = src.find(toReplace);
  if(pos != std::string::npos)
    src.replace(pos, toReplace.size(), replacer);
}

void ReplaceAll(std::string& src, const std::string& toReplace, const std::string& replacer)
{
  std::string::size_type pos = src.find(toReplace);
  while(pos != std::string::npos)
  {
    src.replace(pos, toReplace.size(), replacer);
    pos = src.find(toReplace, pos + replacer.size());
  }
}

std::ifstream OpenFile(const std::string &path)
{
  std::ifstream stream(path);
  if (!stream.is_open())
  {
    throw FilesystemException("File doesn't exist: " + path);
  }

  const int BOMSize = 3;
  char header[BOMSize];
  const unsigned char UTF8Bom[3] = { 0xEF, 0xBB, 0xBF };
  stream.read(header, BOMSize);

  if (!memcmp(header, UTF8Bom, BOMSize))
    stream.seekg(BOMSize);
  else
    stream.seekg(0);

  return stream;
}

std::string Expand(const std::string& path)
{
  std::string res = path;

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

std::string ToLower(const std::string &src)
{
  std::string str = src;
  for (char &c : str)
  {
    c = static_cast<char>(tolower(c));
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

}

