// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

namespace Os
{

void Sleep(uint ms)
{
  SDL_Delay((Uint32)ms);
}

const char* GetEnvironmentList(const char* defaultValue, const char* names[], size_t length)
{
  for (size_t i = 0; i < length; ++i)
  {
    const char* name = names[i];
    const char* result = getenv(name);

    if (result != nullptr && strlen(result) != 0)
      return result;
  }

  return defaultValue;
}

#if !defined(ZeroPlatformNoUserName)
String UserName()
{
  // There is no portable way to get the user name
  const char* names[] = {"USER", "USERNAME", "LOGNAME", "COMPUTERNAME", "HOSTNAME"};
  return GetEnvironmentList("User", names, SDL_arraysize(names));
}
#endif

#if !defined(ZeroPlatformNoComputerName)
String ComputerName()
{
  // There is no portable way to get the computer/host name
  const char* names[] = {"COMPUTERNAME", "HOSTNAME", "USER", "USERNAME", "LOGNAME"};
  return GetEnvironmentList("Computer", names, SDL_arraysize(names));
}
#endif

DeclareEnum4(ReturnCode, Continue, DebugBreak, Terminate, Ignore);

bool ErrorProcessHandler(ErrorSignaler::ErrorData& errorData)
{
  // Stores the resulting quote removed message from below
  String expression = errorData.Expression;

  // Check if no message was provided
  const char* errorMessage = errorData.Message ? errorData.Message : "No message";

  String message = BuildString(errorMessage, "\n");

  String callStack = GetCallStack();
  if (!callStack.Empty())
    message = BuildString(message, callStack, "\n");

  Console::Print(Filter::ErrorFilter, message.c_str());

  // Show a message box
  message = BuildString(message, "Would you like to continue?");

  const SDL_MessageBoxButtonData buttons[] = {
      {0, ReturnCode::Continue, "Continue"},
      {SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, ReturnCode::DebugBreak, "DebugBreak"},
      {SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, ReturnCode::Terminate, "Terminate"},
      {0, ReturnCode::Ignore, "Ignore"},
  };

  SDL_MessageBoxData data = {
      SDL_MESSAGEBOX_INFORMATION, nullptr, "Error", message.c_str(), SDL_arraysize(buttons), buttons, nullptr};

  int buttonId = -1;
  SDL_ShowMessageBox(&data, &buttonId);

  // Based on the exit code...
  switch (buttonId)
  {
  case ReturnCode::Continue:
    // No debug break, just continue
    return false;

  case ReturnCode::DebugBreak:
    // Returning true will cause a break point
    return true;

  case ReturnCode::Terminate:
    // Immediately kill the application
    abort();
    return false;

  case ReturnCode::Ignore:
    errorData.IgnoreFutureAssert = true;
    return false;

  default:
    // Force a break point, we have no idea what we got back
    return true;
  }
}

#if !defined(ZeroPlatformNoWebRequest)
void WebRequest(Status& status,
                StringParam url,
                const Array<WebPostData>& postDatas,
                const Array<String>& additionalRequestHeaders,
                WebRequestHeadersFn onHeadersReceived,
                WebRequestDataFn onDataReceived,
                void* userData)
{
  status.SetFailed("WebRequest not supported by SDL");
}
#endif

#if !defined(ZeroPlatformNoOpenUrl)
void OpenUrl(cstr url)
{
  ShellOpen(url);
}
#endif

unsigned int GetDoubleClickTimeMs()
{
  return 500;
}
} // namespace Os

u64 GenerateUniqueId64()
{
  u64 result = SDL_GetPerformanceCounter() ^ SDL_GetTicks();

  static u64 highCount = 0;
  ++highCount;
  result += (highCount << 48) ^ Os::UserName().Hash();

  return result;
}
} // namespace Zero
