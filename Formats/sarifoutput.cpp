//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020 (c) PVS-Studio LLC

#include "sarifoutput.h"
#include "PVS-StudioVersionInfo.h"

namespace PlogConverter
{

void SarifOutput::Write(const Warning& msg)
{
  if (msg.IsDocumentationLinkMessage())
  {
    return;
  }
  m_warnings.push_back(msg);
}

void SarifOutput::Finish()
{
  m_ostream
    << "{" << std::endl
    << "  \"version\": \"2.1.0\"," << std::endl
    << "  \"$schema\": \"https://raw.githubusercontent.com/oasis-tcs/sarif-spec/master/Schemata/sarif-schema-2.1.0.json\"," << std::endl
    << "  \"runs\": [" << std::endl
    << "    {" << std::endl
    << "       \"tool\": {" << std::endl
    << "        \"driver\": {" << std::endl
    << "          \"name\": \"PVS-Studio\"," << std::endl
    << "          \"semanticVersion\": \""<< IDS_APP_VERSION << "\"," << std::endl
    << "          \"rules\": [" << std::endl;

  std::set<std::string> rules;

  for (auto warning = m_warnings.begin(); warning != m_warnings.end(); warning++)
  {
    if (!rules.insert(warning->code).second)
    {
      continue;
    }
    
    m_ostream
      << "            " << (warning == m_warnings.begin() ? "{" : ",{") << std::endl
      << "              \"id\": \"" << warning->code << "\"," << std::endl
      << "              \"name\": \"Rule " << warning->code << "\"," << std::endl
      << "              \"help\": {" << std::endl
      << "                \"text\": \"" << warning->GetVivaUrl() << "\"" << std::endl
      << "              }" << std::endl;

    if (warning->HasCWE())
    {
      m_ostream
        << "              , \"properties\": {" << std::endl
        << "                \"tags\": [" << std::endl
        << "                  \"security\"," << std::endl
        << "                  \"external/cwe/cwe-" << warning->cwe << "\"" << std::endl
        << "                 ]" << std::endl
        << "               }" << std::endl;
    }
    m_ostream << "            }" << std::endl;
  }

  m_ostream
    << "          ]" << std::endl
    << "        }" << std::endl
    << "      }," << std::endl
    << "      \"results\": [" << std::endl;

  for (auto warning = m_warnings.begin(); warning != m_warnings.end(); warning++)
  {
    m_ostream
      << "        " << (warning == m_warnings.begin() ? "{" : ",{") << std::endl
      << "          \"ruleId\": \"" << warning->code << "\"," << std::endl
      << "          \"message\": {" << std::endl
      << "            \"text\": \"" << EscapeJson(warning->message) << "\"" << std::endl
      << "          }," << std::endl
      << "          \"level\": \"" << warning->GetLevelString() << "\"," << std::endl
      << "          \"locations\": [" << std::endl
      << "            {" << std::endl
      << "              \"physicalLocation\": {" << std::endl
      << "                \"artifactLocation\": {" << std::endl
      << "                  \"uri\": \"" << NormalizeFileName(warning->GetFile()) << "\"" << std::endl
      << "                }," << std::endl
      << "                \"region\": {" << std::endl
      << "                  \"startLine\": " << warning->GetLine() << "," << std::endl
      << "                  \"endLine\": " << warning->GetEndLine() << "," << std::endl
      << "                  \"startColumn\": " << warning->GetStartColumn() << "," << std::endl
      << "                  \"endColumn\": " << warning->GetEndColumn() << std::endl
      << "                }" << std::endl
      << "              }" << std::endl
      << "            }" << std::endl
      << "          ]" << std::endl
      << "        }" << std::endl;
  }

  m_ostream
    << "      ]" << std::endl
    << "    }" << std::endl
    << "  ]" << std::endl
    << "}" << std::endl;
}

std::string SarifOutput::EscapeJson(std::string& src)
{
  std::string str = src;
  ReplaceAll(str, "\b", "\\b");
  ReplaceAll(str, "\f", "\\f");
  ReplaceAll(str, "\n", "\\n");
  ReplaceAll(str, "\r", "\\r");
  ReplaceAll(str, "\t", "\\t");
  ReplaceAll(str, "\\", "\\\\");
  ReplaceAll(str, "\"", "\\\"");
  return str;
}

std::string SarifOutput::NormalizeFileName(const std::string& file)
{
  std::string str = file;
  ReplaceAll(str, "\\", "/");
  if (!str.empty() && str[0] == '/')
  {
    str = str.substr(1);
  }
  return str;
}

}
