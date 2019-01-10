//  2006-2008 (c) Viva64.com Team
//  2008-2019 (c) OOO "Program Verification Systems"

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

void MultipleOutput::Write(const Warning &msg)
{
  for (auto &o : m_outputs)
  {
    o->Write(msg);
  }
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
