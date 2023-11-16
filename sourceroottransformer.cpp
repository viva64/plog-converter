//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2023 (c) PVS-Studio LLC

#include <filesystem>

#include "sourceroottransformer.h"
#include "utils.h"

namespace PlogConverter
{
  SourceRootTransformer::SourceRootTransformer(IOutput<Warning>* output, const ProgramOptions& options)
    : ITransform<Warning>(output)
    , m_options(options)
  {
  }

  Warning SourceRootTransformer::Transform(Warning message) const
  {
    if (m_options.pathTransformationMode != PathTransformationMode::NoTransform)
    {
      for (auto &position : message.positions)
      {
        if (m_options.pathTransformationMode == PathTransformationMode::ToAbsolute)
        {
          ReplaceRelativeRoot(position.file, m_options.projectRoot);
        }
        else if (m_options.pathTransformationMode == PathTransformationMode::ToRelative)
        {
          ReplaceAbsolutePrefix(position.file, m_options.projectRoot);
        }
      }
    }

    return message;
  }
}
