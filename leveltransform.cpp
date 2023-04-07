//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2023 (c) PVS-Studio LLC

#include "leveltransform.h"

namespace PlogConverter 
{

  LevelTransform::LevelTransform(std::unique_ptr<IOutput<Warning>> output, const ProgramOptions &opt)
    : ITransform<Warning>(output.release())
    , m_options(opt)
  {
  }

  Warning LevelTransform::Transform(Warning warning) const
  {
    if (auto &&warningType = warning.GetType();
            m_options.separateNonCriticalToInfo 
        && (warningType == AnalyzerType::Optimization || warningType == AnalyzerType::Viva64))
		{
      warning.level = 4;
    }

	  return warning;
  }

}