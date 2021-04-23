//  2006-2008 (c) Viva64.com Team
//  2008-2020 (c) OOO "Program Verification Systems"
//  2020-2021 (c) PVS-Studio LLC

#include "application.h"
#include "logparserworker.h"
#include "messageparser.h"
#include "ThirdParty/backward.hpp"

int main(int argc, const char** argv)
{
  backward::SignalHandling handler {
    [](backward::Printer &printer, const backward::StackTrace &st)
    {
      const std::string path = "plog-converter.stacktrace.txt";
      if (std::ofstream stream { path })
      {
        std::cerr << "Application has crashed. Please send file "
                  << path 
                  << " to support@viva64.com"
                  << std::endl;
        printer.print(st, stream);
      }
      else
      {
        printer.print(st, std::cout);
      }
    }
  };

  PlogConverter::Application app;
  app.AddWorker(std::make_unique<PlogConverter::LogParserWorker>());
  return app.Exec(argc, argv);
}

