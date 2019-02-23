// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

using namespace Zero;

namespace Zero
{
bool ZeroEditorStartup();
}

extern "C" int main(int argc, char* argv[])
{
  CommandLineToStringArray(gCommandLineArguments, argv, argc);

  SetupApplication(1, 0, 0, 1, sWelderOrganization, sEditorGuid, sEditorName);

  ZeroStartup startup;
  Engine* engine = startup.Initialize();
  startup.Startup();

  // Run application specific startup
  bool success = ZeroEditorStartup();

  // Failed startup do not run
  if (!success)
    return 1;

  // Run engine until termination
  engine->Run();

  startup.Shutdown();

  ZPrint("Terminated\n");

  return 0;
}
