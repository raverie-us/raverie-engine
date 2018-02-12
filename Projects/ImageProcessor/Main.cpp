#include "Precompiled.hpp"

using namespace Zero;

int main(int argc, char** argv)
{
  DebuggerListener debuggerListener;
  StdOutListener stdListener;
  Zero::Console::Add(&debuggerListener);
  Zero::Console::Add(&stdListener);
  ZPrint("Running Image Processor\n");

  Array<String> commandLine;
  CommandLineToStringArray(commandLine, (cstr*)argv, argc);

  Environment environment;
  environment.ParseCommandArgs(commandLine);

  String inputFile = environment.GetParsedArgument("in");
  String outputFile = environment.GetParsedArgument("out");
  String metaFile = environment.GetParsedArgument("metaFile");

  if (metaFile.Empty())
    metaFile = BuildString(inputFile, ".meta");

  if (!FileExists(inputFile))
  {
    ZPrint("Missing image file '%s'\n", inputFile.c_str());
    return Zero::ImageProcessorCodes::Failed;
  }

  if (!FileExists(metaFile))
  {
    ZPrint("Missing meta file '%s'\n", metaFile.c_str());
    return Zero::ImageProcessorCodes::Failed;
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

  TextureImporter importer(inputFile, outputFile, metaFile);

  Status status;
  importer.ProcessTexture(status);

  int returnCode;
  if (status.Failed())
  {
    ZPrint(status.Message.c_str());
    ZPrint("\n");
    returnCode = ImageProcessorCodes::Failed;
  }
  else if (importer.mMetaChanged)
    returnCode = ImageProcessorCodes::Reload;
  else
    returnCode = ImageProcessorCodes::Success;

  return returnCode;
}
