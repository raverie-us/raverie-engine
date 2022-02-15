// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

void RunEngine(RunEngineFunction runFn, void* engine)
{
  runFn(engine);
};

void CrashHandlerParameters::AddParameter(cstr name, cstr value)
{
}

String CrashHandlerParameters::GetParameterString()
{
  return String();
}

void CrashHandler::Enable()
{
}

void CrashHandler::AppendToExtraSymbolPath(StringParam path)
{
}

void CrashHandler::SetRunCrashHandlerCallback(RunCrashHandlerCallback callback, void* userData)
{
}

void CrashHandler::SetCrashStartCallback(CrashStartCallback callback, void* userData)
{
}

void CrashHandler::SetPreMemoryDumpCallback(PreMemoryDumpCallback callback, void* userData)
{
}

void CrashHandler::SetCustomMemoryCallback(CustomMemoryCallback callback, void* userData)
{
}

void CrashHandler::SetLoggingCallback(LoggingCallback callback, void* userData)
{
}

void CrashHandler::SetSendCrashReportCallback(SendCrashReportCallback callback, void* userData)
{
}

void CrashHandler::SetupRescueCallback(FinalRescueCall rescueCall, void* userData)
{
}

void CrashHandler::InvokeCrashStartCallback(CrashInfo& info)
{
}

void CrashHandler::InvokePreMemoryDumpCallback()
{
}

void CrashHandler::WriteMiniDump(CrashHandlerParameters& params, void* crashData, CrashInfo& info)
{
}

void CrashHandler::InvokeWriteCallstack(CrashHandlerParameters& params, void* crashData, CrashInfo& info)
{
}

void CrashHandler::InvokeLoggingCallback(CrashHandlerParameters& params, CrashInfo& info)
{
}

void CrashHandler::InvokeRescueCallback()
{
}

void CrashHandler::InvokeSendCrashReport(CrashHandlerParameters& params)
{
}

void CrashHandler::FatalError(int errorCode)
{
  int* bad = (int*)(uintptr_t)-1;
  *bad = 0;
}

void CrashHandler::DefaultRunCrashHandlerCallback(void* crashData, bool doRescueCall, void* userData)
{
}

void CrashHandler::SetRestartCommandLine(StringRange commandLine)
{
}

void CrashHandler::RestartOnCrash(bool state)
{
}

CrashHandler::RunCrashHandlerCallback CrashHandler::mRunCrashHandlerCallback = nullptr;
void* CrashHandler::mRunCrashHandlerUserData = nullptr;

CrashHandler::CrashStartCallback CrashHandler::mCrashStartCallback = nullptr;
void* CrashHandler::mCrashStartUserData = nullptr;

CrashHandler::PreMemoryDumpCallback CrashHandler::mPreMemoryDumpCallback = nullptr;
void* CrashHandler::mPreMemoryDumpUserData = nullptr;

CrashHandler::CustomMemoryCallback CrashHandler::mCustomMemoryCallback = nullptr;
void* CrashHandler::mCustomMemoryUserData = nullptr;

CrashHandler::LoggingCallback CrashHandler::mLoggingCallback = nullptr;
void* CrashHandler::mLoggingUserData = nullptr;

CrashHandler::SendCrashReportCallback CrashHandler::mSendCrashReportCallback = nullptr;
void* CrashHandler::mSendCrashReportUserData = nullptr;

CrashHandler::FinalRescueCall CrashHandler::mRescueCallback = nullptr;
void* CrashHandler::mRescueUserData = nullptr;

} // namespace Zero
