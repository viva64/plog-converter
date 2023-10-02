#include "samewarningsadaptor.h"

namespace PlogConverter
{
  SameWarningsAdaptor::SameWarningsAdaptor(IOutput<Warning> *output)
    : Base{ output }
    , m_worker{}
  {}

  bool SameWarningsAdaptor::Check(const Warning& warning) const
  {
    return m_worker.IsUnique(warning);
  }
}