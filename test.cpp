#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include "warning.h"
#include "application.h"

using namespace PlogConverter;

TEST_CASE("Warning::GetErrorCode")
{
  Warning msg;

  msg.code = "V112";
  REQUIRE(msg.GetErrorCode() == 112);

  msg.code = "V001";
  REQUIRE(msg.GetErrorCode() == 1);
}

TEST_CASE("Warning::GetType")
{
  Warning msg;

  msg.code = "V112";
  REQUIRE(msg.GetType() == AnalyzerType::Viva64);

  msg.code = "V001";
  REQUIRE(msg.GetType() == AnalyzerType::Fail);
}

TEST_CASE("Warning::GetVivaUrl")
{
  Warning msg;

  msg.code = "V001";
  REQUIRE(msg.GetVivaUrl() == "https://www.viva64.com/en/w/v001/");

  msg.code = "V101";
  REQUIRE(msg.GetVivaUrl() == "https://www.viva64.com/en/w/v101/");

  msg.code = "V1001";
  REQUIRE(msg.GetVivaUrl() == "https://www.viva64.com/en/w/v1001/");

  msg.code = "Renew";
  REQUIRE(msg.GetVivaUrl() == "https://www.viva64.com/en/renewal/");
}

TEST_CASE("plog-converter -a")
{
  std::vector<Analyzer> analyzers;

  ParseEnabledAnalyzers("all", analyzers);
  REQUIRE(analyzers.empty());

  ParseEnabledAnalyzers("ALL", analyzers);
  REQUIRE(analyzers.empty());

  ParseEnabledAnalyzers("GA:1,2;64:1;OP:1,2,3;CS:1;MISRA:1,2", analyzers);
  REQUIRE(analyzers.size() == 5);
  REQUIRE(analyzers[0].type == AnalyzerType::General);
  REQUIRE(analyzers[0].levels == (std::vector<int>{1, 2}));
  REQUIRE(analyzers[1].type == AnalyzerType::Viva64);
  REQUIRE(analyzers[1].levels == (std::vector<int>{1}));
  REQUIRE(analyzers[2].type == AnalyzerType::Optimization);
  REQUIRE(analyzers[2].levels == (std::vector<int>{1, 2, 3}));
  REQUIRE(analyzers[3].type == AnalyzerType::CustomerSpecific);
  REQUIRE(analyzers[3].levels == (std::vector<int>{1}));
  REQUIRE(analyzers[4].type == AnalyzerType::Misra);
  REQUIRE(analyzers[4].levels == (std::vector<int>{1, 2}));
}