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

bool static IsPathMatch(const std::string& srcPath, const std::string& pathMask)
{
  if (!IsGlobPath(pathMask))
  {      
    return ContainsSubPath(srcPath, pathMask);
  }

  return MatchPath(srcPath.c_str(), pathMask.c_str());
}

PathFilter::PathFilter(IOutput<Warning>* output, const ProgramOptions& options)
  : IFilter(output)
  , m_disabledPaths(options.disabledPaths)
  , m_enabledPaths(options.enabledPaths)
{
}

PathFilter::~PathFilter() = default;

bool PathFilter::Check(const Warning &message) const
{
  if (m_sourceTreeRootFound || (m_disabledPaths.empty() && m_enabledPaths.empty()))
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

  return IsIncludedPath(sourceFile) && !IsExcludedPath(sourceFile);
}

bool PathFilter::IsIncludedPath(const std::string &srcPath) const
{
  if (m_enabledPaths.empty() || srcPath.empty())
  {
    return true;
  }

  return std::any_of(std::begin(m_enabledPaths), std::end(m_enabledPaths), [srcPath](const std::string& include)
                                                                                    {
                                                                                      return IsPathMatch(srcPath, include);
                                                                                    });
}

bool PathFilter::IsExcludedPath(const std::string &srcPath) const
{
  if (m_disabledPaths.empty() || srcPath.empty())
  {
    return false;
  }

  return std::any_of(std::begin(m_disabledPaths), std::end(m_disabledPaths), [&srcPath](const std::string& exclude)
                                                                                       {
                                                                                         return IsPathMatch(srcPath, exclude);
                                                                                       });
}

}