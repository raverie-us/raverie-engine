// MIT Licensed (see LICENSE.md).
#pragma once

typedef size_t rsize_t;
typedef int errno_t;

extern int vsprintf_s(char* buffer, size_t numberOfElements, const char* format, va_list argptr);
extern int sprintf_s(char* buffer, size_t sizeOfBuffer, const char* format, ...);
extern int swprintf_s(wchar_t* buffer, size_t sizeOfBuffer, const wchar_t* format, ...);
extern errno_t strcat_s(char* dest, rsize_t destsz, const char* src);
extern errno_t wcscat_s(wchar_t* dest, rsize_t destsz, const wchar_t* src);
extern errno_t strcpy_s(char* dest, rsize_t destsz, const char* src);
extern errno_t wcscpy_s(wchar_t* dest, rsize_t destsz, const wchar_t* src);
extern errno_t strncpy_s(char* dest, rsize_t destsz, const char* src, rsize_t count);

// A temporary non-thread safe buffer that is only used for counting the lengths
// of printfs
extern char gDiscardBuffer[2];

#define RaverieVSPrintf(destination, bufferSizeBytes, format, args) vsprintf_s(destination, bufferSizeBytes, format, args)

#define RaverieSPrintf(destination, bufferSizeBytes, format, ...)                                                         \
  sprintf_s(destination, bufferSizeBytes, format, __VA_ARGS__)

#define RaverieStrCpy(destination, bufferSizeBytes, source) strcpy_s(destination, bufferSizeBytes, source)

#define RaverieCStringCopy(dest, destSize, source, sourceSize) strncpy_s(dest, (destSize), source, sourceSize);

#define RaverieVSPrintfCount(format, pargs, extraSize, resultSize)                                                        \
  do                                                                                                                   \
  {                                                                                                                    \
    va_list sizeArgs;                                                                                                  \
    va_copy(sizeArgs, pargs);                                                                                          \
    resultSize = vsnprintf(gDiscardBuffer, 1, format, sizeArgs) + extraSize;                                           \
  } while (false)

#define RaverieSPrintfCount(format, ...) snprintf(gDiscardBuffer, 1, format, __VA_ARGS__)

#define AutoDeclare(varName, expression) auto varName = expression

#define AutoDeclareReference(varName, expression) auto& varName = expression

typedef std::nullptr_t NullPointerType;
namespace Raverie
{
typedef std::nullptr_t NullPointerType;
}
