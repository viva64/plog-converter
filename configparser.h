//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2022 (c) PVS-Studio LLC

#ifndef CONFIGPARSER_H
#define CONFIGPARSER_H
#include <vector>
#include <map>
#include <string>
#include <stdexcept>
#include <set>

namespace PlogConverter
{

class ConfigException : public std::runtime_error
{
public:
  explicit ConfigException(const std::string& msg) : std::runtime_error(msg)
  {}
};

class ConfigParser
{
  const std::string m_pathToConfig;
  std::multimap<std::string, std::string> m_configMap;
public:
  ConfigParser(const std::string& pathToConfig, const std::vector<std::string>& multiArguments);
  ~ConfigParser();
  void get(const std::string& optionName, std::string& destination);
  void get(const std::string& optionName, std::vector<std::string>& destination, const std::string& delim = ",");
  void get(const std::string& optionName, std::set<std::string>& destination, const std::string& delim = ",");
  void getMulti(const std::string& optionName, std::vector<std::string>& destination);
  void getMulti(const std::string& optionName, std::set<std::string>& destination);
private:
  ConfigParser(const ConfigParser&);
  ConfigParser& operator=(const ConfigParser&);
};

}

#endif // CONFIGPARSER_H