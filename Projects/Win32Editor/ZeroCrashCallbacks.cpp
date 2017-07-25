///////////////////////////////////////////////////////////////////////////////
///
/// \file ZeroCrashCallbacks.cpp
/// Implementation of the Zero specific crash handler functions.
///
/// Authors: Joshua Davis
/// Copyright 2010-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

#include "ZeroCrashCallbacks.hpp"

#include "Engine/DocumentResource.hpp"
#include "Support/FileSupport.hpp"
#include "Support/FileConsoleListener.hpp"

#include "Platform/CrashHandler.hpp"
#include "Editor/Editor.hpp"
#include "Engine/OsWindow.hpp"
#include "Content/ContentSystem.hpp"
#include "Utility/Status.hpp"
#include "Engine/BuildVersion.hpp"

#include "Platform/Windows/WString.hpp"

namespace Zero
{

// We want to avoid allocations, so store how many memory ranges we want to add to the crash dumper
MemoryRange gMemoryRanges[64] = {0};
size_t gMemoryRangeCount = 0;

// How many ranges we've written to the dump (re-entrant callback from mini dump)
size_t gMemoryRangesWritten = 0;

void CrashStartCallback(CrashInfo& info, void* userData)
{
  info.mDumpName = BuildString("ZeroDump_", gEngineStartTime.c_str(), ".dmp");
  info.mLogName = BuildString("ZeroLog_", gEngineStartTime.c_str(), ".txt");
  info.mStackName = BuildString("ZeroStack_", gEngineStartTime.c_str(), ".txt");
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
  const size_t MAX_TEMP_PATH = MAX_PATH - 14;
  wchar_t logFileName[MAX_PATH] = { 0 };
  wchar_t scriptFileName[MAX_PATH] = {0};

  // Collect all scripts on the stack
  Zilch::StackTrace stackTrace;
  ExecutableState::CallingState->GetStackTrace(stackTrace);

  HashMap<String, String> fileNamesToFileContents;
  for(size_t i = 0; i < stackTrace.Stack.Size(); ++i)
  {
    StackEntry& entry = stackTrace.Stack[i];
    // Print the location of the crash to the log
    String location = entry.ExecutingFunction->Location.GetFormattedString(Zilch::MessageFormat::Python);
    ZPrint("%s\n", location.c_str());

    // Only append scripts so we can attach them to crashes
    if(entry.Location.IsNative)
      continue;

    String fileName = FilePath::GetFileNameWithoutExtension(entry.Location.Origin);
    // Collect each file only once
    fileNamesToFileContents[fileName] = entry.Location.Code;
  }

  // Write out each script file and add them as crash data
  HashMap<String, String>::range range = fileNamesToFileContents.All();
  for(; !range.Empty(); range.PopFront())
  {
    HashMap<String, String>::pair pair = range.Front();
    String fileName = pair.first;
    String fileData = pair.second;
    DWORD pathLength = GetTempPath(MAX_TEMP_PATH, scriptFileName);
    ZeroStrCatW(scriptFileName, MAX_TEMP_PATH, Widen(fileName).c_str());
    ZeroStrCatW(scriptFileName, MAX_TEMP_PATH, L".txt");
    String narrowScriptFileName = Narrow(scriptFileName);
    WriteStringRangeToFile(narrowScriptFileName, fileData);
    
    // Add the file's name to parameters for what files we include
    StringRange scriptFileNameRange = StringRange(narrowScriptFileName);
    if(!scriptFileNameRange.Empty())
      params.AddParameter("Files", scriptFileNameRange.Data());
  }

  //Get the log file
  DWORD pathLength = GetTempPath(MAX_TEMP_PATH, logFileName);
  ZeroStrCatW(logFileName, MAX_TEMP_PATH, Widen(info.mLogName).c_str());
  Console::FlushAll();

  //close the file listener so that there's no race condition on the log file.
  FileListener* fileListener = (FileListener*)userData;
  fileListener->Close();
  Console::Remove(fileListener);
  
  params.AddParameter("Log", Narrow(logFileName).c_str());
}

String GetToolsPath()
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

void SendCrashReport(CrashHandlerParameters& params, void* userData)
{
  params.AddParameter("Version", GetBuildIdString());
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
  bool success = Os::SystemOpenFile(status, crashToolPathA.c_str(), NULL, paramString.c_str());

  if(success == true)
    return;

  //May be exported try other path
  String crashToolPathB = FilePath::Combine(GetTemporaryDirectory(), "Zero", "Tools", "ZeroCrashHandler.exe");
  success = Os::SystemOpenFile(status, crashToolPathB.c_str(), NULL, paramString.c_str());
}

}//namespace Zero
