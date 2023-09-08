// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{
extern ZeroShared Array<String> gCommandLineArguments;
extern size_t gCommandLineBufferLength;
extern char* gCommandLineBuffer;

// Not platform specific
typedef OrderedHashMap<String, String> StringMap;
void CommandLineToStringArray();
bool ParseCommandLineStringArray(StringMap& parsedCommandLineArguments, Array<String>& commandLineArguments);

} // namespace Zero
