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

namespace Zero
{

namespace Os
{

void Sleep(uint ms)
{
  Error("Not implemented");
}

void SetTimerFrequency(uint ms)
{
  Error("Not implemented");
}

String UserName()
{
  return "User";
}

String ComputerName()
{
  return "Computer";
}

u64 GetMacAddress()
{
  Error("Not implemented");
  return 0;
}

bool IsDebuggerAttached()
{
  Error("Not implemented");
  return false;
}

void DebugBreak()
{
  Error("Not implemented");
}

void SystemOpenFile(cstr file, uint verb, cstr parameters, cstr workingDirectory)
{
  Error("Not implemented");
}

void GetMemoryStatus(MemoryInfo& data)
{
  Error("Not implemented");
}

String GetEnvironmentalVariable(StringRef variable)
{
  Error("Not implemented");
  return String();
}

String TranslateErrorCode(int errorCode)
{
  Error("Not implemented");
  return String();
}

String GetVersionString()
{
  Error("Not implemented");
  return String();
}

}

u64 GenerateUniqueId64()
{
  Error("Not implemented");
  return 0;
}


}//namespace Zero
