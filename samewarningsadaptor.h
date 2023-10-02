#pragma once

#include "ioutput.h"
#include "samewarningsfilter.h"

namespace PlogConverter
{
  // Adapter between IFilter and SameWarningsFilter.
  // You can use SameWarningsFilter to filter out the duplicate messages.
  class SameWarningsAdaptor : public IFilter<Warning>
  {
  private:
    using Base   = IFilter<Warning>;
    using Worker = SameWarningsFilter;

  public:
    SameWarningsAdaptor(IOutput<Warning> *output);

  public:
    [[nodiscard]] virtual bool Check(const Warning& warning) const override;

  private:
    mutable Worker m_worker;
  };
}