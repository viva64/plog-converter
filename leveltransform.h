//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2023 (c) PVS-Studio LLC

#pragma once

#include "configs.h"
#include "ioutput.h"
#include "warning.h"

namespace PlogConverter
{
	class LevelTransform : public ITransform<Warning>
	{
	public:
		explicit LevelTransform(std::unique_ptr<IOutput<Warning>> warnings, const ProgramOptions &opt);
    ~LevelTransform() override
    {
      delete m_output;
    }

	private:
		Warning Transform(Warning warning) const override;
		ProgramOptions m_options;
	};
}