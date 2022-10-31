//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2022 (c) PVS-Studio LLC

#include "PathFilter.h"
#include "configs.h"
#include "ioutput.h"
#include <string>
#include <string_view>

namespace PlogConverter
{

static bool IsExcludePath(std::string_view srcPath, std::string_view excludePath)
{
  if (!IsGlobPath(excludePath))
  {      
    if (ContainsSubPath(srcPath, excludePath))
    {
      return true;
    }

    std::string excludePathPattern {"*" + std::string{ excludePath } + "*"};
    return MatchPath(srcPath.data(), excludePathPattern.c_str());
  }

  return MatchPath(srcPath.data(), excludePath.data());
}

PathFilter::PathFilter(IOutput<Warning>* output, const ProgramOptions& options)
  : IFilter(output)
  , m_disabledPaths(options.disabledPaths)
{
}

PathFilter::~PathFilter() = default;

bool PathFilter::Check(const Warning &message) const
{
  if (m_sourceTreeRootFound || m_disabledPaths.empty())
  {
    return true;
  }

  std::string sourceFile = message.GetFile();    
  
  if (sourceFile.find(GetSourceTreeRootMarker()) != std::string::npos)
  { 
    std::cout << R"(Warning: Filtering by file is only available for paths without the SourceTreeRoot marker.)"
                 R"(To filter by file for the report, specify the root directory via the '-r' flag.)" << std::endl;
    
    m_sourceTreeRootFound = true;
    return true;
  }

  return std::none_of(m_disabledPaths.begin(), m_disabledPaths.end(),
                      [&sourceFile](const std::string &path)
                      {
                        return IsExcludePath(sourceFile, path);
                      });
}

}