// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{
extern ZeroShared Array<String> gCommandLineArguments;

// Not platform specific
typedef OrderedHashMap<String, String> StringMap;
void CommandLineToStringArray(Array<String>& strings, cstr* argv, int numberOfParameters);
void CommandLineToStringArray(Array<String>& strings, char** argv, int numberOfParameters);
bool ParseCommandLineStringArray(StringMap& parsedCommandLineArguments, Array<String>& commandLineArguments);

} // namespace Zero

// Everyone implements this main instead of platform specific mains.
extern "C" int main(int argc, char* argv[]);
