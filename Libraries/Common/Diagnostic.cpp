// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

ErrorSignaler::ErrorHandler ErrorSignaler::sActiveErrorHandler = Os::ErrorProcessHandler;

bool ErrorSignaler::SignalError(
    SignalErrorType signalType, cstr exp, cstr file, int line, bool& ignore, cstr msgFormat, ...)
{
  if (ignore)
    return false;

  ErrorData errorData;
  errorData.Line = line;
  errorData.File = file;
  errorData.Expression = exp;
  errorData.ErrorType = signalType;
  errorData.Message = nullptr;
  errorData.IgnoreFutureAssert = false;

  if (msgFormat != nullptr)
  {
    va_list args;
    va_start(args, msgFormat);
    // Get the number of characters needed for message
    int bufferSize;
    ZeroVSPrintfCount(msgFormat, args, 1, bufferSize);

    char* messageBuffer = (char*)alloca((bufferSize + 1) * sizeof(char));
    ZeroVSPrintf(messageBuffer, bufferSize + 1, msgFormat, args);
    va_end(args);
    errorData.Message = messageBuffer;
  }

  bool result = (*sActiveErrorHandler)(errorData);
  ignore = errorData.IgnoreFutureAssert;
  return result;
}

} // namespace Zero
