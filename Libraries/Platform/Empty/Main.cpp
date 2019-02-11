// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

int main(int argc, char* argv[])
{
  using namespace Zero;
  CommandLineToStringArray(gCommandLineArguments, argv, argc);
  return PlatformMain(gCommandLineArguments);
}
