//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2023 (c) PVS-Studio LLC

#pragma once

#include "ioutput.h"

namespace PlogConverter
{

template<typename T>
class MultipleOutput : public IOutput<T>
{
public:
  MultipleOutput() = default;
  ~MultipleOutput() override = default;

  void Start() override
  {
    for (auto& o : m_outputs)
    {
      o->Start();
    }
  }

  bool Write(const T& msg) override
  {
    bool written = false;
    for (auto& o : m_outputs)
    {
      written |= o->Write(msg);
    }
    return written;
  }

  void Finish() override
  {
    for (auto& o : m_outputs)
    {
      o->Finish();
    }
  }

  void Add(std::unique_ptr<IOutput<T>> output)
  {
    if (!output)
    {
      throw std::runtime_error("output is nullptr");
    }

    m_outputs.push_back(std::move(output));
  }

  bool empty()
  {
    return m_outputs.empty();
  }

private:
  std::vector<std::unique_ptr<IOutput<T>>> m_outputs;
};

}
