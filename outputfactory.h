//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2021 (c) PVS-Studio LLC

#ifndef OUTPUTFACTORY_H
#define OUTPUTFACTORY_H

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <functional>
#include <memory>
#include "ioutput.h"

namespace PlogConverter
{

class OutputFactory
{
public:
  using AllocFunction = std::function<std::unique_ptr<IOutput>(const ProgramOptions&)>;

  OutputFactory();

  std::unique_ptr<IOutput> createOutput(const ProgramOptions &opt, const std::string& format);
  void registerOutput(const std::string& format, AllocFunction f);

  const std::unordered_map<std::string, AllocFunction>& getMap() const { return m_formats; }

private:
  std::unordered_map<std::string, AllocFunction> m_formats;
};

}

#endif // OUTPUTFACTORY_H
