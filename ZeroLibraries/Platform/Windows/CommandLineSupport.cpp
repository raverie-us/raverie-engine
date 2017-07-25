///////////////////////////////////////////////////////////////////////////////
/// Authors: Dane Curbow, Chris Peters
/// Copyright 2010-2016, DigiPen Institute of Technology
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "Platform/CommandLineSupport.hpp"
#include "String/StringBuilder.hpp"
#include "WString.hpp"

namespace Zero
{

void CommandLineToStringArray(Array<String>& strings, cstr* argv, int numberOfParameters)
{
  for (int i = 0; i < numberOfParameters; ++i)
    strings.PushBack(argv[i]);
}

void CommandLineToStringArray(Array<String>& strings, wchar_t** argv, int numberOfParameters)
{
  for (int i = 0; i < numberOfParameters; ++i)
    strings.PushBack(Narrow(argv[i]));
}

bool ParseCommandLineStringArray(StringMap& parsedCommandLineArguments, Array<String>& commandLineArguments)
{
  //First parameter is exe path
  if(commandLineArguments.Size() == 1)
    return false;

  size_t index = 1;

  while(index < commandLineArguments.Size())
  {
    StringRange optionName = commandLineArguments[index];

    //Check for '-' at beginning of an option
    if(optionName == '-')
    {
      //eat the '-'
      optionName.PopFront();

      size_t paramIndex = index + 1;

      //Is there a parameter?
      if(paramIndex < commandLineArguments.Size() && commandLineArguments[paramIndex].Front() != '-')
      {
        StringRange parameter = commandLineArguments[paramIndex];
        parsedCommandLineArguments[optionName] = parameter;
        index += 2;
      }
      else
      {
        //Add simple bool parameter 
        parsedCommandLineArguments[optionName] = "true";
        ++index;
      }
    }
    else
    {
      if(index == 1)
      {
        parsedCommandLineArguments["file"] = optionName;
        ++index;
      }
      else
      {
        //bad command line
        return false;
      }
    }
  }
  return true;
}

}// namespace Zero