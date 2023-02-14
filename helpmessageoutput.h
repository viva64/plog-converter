//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2023 (c) PVS-Studio LLC

#pragma once

#include "configs.h"
#include "ioutput.h"
#include "utils.h"
#include "warning.h"

namespace PlogConverter
{

  class HelpMessageOutput : public IOutput<Warning>
  {
  public:
    using NextOutputPtr = std::unique_ptr<IOutput<Warning>>;

    explicit HelpMessageOutput(NextOutputPtr output);
    ~HelpMessageOutput() override = default;

    virtual void Start()                       override;
    virtual bool Write(const Warning& message) override;
    virtual void Finish()                      override;

  private:
    NextOutputPtr m_nextOutput;
    bool m_helpMsgPrinted;
  };

}