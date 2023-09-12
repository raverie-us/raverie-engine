// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{
Array<String> gCommandLineArguments;
size_t gCommandLineBufferLength = 0;
char* gCommandLineBuffer = nullptr;

void CommandLineToStringArray()
{
  size_t start = 0;
  for (size_t i = 0; i < gCommandLineBufferLength; ++i) {
    if (gCommandLineBuffer[i] == '\0') {
      if (i == start) {
        break;
      }

      String arg(gCommandLineBuffer + start, gCommandLineBuffer + i);
      start = i + 1;
      gCommandLineArguments.PushBack(arg);
    }
  }
}

bool ParseCommandLineStringArray(StringMap& parsedCommandLineArguments, Array<String>& commandLineArguments)
{
  forRange(String& arg, commandLineArguments.All()) {
    ZPrint("Raw Argument: %s\n", arg.c_str());
  }

  size_t index = 0;

  while (index < commandLineArguments.Size())
  {
    StringRange optionName = commandLineArguments[index];

    // Check for '-' at beginning of an option
    if (optionName == '-')
    {
      // eat the '-'
      optionName.PopFront();

      size_t paramIndex = index + 1;

      // Is there a parameter?
      if (paramIndex < commandLineArguments.Size() && commandLineArguments[paramIndex].Front() != '-')
      {
        StringRange parameter = commandLineArguments[paramIndex];
        parsedCommandLineArguments[optionName] = parameter;
        index += 2;
      }
      else
      {
        // Add simple bool parameter
        parsedCommandLineArguments[optionName] = "true";
        ++index;
      }
    }
    else
    {
      if (index == 1)
      {
        parsedCommandLineArguments["file"] = optionName;
        ++index;
      }
      else
      {
        // bad command line
        return false;
      }
    }
  }
  return true;
}

} // namespace Zero
