// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

void LauncherCrashStartCallback(CrashInfo& info, void* userData);
void LauncherCrashPreMemoryDumpCallback(void* userData);
bool LauncherCrashCustomMemoryCallback(MemoryRange& memRange, void* userData);

void LauncherCrashLoggingCallback(CrashHandlerParameters& params,
                                  CrashInfo& info,
                                  void* userData);
void LauncherSendCrashReport(CrashHandlerParameters& params, void* userData);

} // namespace Zero
