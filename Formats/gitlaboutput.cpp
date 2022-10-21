//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2022 (c) PVS-Studio LLC

#include <iomanip>

#include "json.hpp"
#include "gitlaboutput.h"

namespace PlogConverter
{

GitLabOutput::GitLabOutput(const ProgramOptions& options) 
  : BasicFormatOutput{ options }
{
}

void GitLabOutput::Start()
{
  m_gitLabOutput = std::vector<nlohmann::json>{};
}

constexpr std::string_view GetSeverityString(unsigned level)
{
  switch (level)
  {  
    case 1: return "critical";
    case 2: return "major";
    case 3: return "minor";
    default: return "blocker";
  }
}

bool GitLabOutput::Write(const Warning& msg)
{
  if (msg.GetErrorCode() == 0)
  {
    return false;
  }

  nlohmann::json msgGitLab;

  msgGitLab["description"] = msg.code + ": " + msg.message;

  msgGitLab["severity"] = GetSeverityString(msg.level);

  nlohmann::json locationGitLab;
  
  auto position = msg.positions.front();
    
  ANSItoUTF8(position.file);
    
  auto checkLine = position.file + std::to_string(position.line) + msg.code + msg.message;

  nlohmann::json linesGitLab;
  linesGitLab["begin"] = position.line;

  locationGitLab["lines"] = std::move(linesGitLab);

  ReplaceRelativeRoot(position.file, "");
  locationGitLab["path"] = std::move(position.file);

  msgGitLab["location"] = std::move(locationGitLab);
  msgGitLab["fingerprint"] = std::to_string(PvsStudio::PvsHash(checkLine));

  m_gitLabOutput.emplace_back(std::move(msgGitLab));

  return true;
}

void GitLabOutput::Finish()
{
  m_ostream << std::setw(2) << m_gitLabOutput << std::endl;
  BasicFormatOutput<GitLabOutput>::Finish();
}

}
