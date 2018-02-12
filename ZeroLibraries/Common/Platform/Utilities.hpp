///////////////////////////////////////////////////////////////////////////////
///
/// \file Utilities.hpp
/// Declaration of the Utilities class.
/// 
/// Authors: Trevor Sundberg, Chris Peters
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
/// System Memory Information
struct ZeroShared MemoryInfo
{
  uint Reserve;
  uint Commit;
  uint Free;
};

namespace Os
{

// Sleep the current thread for ms milliseconds.
ZeroShared void Sleep(uint ms);

// Set the Timer Frequency (How often the OS checks threads for sleep, etc)
ZeroShared void SetTimerFrequency(uint ms);

// Get the user name for the current profile
ZeroShared String UserName();

// Get the computer name
ZeroShared String ComputerName();

// Get computer Mac Address of adapter 0
ZeroShared u64 GetMacAddress();

// Check if a debugger is attached
ZeroShared bool IsDebuggerAttached();

// Output a message to any attached debuggers
ZeroShared void DebuggerOutput(const char* message);

// Debug break (only if a debugger is attached)
ZeroShared void DebugBreak();

// Attempts to enable memory leak checking (break on 
ZeroShared void EnableMemoryLeakChecking(int breakOnAllocation = -1);

// When a diagnostic error occurs, this is the default response
ZeroShared bool ErrorProcessHandler(ErrorSignaler::ErrorData& errorData);

namespace WebResponseCode
{
  enum Enum
  {
    Error                         = -1,
    NoServerResponse              = 0,

    // HTTP Log Codes:
    // Informational:
    Continue                      = 100,
    SwitchingProtocols            = 101,
    // Success:
    OK                            = 200,
    Created                       = 201,
    Accepted                      = 202,
    NonauthoritativeInformation   = 203,
    NoContent                     = 204,
    ResetContent                  = 205,
    PartialContent                = 206,
    // Redirection:
    MovedPermanently              = 301,
    ObjectMovedTemporarily        = 302,
    SeeOther                      = 303,
    NotModified                   = 304,
    TemporaryRedirect             = 307,
    // Client Error:
    BadRequest                    = 400,
    AccessDenied                  = 401,
    Forbidden                     = 403,
    NotFound                      = 404,
    HTTPVerbNotAllowed            = 405,
    ClientBrowserRejectsMIME      = 406,
    ProxyAuthenticationRequired   = 407,
    PreconditionFailed            = 412,
    RequestEntityTooLarge         = 413,
    RequestURITooLarge            = 414,
    UnsupportedMediaType          = 415,
    RequestedRangeNotSatisfiable  = 416,
    ExecutionFailed               = 417,
    LockedError                   = 423,
    // Server Error
    InternalServerError           = 500,
    UnimplementedHeaderValueUsed  = 501,
    GatewayProxyReceivedInvalid   = 502,
    ServiceUnavailable            = 503,
    GatewayTimedOut               = 504,
    HTTPVersionNotSupported       = 505
  };
  typedef int Type;
}

typedef void(*WebRequestHeadersFn)(const Array<String>& headers, WebResponseCode::Enum code, void* userData);
typedef void(*WebRequestDataFn)(const byte* data, size_t size, void* userData);

class WebPostData
{
public:
  // The name of the input field in the submitted form
  String mName;

  // Leave this empty if this is not a file
  String mFileName;

  // File contents or a value
  ByteBufferBlock mValue;
};

// Perform an HTTP request (assumed GET, unless POST data is provided)
ZeroShared void WebRequest(
  Status& status,
  StringParam url,
  const Array<WebPostData>& postData,
  const Array<String>& additionalRequestHeaders,
  WebRequestHeadersFn onHeadersReceived,
  WebRequestDataFn onDataReceived,
  void* userData);

// Verb used to open file
DeclareEnum4(Verb, Default, Open, Edit, Run);

// Open the file using the appropriate Os application or
// launch an external application.
ZeroShared void SystemOpenFile(cstr file, uint verb=Verb::Default, cstr parameters = nullptr, cstr workingDirectory = nullptr);
ZeroShared bool SystemOpenFile(Status& status, cstr file, uint verb=Verb::Default, cstr parameters = nullptr, cstr workingDirectory = nullptr);

// Open the network file (including urls) using the appropriate
// Os application or launch an external application
ZeroShared void SystemOpenNetworkFile(cstr file, uint verb = Verb::Default, cstr parameters = nullptr, cstr workingDirectory = nullptr);
ZeroShared bool SystemOpenNetworkFile(Status& status, cstr file, uint verb = Verb::Default, cstr parameters = nullptr, cstr workingDirectory = nullptr);

// Get the memory status of the Os.
ZeroShared void GetMemoryStatus(MemoryInfo& memoryInfo);

// Get an Environmental variable
ZeroShared String GetEnvironmentalVariable(StringParam variable);

// Translate a OS error code.
ZeroShared String TranslateErrorCode(int errorCode);

// Get a string describing the current operating system version.
ZeroShared String GetVersionString();

}

// Generate a 64 bit unique Id. Uses system timer and mac
// address to generate the id.
ZeroShared u64 GenerateUniqueId64();

// Waits for expression to evaluate to true, checking approximately every pollPeriod (in milliseconds)
#define WaitUntil(expression, pollPeriod) \
do { while(!(expression)) { Os::Sleep(pollPeriod); } } while(gConditionalFalseConstant)

}//namespace Zero
