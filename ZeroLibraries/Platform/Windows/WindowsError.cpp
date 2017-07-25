///////////////////////////////////////////////////////////////////////////////
///
/// \file WindowsError.cpp
/// Implementation of Windows error handling.
/// 
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "WindowsError.hpp"

#pragma comment(lib, "user32.lib")

Zero::String ToErrorString(uint errorCode)
{
  //If no error code was provided default to the last error
  //that occurred.
  DWORD error = errorCode != 0 ? errorCode : GetLastError();

  // This is an optimization to avoid allocations for successful operations
  static Zero::String cSuccess("The operation completed successfully.");
  if (error == ERROR_SUCCESS)
    return cSuccess;

  LPVOID messageBuffer = nullptr;

  // Look up windows error string.
  DWORD numberOfChars = FormatMessageA(
    FORMAT_MESSAGE_ALLOCATE_BUFFER  |
    FORMAT_MESSAGE_FROM_SYSTEM      |
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    error,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    (LPSTR)&messageBuffer,
    0,
    NULL);

  Zero::String string((const char*)messageBuffer);
  LocalFree(messageBuffer);

  // When formatting a message, some of them include context (inserts)
  // We don't always have the context, and we don't want the '%1' inserts showing up in the message
  // For the most part, removing them works and the message is still decently valid
  // This also replaces the leading and trailing space for the '%1' Insert
  static Zero::Regex regex(" \\%[0-9]+|\\%[0-9]+ ");
  static Zero::String cEmpty;
  string = regex.Replace(string, cEmpty);
  
  // Add extra error message data
  if (errorCode == ERROR_MOD_NOT_FOUND)
  {
    string = BuildString(string, "This may be because a dependent dll or exe could not be loaded. "
      "Make sure that all executables and dlls have NOT been renamed");
  }

  return string;
}

void FillWindowsErrorStatus(Zero::Status& status)
{
  // If we already failed, don't bother adding more
  if (status.Failed())
    return;

  DWORD errorCode = GetLastError();
  if (errorCode != 0)
    status.SetFailed(ToErrorString(errorCode), errorCode);
}

uint CheckWindowsErrorCode(uint success, cstr format, ...)
{
  if(!success)
  {
    Zero::String errorString = ToErrorString();
    char emptyBuffer[1] = {0};
    char* messageBuffer = emptyBuffer;

    if(format)
    {
      //Use va args to print text
      va_list args;
      va_start(args, format);
      //Get the number of characters needed
      int characters;
      ZeroVSPrintfCount(format, args, 1, characters);
      if(characters > 0)
      {
        messageBuffer = (char*)alloca(characters + 1);
        messageBuffer[characters] = '\0';
        ZeroVSPrintf(messageBuffer, characters + 1, format, args);
      }
      va_end(args);
    }

    // Combine both message and windows error into message.
    ZPrint("%s Windows Error: %s", messageBuffer, errorString.c_str());

    ErrorIf(true, "%s Windows Error: %s", messageBuffer, errorString.c_str());
  }
  return success;
}


cstr GetWindowsExceptionCode(int exceptionCode)
{
  //take from Bruce Dawson's article http://www.altdevblogaday.com/2012/04/20/exceptional-floating-point/
  switch((DWORD)exceptionCode)
  {
  case STATUS_FLOAT_INVALID_OPERATION:
    return "Float Invalid Operation";
  case STATUS_FLOAT_DIVIDE_BY_ZERO:
    return "Float Divide by Zero";
  case STATUS_FLOAT_OVERFLOW:
    return "Float Overflow";
  case STATUS_FLOAT_MULTIPLE_TRAPS:
    return "Float Multiple Traps (sse float exception)";
  case STATUS_INTEGER_DIVIDE_BY_ZERO:
    return "Integer Division by Zero";
  case STATUS_ACCESS_VIOLATION:
    return "Access Violation";
  case STATUS_STACK_OVERFLOW:
    return "Stack Overflow";
  case STATUS_ILLEGAL_INSTRUCTION:
    return "Illegal Instruction";
  case STATUS_BREAKPOINT:
    return "Debug Breakpoint";
  case STATUS_DATATYPE_MISALIGNMENT:
    return "Data Type Misalignment (likely sse not aligned)";
  case STATUS_PRIVILEGED_INSTRUCTION:
    return "Privileged Instruction";
  default:
    return NULL;
  }
}