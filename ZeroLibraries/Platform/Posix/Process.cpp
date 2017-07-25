///////////////////////////////////////////////////////////////////////////////
///
/// \file Process.cpp
///
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "Platform/Process.hpp"

namespace Zero
{

Process::Process()
{
}

Process::~Process()
{
}

uint Process::ExecProcess(cstr debugName, cstr commandLine, TextStream* stream, bool showWindow)
{
  String fullLine = String::Format("wine %s", commandLine);
  int returnCode = system(fullLine.c_str());
  if(returnCode != 0)
  {
      ZERO_DEBUG_BREAK;
  }
  return 0;
}

void Process::WriteToStdIn(cstr text, int size)
{
}

void Process::Shutdown()
{
}

void Process::WaitForClose()
{
}

int Process::GetExitCode()
{
  return 0;
}

OsInt Process::ReadThreadEntryPoint()
{
  return 0;
}

}
