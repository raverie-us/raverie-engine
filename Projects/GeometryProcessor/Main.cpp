// Authors: Nathan Carlson
// Copyright 2015, DigiPen Institute of Technology

#include "Precompiled.hpp"

#include "Platform/Windows/WString.hpp"
#include "Platform/CommandLineSupport.hpp"
#include <windows.h>

using namespace Zero;

// ConsoleListeners copied from Win32Editor for now
class VisualStudioListener : public ConsoleListener
{
public:
  void Print(FilterType filterType, cstr message) override
  {
    OutputDebugStringW(Widen(message).c_str());
  }
};

//**************************************************************************************************
int main(int argc, char** argv)
{
  VisualStudioListener vsListener;
  StdOutListener stdListener;
  Zero::Console::Add(&vsListener);
  Zero::Console::Add(&stdListener);
  ZPrint("Running Geometry Processor\n");

  Array<String> commandLine;
  CommandLineToStringArray(commandLine, (cstr*)argv, argc);

  Environment environment;
  environment.ParseCommandArgs(commandLine);

  String inputFile = environment.GetParsedArgument("in");
  String outputPath = environment.GetParsedArgument("out");
  String metaFile = environment.GetParsedArgument("metaFile");

  if (metaFile.Empty())
    metaFile = BuildString(inputFile, ".meta");

  if (!FileExists(inputFile))
  {
    ZPrint("Missing model file '%s'\n", inputFile.c_str());
    return Zero::GeometryProcessorCodes::Failed;
  }

  if (!FileExists(metaFile))
  {
    ZPrint("Missing meta file '%s'\n", metaFile.c_str());
    return Zero::GeometryProcessorCodes::Failed;
  }

  // Required to serialize content meta data -----------------------------------
  // Initializing the minimum necessary and not doing shutdown because the
  // process is terminated after running and memory leaks won't affect anything.
  RegisterCommonHandleManagers();
  ZeroRegisterHandleManager(ContentComposition);
  ZilchSetup* zilchSetup = new ZilchSetup();
  MetaDatabase::Initialize();
  MetaLibrary::Initialize();
  GeometryLibrary::Initialize();
  SerializationLibrary::Initialize();
  ContentMetaLibrary::Initialize();
  EngineLibrary::BuildStaticLibrary();
  InitializeContentSystem();
  // ---------------------------------------------------------------------------

  GeometryImporter importer(inputFile, outputPath, metaFile);

  int result = importer.ProcessModelFiles();

  return result;
}