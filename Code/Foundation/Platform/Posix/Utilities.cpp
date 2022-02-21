// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"
#include "Common/Utilities.hpp"
#include "Common/FileSystem.hpp"
#include "Common/CrashHandler.hpp"

#include <unistd.h>
#include <pwd.h>

namespace Zero
{

namespace Os
{

void Sleep(uint ms)
{
  usleep(ms * 1000);
}

void DebugBreak()
{
  __builtin_trap();
}

bool ShellOpenDirectory(StringParam directory)
{
  return false;
}

bool ShellOpenFile(StringParam file)
{
  return false;
}

bool ShellEditFile(StringParam file)
{
  return false;
}

bool ShellOpenApplication(StringParam file, StringParam parameters, StringParam workingDirectory)
{
  return false;
}

String GetEnvironmentalVariable(StringRef variable)
{
  return getenv(variable.c_str());
}

bool IsDebuggerAttached()
{
  return false;
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

String GetInstalledExecutable(StringParam organization, StringParam name, StringParam guid)
{
  return GetRelativeExecutable(organization, name);
}

} // namespace Os

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

} // namespace Zero
