// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

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

// Check if a debugger is attached
bool IsDebuggerAttached()
{
  return IsDebuggerPresent() == TRUE;
}

ZeroShared void DebuggerOutput(const char* message)
{
  if (IsDebuggerPresent())
    OutputDebugStringW(Widen(message).c_str());
  else
    printf("%s", message);
}

u64 GetMacAddress()
{
  static u64 macAddress = 0;

  if (macAddress == 0)
  {
    IP_ADAPTER_INFO adapterInfoBuffer[16];
    OsInt sizeOfBuffer = sizeof(adapterInfoBuffer);
    OsInt status = GetAdaptersInfo(adapterInfoBuffer, &sizeOfBuffer);

    u64 address = 0;
    for (uint i = 0; i < 5; ++i)
    {
      u64 val = adapterInfoBuffer[0].Address[5 - i];
      val = val << i * 8;
      address += val;
    }

    macAddress = address;
  }

  return macAddress;
}

// Debug break (only if a debugger is attached)
bool DebugBreak()
{
  // If the debugger is attached...
  if (IsDebuggerAttached() == true)
  {
    // Trigger a break point!
    ZeroDebugBreak();
    return true;
  }

  return false;
}

void EnableMemoryLeakChecking(int breakOnAllocation)
{
  // Set the leak checking flag
  int tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
  tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;
  _CrtSetDbgFlag(tmpDbgFlag);

  // If a valid break alloc provided set the breakAlloc
  if (breakOnAllocation != -1)
    _CrtSetBreakAlloc(breakOnAllocation);
}

DeclareEnum4(ReturnCode, Continue, DebugBreak, Terminate, Ignore);

bool ErrorProcessHandler(ErrorSignaler::ErrorData& errorData)
{
  // Stores the resulting quote removed message from below
  String message;
  String expression = errorData.Expression;
  expression = expression.Replace("\"", "'");

  // Check if no message was provided
  if (errorData.Message != nullptr)
  {
    message = errorData.Message;
    message = message.Replace("\"", "'");
  }
  else
  {
    message = "No message";
  }

  ZPrint("%s\n", message.c_str());

  // Output the command line
  String commandLine = String::Format("ErrorDialog.exe \"%s\" \"%s\" \"%s:%d\" %s",
                                      message.c_str(),
                                      expression.c_str(),
                                      errorData.File,
                                      errorData.Line,
                                      "Default");

  // Create a structure to facilitating starting of a process
  STARTUPINFO startUpInfo;
  memset(&startUpInfo, 0, sizeof(startUpInfo));

  // Create another structure to store process information
  PROCESS_INFORMATION processInfo;
  memset(&processInfo, 0, sizeof(processInfo));

  // Start the child process.
  BOOL result = CreateProcess(NULL,                               // No module name (use command line)
                              (LPTSTR)Widen(commandLine).c_str(), // Command line
                              NULL,                               // Process handle not inheritable
                              NULL,                               // Thread handle not inheritable
                              FALSE,                              // Set handle inheritance to FALSE
                              CREATE_NO_WINDOW,                   // Creation flags
                              NULL,                               // Use parent's environment block
                              NULL,                               // Use parent's starting directory
                              &startUpInfo,                       // Pointer to STARTUPINFO structure
                              &processInfo);

  // If we failed to start the process...
  if (!result)
  {
    Console::Print(Filter::ErrorFilter, "%s", message.c_str());

    // Show a message box instead
    message = BuildString(message, "\nWould you like to continue?");
    int result = MessageBoxA(NULL, message.c_str(), "Error", MB_YESNO | MB_ICONEXCLAMATION);

    // Trigger a break point
    return result == IDNO;
  }

  // Now wait forever for the process to finish
  WaitForSingleObject(processInfo.hProcess, INFINITE);

  // Get the exit code of the process since it should have finished by now
  DWORD exitCode = 0;
  BOOL success = GetExitCodeProcess(processInfo.hProcess, &exitCode);

  // Close unused thread handle
  CloseHandle(processInfo.hThread);

  // If we somehow failed to get the exit code, trigger a break point
  if (!success)
    return true;

  // Based on the exit code...
  switch (exitCode)
  {
  case ReturnCode::Continue:
    // No debug break, just continue
    return false;

  case ReturnCode::DebugBreak:
    // Returning true will cause a break point
    return true;

  case ReturnCode::Terminate:
    // Immediately kill the application
    TerminateProcess(GetCurrentProcess(), 0);
    return false;

  case ReturnCode::Ignore:
    errorData.IgnoreFutureAssert = true;
    return false;

  default:
    // Force a break point, we have no idea what we got back
    return true;
  }
}

bool SystemOpenFile(Status& status, cstr file, cwstr verb, cstr parameters = nullptr, cstr workingDirectory = nullptr)
{
  HINSTANCE success = ShellExecute(NULL,
                                   verb,
                                   Widen(file).c_str(),
                                   Widen(parameters).c_str(),
                                   Widen(workingDirectory).c_str(),
                                   TRUE);

  const HINSTANCE shellSucceed = (HINSTANCE)32;
  if (success > shellSucceed)
    return true;

  int errorCode = (int)GetLastError();
  String errorString = ToErrorString(errorCode);
  String message = String::Format("Failed to execute shell command with file '%s'. %s", file, errorString.c_str());
  status.SetFailed(message, errorCode);

  return false;
}

bool ShellOpenDirectory(StringParam directory)
{
  Status status;
  SystemOpenFile(status, directory.c_str(), L"open");
  return status.Succeeded();
}

bool ShellOpenFile(StringParam file)
{
  Status status;
  SystemOpenFile(status, file.c_str(), NULL);
  return status.Succeeded();
}

bool ShellEditFile(StringParam file)
{
  Status status;
  SystemOpenFile(status, file.c_str(), L"edit");
  return status.Succeeded();
}

bool ShellOpenApplication(StringParam file, StringParam parameters, StringParam workingDirectory)
{
  Status status;
  SystemOpenFile(status, file.c_str(), NULL, parameters.c_str(), !workingDirectory.Empty() ? workingDirectory.c_str() : nullptr);
  return status.Succeeded();
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
  Status status;
  SystemOpenFile(status, url, L"open", NULL, NULL);
}

void MarkAsExecutable(cstr fileName)
{
}

unsigned int GetDoubleClickTimeMs()
{
  return (unsigned int)GetDoubleClickTime();
}

String GetEnvironmentalVariable(StringParam variable)
{
  char* envVarValue = getenv(variable.c_str());

  if (envVarValue)
    return envVarValue;
  else
    return String();
}

void GetMemoryStatus(MemoryInfo& data)
{
  size_t pageRegion = 0;
  size_t foundPage = 1;
  while (foundPage)
  {
    MEMORY_BASIC_INFORMATION memoryInfo;
    // VirtualQueryEx return the size of the MEMORY_BASIC_INFORMATION if it
    // succeeds or zero if no more pages are found.
    foundPage = VirtualQueryEx(GetCurrentProcess(), (void*)pageRegion, &memoryInfo, sizeof(memoryInfo));
    if (foundPage)
    {
      if (memoryInfo.State & MEM_FREE)
      {
        data.Free += memoryInfo.RegionSize;
      }
      else
      {
        if (memoryInfo.State & MEM_RESERVE)
          data.Reserve += memoryInfo.RegionSize;

        if (memoryInfo.State & MEM_COMMIT)
          data.Commit += memoryInfo.RegionSize;
      }

      // Move past this region to find another page.
      pageRegion += memoryInfo.RegionSize;
    }
  }
}

typedef void(WINAPI* GetNativeSystemInfoPtr)(LPSYSTEM_INFO);

String GetVersionString()
{
  OSVERSIONINFOEX osvi;
  SYSTEM_INFO si;

  ZeroMemory(&si, sizeof(SYSTEM_INFO));
  ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));

  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

  // The function 'GetVersionEx' still works on Windows 8.1 but is deprecated,
  // so if the build is made on windows 8.1 this can often cause errors
#pragma warning(push)
#pragma warning(disable : 4996)
  BOOL bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO*)&osvi);
#pragma warning(pop)

  StringBuilder builder;

  if (!bOsVersionInfoEx)
    return String();

  // Call GetNativeSystemInfo if supported or GetSystemInfo otherwise.
  // This gets more information to determine 64bit vs 32bit but
  // is only available on later versions.

  GetNativeSystemInfoPtr pGNSI =
      (GetNativeSystemInfoPtr)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetNativeSystemInfo");

  if (pGNSI != NULL)
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
      if (osvi.dwMinorVersion == 0)
      {
        if (osvi.wProductType == VER_NT_WORKSTATION)
          builder << "Windows Vista ";
        else
          builder << "Windows Server 2008 ";
      }

      if (osvi.dwMinorVersion == 1)
      {
        if (osvi.wProductType == VER_NT_WORKSTATION)
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
      if (GetSystemMetrics(SM_SERVERR2))
        builder << "Windows Server 2003 R2";
      else if (osvi.wSuiteMask & VER_SUITE_STORAGE_SERVER)
        builder << "Windows Storage Server 2003";
      else if (osvi.wSuiteMask & VER_SUITE_WH_SERVER)
        builder << "Windows Home Server";
      else if (osvi.wProductType == VER_NT_WORKSTATION && si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
        builder << "Windows XP Professional x64 Edition";
      else
        builder << "Windows Server 2003";
    }

    if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1)
    {
      builder << "Windows XP ";
      if (osvi.wSuiteMask & VER_SUITE_PERSONAL)
        builder << "Home Edition";
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
      if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
        builder << ", 64-bit";
      else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
        builder << ", 32-bit";
    }

    return builder.ToString();
  }

  else
  {
    return String("Unknown");
  }
}

bool GetRegistryValue(void* key, StringParam subKey, StringParam value, String& result)
{
  HKEY hRoot = (HKEY)key;

  // Read OEM Name from the key
  wchar_t nameBuffer[256] = {0};
  DWORD length = sizeof(nameBuffer);

  HKEY hKey;
  LONG queryResult = RegOpenKeyEx(hRoot, Widen(subKey).c_str(), 0, KEY_QUERY_VALUE, &hKey);
  if (queryResult != ERROR_SUCCESS)
    return false;

  queryResult = RegQueryValueEx(hKey, Widen(value).c_str(), 0, 0, (LPBYTE)nameBuffer, &length);
  if (queryResult != ERROR_SUCCESS)
    return false;

  result = Narrow(nameBuffer);
  return true;
}

String GetInstalledExecutable(StringParam organization, StringParam name, StringParam guid)
{
  String path1 = String::Format("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{%s}_is1", guid.c_str());
  String path2 = String::Format("SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\Cur"
                                "rentVersion\\Uninstall\\{%s}}_is1",
                                guid.c_str());
  static const String cInstallLocation("InstallLocation");
  String directory;
  bool result = GetRegistryValue(HKEY_CURRENT_USER, path1, cInstallLocation, directory) ||
                GetRegistryValue(HKEY_CURRENT_USER, path2, cInstallLocation, directory) ||
                GetRegistryValue(HKEY_LOCAL_MACHINE, path1, cInstallLocation, directory) ||
                GetRegistryValue(HKEY_LOCAL_MACHINE, path2, cInstallLocation, directory);
  if (!result)
    return GetRelativeExecutable(organization, name);
  return FilePath::Combine(directory, organization + name + cExecutableExtensionWithDot);
}

} // namespace Os

u64 GenerateUniqueId64()
{
  if (gDeterministicMode)
  {
    static u64 deterministicId = 0;
    ++deterministicId;
    return deterministicId;
  }

  // Get the mac address of the machine
  static u64 cachedMacAdress = (Os::GetMacAddress() << 16);

  u64 newId = cachedMacAdress;

  /// Get the low part of the performance counter
  LARGE_INTEGER performanceCounter;
  BOOL status = QueryPerformanceCounter(&performanceCounter);
  u64 lowCount = performanceCounter.LowPart;

  // Get the current system time (since 1970)
  time_t systemTimeT;
  time(&systemTimeT);

  u64 systemTime = systemTimeT;

  // combine the two time parts
  u64 lowId = (u64(systemTime) << 32) | (lowCount);

  // Keep incrementing the low value
  static uint shiftCount = 0;
  ++shiftCount;
  lowId += shiftCount;

  // Or it with the mac part
  newId ^= lowId;

  return newId;
}

} // namespace Zero
