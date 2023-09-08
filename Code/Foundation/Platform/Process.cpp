// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

struct ProcessPrivateData
{
};

Process::Process()
{
}

Process::~Process()
{
}

void Process::Start(Status& status, ProcessStartInfo& info)
{
}

int Process::WaitForClose()
{
  return 0;
}

int Process::WaitForClose(unsigned long milliseconds)
{
  return 0;
}

bool Process::IsRunning()
{
  return false;
}

void Process::Close()
{
}

void Process::Terminate()
{
}

void Process::OpenStandardOut(File& fileStream)
{
}

void Process::OpenStandardError(File& fileStream)
{
}

void Process::OpenStandardIn(File& fileStream)
{
}

bool Process::IsStandardOutRedirected()
{
  return false;
}

bool Process::IsStandardErrorRedirected()
{
  return false;
}

bool Process::IsStandardInRedirected()
{
  return false;
}

void GetProcesses(Array<ProcessInfo>& results)
{
}

void KillProcess(OsInt processId, int exitCode)
{
}

void RegisterApplicationRestartCommand(StringParam commandLineArgs, uint flags)
{
}

} // namespace Zero
