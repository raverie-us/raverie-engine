// MIT Licensed (see LICENSE.md).

#pragma once

namespace Raverie
{
namespace SetupFlags
{
enum Enum
{
  None = 0,
  CustomAssertHandlerOrNoAsserts = (1 << 0),
  NoDocumentationStrings = (1 << 1),
  DoNotShutdown = (1 << 2),
  DoNotShutdownMemory = (1 << 3)
};
typedef unsigned Type;
} // namespace SetupFlags

// Initializes the shared global memory manager and builds all the static bound
// libraries
class RaverieSetup
{
public:
  // Controls default setup parameters (such as whether we optimize out
  // documentation string, etc) Note: No Raverie classes should be created before
  // this occurs
  RaverieSetup(SetupFlags::Type flags = SetupFlags::None);

  // Shuts down the shared global memory manager and releases any static
  // libraries Note: No Raverie classes should be created after this occurs
  ~RaverieSetup();

  // The setup is a singleton, which means two may not exist at the same time
  // However, you can create RaverieSetup and destroy it, then create another
  // RaverieSetup On destruction, this Instance will be cleared to null
  static RaverieSetup* Instance;

  // Whatever flags we were created with
  SetupFlags::Enum Flags;
};

// A simple macro that we specle everywhere to ensure that the user initializes
// Raverie
#define RaverieErrorIfNotStarted(Name) ErrorIf(RaverieSetup::Instance == nullptr, "In order to use the Raverie " #Name " you must create the RaverieSetup type and hold on to it")

// A convenient form of parsed main arguments (easily comparable and queryable)
class MainArguments
{
public:
  // The first argument of the argv is generally a path to the executable
  String ExecutablePath;

  // All commands start with a '-' and generally a value follows (or empty if
  // another command directly followed)
  HashMap<String, Array<String>> CommandToValues;

  // Any stray value that isn't preceeded by a command gets put here (a typical
  // use is for file inputs and so on)
  Array<String> InputValues;

  // Whether a particular command was present
  bool HasCommand(StringParam command);

  // Gets the last value passed in for a particular command
  String GetCommandValue(StringParam command);

  // Gets the last value passed in for a particular command as a pointer (easy
  // to test for null)
  String* GetCommandValuePointer(StringParam command);

  // Gets the array of all commands
  Array<String>& GetCommandValues(StringParam command);
};

// Parsers arguments that we typically get from main into the above structure
void RaverieParseMainArguments(int argc, char* argv[], MainArguments& argumentsOut);

// Processes command line arguments for running Raverie standalone (invokes
// Startup/Shutdown)
int RaverieMain(int argc, char* argv[]);
} // namespace Raverie
