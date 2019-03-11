// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

// We want to avoid allocations, so store how many memory ranges we want to add
// to the crash dumper
MemoryRange gMemoryRanges[64] = {0};
size_t gMemoryRangeCount = 0;

// How many ranges we've written to the dump (re-entrant callback from mini
// dump)
size_t gMemoryRangesWritten = 0;

void CrashStartCallback(CrashInfo& info, void* userData)
{
  info.mDumpName = BuildString("ZeroDump_", gEngineStartTime.c_str(), ".dmp");
  info.mLogName = BuildString("ZeroLog_", gEngineStartTime.c_str(), ".txt");
  info.mStackName = BuildString("ZeroStack_", gEngineStartTime.c_str(), ".txt");
  info.mStripModules = true;

  // hide the window
  Editor* editor = Z::gEditor;
  if (editor != NULL)
  {
    RootWidget* rootWidget = editor->GetRootWidget();
    OsWindow* osWindow = rootWidget->GetOsWindow();
    osWindow->SetVisible(false);
  }
}

void CrashPreMemoryDumpCallback(void* userData)
{
  // Include Zilch opcode into memory
  gMemoryRanges[gMemoryRangeCount].Begin = ZilchLastRunningOpcode;
  gMemoryRanges[gMemoryRangeCount].Length = ZilchLastRunningOpcodeLength;
  ++gMemoryRangeCount;
}

bool CrashCustomMemoryCallback(MemoryRange& memRange, void* userData)
{
  if (gMemoryRangesWritten == gMemoryRangeCount)
    return false;

  memRange.Begin = gMemoryRanges[gMemoryRangesWritten].Begin;
  memRange.Length = gMemoryRanges[gMemoryRangesWritten].Length;
  ++gMemoryRangesWritten;
  return true;
}

void CrashLoggingCallback(CrashHandlerParameters& params, CrashInfo& info, void* userData)
{
  String tempDirectory = GetTemporaryDirectory();

  // Collect all scripts on the stack
  Zilch::StackTrace stackTrace;
  ExecutableState::CallingState->GetStackTrace(stackTrace);

  HashMap<String, String> fileNamesToFileContents;
  for (size_t i = 0; i < stackTrace.Stack.Size(); ++i)
  {
    StackEntry& entry = stackTrace.Stack[i];
    // Print the location of the crash to the log
    String location = entry.Location.GetFormattedString(Zilch::MessageFormat::Python);
    ZPrint("%s\n", location.c_str());

    // Only append scripts so we can attach them to crashes
    if (entry.Location.IsNative)
      continue;

    String fileName = FilePath::GetFileNameWithoutExtension(entry.Location.Origin);
    // Collect each file only once
    fileNamesToFileContents[fileName] = entry.Location.Code;
  }

  // Write out each script file and add them as crash data
  HashMap<String, String>::range range = fileNamesToFileContents.All();
  for (; !range.Empty(); range.PopFront())
  {
    HashMap<String, String>::pair pair = range.Front();
    String fileName = pair.first;
    String fileData = pair.second;
    String scriptFileName = FilePath::Combine(tempDirectory, fileName, ".txt");
    WriteStringRangeToFile(scriptFileName, fileData);

    // Add the file's name to parameters for what files we include
    if (!scriptFileName.Empty())
      params.AddParameter("Files", scriptFileName.c_str());
  }

  // Get the log file
  String logFileName = FilePath::Combine(tempDirectory, info.mLogName);
  Console::FlushAll();

  // close the file listener so that there's no race condition on the log file.
  FileListener* fileListener = (FileListener*)userData;
  fileListener->Close();
  Console::Remove(fileListener);

  params.AddParameter("Log", logFileName.c_str());
}

String GetToolsPath()
{
  if (Z::gContentSystem && !Z::gContentSystem->ToolPath.Empty())
  {
    return FilePath::Combine(Z::gContentSystem->ToolPath, "ZeroCrashHandler.exe");
  }
  else
  {
    return FilePath::Combine(GetApplicationDirectory(), "Tools", "ZeroCrashHandler.exe");
  }
}

void SendCrashReport(CrashHandlerParameters& params, void* userData)
{
  params.AddParameter("Version", GetBuildIdString().c_str());
  params.AddParameter("Name", "Zero Engine");
  params.AddParameter("Guid", GetGuidString());
  params.AddParameter("Revision", GetRevisionNumberString());
  params.AddParameter("ChangeSet", GetChangeSetString());
  params.AddParameter("ChangeSetDate", GetChangeSetDateString());
  params.AddParameter("Configuration", GetConfigurationString());
  params.AddParameter("Platform", GetPlatformString());

  params.AddParameter("ExePath", GetApplication().c_str());

  String paramString = params.GetParameterString();

  // Now shell out to our application and tell it to upload the dump

  String crashToolPathA = GetToolsPath();

  Status status;
  bool success = Os::SystemOpenFile(status, crashToolPathA.c_str(), Os::Verb::Default, paramString.c_str());

  if (success == true)
    return;

  // May be exported try other path
  String crashToolPathB = FilePath::Combine(GetTemporaryDirectory(), "Zero", "Tools", "ZeroCrashHandler.exe");
  success = Os::SystemOpenFile(status, crashToolPathB.c_str(), Os::Verb::Default, paramString.c_str());
}

} // namespace Zero
