///////////////////////////////////////////////////////////////////////////////
///
/// \file WindowsError.hpp
/// Declaration of Windows error handling.
/// 
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Utility/Typedefs.hpp"

const uint cMaxErrorMessageSize = 1024;
// Convert a windows error code to a human readable string. If the errorCode
// is zero uses the last error.
Zero::String ToErrorString(uint errorCode=0);
void FillWindowsErrorStatus(Zero::Status& status);

// Check success and send error on failure.
uint CheckWindowsErrorCode(uint success, ::cstr format=0, ...);
// Get string from exception code. Unknown will return NULL.
cstr GetWindowsExceptionCode(int exceptionCode);

// Windows Error Macros (always output, even in release, so we get error messages in logs).
#define VerifyWin(expression, ...) do { if(!(expression)) CheckWindowsErrorCode(FALSE, ## __VA_ARGS__); } while(gConditionalFalseConstant)
#define CheckWin(expression, ... ) VerifyWin(expression, ## __VA_ARGS__)

#define WinReturnIf(command, ...) \
{ uint success = command;  if(!success) { VerifyWin(success, ## __VA_ARGS__  ); return (uint)-1; } }

#define WinReturnIfStatus(status, ...)  \
  do                                    \
  {                                     \
    FillWindowsErrorStatus(status);     \
    if((status).Failed())               \
      return __VA_ARGS__;               \
  } while(gConditionalFalseConstant)
