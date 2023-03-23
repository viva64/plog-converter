//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2023 (c) PVS-Studio LLC

#pragma once

#include "configs.h"
#include "ioutput.h"
#include "utils.h"
#include "warning.h"
#include "helpmessageoutput.h"

namespace PlogConverter
{
  class SourceRootRemover : public ITransform<Warning>
  {
  public:
    explicit SourceRootRemover(std::unique_ptr<IOutput<Warning>> output, const ProgramOptions &options);
    ~SourceRootRemover() override
    {
      delete m_output;
    }

  private:
    Warning Transform(Warning message) const override;
    const ProgramOptions &m_options;
  };


  class SourceRootChecker : public IFilter<Warning>
  {
    using Base = IFilter<Warning>;

  public:

    explicit SourceRootChecker(std::unique_ptr<IOutput<Warning>> output, const ProgramOptions &options)
      : Base(output.release())
      , m_options{ options }
    {
    }

    ~SourceRootChecker() override
    {
      delete m_output;
    }

  private:
    bool Check(const Warning &warning) const override
    {
      const auto GetFormatName = [](IOutput<PlogConverter::Warning> * output)
      {
        if (auto base = dynamic_cast<INameable *>(output))
        {
          return base->FormatName_();
        }
        return std::string_view("");
      };


      if (m_options.projectRoot.empty())
      {
        if (StartsWith(warning.GetFile(), GetSourceTreeRootMarker()))
        {
          if (!m_matched)
          {
            auto form = GetFormatName(m_output);
            m_matched = true;
            
            if (auto base = dynamic_cast<IFileClearable *>(m_output))
            {
              base->HardClearOutput();
            }

            std::cout << "Initial log has source tree root marker \"|?|\". The \'" << form << "\' log wasn't generated. Use \"-r\" option to form correct log." << std::endl;
          }
          return false;
        }
      }

      return true;
    }
    const ProgramOptions &m_options;
    mutable bool m_matched = false;
  };
}
