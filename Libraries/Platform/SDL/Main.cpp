// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

extern "C" int main(int argc, char* argv[])
{
  using namespace Zero;
  printf("Welcome to the Zero Engine!\n");
  fflush(stdout);
  CommandLineToStringArray(gCommandLineArguments, argv, argc);
  return PlatformMain(gCommandLineArguments);
}
