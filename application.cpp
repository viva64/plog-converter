//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2023 (c) PVS-Studio LLC

#include <args.hxx>
#include <filesystem>
#include <cstring>
#include <iostream>
#include <iterator>
#include <string_view>

#include "application.h"
#include "argsextentions.h"
#include "configparser.h"
#include "Formats/misracomplianceoutput.h"
#include "logparserworker.h"
#include "outputfactory.h"
#include "utils.h"
#include "warning.h"

using namespace std::string_view_literals;
using namespace std::string_literals;

namespace PlogConverter
{

Analyzer ParseEnabledAnalyzer(const std::string &str)
{
  Analyzer res;
  auto pos = str.find(':');
  if (pos != std::string::npos)
  {
    Split(str.substr(pos + 1), ",", std::back_inserter(res.levels), [](const std::string &s) { return std::stoi(s); });
  }

  auto name = std::string_view { str }.substr(0, pos);
  if (name == "FAIL")
    res.type = AnalyzerType::Fail;
  else if (name == "GA")
    res.type = AnalyzerType::General;
  else if (name == "64")
    res.type = AnalyzerType::Viva64;
  else if (name == "OP")
    res.type = AnalyzerType::Optimization;
  else if (name == "CS")
    res.type = AnalyzerType::CustomerSpecific;
  else if (name == "MISRA")
    res.type = AnalyzerType::Misra;
  else if (name == "AUTOSAR")
    res.type = AnalyzerType::Autosar;
  else if (name == "OWASP")
    res.type = AnalyzerType::Owasp;
  else
    throw ConfigException("Wrong analyzer name: " + std::string { name });

  return res;
}

void ParseEnabledAnalyzers(std::string str, std::vector<Analyzer>& analyzers)
{
  analyzers.clear();

  try
  {
    if (str == "all" || str == "ALL")
    {
      return;
    }

    ReplaceAll(str, "+", ";");
    Split(str, ";", std::back_inserter(analyzers), &ParseEnabledAnalyzer);
  }
  catch (std::exception&)
  {
    throw ConfigException("Wrong analyzers format: " + str);
  }
}

void Application::AddWorker(std::unique_ptr<IWorker> worker)
{
  if (!worker)
  {
    throw std::invalid_argument("worker is nullptr");
  }

  m_workers.push_back(std::move(worker));
}

int GetErrorCode(ConverterRunState state)
{
  return static_cast<int>(state);
}

int Application::Exec(int argc, const char** argv)
{
  bool isUnsopportedTransformation = false;

  try
  {
#ifdef _WIN32
    std::cout << AppName_Win << std::endl;
#else
    std::cout << AppName_Default << std::endl;
#endif
    std::cout << "Copyright (c) 2023 PVS-Studio LLC" << std::endl;
    std::cout << AboutPVSStudio << std::endl;
    
    SetCmdOptions(argc, argv);
    SetConfigOptions(m_options.configFile);

    if (m_workers.empty())
    {
      throw std::logic_error("No workers set, nothing to do");
    }

    if (!m_options.enabledAnalyzers.empty())
    {
      if (std::find(m_options.codeMappings.cbegin(), m_options.codeMappings.cend(), SecurityCodeMapping::MISRA) != m_options.codeMappings.cend() &&
        std::find_if(m_options.enabledAnalyzers.cbegin(), m_options.enabledAnalyzers.cend(), [](const Analyzer& item) { return item.type == AnalyzerType::Misra; }) == m_options.enabledAnalyzers.cend())
      {
        std::cout << "MISRA mapping is specified, but MISRA rules group is not enabled. Check the '-" << CmdAnalyzerFlagName_Short << "' flag." << std::endl;
      }

      if (std::find(m_options.codeMappings.cbegin(), m_options.codeMappings.cend(), SecurityCodeMapping::AUTOSAR) != m_options.codeMappings.cend() &&
        std::find_if(m_options.enabledAnalyzers.cbegin(), m_options.enabledAnalyzers.cend(), [](const Analyzer& item) { return item.type == AnalyzerType::Autosar; }) == m_options.enabledAnalyzers.cend())
      {
        std::cout << "AUTOSAR mapping is specified, but AUTOSAR rules group is not enabled. Check the '-" << CmdAnalyzerFlagName_Short << "' flag." << std::endl;
      }

      if (std::find(m_options.codeMappings.cbegin(), m_options.codeMappings.cend(), SecurityCodeMapping::OWASP) != m_options.codeMappings.cend() &&
        std::find_if(m_options.enabledAnalyzers.cbegin(), m_options.enabledAnalyzers.cend(), [](const Analyzer& item) { return item.type == AnalyzerType::Owasp; }) == m_options.enabledAnalyzers.cend())
      {
        std::cout << "OWASP mapping is specified, but OWASP rules group is not enabled. Check the '-" << CmdAnalyzerFlagName_Short << "' flag." << std::endl;
      }
    }

    for (int i = 0; i < argc; ++i)
    {
      for (const char *arg = argv[i]; *arg != '\0'; ++arg)
      {
        char c = *arg;
        if (isalpha(c) || isdigit(c) || strchr(".,_-/", c) != nullptr)
        {
          m_options.cmdLine += c;
        }
        else
        {
          m_options.cmdLine += '\\';
          m_options.cmdLine += c;
        }
      }
      m_options.cmdLine += ' ';
    }

    for (auto &worker : m_workers)
    {
      worker->Run(m_options);

      if (worker->IsErrorHappend())
      {
        isUnsopportedTransformation = true;
      }
    }
  }
  catch (const ConfigException& e)
  {
    const char* err = e.what();
    if (err != nullptr && err[0] != '\0')
      std::cerr << e.what() << std::endl;
    return GetErrorCode(ConverterRunState::GenericException);
  }
  catch (const std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << std::endl;
    return GetErrorCode(ConverterRunState::GenericException);
  }

  if (isUnsopportedTransformation)
  {
    return GetErrorCode(ConverterRunState::UnsopportedPathTransofrmation);
  }

  if (m_options.indicateWarnings)
  {
    for (auto &worker : m_workers)
    {
      auto logWorker = dynamic_cast<const PlogConverter::LogParserWorker*>(worker.get());
      if (logWorker != nullptr && logWorker->GetPrintedWarnings() > 0)
      {
        return GetErrorCode(ConverterRunState::OutputLogNotEmpty);
      }
    }
  }

  return GetErrorCode(ConverterRunState::Success);
}

static std::set<std::string> ParseMisraDiviations(std::string_view flagValue)
{
  std::set<std::string> result;

  auto startPos = flagValue.begin();
  while (startPos != flagValue.end())
  {
    auto semicolonPos = std::find(startPos, flagValue.end(), ';');

    std::string word = { startPos, semicolonPos };
    if (MisraComplianceOutput::Categories().count(word) == 0)
    {
      std::cout << "Warning: The rule \"" << word << "\" not found." << std::endl;
    }

    result.emplace(std::move(word));

    startPos = semicolonPos;
    if (semicolonPos != flagValue.end())
    {
      ++startPos; // skip semicolon
    }
  }

  return result;
}

static PathTransformationMode ParsePathTransformationMode(std::string_view flagValue, std::string_view srcRoot)
{
  if (srcRoot.empty())
  {
    return PathTransformationMode::NoTransform;
  }

  if (flagValue.empty() || flagValue == "toAbsolute"sv)
  {
    return PathTransformationMode::ToAbsolute;
  }
  else if (flagValue == "toRelative"sv)
  {
    return PathTransformationMode::ToRelative;
  }

  return PathTransformationMode::NoTransform;
}

const std::string Application::AppName_Default = "Analyzer log conversion tool.";
const std::string Application::AppName_Win = "Analyzer log conversion tool.";
const std::string Application::AboutPVSStudio = R"(
PVS-Studio is a static code analyzer and SAST (static application security
testing) tool that is available for C and C++ desktop and embedded development,
C# and Java under Windows, Linux and macOS.
)";

const char Application::CmdAnalyzerFlagName_Short = 'a';
const std::string Application::CmdAnalyzerFlagName_Full = "analyzer";

void Application::SetCmdOptions(int argc, const char** argv)
{
  using namespace args;

  ArgumentParser parser("");
#ifdef _WIN32
  parser.Prog(AppName_Win);
#else
  parser.Prog(AppName_Default);
#endif
  parser.helpParams.usageString = "Usage:";
  parser.helpParams.progindent = 0;
  parser.helpParams.width = 70;
  parser.helpParams.progtailindent = unsigned(parser.helpParams.usageString.length() + parser.Prog().length() + 2);
  parser.helpParams.helpindent = 30;
  parser.helpParams.flagindent = 4;
  parser.helpParams.optionsString = "Options:";
  parser.helpParams.proglineShowFlags = true;
  parser.helpParams.proglinePreferShortFlags = true;
  parser.helpParams.showTerminator = false;
  parser.helpParams.showValueName = false;
  parser.helpParams.addDefault = true;
  parser.helpParams.addChoices = true;

  //todo case-insensitive flags/args
  HelpFlag helpFlag(parser, "HELP", "Show this help page", { 'h', "help" }, Options::Hidden);

  MapFlagListMulti<std::string_view, OutputFactory::AllocFunction> renderTypes(parser, "TYPES", "Render types for output.",
                                         { 't', "renderTypes" }, outputFactory.getMap());
  ValueFlag<std::string> outputFile(parser, "FILE", "Output file.", { 'o', "output" }, Options::Single);
  outputFile.HelpDefault("<stdout>");
  Flag useStdout(parser, "USESTDOUT", "Display the report to standard output when '--output' parameter specified.", { "stdout" }, Options::Single);
  Flag separateNonCriticalToInfo(parser, "SEPARATENONCRITICAL", "Tag all the warnings from the 64-bit diagnostic groups and micro-optimization diagnostic groups as informational. Only for fullhtml report.", { "separateNonCriticalToInfo" }, Options::Single | Options::Hidden);
  ValueFlag<std::string> sourceRoot(parser, "PATH", "A path to the project directory.", { 'r', "srcRoot" }, Options::Single);
  ValueFlag<std::string> analyzer(parser, "TYPES", "Specifies analyzer(s) and level(s) to be used for filtering, i.e. 'GA:1,2;64:1;OP:1,2,3;CS:1;MISRA:1,2'",
                                  { CmdAnalyzerFlagName_Short, CmdAnalyzerFlagName_Full }, "GA:1,2", Options::Single);
  ValueFlagListMulti<std::string, PathSplitter> excludePaths{ parser, "PATH", "Excludes from the report all warnings issued in certain files. Separate the paths or masks with the ';' character.", {'E', "excludePaths"} };
  ValueFlagListMulti<std::string, PathSplitter> includePaths{ parser, "PATH", "Include in the report only warnings issued on specified files. Separate the paths or masks with the ';' character.", {'I', "includePaths"} };
  ValueFlag<std::string> excludedCodes(parser, "CODES", "Error codes to disable, i.e. V112,V122.", { 'd', "excludedCodes" }, Options::Single);
  ValueFlag<std::string> settings(parser, "FILE", "Path to PVS-Studio settings file. Can be used to specify additional disabled error codes.",
                                  { 's', "settings" }, Options::Single);
  ValueFlag<std::string> name(parser, "FILENAME", "Template name for resulting output files.", { 'n', "name" }, Options::Single);
  MapFlagList<std::string, SecurityCodeMapping> mappingTypes(parser, "NAME", "Enable mapping of PVS-Studio error codes to other rule sets.",
                                                             { 'm',"errorCodeMapping" }, { { "cwe", SecurityCodeMapping::CWE }, { "misra", SecurityCodeMapping::MISRA }, { "autosar", SecurityCodeMapping::AUTOSAR }, { "owasp", SecurityCodeMapping::OWASP } });
  ValueFlag<std::string> projectName(parser, "PROJNAME", "Name of the project for fullhtml render type.", { 'p', "projectName" }, "", Options::Single);
  ValueFlag<std::string> projectVersion(parser, "PROJVERSION", "Version of the project for fullhtml render type.", { 'v', "projectVersion" }, "", Options::Single);
  Flag useCerr(parser, "CERR", "Use stderr instead of stdout.", { 'e', "cerr" }, Options::Single);
  Flag indicateWarningsDeprecated(parser, "INDICATEWARNINGS (Deprecated)", 
                                  "(Deprecated) Set this option to detect the presense of analyzer warnings after "
                                  "filtering analysis log by setting the converter exit code to '2'.\nThis flag is alias to \'indicateWarnings\'", 
                                  { "indicate-warnings" }, Options::Single | Options::Hidden);
  Flag indicateWarnings          (parser, "INDICATEWARNINGS", 
                                  "Set this option to detect the presense of analyzer warnings after "
                                  "filtering analysis log by setting the converter exit code to '2'.", 
                                  { 'w', "indicateWarnings" }, Options::Single);
  PositionalList<std::string> logs(parser, "logs", "Input files.", Options::Required | Options::HiddenFromDescription);
  CompletionFlag comp(parser, {"complete"});
  ValueFlag<std::string> grp(parser, "GRP", "Path to txt file with Guideline Re-categorization Plan. Used only for generating misra compliance report.", { "grp" }, Options::Single);
  ValueFlag<std::string> misraDiviations(parser, "Misra Diviations", "Misra Diviations.", { "misraDeviations" }, Options::Single);
  Flag noHelp(parser, "NOHELP", "Do not display documentation messages in warnings output.", { "noHelpMessages" }, Options::Single);
  ValueFlag<std::string> pathTransformationMode { parser,
                                                  "PATHTRANSFORMATIONMODE",
                                                  "Trasformation mode:\n"
                                                  "toAbsolute - transform to absolute path,\n"
                                                  "toRelative - transform to relative with source root (--srcRoot option).",
                                                  { 'R', "pathTransformationMode" },
                                                  Options::Single
                                                };

  try
  {
    parser.ParseCLI(argc, argv);

    auto outputPath = get(outputFile);

    m_options.output = Expand(get(outputFile));
    std::transform(get(logs).begin(), get(logs).end(), std::back_inserter(m_options.inputFiles), &Expand);
    m_options.projectRoot = Expand(get(sourceRoot));
    m_options.formats = get(renderTypes);
    m_options.projectName = get(projectName);
    m_options.projectVersion = get(projectVersion);
    m_options.codeMappings = get(mappingTypes);
    m_options.configFile = Expand(get(settings));
    m_options.outputName = Expand(get(name));
    m_options.grp = Expand(get(grp));
    m_options.misraDivations = ParseMisraDiviations(get(misraDiviations));
    m_options.useStderr = useCerr;
    m_options.noHelp = noHelp;
    m_options.useStdout = useStdout;
    m_options.separateNonCriticalToInfo = separateNonCriticalToInfo;
    m_options.indicateWarnings = indicateWarnings || indicateWarningsDeprecated;
    m_options.pathTransformationMode = ParsePathTransformationMode(get(pathTransformationMode), m_options.projectRoot);
    std::transform(std::begin(excludePaths), std::end(excludePaths), std::back_inserter(m_options.disabledPaths), &GetAbsolutePath);
    std::transform(std::begin(includePaths), std::end(includePaths), std::back_inserter(m_options.enabledPaths), &GetAbsolutePath);

    Split(get(excludedCodes), ",", std::inserter(m_options.disabledWarnings, m_options.disabledWarnings.begin()));
    ParseEnabledAnalyzers(get(analyzer), m_options.enabledAnalyzers);

    if (m_options.formats.size() > 1 && (m_options.useStdout || (m_options.output.empty() && m_options.outputName.empty())))
    {
      throw ConfigException("Standard output is not supported for multiple output formats. Specify output directory --output (-o).");
    }
  }
  catch (Completion &e)
  {
    std::cout << e.what();
    exit(GetErrorCode(ConverterRunState::Success));
  }
  catch (Help&)
  {
    std::cout << parser;
    throw ConfigException("");
  }
  catch (Error &e)
  {
    std::cerr << e.what() << std::endl;
    std::cout << parser;
    throw ConfigException("");
  }
}

void Application::SetConfigOptions(const std::string& path)
{
  if (path.empty())
  {
    return;
  }

  const static std::vector<std::string> multiArguments = { "exclude-path" };
  std::string enabledAnalyzers;

  ConfigParser configParser(path, multiArguments);
  configParser.get("errors-off", m_options.disabledWarnings, " ");
  configParser.getMulti("exclude-path", m_options.disabledPaths);
  configParser.get("disabled-keywords", m_options.disabledKeywords);
  configParser.get("enabled-analyzers", enabledAnalyzers);

  if (m_options.projectRoot.empty())
  {
    configParser.get("sourcetree-root", m_options.projectRoot);
    if (!m_options.projectRoot.empty())
    {
      m_options.projectRoot = Expand(m_options.projectRoot);
    }
  }

  if (!enabledAnalyzers.empty())
  {
    ParseEnabledAnalyzers(enabledAnalyzers, m_options.enabledAnalyzers);
  }
}

}
