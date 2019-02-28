// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

using namespace Zero;

extern "C" int main(int argc, char* argv[])
{
  CommandLineToStringArray(gCommandLineArguments, argv, argc);
  SetupApplication(1, 0, 0, 1, sWelderOrganization, sLauncherGuid, sLauncherName);

  return (new LauncherStartup())->Run();
}
