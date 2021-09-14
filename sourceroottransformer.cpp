//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2021 (c) PVS-Studio LLC

#include "sourceroottransformer.h"
#include "utils.h"

namespace PlogConverter
{

  static void ReplaseRelativeRoot(std::string& str, const std::string& root)
  {
    Replace(str, "|?|", root);
  }

  SourceRootTransformer::SourceRootTransformer(IOutput* output, const ProgramOptions& options)
    : ITransform(output)
    , m_options(options)
  {

  }

  SourceRootTransformer::~SourceRootTransformer() = default;

  Warning SourceRootTransformer::Transform(const Warning& message) const
  {
    auto copy = message;

    for (auto &position : copy.positions)
    {
      ReplaseRelativeRoot(position.file, m_options.projectRoot);
    }
    
    return copy;
  }
}
