///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2010-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

#include "Platform/Windows/WString.hpp"

namespace Zero
{

// We want to avoid allocations, so store how many memory ranges we want to add to the crash dumper
MemoryRange gMemoryRanges[64] = { 0 };
size_t gMemoryRangeCount = 0;

// How many ranges we've written to the dump (re-entrant callback from mini dump)
size_t gMemoryRangesWritten = 0;

void LauncherCrashStartCallback(CrashInfo& info, void* userData)
{
  info.mDumpName = BuildString("ZeroLauncherDump_", gEngineStartTime.c_str(), ".dmp");
  info.mLogName = BuildString("ZeroLauncherLog_", gEngineStartTime.c_str(), ".txt");
  info.mStackName = BuildString("ZeroLauncherStack_", gEngineStartTime.c_str(), ".txt");
  info.mModuleName = "ZeroLauncher";
  info.mStripModules = true;

  //hide the window
  Editor* editor = Z::gEditor;
  if(editor != NULL)
  {
    RootWidget* rootWidget = editor->GetRootWidget();
    OsWindow* osWindow = rootWidget->GetOsWindow();
    osWindow->SetVisible(false);
  }
}

void LauncherCrashPreMemoryDumpCallback(void* userData)
{
  //// Include special regions in memory
  //gMemoryRanges[gMemoryRangeCount].Begin = ZilchLastRunningOpcode;
  //gMemoryRanges[gMemoryRangeCount].Length = ZilchLastRunningOpcodeLength;
  //++gMemoryRangeCount;
}

bool LauncherCrashCustomMemoryCallback(MemoryRange& memRange, void* userData)
{
  if(gMemoryRangesWritten == gMemoryRangeCount)
    return false;

  memRange.Begin = gMemoryRanges[gMemoryRangesWritten].Begin;
  memRange.Length = gMemoryRanges[gMemoryRangesWritten].Length;
  ++gMemoryRangesWritten;
  return true;
}

void LauncherCrashLoggingCallback(CrashHandlerParameters& params, CrashInfo& info, void* userData)
{
  const size_t MAX_TEMP_PATH = MAX_PATH - 14;
  wchar_t logFileName[MAX_PATH] = { 0 };
  char scriptFileName[MAX_PATH] = { 0 };

  //Get the log file
  DWORD pathLength = GetTempPath(MAX_TEMP_PATH, logFileName);
  ZeroStrCatW(logFileName, MAX_TEMP_PATH, Widen(info.mLogName).c_str());
  Console::FlushAll();

  //close the file listener so that there's no race condition on the log file.
  FileListener* fileListener = (FileListener*)userData;
  fileListener->Close();
  Console::Remove(fileListener);

  // Parameters that we provide to shell execute
  StringRange scriptFileNameRange = StringRange(scriptFileName);
  if(!scriptFileNameRange.Empty())
    params.AddParameter("Files", scriptFileNameRange.Data());
  params.AddParameter("Log", Narrow(logFileName).c_str());
}

String LauncherGetToolsPath()
{
  if(Z::gContentSystem && !Z::gContentSystem->ToolPath.Empty())
  {
    return FilePath::Combine(Z::gContentSystem->ToolPath, "ZeroCrashHandler.exe");
  }
  else
  {
    return FilePath::Combine(GetApplicationDirectory(), "Tools", "ZeroCrashHandler.exe");
  }
}

void LauncherSendCrashReport(CrashHandlerParameters& params, void* userData)
{
  params.AddParameter("Version", GetBuildIdString());
  params.AddParameter("Name", "Zero Launcher");
  params.AddParameter("Guid", GetLauncherGuidString());
  params.AddParameter("Revision", GetRevisionNumberString());
  params.AddParameter("ChangeSet", GetChangeSetString());
  params.AddParameter("ChangeSetDate", GetChangeSetDateString());
  params.AddParameter("Configuration", GetConfigurationString());
  params.AddParameter("Platform", GetPlatformString());

  params.AddParameter("ExePath", GetApplication().Data());


  String paramString = params.GetParameterString();

  // Now shell out to our application and tell it to upload the dump

  String crashToolPathA = LauncherGetToolsPath();

  Status status;
  bool success = Os::SystemOpenFile(status, crashToolPathA.c_str(), NULL, paramString.c_str());
}

}//namespace Zero
