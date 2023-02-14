//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2023 (c) PVS-Studio LLC

#include <iomanip>
#include "totalsoutput.h"

namespace PlogConverter
{

TotalsOutput::TotalsOutput(const ProgramOptions& options) : BasicFormatOutput{ options }
{
}

void TotalsOutput::AddByLevel(TotalsOutput::Staticstic::WarningsCount &value, size_t level)
{
  switch (level)
  {
    case 1:
      ++std::get<0>(value);
      break;
    case 2:
      ++std::get<1>(value);
      break;
    case 3:
      ++std::get<2>(value);
      break;
    default:
      break;
  }
}

bool TotalsOutput::Write(const Warning& msg)
{
  switch (msg.GetType())
  {
    case AnalyzerType::Fail:
      ++m_stats.fails;
      return true;
    case AnalyzerType::General:
      AddByLevel(m_stats.generalAnalysis, msg.level);
      break;
    case AnalyzerType::Viva64:
      AddByLevel(m_stats.x64BitIssues, msg.level);
      break;
    case AnalyzerType::Optimization:
      AddByLevel(m_stats.optimization, msg.level);
      break;
    case AnalyzerType::CustomerSpecific:
      AddByLevel(m_stats.customerSpecific, msg.level);
      break;
    case AnalyzerType::Misra:
      AddByLevel(m_stats.misra, msg.level);
      break;
    case AnalyzerType::Owasp:
      AddByLevel(m_stats.owasp, msg.level);
      break;
    case AnalyzerType::Autosar:
      AddByLevel(m_stats.autosar, msg.level);
      break;
    case AnalyzerType::Unknown:
      return true;
  }

  AddByLevel(m_stats.total, msg.level);
  return true;
}

std::string TotalsOutput::ToLevelTriplet(const Staticstic::WarningsCount &value)
{
  auto &&[l1, l2, l3] = value;
  return "L1:" + std::to_string(l1) +
      " + L2:" + std::to_string(l2) +
      " + L3:" + std::to_string(l3)
      + " = "  + std::to_string(l1 + l2 + l3);
}

void TotalsOutput::Finish()
{
  std::string m_totalsOutput { "PVS - Studio analysis results\n" };
  m_totalsOutput += "General Analysis " + ToLevelTriplet(m_stats.generalAnalysis) + '\n';
  m_totalsOutput += "Optimization " + ToLevelTriplet(m_stats.optimization) + '\n';
  m_totalsOutput += "64-bit issues " + ToLevelTriplet(m_stats.x64BitIssues) + '\n';
  m_totalsOutput += "Customer Specific " + ToLevelTriplet(m_stats.customerSpecific) + '\n';
  m_totalsOutput += "MISRA " + ToLevelTriplet(m_stats.misra) + "\n";
  m_totalsOutput += "AUTOSAR " + ToLevelTriplet(m_stats.autosar) + "\n";
  m_totalsOutput += "OWASP " + ToLevelTriplet(m_stats.owasp) + "\n";
  m_totalsOutput += "Fails = " + std::to_string(m_stats.fails)  + "\n";
  m_totalsOutput += "Total " + ToLevelTriplet(m_stats.total);
  m_ostream << std::setw(2) << m_totalsOutput << std::endl;
  BasicFormatOutput<TotalsOutput>::Finish();
}

}
