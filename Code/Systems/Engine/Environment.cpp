// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

RaverieDefineType(Environment, builder, type)
{
  // This is created once at startup and exists for the entirety of
  // the application so it's safe to bind as a raw pointer.
  type->HandleManager = RaverieManagerId(PointerManager);

  RaverieBindFieldProperty(mCommandLine);
  RaverieBindMethod(GetParsedArgument);
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

void Environment::SetCommandLineArguments(const Array<String>& commandLineArgs)
{
  mCommandLineArguments = commandLineArgs;
}

bool Environment::ParseCommandLine()
{
  bool result = ParseCommandLineStringArray(mParsedCommandLineArguments, mCommandLineArguments);
  forRange (StringMap::PairType& pair, mParsedCommandLineArguments.All()) {
    ZPrint("Parsed Argument: %s = %s\n", pair.first.c_str(), pair.second.c_str());
  }
  return result;
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

} // namespace Raverie
