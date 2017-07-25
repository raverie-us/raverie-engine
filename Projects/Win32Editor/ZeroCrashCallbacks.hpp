///////////////////////////////////////////////////////////////////////////////
///
/// \file ZeroCrashCallbacks.cpp
/// Implementation of the Zero specific crash handler functions.
///
/// Authors: Joshua Davis
/// Copyright 2010-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Platform/CrashHandler.hpp"

namespace Zero
{

void CrashStartCallback(CrashInfo& info, void* userData);
void CrashPreMemoryDumpCallback(void* userData);
bool CrashCustomMemoryCallback(MemoryRange& memRange, void* userData);

void CrashLoggingCallback(CrashHandlerParameters& params, CrashInfo& info, void* userData);
void SendCrashReport(CrashHandlerParameters& params, void* userData);

}//namespace Zero
