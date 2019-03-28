// MIT Licensed (see LICENSE.md).
#pragma once
#include "Precompiled.hpp"

namespace Zero
{

void CrashStartCallback(CrashInfo& info, void* userData);
void CrashPreMemoryDumpCallback(void* userData);
bool CrashCustomMemoryCallback(MemoryRange& memRange, void* userData);

void CrashLoggingCallback(CrashHandlerParameters& params, CrashInfo& info, void* userData);
void SendCrashReport(CrashHandlerParameters& params, void* userData);

} // namespace Zero
