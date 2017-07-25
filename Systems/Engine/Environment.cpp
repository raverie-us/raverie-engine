///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

#include "Platform/CommandLineSupport.hpp"

namespace Zero
{

ZilchDefineType(Environment, builder, type)
{
  // This is created once at startup and exists for the entirety of 
  // the application so it's safe to bind as a raw pointer.
  type->HandleManager = ZilchManagerId(PointerManager);

  ZilchBindFieldProperty(mCommandLine);
  ZilchBindMethod(GetParsedArgument);
  ZilchBindMethod(GetEnvironmentalVariable);
}

Environment* Environment::GetInstance()
{
  static Environment sInstance;
  return &sInstance;
}

void Environment::ParseCommandArgs(const Array<String>& commandLineArgs)
{
  SetCommandLineArguments(commandLineArgs);
  ParseCommandLine();
  BuildCommandLine();
}

String Environment::GetParsedArgument(StringParam parameterName)
{
  return mParsedCommandLineArguments.FindValue(parameterName, String());
}

String Environment::GetEnvironmentalVariable(StringParam variableName)
{
  return Os::GetEnvironmentalVariable(variableName);
}

void Environment::SetCommandLineArguments(const Array<String>& commandLineArgs)
{
  mCommandLineArguments = commandLineArgs;
}

bool Environment::ParseCommandLine()
{
  return ParseCommandLineStringArray(mParsedCommandLineArguments, mCommandLineArguments);
}

void Environment::BuildCommandLine()
{
  StringBuilder builder;
  // Skip the first parameters since it is always the exe name
  for (size_t i = 1; i < mCommandLineArguments.Size(); ++i)
  {
    if (i != 1)
      builder.Append(" ");
    builder.Append(mCommandLineArguments[i]);
  }

  mCommandLine = builder.ToString();
}

}//namespace Zero
