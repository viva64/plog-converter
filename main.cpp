//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"

#include "application.h"
#include "logparserworker.h"
#include "messageparser.h"

int main(int argc, const char** argv)
{
  PlogConverter::Application app;
  app.AddWorker(std::make_unique<PlogConverter::LogParserWorker>());
  return app.Exec(argc, argv);
}

