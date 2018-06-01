////////////////////////////////////////////////////////////////////////////////
/// Authors: Dane Curbow
/// Copyright 2018, DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

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
  Error("Not implemented");
  return String();
}

String ComputerName()
{
  Error("Not implemented");
  return String();
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

void DebuggerOutput(const char* message)
{
  Error("Not implemented");
}

void DebugBreak()
{
  Error("Not implemented");
}

void EnableMemoryLeakChecking(int breakOnAllocation)
{
  Error("Not implemented");
}

bool ErrorProcessHandler(ErrorSignaler::ErrorData& errorData)
{
  Error("Not implemented");
  return false;
}

void WebRequest(Status& status, StringParam url, const Array<WebPostData>& postData, const Array<String>& additionalRequestHeaders,
                  WebRequestHeadersFn onHeadersReceived, WebRequestDataFn onDataReceived, void* userData)
{
  Error("Not implemented");
}

void SystemOpenFile(cstr file, uint verb, cstr parameters, cstr workingDirectory)
{
  Error("Not implemented");
}

bool SystemOpenFile(Status& status, cstr file, uint verb, cstr parameters, cstr workingDirectory)
{
  Error("Not implemented");
  return false;
}

void SystemOpenNetworkFile(cstr file, uint verb, cstr parameters, cstr workingDirectory)
{
  Error("Not implemented");
}

bool SystemOpenNetworkFile(Status& status, cstr file, uint verb, cstr parameters, cstr workingDirectory)
{
  Error("Not implemented");
  return false;
}

void GetMemoryStatus(MemoryInfo& data)
{
  Error("Not implemented");
}

String GetEnvironmentalVariable(StringParam variable)
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
