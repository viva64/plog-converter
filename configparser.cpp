//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2021 (c) PVS-Studio LLC

#include "configparser.h"
#include "utils.h"
#include <iostream>
#include <utility>
#include <iterator>
#include <algorithm>

namespace PlogConverter
{

ConfigParser::ConfigParser(const std::string& pathToConfig, const std::vector<std::string>& multiArguments) :
  m_pathToConfig(pathToConfig)
{
  try
  {
    auto file = OpenFile(m_pathToConfig);
    std::string configLine;
    while (std::getline(file, configLine))
    {
      if (configLine.empty() || StartsWith(configLine, "#"))
        continue;
      auto arg = SplitKeyValue(configLine);
      if (arg.first.empty())
        continue;

      auto it = m_configMap.find(arg.first);
      if (it != m_configMap.end() && std::find(multiArguments.begin(), multiArguments.end(), arg.first) == multiArguments.end())
        it->second = std::move(arg.second);
      else
        m_configMap.insert(arg);
    }
  }
  catch (const FilesystemException& e)
  {
    throw ConfigException(std::string("Unable to read config: ") + e.what());
  }
}

ConfigParser::~ConfigParser() = default;

void ConfigParser::get(const std::string& optionName, std::string& destination)
{
  auto it = m_configMap.find(optionName);
  if (it != m_configMap.end())
    destination.assign(it->second);
}

void ConfigParser::get(const std::string& optionName, std::vector<std::string>& destination, const std::string& delim)
{
  auto it = m_configMap.find(optionName);
  if (it != m_configMap.end())
    Split(it->second, delim, std::back_inserter(destination));
}

void ConfigParser::get(const std::string& optionName, std::set<std::string>& destination, const std::string& delim)
{
  auto it = m_configMap.find(optionName);
  if (it != m_configMap.end())
    Split(it->second, delim, std::inserter(destination, destination.end()));
}

void ConfigParser::getMulti(const std::string& optionName, std::vector<std::string>& destination)
{
  auto range = m_configMap.equal_range(optionName);
  for (auto it = range.first; it != range.second; ++it)
    destination.push_back(it->second);
}

void ConfigParser::getMulti(const std::string& optionName, std::set<std::string>& destination)
{
  auto range = m_configMap.equal_range(optionName);
  for (auto it = range.first; it != range.second; ++it)
    destination.insert(it->second);
}

}
