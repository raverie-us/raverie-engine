///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Environment object for command-line arguments and environmental variables.
class Environment : public Object
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  static Environment* GetInstance();

  /// Parse the given command-line arguments into a map of name/value pairs.
  /// Parameters are denoted by a leading '-' and the next argument is assumed
  /// to be its value. If the next argument is another parameter then the previous
  /// argument is assumed to be a bool with the value of "true".
  void ParseCommandArgs(const Array<String>& commandLineArgs);

  /// Returns the value of the given parameter as parsed from the application's
  /// command-line arguments. If the parameter doesn't exist, an empty string is returned.
  String GetParsedArgument(StringParam parameterName);

  /// Queries the operating system for an environmental variable.
  /// Returns an empty string if the variable doesn't exist.
  String GetEnvironmentalVariable(StringParam variableName);

//-------------------------------------------------------------------Internal
  void SetCommandLineArguments(const Array<String>& commandLineArgs);
  bool ParseCommandLine();
  void BuildCommandLine();

  /// The unprocessed command-line that the application started up with.
  String mCommandLine;

  /// The original command-line, split up by arguments. 
  Array<String> mCommandLineArguments;

  /// The arguments split up into name/value pairs. If a parameter name had no value
  /// then it is parsed as a boolean with the value of true.
  StringMap mParsedCommandLineArguments;
};

}//namespace Zero
