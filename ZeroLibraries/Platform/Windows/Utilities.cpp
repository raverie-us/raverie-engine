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
  OutputDebugStringW(Widen(message).c_str());
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
    ZeroDebugBreak();
  }
}

void EnableMemoryLeakChecking(int breakOnAllocation)
{
  //Set the leak checking flag
  int tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
  tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;
  _CrtSetDbgFlag(tmpDbgFlag);

  //If a valid break alloc provided set the breakAlloc
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
    message.c_str(), expression.c_str(), errorData.File, errorData.Line, "Default");

  // Create a structure to facilitating starting of a process
  STARTUPINFO startUpInfo;
  memset(&startUpInfo, 0, sizeof(startUpInfo));

  // Create another structure to store process information
  PROCESS_INFORMATION processInfo;
  memset(&processInfo, 0, sizeof(processInfo));

  // Start the child process.
  BOOL result = CreateProcess(
    NULL,                 // No module name (use command line)
    (LPTSTR)Widen(commandLine).c_str(),  // Command line
    NULL,                 // Process handle not inheritable
    NULL,                 // Thread handle not inheritable
    FALSE,                // Set handle inheritance to FALSE
    CREATE_NO_WINDOW,     // Creation flags
    NULL,                 // Use parent's environment block
    NULL,                 // Use parent's starting directory
    &startUpInfo,         // Pointer to STARTUPINFO structure
    &processInfo);

  // If we failed to start the process...
  if (!result)
  {
    Console::Print(Filter::ErrorFilter, message.c_str());

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

class WebHandle
{
public:
  WebHandle() :
    mHandle(nullptr)
  {
  }
  WebHandle(HINTERNET handle) :
    mHandle(handle)
  {
  }

  ~WebHandle()
  {
    WinHttpCloseHandle(mHandle);
  }

  operator HINTERNET()
  {
    return mHandle;
  }

  HINTERNET mHandle;
};

void WebRequest(
  Status& status,
  StringParam url,
  const Array<WebPostData>& postDatas,
  const Array<String>& additionalRequestHeaders,
  WebRequestHeadersFn onHeadersReceived,
  WebRequestDataFn onDataReceived,
  void* userData)
{
  WString wurl = Widen(url);
  URL_COMPONENTS urlComp;
  ZeroMemory(&urlComp, sizeof(urlComp));
  urlComp.dwStructSize = sizeof(urlComp);
  urlComp.dwSchemeLength = (DWORD)-1;
  urlComp.dwHostNameLength = (DWORD)-1;
  urlComp.dwUrlPathLength = (DWORD)-1;

  BOOL urlResult = WinHttpCrackUrl(wurl.c_str(), 0, 0, &urlComp);

  if (!urlResult)
  {
    FillWindowsErrorStatus(status, "WinHttpCrackUrl");
    return;
  }

  WebHandle hSession = WinHttpOpen(
    L"WebRequest",
    WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
    WINHTTP_NO_PROXY_NAME,
    WINHTTP_NO_PROXY_BYPASS,
    0);

  if (!hSession)
  {
    FillWindowsErrorStatus(status, "WinHttpOpen");
    return;
  }

  WString hostName(urlComp.lpszHostName, urlComp.dwHostNameLength);

  WebHandle hConnect = WinHttpConnect(
    hSession,
    hostName.c_str(),
    INTERNET_DEFAULT_HTTPS_PORT,
    0);

  if (!hConnect)
  {
    FillWindowsErrorStatus(status, "WinHttpConnect");
    return;
  }
  
  LPCWSTR verb = L"GET";
  if (!postDatas.Empty())
    verb = L"POST";

  WString urlPath(urlComp.lpszUrlPath, urlComp.dwUrlPathLength);

  WebHandle hRequest = WinHttpOpenRequest(
    hConnect,
    verb,
    urlPath.c_str(),
    nullptr,
    WINHTTP_NO_REFERER,
    WINHTTP_DEFAULT_ACCEPT_TYPES,
    WINHTTP_FLAG_SECURE);

  if (!hRequest)
  {
    FillWindowsErrorStatus(status, "WinHttpOpenRequest");
    return;
  }

  WString headers;

  // Turn the additional headers into newline separated strings
  StringBuilder builder;

  forRange(StringParam header, additionalRequestHeaders)
  {
    builder.Append(header);
    builder.Append("\r\n");
  }

  const char* boundary = "----------ZeroEngine43476095-a5a0-4190-a9b5-bce2d2de5eef$";

  builder.Append("Content-Type:multipart/form-data; boundary=");
  builder.Append(boundary);
  builder.Append("\r\n");

  headers = Widen(builder.ToString());

  StringBuilder post;

  forRange(WebPostData& postData, postDatas)
  {
    post.Append("--");
    post.Append(boundary);
    post.Append("\r\n");
    post.Append("Content-Disposition: form-data; name=\"");
    post.Append(postData.mName);
    post.Append("\"");

    if (!postData.mFileName.Empty())
    {
      post.Append("; filename=\"");
      post.Append(postData.mFileName);
      post.Append("\"");
    }

    post.Append("\r\n\r\n");

    // We rely on the fact that this can write binary data
    post.Write(postData.mValue.GetBegin(), postData.mValue.Size());

    post.Append("\r\n");
  }

  post.Append("--");
  post.Append(boundary);
  post.Append("--");

  String postString = post.ToString();

  BOOL bResults = WinHttpSendRequest(
    hRequest,
    headers.c_str(),
    -1,
    (void*)postString.Data(),
    postString.SizeInBytes(),
    postString.SizeInBytes(),
    0);

  if (!bResults)
  {
    FillWindowsErrorStatus(status, "WinHttpSendRequest");
    return;
  }

  bResults = WinHttpReceiveResponse(hRequest, nullptr);

  if (!bResults)
  {
    FillWindowsErrorStatus(status, "WinHttpReceiveResponse");
    return;
  }

  // Call it once just to get the size, then create it again
  DWORD dwHeaderSize = 0;
  WinHttpQueryHeaders(
    hRequest,
    WINHTTP_QUERY_RAW_HEADERS,
    WINHTTP_HEADER_NAME_BY_INDEX,
    nullptr,
    &dwHeaderSize,
    WINHTTP_NO_HEADER_INDEX);

  if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
  {
    WCHAR* lpBuffer = new WCHAR[dwHeaderSize / sizeof(WCHAR)];

    bResults = WinHttpQueryHeaders(
      hRequest,
      WINHTTP_QUERY_RAW_HEADERS,
      WINHTTP_HEADER_NAME_BY_INDEX,
      lpBuffer,
      &dwHeaderSize,
      WINHTTP_NO_HEADER_INDEX);

    if (!bResults)
    {
      FillWindowsErrorStatus(status, "WinHttpQueryHeaders");
      return;
    }

    Array<String> headers;

    while (*lpBuffer != 0)
    {
      headers.PushBack(Narrow(lpBuffer));
      lpBuffer += wcslen(lpBuffer) + 1;
    }

    DWORD dwStatusCode = 0;
    DWORD dwStatusCodeSize = sizeof(dwStatusCode);
    bResults = WinHttpQueryHeaders(
      hRequest,
      WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
      WINHTTP_HEADER_NAME_BY_INDEX,
      &dwStatusCode,
      &dwStatusCodeSize,
      WINHTTP_NO_HEADER_INDEX);

    if (!bResults)
    {
      FillWindowsErrorStatus(status, "WinHttpQueryHeaders");
      return;
    }

    onHeadersReceived(headers, (WebResponseCode::Enum)dwStatusCode, userData);
  }
  else
  {
    FillWindowsErrorStatus(status, "WinHttpQueryHeaders");
    return;
  }
  
  Array<byte> buffer;

  DWORD dwSize = 0;
  do
  {
    // Check for available data
    dwSize = 0;

    if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
    {
      FillWindowsErrorStatus(status, "WinHttpQueryDataAvailable");
      return;
    }

    buffer.Resize(dwSize);

    DWORD dwDownloaded = 0;
    if (!WinHttpReadData(hRequest, (LPVOID)buffer.Data(), dwSize, &dwDownloaded))
    {
      FillWindowsErrorStatus(status, "WinHttpReadData");
      return;
    }

    onDataReceived(buffer.Data(), (size_t)dwDownloaded, userData);
  }
  while (dwSize > 0);
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
  if (gDeterministicMode)
  {
    static u64 deterministicId = 0;
    ++deterministicId;
    return deterministicId;
  }

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
