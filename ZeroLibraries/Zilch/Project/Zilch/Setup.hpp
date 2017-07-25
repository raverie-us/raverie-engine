/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2012-2014, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_HPP
#define ZILCH_HPP

namespace Zilch
{
  namespace SetupFlags
  {
    enum Enum
    {
      None = 0,
      CustomAssertHandlerOrNoAsserts = (1 << 0),
      NoDocumentationStrings = (1 << 1),
      DoNotShutdown = (1 << 2)
    };
    typedef unsigned Type;
  }
  
  // Initializes the shared global memory manager and builds all the static bound libraries
  class ZeroShared ZilchSetup
  {
  public:
    // Controls default setup parameters (such as whether we optimize out documentation string, etc)
    // Note: No Zilch classes should be created before this occurs
    ZilchSetup(SetupFlags::Type flags = SetupFlags::None);
    
    // Shuts down the shared global memory manager and releases any static libraries
    // Note: No Zilch classes should be created after this occurs
    ~ZilchSetup();

    // The setup is a singleton, which means two may not exist at the same time
    // However, you can create ZilchSetup and destroy it, then create another ZilchSetup
    // On destruction, this Instance will be cleared to null
    static ZilchSetup* Instance;

    // Whatever flags we were created with
    SetupFlags::Enum Flags;
  };

  // A simple macro that we specle everywhere to ensure that the user initializes Zilch
  #define ZilchErrorIfNotStarted(Name)                                                              \
    ErrorIf(ZilchSetup::Instance == nullptr,                                                        \
      "In order to use the Zilch " #Name " you must create the ZilchSetup type and hold on to it")

  // A convenient form of parsed main arguments (easily comparable and queryable)
  class ZeroShared MainArguments
  {
  public:
    // The first argument of the argv is generally a path to the executable
    String ExecutablePath;

    // All commands start with a '-' and generally a value follows (or empty if another command directly followed)
    HashMap<String, Array<String> > CommandToValues;

    // Any stray value that isn't preceeded by a command gets put here (a typical use is for file inputs and so on)
    Array<String> InputValues;

    // Whether a particular command was present
    bool HasCommand(StringParam command);

    // Gets the last value passed in for a particular command
    String GetCommandValue(StringParam command);

    // Gets the last value passed in for a particular command as a pointer (easy to test for null)
    String* GetCommandValuePointer(StringParam command);

    // Gets the array of all commands
    Array<String>& GetCommandValues(StringParam command);
  };

  // Parsers arguments that we typically get from main into the above structure
  ZeroShared void ZilchParseMainArguments(int argc, char* argv[], MainArguments& argumentsOut);

  // Processes command line arguments for running Zilch standalone (invokes Startup/Shutdown)
  ZeroShared int ZilchMain(int argc, char* argv[]);

  // Waits for a debugger to be attached (optionally can breakpoint upon attachment)
  // Note, if this is called and the breakpoint option is on, it will ALWAYS breakpoint when running from a debugger
  ZeroShared void ZilchWaitForDebugger(bool breakpointWhenAttached);
}

#endif
