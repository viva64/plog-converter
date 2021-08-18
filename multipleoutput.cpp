//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2021 (c) PVS-Studio LLC

#include "multipleoutput.h"

namespace PlogConverter
{

MultipleOutput::MultipleOutput() = default;
MultipleOutput::~MultipleOutput() = default;

void MultipleOutput::Start()
{
  for (auto &o : m_outputs)
  {
    o->Start();
  }
}

bool MultipleOutput::Write(const Warning &msg)
{
  bool written = false;
  for (auto &o : m_outputs)
  {
    written |= o->Write(msg);
  }
  return written;
}

void MultipleOutput::Finish()
{
  for (auto &o : m_outputs)
  {
    o->Finish();
  }
}

void MultipleOutput::Add(std::unique_ptr<IOutput> output)
{
  if (!output)
  {
    throw std::runtime_error("output is nullptr");
  }

  m_outputs.push_back(std::move(output));
}

}
