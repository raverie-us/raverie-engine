///////////////////////////////////////////////////////////////////////////////
///
/// \file Process.cpp
/// Declaration of the Process class.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

struct ProcessPrivateData
{
};

Process::Process()
{
  Error("Not implemented");
}

Process::~Process()
{
  Error("Not implemented");
}

void Process::Start(Status& status, ProcessStartInfo& info)
{
  Error("Not implemented");
}

void Process::Start(Status& status, StringRange commandLine, bool redirectOut,
                      bool redirectError, bool redirectIn, bool showWindow)
{
  Error("Not implemented");
}

int Process::WaitForClose()
{
  Error("Not implemented");
  return 0;
}

int Process::WaitForClose(unsigned long milliseconds)
{
  Error("Not implemented");
  return 0;
}

bool Process::IsRunning()
{
  Error("Not implemented");
  return false;
}

void Process::Close()
{
  Error("Not implemented");
}

void Process::Terminate()
{
  Error("Not implemented");
}

void Process::OpenStandardOut(File& fileStream)
{
  Error("Not implemented");
}

void Process::OpenStandardError(File& fileStream)
{
  Error("Not implemented");
}

void Process::OpenStandardIn(File& fileStream)
{
  Error("Not implemented");
}

bool Process::IsStandardOutRedirected()
{
  Error("Not implemented");
  return false;
}

bool Process::IsStandardErrorRedirected()
{
  Error("Not implemented");
  return false;
}

bool Process::IsStandardInRedirected()
{
  Error("Not implemented");
  return false;
}

void GetProcesses(Array<ProcessInfo>& results)
{
  Error("Not implemented");
}

void KillProcess(OsInt processId, int exitCode)
{
  Error("Not implemented");
}

void RegisterApplicationRestartCommand(StringParam commandLineArgs, uint flags)
{
  Error("Not implemented");
}

}