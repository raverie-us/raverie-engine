#include "Zilch.hpp"

using namespace Zilch;

int main()
{
  ZilchStartup(Debugging::UseZilchErrorHandler);

  CompilationErrors errors;
  errors.AddCallback(DefaultErrorCallback, nullptr);
  Project proj(errors);

  char buffer[4096] = {0};
  gets(buffer);

  proj.AddCodeFromString(buffer);

  Module dep(errors);
  proj.Compile("Test", dep, EvaluationMode::Project);
  
  ZilchShutdown();

  system("pause");
  
  return 0;
}
