///////////////////////////////////////////////////////////////////////////////
///
/// \file Standard.hpp
/// Include standard header files.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

// A temporary non-thread safe buffer that is only used for counting the lengths of printfs
extern char gDiscardBuffer[2];

#define ZeroVSPrintf(destination, bufferSizeBytes, format, args) \
  vsprintf_s(destination, bufferSizeBytes, format, args)

#define ZeroSPrintf(destination, bufferSizeBytes, format, ...) \
  sprintf_s(destination, bufferSizeBytes, format, __VA_ARGS__)

#define ZeroSWPrintf(destination, bufferSizeBytes, format, ...) \
  swprintf_s(destination, bufferSizeBytes, format, __VA_ARGS__)

#define ZeroStrCat(destination, bufferSizeBytes, source) \
  strcat_s(destination, bufferSizeBytes, source)

#define ZeroStrCatW(destination, bufferSizeBytes, source) \
  wcscat_s(destination, bufferSizeBytes, source)

#define ZeroStrCpy(destination, bufferSizeBytes, source) \
  strcpy_s(destination, bufferSizeBytes, source)

#define ZeroStrCpyW(destination, bufferSizeBytes, source) \
  wcscpy_s(destination, bufferSizeBytes, source)

#define ZeroCStringCopy(dest, destSize, source, sourceSize) \
  strncpy_s(dest, (destSize), source, sourceSize);

#define ZeroVSPrintfCount(format, pargs, extraSize, resultSize)             \
do                                                                          \
{                                                                           \
  va_list sizeArgs;                                                         \
  va_copy(sizeArgs, pargs);                                                 \
  resultSize = vsnprintf(gDiscardBuffer, 1, format, sizeArgs) + extraSize;  \
}                                                                           \
while (false)

#define ZeroSPrintfCount(format, ...) snprintf(gDiscardBuffer, 1, format, __VA_ARGS__)

#define AutoDeclare(varName, expression) \
    auto varName = expression

#define AutoDeclareReference(varName, expression) \
    auto& varName = expression

typedef std::nullptr_t NullPointerType;
namespace Zero
{
  typedef std::nullptr_t NullPointerType;
}

