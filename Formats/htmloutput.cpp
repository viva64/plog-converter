//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020 (c) PVS-Studio LLC

#include <unordered_map>
#include <algorithm>
#include <fstream>
#include <ctime>
#include <string>
#include "htmloutput.h"
#include <resources.h>
#include <iomanip>
#include <sstream>

#include "compact_enc_det/compact_enc_det.h"

namespace PlogConverter
{

HTMLOutput::HTMLOutput(const ProgramOptions &options)
{
  m_errorCodeMappings = options.codeMappings;
  m_directory = options.output;
  m_cmdline = options.cmdLine;
  m_projectName = options.projectName;
  m_projectVersion = options.projectVersion;
  if (m_directory.empty())
  {
    throw std::logic_error("No output directory for html report");
  }

  if (m_directory.back() == '/' || m_directory.back() == '\\')
  {
    m_directory.pop_back();
  }

  if (Exists(m_directory))
  {
    m_directory += "/fullhtml";
  }

  if (Exists(m_directory))
  {
    throw std::runtime_error("Output directory already exists: " + m_directory);
  }

  std::vector<std::string> subdirs = { m_directory, m_directory + "/images", m_directory + "/sources" };
  for (const auto &s : subdirs)
  {
    if (!MakeDirectory(s))
    {
      throw std::runtime_error("Couldn't create directory for HTML report: " + s);
    }
  }

  m_ofstream.open(m_directory + "/index.html");
  if (!m_ofstream.is_open())
  {
    throw FilesystemException("Couldn't open " + m_directory + "/index.html");
  }

  m_desc.emplace(AnalyzerType::Fail, "Fail/Info");
  m_desc.emplace(AnalyzerType::General, "General Analysis");
  m_desc.emplace(AnalyzerType::Optimization, "Micro-optimizations");
  m_desc.emplace(AnalyzerType::Viva64, "64-bit errors");
  m_desc.emplace(AnalyzerType::CustomerSpecific, "Customers Specific");
  m_desc.emplace(AnalyzerType::Misra, "MISRA");
}

HTMLOutput::~HTMLOutput() = default;

static char HtmlHead[] = R"(
<!DOCTYPE HTML>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>PVS-Studio HTML Report</title>
<style type="text/css">
   TABLE {
    width: 100%;
   }
   #col_group, #col_level, #col_code, #col_cwe, #col_misra {
    text-align: center;
    white-space: nowrap;
    width: 0;
   }
   #col_file, #col_info, #col_projects {
    white-space: nowrap;
    text-align: left;
    width: 0;
   }
   #col_info {
    width: 120px;
   }
   #col_message {
    text-align: left;
   }
  </style>
<link rel="stylesheet" href="style.css" />
<script type="text/javascript" src="script.js"></script>
</head>
<body>
  <div class="header">
    <img src="images/logo.png" alt=""/>
    <h1>PVS-Studio Analysis Results</h1>
    <br class="clear">
  </div>
)";

static char HtmlEnd[] = R"(
  </table>
<script type="text/javascript">
var sorter=new table.sorter("sorter");
sorter.init("sorter",0);
</script>
</body>
</html>
)";

static char SourceHead1[] = R"(
<html>
<head>
)";

static char SourceHead2[] = R"(
  <link rel="stylesheet" href="../style.css"/>
  <script src="../jquery-3.5.1.min.js"></script>
</head>
<body>
)";

static char SourceEndPre[] = R"(
</code></pre>
)";

static char SourceEnd[] = R"(
<link rel="stylesheet" href="highlight.css">
<script src="highlight.pack.js"></script>
<script src="highlightjs-line-numbers.js"></script>
<script>hljs.initHighlightingOnLoad();</script>
<script>hljs.initLineNumbersOnLoad();</script>
<script>
  $(document).ready(function() {
      $('.balloon').each(function () {
          var bl = $(this);
          var line = bl.attr('rel');
          var text = $('a[name="ln'+line+'"]').text();

          var space_count = 0;
          for(var i = 0; i<text.length; i++){
              var char = text[i];
              if((char !== ' ')&&(char !== '\t'))break;
              if(char === '\t')space_count++;
              space_count++;
          }

          bl.css('margin-left', space_count*8);
          $('a[name="ln'+line+'"]').after(bl);
      });

      window.location = window.location;
  });
</script>
</body>
</html>
)";

static std::string TextLevel(unsigned level)
{
  if (level == 1)
    return "High";
  else if (level == 2)
    return "Medium";
  else if (level == 3)
    return "Low";
  else
    return "";
}

static std::string ColorLevel(unsigned level)
{
  if (level == 1)
    return "#FF0000";
  else if (level == 2)
    return "#FFA500";
  else if (level == 3)
    return "#CACA00";
  else
    return "#000000";
}

static std::string PVSStudioVersion()
{
#ifdef IDS_APP_VERSION
  return IDS_APP_VERSION;
#else
  return "Beta";
#endif
}

static std::string CurrentDateTime()
{
  std::ostringstream oss;
  time_t t = time(nullptr);
  oss << std::put_time(localtime(&t), "%c");
  return oss.str();
}

void HTMLOutput::CheckProjectsAndCWEAndMISRA()
{
  for (auto const &err : m_messages)
  {
    if (err.HasCWE())
    {
      m_hasAnyCWE = true;
    }

    if (err.HasMISRA())
    {
      m_hasAnyMISRA = true;
    }

    if (err.HasProjects())
    {
      m_hasAnyProjects = true;
    }

    if (m_hasAnyCWE && m_hasAnyMISRA && m_hasAnyProjects)
      break;
  }
}

void HTMLOutput::PrintFileExtra(const std::string&fileName, const std::string &data, std::ios_base::openmode mode)
{
  std::ofstream file(m_directory + fileName, mode);
  file.write(data.c_str(), data.length());
}

void HTMLOutput::PrintHtmlStart()
{
  m_ofstream << HtmlHead << std::endl;
}

void HTMLOutput::PrintHtmlEnd()
{
  m_ofstream << HtmlEnd << std::endl;
}

void HTMLOutput::PrintTableInfo()
{
  auto print_tr_th = [&](const std::string &caption, const std::string &value) {
    m_ofstream << R"(     <tr><th id="col_info">)" << caption << R"(</th><td>)" << value << R"(</td></tr>)" << std::endl;
  };

  m_ofstream << R"(  <table cellpadding="0" cellspacing="0" border="0" class="infotopic">)" << std::endl;

  print_tr_th("Date:", CurrentDateTime());

  if (!m_projectName.empty())
  {
    print_tr_th("Project name:", EscapeHtml(m_projectName));
  }

  if (!m_projectVersion.empty())
  {
    print_tr_th("Project version:", EscapeHtml(m_projectVersion));
  }

  print_tr_th("PVS-Studio Version:", PVSStudioVersion());

#ifndef _WIN32
  if (!m_cmdline.empty())
  {
    print_tr_th("Command Line:", m_cmdline);
  }
#endif

  if (m_ga > 0) print_tr_th("Total Warnings (GA):", std::to_string(m_ga));
  if (m_op > 0) print_tr_th("Total Warnings (OP):", std::to_string(m_op));
  if (m_64 > 0) print_tr_th("Total Warnings (64):", std::to_string(m_64));
  if (m_cs > 0) print_tr_th("Total Warnings (CS):", std::to_string(m_cs));
  if (m_misra > 0) print_tr_th("Total Warnings (MISRA):", std::to_string(m_misra));
  if (m_fails > 0) print_tr_th("Fails/Info:", std::to_string(m_fails));
  m_ofstream << R"(  </table>)" << std::endl;
}

void HTMLOutput::PrintTableCaption()
{
  m_ofstream << R"(  <table cellpadding="0" cellspacing="0" border="0" class="sortable" id="sorter">)" << std::endl;
  m_ofstream << R"(    <tr>)" << std::endl;
  m_ofstream << R"(      <th class="gsort">Group</th>)" << std::endl;
  if (m_hasAnyProjects)
    m_ofstream << R"(      <th class="psort">Projects</th>)" << std::endl;
  m_ofstream << R"(      <th>Location</th>)" << std::endl;
  m_ofstream << R"(      <th class="lsort">Level</th>)" << std::endl;
  m_ofstream << R"(      <th class="vsort">Code</th>)" << std::endl;

  for (const auto& security : m_errorCodeMappings)
  {
    if (security == SecurityCodeMapping::CWE && m_hasAnyCWE)
      m_ofstream << R"(      <th class="csort">CWE</th>)" << std::endl;

    if (security == SecurityCodeMapping::MISRA && m_hasAnyMISRA)
      m_ofstream << R"(      <th class="rsort">MISRA</th>)" << std::endl;
  }

  m_ofstream << R"(      <th>Message</th>)" << std::endl;
  m_ofstream << R"(    </tr>)" << std::endl;
}

void HTMLOutput::PrintTableBody()
{
  if (m_messages.empty())
  {
    m_ofstream << R"(    <tr>)" << std::endl;
    m_ofstream << R"(      <td id="col_group">)" << m_desc[AnalyzerType::Fail] << R"(</td>)" << std::endl;
    m_ofstream << R"(      <td id="col_file"></td>)" << std::endl;
    m_ofstream << R"(      <td id="col_level" style="color:)" << ColorLevel(1) << R"("><b>)" << TextLevel(1) << R"(</b></td>)" << std::endl;
    m_ofstream << R"(      <td id="col_code"></td>)" << std::endl;
    m_ofstream << R"(      <td id="col_message">)" << EscapeHtml("Congratulations! PVS-Studio has not found any issues in your source code!") << R"(</td>)" << std::endl;
    m_ofstream << R"(    </tr>)" << std::endl;
    return;
  }

  for (auto const &err : m_messages)
  {
    AnalyzerType analyzerType = err.GetType();
    std::string shortFileName = FileBaseName(err.GetFile());
    size_t id = Trim(err.GetFile()).empty() ? 0 : m_map[err.GetFile()];
    m_ofstream << R"(    <tr>)" << std::endl;
    m_ofstream << R"(      <td id="col_group">)" << m_desc[analyzerType] << R"(</td>)" << std::endl;

    if (m_hasAnyProjects)
    {
      if (err.projects.size() == 0)
        m_ofstream << R"(      <td id="col_projects"><p></p></td>)" << std::endl;
      else if (err.projects.size() == 1)
        m_ofstream << R"(      <td id="col_projects"><p>)" << err.projects[0] << R"(</p></td>)" << std::endl;
      else
        m_ofstream << R"(      <td id="col_projects"><p title=")" << Join(err.projects, ", ") << R"(">)" << err.projects[0] << R"( (...)</p></td>)" << std::endl;
    }
    if (shortFileName.empty())
    {
      m_ofstream << R"(      <td id="col_file"></td>)" << std::endl;
    }
    else
    {
      m_ofstream << R"(      <td id="col_file"><a target="_blank" href="sources/)"
                 << shortFileName << "_" << id << R"(.html#ln)" << err.GetLine() << R"(">)"
                 << EscapeHtml(FileBaseName(shortFileName)) << ':' << err.GetLine() << R"(</a></td>)" << std::endl;
    }
    m_ofstream << R"(      <td id="col_level" style="color:)" << ColorLevel(err.level) << R"("><b>)" << TextLevel(err.level) << R"(</b></td>)" << std::endl;
    m_ofstream << R"(      <td id="col_code"><a target="_blank" href=")" << err.GetVivaUrl() << R"(">)" << err.code << R"(</td>)" << std::endl;

    for (const auto& security : m_errorCodeMappings)
    {
      if (security == SecurityCodeMapping::CWE && m_hasAnyCWE)
      {
        if (err.HasCWE())
          m_ofstream << R"(      <td id="col_cwe"><a target="_blank" href=")" << err.GetCWEUrl() << R"(">)"
                     << err.GetCWEString() << R"(</td>)" << std::endl;
        else
          m_ofstream << R"(      <td id="col_cwe"></td>)" << std::endl;
      }

      if (security == SecurityCodeMapping::MISRA && m_hasAnyMISRA)
      {
        if (err.HasMISRA())
          m_ofstream << R"(      <td id="col_misra"><a target="_blank" href=")" << err.GetVivaUrl() << R"(">)"
                     << err.GetMISRAStringWithLanguagePrefix() << R"(</td>)" << std::endl;
        else
          m_ofstream << R"(      <td id="col_misra"></td>)" << std::endl;
      }
    }

    m_ofstream << R"(      <td id="col_message">)" << EscapeHtml(err.message) << R"(</td>)" << std::endl;
    m_ofstream << R"(    </tr>)" << std::endl;
  }
}

std::string HTMLOutput::GenerateSourceHtml(std::stringstream &sourceFile)
{
  std::string sourceHtml;
  std::string line;
  size_t i = 1;

  while (std::getline(sourceFile, line))
  {
    if (line.empty())
    {
      line += ' ';
    }

    if (line.back() == '\r')
    {
      line.pop_back();
    }

    line = R"(<a name="ln)" + std::to_string(i) + R"(">)" + EscapeHtml(line) + R"(</a>)";
    sourceHtml += line + "\n";
    i++;
  }

  return sourceHtml;
}

void HTMLOutput::PrintFileSources()
{
  for (const auto &p : m_map)
  {
    const auto &sourcePath = p.first;

    std::ifstream sourceFile(sourcePath);
    if (!sourceFile.is_open())
    {
      std::cerr << "Warning: Can't open file: " << sourcePath << '\n';
      continue;
    }
    std::stringstream sourceFileStr;
    sourceFileStr << sourceFile.rdbuf();
    sourceFile.close();
    const auto sourceHtml = GenerateSourceHtml(sourceFileStr);

    if (!sourceHtml.empty())
    {
      const auto shortFileName = FileBaseName(sourcePath);
      const auto fileExt = ToLower(FileExtension(shortFileName));
      const auto htmlPath = m_directory + "/sources/" + shortFileName + "_" + std::to_string(p.second) + ".html";
      int bytes_consumed;
      bool is_reliable;
      const auto fileEncoding = CompactEncDet::DetectEncoding(sourceFileStr.str().c_str(),
        static_cast<int>(sourceFileStr.str().length()), nullptr, nullptr, nullptr,
        UNKNOWN_ENCODING, UNKNOWN_LANGUAGE, CompactEncDet::TextCorpusType::QUERY_CORPUS, true,
        &bytes_consumed, &is_reliable);
      const auto mimeEncodingString = MimeEncodingName(fileEncoding);

      std::string sourceLanguage;
      if(fileExt == "java")
      {
        sourceLanguage = R"(<pre><code class = "java">)";
      }
      else if(fileExt == "cs")
      {
        sourceLanguage = R"(<pre><code class = "cs">)";
      }
      else
      {
        sourceLanguage = R"(<pre><code class = "cpp">)";
      }

      std::ofstream stream(htmlPath);

      stream << SourceHead1 << std::endl;
      stream << R"(  <meta http-equiv="Content-Type" content="text/html; charset=)" << mimeEncodingString << R"(" />)" << std::endl;
      stream << R"(  <title>)" << shortFileName << R"(</title>)" << std::endl;
      stream << SourceHead2 << std::endl;
      stream << sourceLanguage << std::endl;
      stream << sourceHtml << SourceEndPre;

      for (auto const &msg : m_messages)
      {
        if (msg.GetFile() == sourcePath)
        {
          stream << R"(<div class="balloon" rel=")" << std::to_string(msg.GetLine()) << R"(">)"
                 << R"(<p><span style="font-size:18px">&uarr;</span> <a href=")" << msg.GetVivaUrl() << R"(" )"
                 << R"(target="_blank">)" << msg.code << R"(</a> )" << msg.message << R"(</p></div>)"
                 << "\n";
        }
      }

      stream << SourceEnd;
    }
  }
}

void HTMLOutput::Write(const Warning &msg)
{
  if (msg.IsDocumentationLinkMessage())
  {
    return;
  }

  if (!Trim(msg.GetFile()).empty() && m_map.find(msg.GetFile()) == m_map.end())
  {
    m_map.emplace(msg.GetFile(), m_currentId++);
  }

  const auto analyzerType = msg.GetType();
  if (analyzerType == AnalyzerType::General)
    ++m_ga;
  else if (analyzerType == AnalyzerType::Optimization)
    ++m_op;
  else if (analyzerType == AnalyzerType::Viva64)
    ++m_64;
  else if (analyzerType == AnalyzerType::CustomerSpecific)
    ++m_cs;
  else if (analyzerType == AnalyzerType::Misra)
    ++m_misra;
  else
    ++m_fails;

  m_messages.push_back(msg);
}

void HTMLOutput::Finish()
{
  CheckProjectsAndCWEAndMISRA();

  PrintHtmlStart();
  PrintTableInfo();
  PrintTableCaption();
  PrintTableBody();
  PrintHtmlEnd();
  PrintFileExtra("/script.js", PlogConverter::Resources::SortJs());
  PrintFileExtra("/style.css", PlogConverter::Resources::StyleCss());
  PrintFileExtra("/jquery-3.5.1.min.js", PlogConverter::Resources::JQueryJs());

  PrintFileExtra("/sources/highlight.css", PlogConverter::Resources::HighlightCodeCss());
  PrintFileExtra("/sources/highlight.pack.js", PlogConverter::Resources::HighlightPackJs());
  PrintFileExtra("/sources/highlightjs-line-numbers.js", PlogConverter::Resources::HighlightLineJs());

  PrintFileExtra("/images/asc.gif", PlogConverter::Resources::AscGif(), std::ios_base::binary);
  PrintFileExtra("/images/desc.gif", PlogConverter::Resources::DescGif(), std::ios_base::binary);
  PrintFileExtra("/images/sort.gif", PlogConverter::Resources::SortGif(), std::ios_base::binary);
  PrintFileExtra("/images/logo.png", PlogConverter::Resources::LogoPng(), std::ios_base::binary);
  PrintFileSources();
}

}
