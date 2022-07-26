//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2022 (c) PVS-Studio LLC

#pragma once

#include <memory>
#include <string_view>
#include <unordered_map>

#include "ioutput.h"

namespace PlogConverter
{

class OutputFactory
{
public:
  using AllocFunction = std::function<std::unique_ptr<BaseFormatOutput>(const ProgramOptions&)>;

  OutputFactory();

  std::unique_ptr<BaseFormatOutput> createOutput(const ProgramOptions &opt, const std::string& format);
  void registerOutput(std::string_view format, AllocFunction f);

  [[nodiscard]] const std::unordered_map<std::string_view, AllocFunction>& getMap() const { return m_formats; }

private:
  std::unordered_map<std::string_view, AllocFunction> m_formats;
};

}
