//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2023 (c) PVS-Studio LLC

#pragma once

#include "ioutput.h"
#include "ThirdParty/strnatcmp/strnatcmp.h"

namespace PlogConverter
{

enum class Category
{
  None = 0,
  Disapplied = 1,
  Advisory = 2,
  Required = 3,
  Mandatory = 4
};

enum class Compliance
{
  Compliant,
  Deviations,
  Violations,
  ViolationsDeviations,
  NotSupported,
  Disapplied
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

class MisraComplianceOutput : public BasicFormatOutput<MisraComplianceOutput>
{
public:
  using CategoriesMap = std::map<std::string, ComplianceData, naturalCmp>;

  explicit MisraComplianceOutput(const ProgramOptions& opt);
  ~MisraComplianceOutput() override = default;

  void Start() override;
  bool Write(const Warning& msg) override;
  void Finish() override;

  [[nodiscard]] static bool SupportsRelativePath() noexcept;
  [[nodiscard]] static bool OutputIsFile() noexcept;
  [[nodiscard]] static std::string_view FormatName() noexcept;
  [[nodiscard]] static std::string_view OutputSuffix() noexcept;

  static CategoriesMap& Categories();

private:
  const std::string misraPrefix = "MISRA-C-";
  std::filesystem::path m_grpFile;
  std::set<std::string> m_customDiviations;

  void PrintHtmlStart();
  void PrintHtmlEnd();
  void PrintTableRow(const ComplianceData&, bool);
  void PrintHtmpComplianceReport();
  void PrintHtmlComplianceHeader();

  std::string GetMisraCCode(const std::string&);
  std::pair<bool, std::string> GetComplianceResult();
  std::string GetSummaryGuidelines(Compliance, int, int, int);
  void SetComplianceContent(ComplianceData&);
  void RecategoriesByGRP();

  std::string ToString(Category);
  std::string ToString(Compliance, int, int);
  Category ToCategory(std::string_view);
};

}
