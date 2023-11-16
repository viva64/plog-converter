//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2023 (c) PVS-Studio LLC

#pragma once

#include "configs.h"
#include "ioutput.h"
#include "utils.h"
#include "warning.h"
#include "utils.h"

namespace PlogConverter
{

class SourceRootTransformer : public ITransform<Warning>
{
public:
  explicit SourceRootTransformer(IOutput<Warning>* output, const ProgramOptions& options);
  ~SourceRootTransformer() override = default;

private:
  Warning Transform(Warning message) const override;
  const ProgramOptions &m_options;
};

}
