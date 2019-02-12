// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

using namespace Zero;

extern "C" int main(int argc, char* argv[])
{
  CommandLineToStringArray(gCommandLineArguments, argv, argc);

  CommonLibrary::Initialize();
  int result = BrowserSubProcess::Execute();
  CommonLibrary::Shutdown();
  return result;
}
