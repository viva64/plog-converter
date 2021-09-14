//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2021 (c) PVS-Studio LLC

#ifndef SOURCEROOTTRANSFORMER_H
#define SOURCEROOTTRANSFORMER_H
#include "configs.h"
#include "warning.h"
#include "utils.h"
#include "ioutput.h"

namespace PlogConverter
{

class SourceRootTransformer : public ITransform
{
public:
  explicit SourceRootTransformer(IOutput* output, const ProgramOptions& options);
  ~SourceRootTransformer() override;

private:
  Warning Transform(const Warning& message) const override;
  const ProgramOptions &m_options;
};

}

#endif // SOURCEROOTTRANSFORMER_H
