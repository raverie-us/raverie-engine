// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

// For platforms where we haven't ported the executables over, but they have an
// emulator, we can fill this string out (must have a trailing space, e.g. "wine
// ");
const char* gProcessEmulator = "";

namespace Zero
{
struct ProcessPrivateData
{
  Thread mThread;
  String mCommandLine;
};

Process::Process()
{
  ZeroConstructPrivateData(ProcessPrivateData);
}

Process::~Process()
{
  Close();
  ZeroDestructPrivateData(ProcessPrivateData);
}

OsInt ProcessThread(void* commandLinePtr)
{
  const char* commandLine = (const char*)commandLinePtr;
  return system(commandLine);
}

void Process::Start(Status& status, ProcessStartInfo& info)
{
  ZeroGetPrivateData(ProcessPrivateData);

  String commandLine = info.mArguments;

  if (!info.mApplicationName.Empty())
    commandLine =
        String::Format("%s\"%s\" %s", gProcessEmulator, info.mApplicationName.c_str(), info.mArguments.c_str());

  // For some strange reason, 'system' doesn't work if the executable
  // is quoted, even though that properly works in the shell.
  if (commandLine.Front() == Rune('"'))
  {
    size_t foundQuotes = 0;
    StringBuilder builder;
    forRange (Rune rune, commandLine)
    {
      if (foundQuotes >= 2)
        builder.Append(rune);
      else if (rune == Rune('"'))
        ++foundQuotes;
      else
        builder.Append(rune);
    }
    commandLine = builder.ToString();
  }

  self->mCommandLine = commandLine;
  self->mThread.Initialize(&ProcessThread, (void*)commandLine.c_str(), "Process");
}

void Process::Close()
{
  ZeroGetPrivateData(ProcessPrivateData);

  // Let the process thread continue running.
  self->mThread.Detach();
}

void Process::Terminate()
{
  ZeroGetPrivateData(ProcessPrivateData);

  // We can't actually terminate the process.
  self->mThread.Detach();
}

bool Process::IsRunning()
{
  ZeroGetPrivateData(ProcessPrivateData);
  return !self->mThread.IsCompleted();
}

int Process::WaitForClose()
{
  ZeroGetPrivateData(ProcessPrivateData);
  return (int)self->mThread.WaitForCompletion();
}

int Process::WaitForClose(unsigned long milliseconds)
{
  Error("Waiting a specified amount of time is unsupported");
  return WaitForClose();
}

void Process::OpenStandardOut(File& fileStream)
{
  // Unsupported.
}

void Process::OpenStandardError(File& fileStream)
{
  // Unsupported.
}

void Process::OpenStandardIn(File& fileStream)
{
  // Unsupported.
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
  // Unsupported. When we get our current executable from the
  // command line we should probably fill out that single entry.
}

void KillProcess(OsInt processId, int exitCode)
{
  // Unsupported.
}

void RegisterApplicationRestartCommand(StringParam commandLineArgs, uint flags)
{
  // Unsupported.
}

} // namespace Zero
