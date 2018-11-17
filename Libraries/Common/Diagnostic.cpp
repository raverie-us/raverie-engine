///////////////////////////////////////////////////////////////////////////////
///
/// \file Diagnostic.cpp
/// Implementation of the basic debug diagnostic functions.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

const int cDebugBufferLength = 1024;

bool DefaultErrorHandler(ErrorSignaler::ErrorData& errorData)
{
  char buffer[cDebugBufferLength];
  ZeroSPrintf(buffer, cDebugBufferLength, "%s(%d) : %s %s\n", errorData.File, 
            errorData.Line, errorData.Message, errorData.Expression);
  Console::Print(Filter::ErrorFilter, buffer);
  return true;
}

ErrorSignaler::ErrorHandler ErrorSignaler::activeErrorHandler = DefaultErrorHandler;

bool ErrorSignaler::SignalError(SignalErrorType signalType, cstr exp,
                               cstr file, int line, bool& ignore, 
                               cstr msgFormat, ...)
{
  if(ignore)
    return false;

  ErrorData errorData;
  errorData.Line = line;
  errorData.File = file;
  errorData.Expression = exp;
  errorData.ErrorType = signalType;
  errorData.Message = nullptr;
  errorData.IgnoreFutureAssert = false;

  if(msgFormat != nullptr)
  {
    va_list args;
    va_start(args, msgFormat);
    //Get the number of characters needed for message
    int bufferSize;
    ZeroVSPrintfCount(msgFormat, args, 1, bufferSize);

    char* messageBuffer = (char*)alloca((bufferSize+1)*sizeof(char));
    ZeroVSPrintf(messageBuffer, bufferSize+1, msgFormat, args);
    va_end(args);
    errorData.Message = messageBuffer;
  }

  bool result = (*activeErrorHandler)(errorData);
  ignore = errorData.IgnoreFutureAssert;
  return result;
}

}//namespace Zero
