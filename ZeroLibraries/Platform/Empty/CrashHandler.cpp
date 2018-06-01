///////////////////////////////////////////////////////////////////////////////
///
/// \file CrashHandler.cpp
/// Implementation of the crash handler class.
/// 
/// Authors: Joshua Davis
/// Copyright 2010-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

void RunEngine(RunEngineFunction runFn, void* engine)
{
  Error("Not implemented");
};

//-------------------------------------------------------------------CrashHandlerParameters
void CrashHandlerParameters::AddParameter(cstr name, cstr value)
{
  Error("Not implemented");
}

String CrashHandlerParameters::GetParameterString()
{
  Error("Not implemented");
  return String();
}

//-------------------------------------------------------------------CrashHandler
void CrashHandler::Enable()
{
  Error("Not implemented");
}

void CrashHandler::SetRunCrashHandlerCallback(RunCrashHandlerCallback callback, void* userData)
{
  Error("Not implemented");
}

void CrashHandler::SetCrashStartCallback(CrashStartCallback callback, void* userData)
{
  Error("Not implemented");
}

void CrashHandler::SetPreMemoryDumpCallback(PreMemoryDumpCallback callback, void* userData)
{
  Error("Not implemented");
}

void CrashHandler::SetCustomMemoryCallback(CustomMemoryCallback callback, void* userData)
{
  Error("Not implemented");
}

void CrashHandler::SetLoggingCallback(LoggingCallback callback, void* userData)
{
  Error("Not implemented");
}

void CrashHandler::SetSendCrashReportCallback(SendCrashReportCallback callback, void* userData)
{
  Error("Not implemented");
}

void CrashHandler::SetupRescueCallback(FinalRescueCall rescueCall, void* userData)
{
  Error("Not implemented");
}

void CrashHandler::InvokeCrashStartCallback(CrashInfo& info)
{
  Error("Not implemented");
}

void CrashHandler::InvokePreMemoryDumpCallback()
{
  Error("Not implemented");
}

void CrashHandler::WriteMiniDump(CrashHandlerParameters& params, void* crashData, CrashInfo& info)
{
  Error("Not implemented");
}

void CrashHandler::InvokeWriteCallstack(CrashHandlerParameters& params, void* crashData, CrashInfo& info)
{
  Error("Not implemented");
}

void CrashHandler::InvokeLoggingCallback(CrashHandlerParameters& params, CrashInfo& info)
{
  Error("Not implemented");
}

void CrashHandler::InvokeRescueCallback()
{
  Error("Not implemented");
}

void CrashHandler::InvokeSendCrashReport(CrashHandlerParameters& params)
{
  Error("Not implemented");
}

void CrashHandler::FatalError(int errorCode)
{
  Error("Not implemented");
}

void CrashHandler::DefaultRunCrashHandlerCallback(void* crashData, bool doRescueCall, void* userData)
{
  Error("Not implemented");
}

void CrashHandler::SetRestartCommandLine(StringRange commandLine)
{
  Error("Not implemented");
}

void CrashHandler::RestartOnCrash(bool state)
{
  Error("Not implemented");
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

}//namespace Zero
