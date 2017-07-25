///////////////////////////////////////////////////////////////////////////////
///
/// \file Timer.cpp
/// Declaration of the Os High precision Timer class.
/// 
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "Platform/Utilities.hpp"
#include "Platform/FileSystem.hpp"
#include "Platform/CrashHandler.hpp"


#include <unistd.h>
#include <pwd.h>

namespace Zero
{

namespace Os
{

void Sleep(uint ms)
{
  usleep (ms * 1000);
}

void DebugBreak()
{
  __builtin_trap();
}

void SystemOpenFile(cstr file, uint verb, cstr workingDirectory)
{
  if(workingDirectory)
    SetWorkingDirectory(workingDirectory);
  system(file);
}

String TranslateErrorCode(int errorCode)
{
  return "None";
}

String GetEnvironmentalVariable(StringRef variable)
{
  return getenv(variable.c_str());
}

bool IsDebuggerAttached()
{
  return true;
}

void SystemOpenFile(cstr file, uint verb, cstr parameters, cstr workingDirectory)
{
    
}

String UserName()
{
  return getlogin();
}

// Get the computer name
String ComputerName()
{
  char hostname[1024];
  hostname[1023] = '\0';
  gethostname(hostname, 1023);
  return hostname;
}

u64 GetMacAddress()
{
  return 0;
}

void SetTimerFrequency(uint ms)
{
  // Not available on linux
}

}//End os

u64 GenerateUniqueId64()
{
  static u64 idGen = 0;
  ++idGen;
  return idGen + rand();
}


void CheckClassMemory(cstr className, byte* classMemory)
{

}

void FatalError(int exitCode)
{
  exit(exitCode);
}

}//namespace Zero
