// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

/// Environment object for command-line arguments and environmental variables.
class Environment : public Object
{
public:
  RaverieDeclareType(Environment, TypeCopyMode::ReferenceType);

  static Environment* GetInstance();

  /// Parse the given command-line arguments into a map of name/value pairs.
  /// Parameters are denoted by a leading '-' and the next argument is assumed
  /// to be its value. If the next argument is another parameter then the
  /// previous argument is assumed to be a bool with the value of "true".
  void ParseCommandArgs(const Array<String>& commandLineArgs);

  /// Returns the value of the given parameter as parsed from the application's
  /// command-line arguments. If the parameter doesn't exist, an empty string is
  /// returned.
  String GetParsedArgument(StringParam parameterName);

  void SetCommandLineArguments(const Array<String>& commandLineArgs);
  bool ParseCommandLine();
  void BuildCommandLine();

  /// The unprocessed command-line that the application started up with.
  String mCommandLine;

  /// The original command-line, split up by arguments.
  Array<String> mCommandLineArguments;

  /// The arguments split up into name/value pairs. If a parameter name had no
  /// value then it is parsed as a boolean with the value of true.
  StringMap mParsedCommandLineArguments;

  template <typename type>
  static type GetValue(StringParam key, const type& valueIfNotFound = type())
  {
    auto environment = Environment::GetInstance();
    String* value = environment->mParsedCommandLineArguments.FindPointer(key);

    type result = type();
    if (value)
      ToValue(*value, result);
    else
      result = valueIfNotFound;

    return result;
  }
};

} // namespace Raverie
