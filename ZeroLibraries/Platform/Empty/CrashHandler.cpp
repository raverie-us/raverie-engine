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
#include "Platform/CrashHandler.hpp"

namespace Zero
{


void RunEngine(RunEngineFunction runFn, void* engine)
{
  Error("Not implemented");
  //runFn(engine);
};

//-------------------------------------------------------------------CrashHandlerParameters
void CrashHandlerParameters::AddParameter(StringRange name, StringRange value)
{
  mParameters.Append("\"--");
  mParameters.Append(name);
  mParameters.Append("\" ");
  mParameters.Append("\"");
  mParameters.Append(value);
  mParameters.Append("\" ");
}

String CrashHandlerParameters::GetParameterString()
{
  return mParameters.ToString();
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

CrashHandler::RunCrashHandlerCallback CrashHandler::mRunCrashHandlerCallback = NULL;
void* CrashHandler::mRunCrashHandlerUserData = NULL;

CrashHandler::CrashStartCallback CrashHandler::mCrashStartCallback = NULL;
void* CrashHandler::mCrashStartUserData = NULL;

CrashHandler::PreMemoryDumpCallback CrashHandler::mPreMemoryDumpCallback = NULL;
void* CrashHandler::mPreMemoryDumpUserData = NULL;

CrashHandler::CustomMemoryCallback CrashHandler::mCustomMemoryCallback = NULL;
void* CrashHandler::mCustomMemoryUserData = NULL;

CrashHandler::LoggingCallback CrashHandler::mLoggingCallback = NULL;
void* CrashHandler::mLoggingUserData = NULL;

CrashHandler::SendCrashReportCallback CrashHandler::mSendCrashReportCallback = NULL;
void* CrashHandler::mSendCrashReportUserData = NULL;

CrashHandler::FinalRescueCall CrashHandler::mRescueCallback = NULL;
void* CrashHandler::mRescueUserData = NULL;

}//namespace Zero
