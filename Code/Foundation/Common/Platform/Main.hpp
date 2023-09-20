// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{
extern Array<String> gCommandLineArguments;
extern String gCommandLine;

// Not platform specific
typedef OrderedHashMap<String, String> StringMap;
void CommandLineToStringArray();
bool ParseCommandLineStringArray(StringMap& parsedCommandLineArguments, Array<String>& commandLineArguments);

} // namespace Raverie
