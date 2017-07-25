///////////////////////////////////////////////////////////////////////////////
///
/// \file Utilities.cpp
/// Implementation of the Utilities class.
/// 
/// Authors: Trevor Sundberg
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

#include <Lmcons.h>
#include <shellapi.h>
#include <iptypes.h>
#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "Advapi32.lib")

namespace Zero
{

namespace Os
{

void Sleep(uint ms)
{
  ::Sleep(ms);
}

void SetTimerFrequency(uint ms)
{
  ::timeBeginPeriod(ms);
}

String UserName()
{
  wchar_t buffer[UNLEN + 1];
  DWORD size = UNLEN + 1;
  GetUserName(buffer, &size);
  return Narrow(buffer);
}

String ComputerName()
{
  wchar_t buffer[MAX_COMPUTERNAME_LENGTH + 1];
  DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
  GetComputerName(buffer, &size);
  return Narrow(buffer);
}

#ifndef _MSC_VER
BOOL CheckRemoteDebuggerPresent(HANDLE, PBOOL pbDebuggerPresent)
{
  *pbDebuggerPresent = FALSE;
  return FALSE;
}
#endif

// Check if a debugger is attached
bool IsDebuggerAttached()
{
  return IsDebuggerPresent() == TRUE;
}

u64 GetMacAddress()
{
  static u64 macAddress = 0;

  if(macAddress == 0)
  {
    IP_ADAPTER_INFO adapterInfoBuffer[16];
    OsInt sizeOfBuffer = sizeof(adapterInfoBuffer);
    OsInt status = GetAdaptersInfo(adapterInfoBuffer, &sizeOfBuffer);

    u64 address = 0;
    for(uint i=0;i<5;++i)
    {
      u64 val = adapterInfoBuffer[0].Address[5 - i];
      val = val << i*8;
      address += val;
    }

    macAddress = address;
  }

  return macAddress;
}


// Debug break (only if a debugger is attached)
void DebugBreak()
{
  // If the debugger is attached...
  if (IsDebuggerAttached() == true)
  {
    // Trigger a break point!
    ZERO_DEBUG_BREAK;
  }
}

cwstr windowsVerbNames[] = {NULL, L"open", L"edit", L"run"};
void SystemOpenFile(cstr file, uint verb, cstr parameters,
  cstr workingDirectory)
{
  Status status;
  SystemOpenFile(status, file, verb, parameters, workingDirectory);
}

bool SystemOpenFile(Status& status, cstr file, uint verb, cstr parameters, cstr workingDirectory)
{
  HINSTANCE success = ShellExecute(NULL, windowsVerbNames[verb], Widen(file).c_str(), Widen(parameters).c_str(), Widen(workingDirectory).c_str(), TRUE);
  
  const HINSTANCE shellSucceed = (HINSTANCE)32;
  if(success > shellSucceed)
    return true;

  int errorCode = (int)GetLastError();
  String errorString = ToErrorString(errorCode);
  String message = String::Format("Failed to execute shell command with file '%s'. %s", file, errorString.c_str());
  status.SetFailed(message, errorCode);

  return false;
}

void SystemOpenNetworkFile(cstr file, uint verb, cstr parameters, cstr workingDirectory)
{
  Status status;
  SystemOpenNetworkFile(status, file, verb, parameters, workingDirectory);
}

bool SystemOpenNetworkFile(Status& status, cstr file, uint verb, cstr parameters, cstr workingDirectory)
{
  return SystemOpenFile(status, file, verb, parameters, workingDirectory);
}

String GetEnvironmentalVariable(StringParam variable)
{
  char* envVarValue = getenv(variable.c_str());

  if(envVarValue)
    return envVarValue;
  else
    return String();

}

//---------------------------------------------------------------- Memory Status 
void GetMemoryStatus(MemoryInfo& data)
{
  size_t pageRegion = 0;
  size_t foundPage = 1;
  while (foundPage)
  {
    MEMORY_BASIC_INFORMATION memoryInfo;
    //VirtualQueryEx return the size of the MEMORY_BASIC_INFORMATION if it 
    //succeeds or zero if no more pages are found.
    foundPage = VirtualQueryEx(GetCurrentProcess(), (void*)pageRegion,
      &memoryInfo, sizeof(memoryInfo));
    if(foundPage)
    {
      if(memoryInfo.State & MEM_FREE)
      {
        data.Free += memoryInfo.RegionSize;
      }
      else
      {
        if(memoryInfo.State & MEM_RESERVE)
          data.Reserve += memoryInfo.RegionSize;

        if(memoryInfo.State & MEM_COMMIT)
          data.Commit += memoryInfo.RegionSize;
      }

      //Move past this region to find another page.
      pageRegion += memoryInfo.RegionSize;
    }
  }
}


String TranslateErrorCode(int errorCode)
{
  //Try exception codes
  cstr exceptionCode = GetWindowsExceptionCode(errorCode);
  if(exceptionCode)
    return exceptionCode;

  //Try windows formatting
  String errorString = ToErrorString(errorCode);
  return errorString;
}

typedef void (WINAPI *GetNativeSystemInfoPtr)(LPSYSTEM_INFO);

String GetVersionString()
{
  OSVERSIONINFOEX osvi;
  SYSTEM_INFO si;

  ZeroMemory(&si, sizeof(SYSTEM_INFO));
  ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));

  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

  // The function 'GetVersionEx' still works on Windows 8.1 but is deprecated, so if the build
  // is made on windows 8.1 this can often cause errors
#pragma warning(push)
#pragma warning(disable: 4996)
  BOOL bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO*) &osvi);
#pragma warning(pop)

  StringBuilder builder;

  if(!bOsVersionInfoEx) return String();

  // Call GetNativeSystemInfo if supported or GetSystemInfo otherwise.
  // This gets more information to determine 64bit vs 32bit but
  // is only available on later versions.

  GetNativeSystemInfoPtr pGNSI = (GetNativeSystemInfoPtr) GetProcAddress(
    GetModuleHandle(TEXT("kernel32.dll")), "GetNativeSystemInfo");

  if(pGNSI != NULL)
    pGNSI(&si);
  else 
    GetSystemInfo(&si);

  if (VER_PLATFORM_WIN32_NT == osvi.dwPlatformId && osvi.dwMajorVersion > 4)
  {
    builder << "Microsoft ";

    // Test for the specific product.
    if (osvi.dwMajorVersion == 10)
    {
      if (osvi.dwMinorVersion == 0)
      {
        if (osvi.wProductType == VER_NT_WORKSTATION)
          builder << "Windows 10 ";
        else
          builder << "Windows Server 2016 ";
      }
    }

    if (osvi.dwMajorVersion == 6)
    {
      if(osvi.dwMinorVersion == 0)
      {
        if(osvi.wProductType == VER_NT_WORKSTATION)
          builder << "Windows Vista ";
        else
          builder << "Windows Server 2008 ";
      }

      if (osvi.dwMinorVersion == 1)
      {
        if(osvi.wProductType == VER_NT_WORKSTATION)
          builder << "Windows 7 ";
        else
          builder << "Windows Server 2008 R2 ";
      }

      if (osvi.dwMinorVersion == 2)
      {
        if (osvi.wProductType == VER_NT_WORKSTATION)
          builder << "Windows 8 ";
        else
          builder << "Windows Server 2012 ";
      }

      if (osvi.dwMinorVersion == 3)
      {
        if (osvi.wProductType == VER_NT_WORKSTATION)
          builder << "Windows 8.1 ";
        else
          builder << "Windows Server 2012 R2 ";
      }
    }

    if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2)
    {
      if(GetSystemMetrics(SM_SERVERR2))
        builder << "Windows Server 2003 R2";
      else if (osvi.wSuiteMask & VER_SUITE_STORAGE_SERVER)
        builder << "Windows Storage Server 2003";
      else if (osvi.wSuiteMask & VER_SUITE_WH_SERVER)
        builder << "Windows Home Server";
      else if(osvi.wProductType == VER_NT_WORKSTATION &&
        si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
        builder << "Windows XP Professional x64 Edition";
      else
        builder << "Windows Server 2003";
    }

    if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1)
    {
      builder << "Windows XP ";
      if(osvi.wSuiteMask & VER_SUITE_PERSONAL)
        builder <<  "Home Edition";
      else
        builder << "Professional";
    }

    if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0)
    {
      builder << "Windows 2000 ";
    }

    // Include service pack (if any) and build number.

    if (wcslen(osvi.szCSDVersion) > 0)
    {
      builder << (const char*)osvi.szCSDVersion;
    }

    builder << " (Build " << int(osvi.dwBuildNumber) << ")";

    if (osvi.dwMajorVersion >= 6)
    {
      if (si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
        builder << ", 64-bit";
      else if (si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_INTEL)
        builder << ", 32-bit";
    }

    return builder.ToString(); 
  }

  else
  {
    return String("Unknown");
  }
}

}

u64 GenerateUniqueId64()
{
  //Get the mac address of the machine
  static u64 cachedMacAdress = (Os::GetMacAddress() << 16);

  u64 newId = cachedMacAdress;

  ///Get the low part of the performance counter
  LARGE_INTEGER performanceCounter;
  BOOL status = QueryPerformanceCounter(&performanceCounter);
  u64 lowCount = performanceCounter.LowPart;

  //Get the current system time (since 1970)
  time_t systemTimeT;
  time(&systemTimeT);

  u64 systemTime = systemTimeT;

  //combine the two time parts
  u64 lowId =  (u64(systemTime) << 32) | (lowCount);

  //Keep incrementing the low value
  static uint shiftCount = 0;
  ++shiftCount;  
  lowId += shiftCount;

  //Or it with the mac part
  newId ^= lowId;

  return newId;
}
}
