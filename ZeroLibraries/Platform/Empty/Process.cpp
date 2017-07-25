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
#include "Platform/Process.hpp"

namespace Zero
{

struct ProcessPrivateData
{
};

Process::Process()
{
  Error("Not implemented");
  ZeroConstructPrivateData(ProcessPrivateData);
}

Process::~Process()
{
  ZeroGetPrivateData(ProcessPrivateData);
  // Destruction logic
  ZeroDestructPrivateData(ProcessPrivateData);
}

uint Process::ExecProcess(cstr debugName, cstr commandLine, 
                          TextStream* stream, bool showWindow)
{
  ZeroGetPrivateData(ProcessPrivateData);
  return 0;
}

void Process::WriteToStdIn(cstr text, int size)
{
  ZeroGetPrivateData(ProcessPrivateData);
}

void Process::Shutdown()
{
  ZeroGetPrivateData(ProcessPrivateData);
}

void Process::WaitForClose()
{
  ZeroGetPrivateData(ProcessPrivateData);
}

int Process::GetExitCode()
{
  ZeroGetPrivateData(ProcessPrivateData);
  return 0;
}

void Process::Terminate()
{
  ZeroGetPrivateData(ProcessPrivateData);
}

OsInt Process::ReadThreadEntryPoint()
{
  ZeroGetPrivateData(ProcessPrivateData);
  return 0;
}

}