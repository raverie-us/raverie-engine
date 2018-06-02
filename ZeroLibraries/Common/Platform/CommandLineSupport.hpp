///////////////////////////////////////////////////////////////////////////////
/// Authors: Dane Curbow, Chris Peters
/// Copyright 2010-2016, DigiPen Institute of Technology
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
typedef HashMap<String, String> StringMap;

ZeroShared String GetFullCommandLine();
ZeroShared void GetCommandLineStringArray(Array<String>& strings);

// Not platform specific
void CommandLineToStringArray(Array<String>& strings, cstr* argv, int numberOfParameters);
bool ParseCommandLineStringArray(StringMap& parsedCommandLineArguments, Array<String>& commandLineArguments);

}// namespace Zero