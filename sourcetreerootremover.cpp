//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2023 (c) PVS-Studio LLC

#include <filesystem>

#include "sourcetreerootremover.h"
#include "utils.h"

namespace PlogConverter
{
  SourceRootRemover::SourceRootRemover(std::unique_ptr<IOutput<Warning>> output, const ProgramOptions &options)
    : ITransform<Warning>(output.release())
    , m_options(options)
  {
  }

  Warning SourceRootRemover::Transform(Warning message) const
  {
    if (m_options.pathTransformationMode != PathTransformationMode::ToRelative)
    {
      for (auto &position : message.positions)
      {
        ReplaceRelativeRoot(position.file, ".");
      }
    }

    return message;
  }
}
