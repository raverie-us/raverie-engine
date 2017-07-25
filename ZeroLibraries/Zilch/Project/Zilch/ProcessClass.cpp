/**************************************************************\
* Author: Joshua T. Fisher, Joshua Davis
* Copyright 2015, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  ZilchDefineExternalBaseType(Zero::ProcessStartInfo, TypeCopyMode::ValueType, builder, type)
  {
    ZilchFullBindConstructor(builder, type, Zero::ProcessStartInfo, nullptr)->Description
      = ZilchDocumentString("Class used to set up parameters before launching a process.");
    ZilchFullBindDestructor(builder, type, Zero::ProcessStartInfo);

    ZilchBindField(mApplicationName)->Description
      = ZilchDocumentString("Name of the application to execute. No quoting of this string is necessary.");
    ZilchBindField(mArguments)->Description
      = ZilchDocumentString("Arguments to pass to the application.");
    ZilchBindField(mWorkingDirectory)->Description
      = ZilchDocumentString("The working directory for the process to start with. No quoting of this string is necessary.");
    ZilchBindField(mShowWindow)->Description
      = ZilchDocumentString("Whether or not the window of the launched application should be shown.");
    ZilchBindField(mSearchPath)->Description
      = ZilchDocumentString("Whether or not we should search the path for the application.");
    ZilchBindField(mRedirectStandardOutput)->Description
      = ZilchDocumentString("Whether or not we should redirect the Standard Output of the process for capturing.");
    ZilchBindField(mRedirectStandardError)->Description
      = ZilchDocumentString("Whether or not we should redirect the Standard Error of the process for capturing.");
    ZilchBindField(mRedirectStandardInput)->Description
      = ZilchDocumentString("Whether or not we should redirect the Standard Input of the process for writing.");
  }

  //***************************************************************************
  ZilchDefineType(ProcessClass, builder, type)
  {
    ZilchFullBindConstructor(builder, type, ProcessClass, nullptr)->Description
      = ZilchDocumentString("Process class used for managing external processes and redirecting their stdio. "
        "Used to launch and monitor various external programs.");
    ZilchFullBindDestructor(builder, type, ProcessClass);

    ZilchFullBindMethod(builder, type, &ProcessClass::Start, (void (ProcessClass::*)(Zero::ProcessStartInfo&)), "Start", "startInfo")->Description
      = ZilchDocumentString("Begins the execution of another process using the given parameters. ");

    ZilchFullBindMethod(builder, type, &ProcessClass::WaitForClose, ZilchNoOverload, "WaitForClose", ZilchNoNames)->Description
      = ZilchDocumentString("Waits for a process to close, this will block until the process closes.");
    ZilchFullBindMethod(builder, type, &ProcessClass::IsRunning, ZilchNoOverload, "IsRunning", ZilchNoNames)->Description
      = ZilchDocumentString("Returns true if the process is still running, false otherwise.");
    ZilchFullBindMethod(builder, type, &ProcessClass::Close, ZilchNoOverload, "Close", ZilchNoNames)->Description
      = ZilchDocumentString("Closes the wrapper around the process, does not close the process launched.");
    ZilchFullBindMethod(builder, type, &ProcessClass::Terminate, ZilchNoOverload, "Terminate", ZilchNoNames)->Description
      = ZilchDocumentString("Attempts to manually shut down the process. This is not safe for the other process or what it's handling.");

    ZilchBindGetter(StandardError)->Description
      = ZilchDocumentString("The stream where standard error is re-directed to. Null if not re-directed");
    ZilchBindGetter(StandardInput)->Description
      = ZilchDocumentString("The stream where standard input is re-directed to. Null if not re-directed");
    ZilchBindGetter(StandardOutput)->Description
      = ZilchDocumentString("The stream where standard output is re-directed to. Null if not re-directed");
  }

  ProcessClass::ProcessClass()
  {
    mRedirectStandardOutput = false;
    mRedirectStandardInput = false;
    mRedirectStandardError = false;
  }

  ProcessClass::~ProcessClass()
  {
    CloseStreams();
    Close();
  }

  void ProcessClass::Start(Zero::ProcessStartInfo& info)
  {
    // Close if we were already open
    CloseStreams();
    Close();

    Status status;
    Process::Start(status, info);

    // If we failed to open the process then throw an exception and return
    if(status.Failed())
    {
      ExecutableState::CallingState->ThrowException(status.Message);
      return;
    }

    if(info.mRedirectStandardError)
    {
      FileStream* standardError = new FileStream();
      Process::OpenStandardError(standardError->InternalFile);
      standardError->Capabilities = StreamCapabilities::Read;
      mStandardError = standardError;
    }

    if(info.mRedirectStandardInput)
    {
      FileStream* standardInput = new FileStream();
      Process::OpenStandardIn(standardInput->InternalFile);
      standardInput->Capabilities = StreamCapabilities::Write;
      mStandardInput = standardInput;
    }

    if(info.mRedirectStandardOutput)
    {
      FileStream* standardOutput = new FileStream();
      Process::OpenStandardOut(standardOutput->InternalFile);
      standardOutput->Capabilities = StreamCapabilities::Read;
      mStandardOutput = standardOutput;
    }
  }

  void ProcessClass::Close()
  {
    Process::Close();
  }

  void ProcessClass::CloseStreams()
  {
    // Clear out the stream handles
    mStandardError = nullptr;
    mStandardInput = nullptr;
    mStandardOutput = nullptr;
  }

  FileStream* ProcessClass::GetStandardError()
  {
    return mStandardError;
  }

  FileStream* ProcessClass::GetStandardInput()
  {
    return mStandardInput;
  }

  FileStream* ProcessClass::GetStandardOutput()
  {
    return mStandardOutput;
  }

}
