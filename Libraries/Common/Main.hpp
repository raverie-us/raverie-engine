// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{
extern ZeroShared Array<String> gCommandLineArguments;

// Not platform specific
typedef HashMap<String, String> StringMap;
void CommandLineToStringArray(Array<String>& strings,
                              cstr* argv,
                              int numberOfParameters);
void CommandLineToStringArray(Array<String>& strings,
                              char** argv,
                              int numberOfParameters);
bool ParseCommandLineStringArray(StringMap& parsedCommandLineArguments,
                                 Array<String>& commandLineArguments);

// Everyone implements this main instead of int main or platform specific mains
// like WinMain.
extern int PlatformMain(const Array<String>& arguments);

} // namespace Zero