///////////////////////////////////////////////////////////////////////////////
/// Authors: Dane Curbow, Chris Peters
/// Copyright 2010-2016, DigiPen Institute of Technology
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

String GetFullCommandLine()
{
  return Narrow(GetCommandLineW());
}

void GetCommandLineStringArray(Array<String>& strings)
{
  int numArguments = 0;
  wchar_t** commandLineArgs = CommandLineToArgvW(GetCommandLineW(), &numArguments);

  if (commandLineArgs && commandLineArgs)
    CommandLineToStringArray(strings, commandLineArgs, numArguments);
}

void CommandLineToStringArray(Array<String>& strings, wchar_t** argv, int numberOfParameters)
{
  for (int i = 0; i < numberOfParameters; ++i)
    strings.PushBack(Narrow(argv[i]));
}

}// namespace Zero