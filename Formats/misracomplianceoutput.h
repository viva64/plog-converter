//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2021 (c) PVS-Studio LLC

#ifndef MISRACOMPLIANCEOUTPUT_H
#define MISRACOMPLIANCEOUTPUT_H

#include "ioutput.h"

#include "ThirdParty/strnatcmp/strnatcmp.h"

namespace PlogConverter
{

enum class Category
{
  None = 0,
  Advisory = 1,
  Required = 2,
  Mandatory = 3
};

enum class Compliance
{
  Compliant,
  Deviations,
  Violations,
  ViolationsDeviations,
  NotSupported
};

struct ComplianceData
{
  std::string guideline;
  Category defaultCategory = Category::None;
  Category recategorization = Category::None;
  Compliance compliance = Compliance::Compliant;
  int deviationsCount = 0;
  int violationsCount = 0;

  ComplianceData() = default;

  ComplianceData(Category category)
    : defaultCategory(category)
  {
  };

  ComplianceData(Category category, Compliance compliance)
    : defaultCategory(category), compliance(compliance)
  {
  };
};

struct naturalCmp
{
  bool operator()(const std::string& a, const std::string& b) const
  {
    return strnatcmp(a.data(), b.data()) < 0;
  }
};

class MisraComplianceOutput : public IOutput
{
public:
  explicit MisraComplianceOutput(const ProgramOptions& opt);
  void Start() override;
  void Write(const Warning& msg) override;
  void Finish() override;
  ~MisraComplianceOutput() override = default;

private:
  const std::string misraPrefix = "MISRA-C-";
  std::string m_directory;
  std::string m_grpFile;

  void PrintHtmlStart();
  void PrintHtmlEnd();
  void PrintTableRow(const ComplianceData&, bool);
  void PrintHtmpComplianceReport();
  void PrintHtmlComplianceHeader();
  void PrintFileExtra(const std::string&, const std::string&, std::ios_base::openmode);

  std::string GetMisraCCode(const std::string&);
  std::pair<bool, std::string> GetComplianceResult();
  std::string GetSummaryGuidelines(Compliance, int, int, int);
  std::map<std::string, ComplianceData, naturalCmp> &Categories();
  void SetComplianceContent(ComplianceData&);
  void RecategoriesByGRP();

  std::string ToString(Category);
  std::string ToString(Compliance, int, int);
  Category ToCategory(const std::string&);
};

}

#endif
