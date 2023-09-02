// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

using namespace Zero;

extern "C" int main(int argc, char* argv[])
{
  ZPrint("argc: %d\n", argc);
  for (int i = 0; i < argc; ++i) {
    ZPrint("argv[%d]: %s\n", i, argv[i]);
  }
  
  CommandLineToStringArray(gCommandLineArguments, argv, argc);
  SetupApplication(1, sRaverieOrganization, sEditorGuid, sEditorName);

  return (new GameOrEditorStartup())->Run();
}
