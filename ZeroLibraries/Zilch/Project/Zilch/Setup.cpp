/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  ZilchSetup* ZilchSetup::Instance = nullptr;

  //***************************************************************************
  ZilchSetup::ZilchSetup(SetupFlags::Type flags)
  {
    Instance = this;

    // Initialize static native binding list before creating any bound types
    // This ensures the native binding list will be destroyed after static library bound types are destroyed
    NativeBindingList::GetInstance();

    // Make sure the jump table is initialized
    VirtualMachine::InitializeJumpTable();

    // The user can disable runtime documentation processing by passing in a flag to ZilchStartup
    // However, if the user defines 'ZilchDisableDocumentation', this will completely disable
    // both compile-time and runtime documentation processing
#if defined(ZilchDisableDocumentation)
    flags |= SetupFlags::NoDocumentationStrings;
#endif

    // Store the flags for later use (eg in the destructor)
    this->Flags = (SetupFlags::Enum)flags;

    // Make sure all of our statics are initialized (guarantees thread safety)
    Grammar::GetUsedKeywords();
    Grammar::GetReservedKeywords();
    Grammar::GetSpecialKeywords();
    IEncoding::GetAscii();
    IEncoding::GetUtf8();

    // Make sure to initialize all the compilation errors
    ErrorDatabase::GetInstance();

    // If the user wants us to, we'll use our own custom error handler,
    // Otherwise we'll use our own error handler
    bool useZilchErrorHandler = !(flags & SetupFlags::CustomAssertHandlerOrNoAsserts);
    if (useZilchErrorHandler)
    {
      ErrorSignaler::SetErrorHandler(DebugErrorHandler);
    }

    // Register the command handle managers we use
    ZilchRegisterUniqueHandleManager(HeapManager);
    ZilchRegisterUniqueHandleManager(StackManager);
    ZilchRegisterSharedHandleManager(PointerManager);
    ZilchRegisterSharedHandleManager(StringManager);

    // Lock any future handle managers from being added
    HandleManagers::GetInstance().Lock();

    // Everyone depends on the Core library, so build it first
    Core::InitializeInstance();
    Syntax::InitializeInstance();
    WebSockets::InitializeInstance();

    // Everyone depends on the Core library, so build it first
    Core::GetInstance().BuildLibrary();
    Syntax::GetInstance().BuildLibrary();
    WebSockets::GetInstance().BuildLibrary();

    // Finally, after everything else is built, make the shared library
    Shared::GetInstance();

    // Validate all built in types at this point
    NativeBindingList::ValidateTypes();
  }
  
  //***************************************************************************
  template <typename T>
  void ReconstructSingleton()
  {
    // Explicitly destructs a singleton then uses placement new to create it again (should be reset)
    ZilchTodo("Make singletons into pointers so we don't have to do this silly stuff");
    T* singleton = &T::GetInstance();
    singleton->~T();
    new (singleton) T();
  }
  
  //***************************************************************************
  ZilchSetup::~ZilchSetup()
  {
    // Do an extra validation pass over the natively bound types
    // even though we're shutting down, we can fully find out what the user forgot to initialize
    NativeBindingList::ValidateTypes();

    // If the user specified to not run shutdown code, then early out here
    if (this->Flags & SetupFlags::DoNotShutdown)
      return;

    // Manually invoke destructors on static objects and in place construct them again (so they can be used again)
    // Static shutdown will properly take care of removing them (these should probably be changed to allocated pointers)
    ReconstructSingleton<HandleManagers>();
    ReconstructSingleton<Shared>();

    // Everyone depends on the Core library, so build it first
    Core::Destroy();
    Syntax::Destroy();
    WebSockets::Destroy();

    // Shutdown the memory manager
    Shutdown();

    Instance = nullptr;
  }

  //***************************************************************************
  bool MainArguments::HasCommand(StringParam command)
  {
    return this->CommandToValues.ContainsKey(command);
  }
  
  //***************************************************************************
  String MainArguments::GetCommandValue(StringParam command)
  {
    String* value = this->GetCommandValuePointer(command);
    if (value != nullptr)
      return *value;

    // If we didn't get a valid pointer back, then return an empty string
    return String();
  }
  
  //***************************************************************************
  String* MainArguments::GetCommandValuePointer(StringParam command)
  {
    // Look for the commands
    Array<String>* values = this->CommandToValues.FindPointer(command);
    if (values == nullptr)
      return nullptr;

    // If there are no values under the array, then return an empty string
    if (values->Empty())
      return nullptr;

    // We always return the last input value for that command
    return &values->Back();
  }
  
  //***************************************************************************
  Array<String>& MainArguments::GetCommandValues(StringParam command)
  {
    return this->CommandToValues[command];
  }
  
  //***************************************************************************
  void ZilchParseMainArguments(int argc, char* argv[], MainArguments& argumentsOut)
  {
    static const String CommandDash('-');
    String lastCommand;

    // Get the executable path from the first argument
    if (argc >= 1)
      argumentsOut.ExecutablePath = argv[0];

    // Walk through all arguments storing commands as we find them
    for (int i = 1; i <= argc; ++i)
    {
      // Get the current argument (let the last one be an empty string)
      String argument;
      if (i < argc)
        argument = argv[i];

      // If we found a new command...
      if (argument.StartsWith(CommandDash))
      {
        // If we already had a previous command, then the previous one did not have a value
        if (lastCommand.Empty() == false)
        {
          // Give the last command an empty value
          argumentsOut.CommandToValues[lastCommand].PushBack(String());
        }
        
        // Store the new command...
        lastCommand = argument;
      }
      else
      {
        // If we have a last command, then this argument is its value
        if (lastCommand.Empty() == false)
        {
          // Store the command and its argument, then clear it for the next time
          // we come around (so we don't think its an valueless command)
          argumentsOut.CommandToValues[lastCommand].PushBack(argument);
          lastCommand.Clear();
        }
        else if (argument.Empty() == false)
        {
          // We found a value without a command, just add it to the values list
          argumentsOut.InputValues.PushBack(argument);
        }
      }
    }
  }
  
  //***************************************************************************
  void GetErrorEvent(ErrorEvent* e, void* userData)
  {
    // Copy the event out
    *((ErrorEvent*)userData) = *e;
  }
  
  //***************************************************************************
  int ZilchMain(int argc, char* argv[])
  {
    int result = 0;

    ZilchSetup setup(SetupFlags::None);

    // Get the arguments in a convenient to query form
    MainArguments arguments;
    ZilchParseMainArguments(argc, argv, arguments);

    // For our own internal use, we may want to attach a debugger to Zilch
    if (arguments.HasCommand("-WaitForDebugger"))
      ZilchWaitForDebugger(true);

    // Hook up the standard write and read callbacks to the console (which allows us to read from stdin and write to stdout)
    EventConnect(&Console::Events, Events::ConsoleWrite, DefaultWriteText);
    EventConnect(&Console::Events, Events::ConsoleRead, DefaultReadText);

    // Create an empty project and listen for compilation errors
    Project project;
    ErrorEvent errorEvent;
    EventConnect(&project, Events::CompilationError, GetErrorEvent, &errorEvent);

    // We always load plugins from the directory next to the executable
    project.PluginDirectories.PushBack(Zero::GetApplicationDirectory());
    
    ZilchForEach(String& pluginDirectory, arguments.GetCommandValues("-PluginDirectory"))
    {
      project.PluginDirectories.PushBack(pluginDirectory);
    }
    
    ZilchForEach(String& pluginFile, arguments.GetCommandValues("-Plugin"))
    {
      project.PluginFiles.PushBack(pluginFile);
    }

    // Treat all the stray input values as file names
    ZilchForEach(String& fileName, arguments.InputValues)
    {
      // Load the code from a file (and if it fails, error out)
      if (project.AddCodeFromFile(fileName) == false)
      {
        printf("* Unable to open file: '%s'\n", fileName.c_str());
        result = -1;
      }
    }
    
    // If requested, also compile code from a string
    if (String* codeString = arguments.GetCommandValuePointer("-CodeString"))
      project.AddCodeFromString(*codeString, CodeString);

    // If we want to compile the code we added above and report error information (or run the code)
    bool compileAndReport = arguments.HasCommand("-CompileAndReport");
    bool compileOnly = arguments.HasCommand("-CompileOnly");
    bool run = arguments.HasCommand("-Run");
    if (compileAndReport || compileOnly || run)
    {
      Module module;
      LibraryRef library = project.Compile("Main", module, EvaluationMode::Project);

      if (compileAndReport)
      {
        JsonBuilder builder;
        builder.Begin(JsonType::Object);
        {
          builder.Key("IsError");
          builder.Value(library == nullptr);

          builder.Key("StartLine");
          builder.Value(errorEvent.Location.StartLine);
          builder.Key("StartCharacter");
          builder.Value(errorEvent.Location.StartCharacter);

          builder.Key("PrimaryLine");
          builder.Value(errorEvent.Location.PrimaryLine);
          builder.Key("PrimaryCharacter");
          builder.Value(errorEvent.Location.PrimaryCharacter);

          builder.Key("EndLine");
          builder.Value(errorEvent.Location.EndLine);
          builder.Key("EndCharacter");
          builder.Value(errorEvent.Location.EndCharacter);

          builder.Key("Origin");
          builder.Value(errorEvent.Location.Origin);

          builder.Key("Message");
          builder.Value(errorEvent.ExactError);

          builder.Key("FormattedMessage");
          builder.Value(errorEvent.GetFormattedMessage(MessageFormat::Zilch));
        }
        builder.End();

        String json = builder.ToString();
        printf("%s", json.c_str());
      }
      else
      {
        if (library == nullptr)
        {
          // Print out the error message directly
          String errorMessage = errorEvent.GetFormattedMessage(MessageFormat::Zilch);
          printf("* %s\n", errorMessage.c_str());
          result = -1;
        }
        else if (run)
        {
          BoundType* programType = library->BoundTypes.FindValue("Program", nullptr);
          if (programType != nullptr)
          {
            Function* mainFunction = programType->FindFunction("Main", Array<Type*>(), ZilchTypeId(int), FindMemberOptions::None);
            if (mainFunction != nullptr)
            {
              Module libraries;
              libraries.PushBack(library);
              ExecutableState* state = libraries.Link();
              EventConnect(state, Events::UnhandledException, DefaultExceptionCallback);
              {
                ExceptionReport report;
                Handle programHandle = state->AllocateDefaultConstructedHeapObject(programType, report, HeapFlags::ReferenceCounted);

                if (report.HasThrownExceptions())
                {
                  result = -2;
                }
                else
                {
                  Call call(mainFunction, state);
                  call.Set(Call::This, programHandle);
                  call.Invoke(report);

                  if (report.HasThrownExceptions())
                    result = -2;
                  else
                    result = call.Get<Integer>(Call::Return);
                }
              }
              delete state;
            }
            else
            {
              printf("* Unable to find instance entry-point 'function Main() : Integer' on type 'Program'\n");
              result = -1;
            }
          }
          else
          {
            printf("* Unable to find entry-point type 'Program'\n");
            result = -1;
          }
        }
      }
    }
    else
    {
      // If the user wants auto complete information...
      String* autoCompleteCursor = arguments.GetCommandValuePointer("-AutoCompleteCursor");
      String* autoCompleteOrigin = arguments.GetCommandValuePointer("-AutoCompleteOrigin");
      if (autoCompleteCursor != nullptr && autoCompleteOrigin != nullptr)
      {
        // Read the value the user specified for the cursor position
        long long cursorPosition = 0;
        Zero::ToValue(*autoCompleteCursor, cursorPosition);

        // Attempt to get auto complete information if possible
        AutoCompleteInfo info;
        Module module;
        project.TolerantMode = true;
        project.GetAutoCompleteInfo(module, (size_t)cursorPosition, *autoCompleteOrigin, info);

        String json = info.GetJson();
        printf("%s\n", json.c_str());
      }
      else if (autoCompleteCursor == nullptr && autoCompleteOrigin != nullptr)
      {
        printf("* When specifying 'AutoCompleteOrigin' you must also specify 'AutoCompleteCursor'\n");
        result = -1;
      }
      else if (autoCompleteCursor != nullptr && autoCompleteOrigin == nullptr)
      {
        printf("* When specifying 'AutoCompleteCursor' you must also specify 'AutoCompleteOrigin'\n");
        result = -1;
      }
    }

    // If they passed in the pause command, then we'll wait to exit
    if (arguments.HasCommand("-Pause"))
    {
      printf("Press enter/return to exit...");
      getchar();
    }

    return result;
  }
  
  //***************************************************************************
  void ZilchWaitForDebugger(bool breakpointWhenAttached)
  {
    // Wait until the debugger gets attached by constantly sleeping and checking
    while (Zero::Os::IsDebuggerAttached() == false)
      Zero::Os::Sleep(1);

    // We got here and a debugger is attached, so breakpoint!
    Zero::Os::DebugBreak();
  }
}