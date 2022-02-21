// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

namespace Os
{

void Sleep(uint ms)
{
}

void SetTimerFrequency(uint ms)
{
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
  return 0;
}

bool IsDebuggerAttached()
{
  return false;
}

void DebuggerOutput(const char* message)
{
}

bool DebugBreak()
{
  return false;
}

void EnableMemoryLeakChecking(int breakOnAllocation)
{
}

bool ErrorProcessHandler(ErrorSignaler::ErrorData& errorData)
{
  const int cDebugBufferLength = 1024;
  char buffer[cDebugBufferLength];
  ZeroSPrintf(buffer,
              cDebugBufferLength,
              "%s(%d) : %s %s\n",
              errorData.File,
              errorData.Line,
              errorData.Message,
              errorData.Expression);
  Console::Print(Filter::ErrorFilter, buffer);
  return true;
}

void WebRequest(Status& status,
                StringParam url,
                const Array<WebPostData>& postData,
                const Array<String>& additionalRequestHeaders,
                WebRequestHeadersFn onHeadersReceived,
                WebRequestDataFn onDataReceived,
                void* userData)
{
  status.SetFailed("WebRequest not implemented");
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

bool SupportsDownloadingFiles()
{
  return false;
}

void DownloadFile(cstr fileName, const DataBlock& data)
{
}

void OpenUrl(cstr url)
{
}

void MarkAsExecutable(cstr fileName)
{
}

unsigned int GetDoubleClickTimeMs()
{
  return 500;
}

void GetMemoryStatus(MemoryInfo& data)
{
}

String GetEnvironmentalVariable(StringParam variable)
{
  return String();
}

String GetVersionString()
{
  return String();
}

String GetInstalledExecutable(StringParam organization, StringParam name, StringParam guid)
{
  return GetRelativeExecutable(organization, name);
}
} // namespace Os

u64 GenerateUniqueId64()
{
  static u64 counter = 0;
  return ++counter;
}

} // namespace Zero
