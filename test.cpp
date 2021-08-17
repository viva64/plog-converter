// 2006-2008 (c) Viva64.com Team
// 2008-2020 (c) OOO "Program Verification Systems"
// 2020-2021 (c) PVS-Studio LLC

#include <gtest/gtest.h>
#include "warning.h"
#include "application.h"

using namespace PlogConverter;

TEST(plog_converter_test, WarningGetErrorCode)
{
  Warning msg;

  msg.code = "V112";
  ASSERT_TRUE(msg.GetErrorCode() == 112);

  msg.code = "V001";
  ASSERT_TRUE(msg.GetErrorCode() == 1);
}

TEST(plog_converter_test, WarningGetType)
{
  Warning msg;

  msg.code = "V112";
  ASSERT_TRUE(msg.GetType() == AnalyzerType::Viva64);

  msg.code = "V001";
  ASSERT_TRUE(msg.GetType() == AnalyzerType::Fail);
}

TEST(plog_converter_test, WarningGetVivaUrl)
{
  Warning msg;

  msg.code = "V001";
  ASSERT_TRUE(msg.GetVivaUrl() == "https://pvs-studio.com/en/docs/warnings/v001/");

  msg.code = "V101";
  ASSERT_TRUE(msg.GetVivaUrl() == "https://pvs-studio.com/en/docs/warnings/v101/");

  msg.code = "V1001";
  ASSERT_TRUE(msg.GetVivaUrl() == "https://pvs-studio.com/en/docs/warnings/v1001/");

  msg.code = "Renew";
  ASSERT_TRUE(msg.GetVivaUrl() == "https://pvs-studio.com/en/renewal/");
}

TEST(plog_converter_test, plog_converter_a)
{
  std::vector<Analyzer> analyzers;

  ParseEnabledAnalyzers("all", analyzers);
  ASSERT_TRUE(analyzers.empty());

  ParseEnabledAnalyzers("ALL", analyzers);
  ASSERT_TRUE(analyzers.empty());

  ParseEnabledAnalyzers("GA:1,2;64:1;OP:1,2,3;CS:1;MISRA:1,2;AUTOSAR:1,2;OWASP:1,2", analyzers);
  ASSERT_TRUE(analyzers.size() == 7);
  ASSERT_TRUE(analyzers[0].type == AnalyzerType::General);
  ASSERT_TRUE(analyzers[0].levels == (std::vector<int>{1, 2}));
  ASSERT_TRUE(analyzers[1].type == AnalyzerType::Viva64);
  ASSERT_TRUE(analyzers[1].levels == (std::vector<int>{1}));
  ASSERT_TRUE(analyzers[2].type == AnalyzerType::Optimization);
  ASSERT_TRUE(analyzers[2].levels == (std::vector<int>{1, 2, 3}));
  ASSERT_TRUE(analyzers[3].type == AnalyzerType::CustomerSpecific);
  ASSERT_TRUE(analyzers[3].levels == (std::vector<int>{1}));
  ASSERT_TRUE(analyzers[4].type == AnalyzerType::Misra);
  ASSERT_TRUE(analyzers[4].levels == (std::vector<int>{1, 2}));
  ASSERT_TRUE(analyzers[5].type == AnalyzerType::Autosar);
  ASSERT_TRUE(analyzers[5].levels == (std::vector<int>{1, 2}));
  ASSERT_TRUE(analyzers[6].type == AnalyzerType::Owasp);
  ASSERT_TRUE(analyzers[6].levels == (std::vector<int>{1, 2}));
}
