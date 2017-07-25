///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

void LauncherCrashStartCallback(CrashInfo& info, void* userData);
void LauncherCrashPreMemoryDumpCallback(void* userData);
bool LauncherCrashCustomMemoryCallback(MemoryRange& memRange, void* userData);

void LauncherCrashLoggingCallback(CrashHandlerParameters& params, CrashInfo& info, void* userData);
void LauncherSendCrashReport(CrashHandlerParameters& params, void* userData);

}//namespace Zero
