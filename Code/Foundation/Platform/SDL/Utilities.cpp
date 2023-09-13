// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

namespace Os
{


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

} // namespace Zero
