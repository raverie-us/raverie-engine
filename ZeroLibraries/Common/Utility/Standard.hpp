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

// Make sure the min and max macros aren't defined
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#ifdef NDEBUG
  #define ZeroRelease 1
#else
  #define ZeroDebug 1
#endif

// Compiler Errors / Warnings
#ifdef _MSC_VER
  #define ZNestedTemplate

  #ifndef ZeroDebug
  #define _SECURE_SCL 0
  #endif

  // Enable these warnings by setting them to level 3
  // Enable warning function does not override any base class virtual member function
  // this is useful to catch renaming virtual functions
  #pragma warning(3: 4263)

  // Enable warning level 3 Conversion of an Enum to a integral type
  #pragma warning(3: 4239)

  // Ignore the warning about unusual use of type bool
  #pragma warning(disable: 4804)

  // Ignore the warning about forcing value to bool
  #pragma warning(disable: 4800)

  // We don't care about 'behavior change - pod constructors'
  #pragma warning(disable: 4345)

  // Ignore the warning about placement-new for POD data
  #pragma warning(disable: 4345)

  // Constants in 'if' and 'while' are used for debug macros
  #pragma warning(disable: 4127)

  // Disable the warning 'this' in base initializer
  #pragma warning(disable: 4355)

  // We use the nameless struct/union extension and it is legal
  // on other compilers
  #pragma warning(disable: 4201)

  // Disable deprecation warnings
  #pragma warning(disable:4996)

#ifdef WIN64
  // Temporary 64-bit support (ignore size_t conversion warnings)
  #pragma warning(disable: 4244)
  #pragma warning(disable: 4267)
  #pragma warning(disable: 4309)
#endif

#else

  // gcc requires this keyword with a templated class that
  // returns another templated class inside of itself
  #define ZNestedTemplate template

  // Make override empty
  #define override
#endif

// A temporary non-thread safe buffer that is only used for counting the lengths of printfs
extern char gDiscardBuffer[2];

// These functions are specific to all compilers on Windows
#if defined(_WIN32) || defined(WIN32)
  #define ZeroVSPrintfCount(format, pargs, extraSize, resultSize) resultSize = _vscprintf(format, pargs) + extraSize
  #define ZeroSPrintfCount(format, ...) _scprintf(format, __VA_ARGS__)
#endif

// Printing Macros
#ifdef _MSC_VER

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

#if _MSC_VER < 1800
  #define va_copy(dest, source) \
    dest = source;
#endif

#else
#include <stdio.h>
// If these functions weren't defined above, define them now
#ifndef ZeroVSPrintfCount
  #define ZeroVSPrintfCount(format, pargs, extraSize, resultSize)             \
  do                                                                          \
  {                                                                           \
    va_list sizeArgs;                                                         \
    va_copy(sizeArgs, pargs);                                                 \
    resultSize = vsnprintf(gDiscardBuffer, 1, format, sizeArgs) + extraSize;  \
  }                                                                           \
  while (false)

  #define ZeroSPrintfCount(format, ...) snprintf(gDiscardBuffer, 1, format, __VA_ARGS__)

#endif

  #define ZeroVSPrintf(destination, bufferSizeBytes, format, args) \
    vsnprintf(destination, bufferSizeBytes, format, args)

  #define ZeroSPrintf(destination, bufferSizeBytes, format, ...) \
    snprintf(destination, bufferSizeBytes, format, __VA_ARGS__)

  #define ZeroStrCat(destination, bufferSizeBytes, source) \
    strncat(destination, source, bufferSizeBytes)

  #define ZeroStrCpy(destination, bufferSizeBytes, source) \
    strncpy(destination, source, bufferSizeBytes)

  #define ZeroCStringCopy(dest, destSize, source, sourceSize) \
    strncpy(dest, source, sourceSize);
#endif

// Includes for alloca
// This behavior changes per platform rather than per compiler (Clang on Windows still uses malloc.h)
#if !defined(WIN64) &&  !defined(WIN32) && !defined(_WIN64) &&  !defined(_WIN32)
#include <alloca.h>
#else
#include <malloc.h>
#endif

#if defined(__GXX_EXPERIMENTAL_CXX0X) ||  __cplusplus >= 201103L || _MSC_VER >= 1600
  #define ZeroSupportsDecltypeAuto 1
  #define ZeroSupportsStaticAssert 1

  #if defined(__GNUC__)
    #if __GNUC__ >= 4 && __GNUC_MINOR__ >= 7
      #define ZeroSupportsNullptr 1
    #endif
  #else
    #define ZeroSupportsNullptr 1
  #endif
#endif

#ifdef ZeroSupportsDecltypeAuto
#define TypeOf(type) decltype(type)
#else
#define TypeOf(type) __typeof__(type)
#endif

#define ZeroCompileTimeTestType(T) T = 0;
#define ZeroCompileTimeTestTypeExpression(Expression) TypeOf(Expression) = 0;

// Visual Studio 2010's (and maybe other compilers) decltype has an issue with getting the type
// of the address of a function template instantiation (decltype(Lerp<Real>) for instance).
// This template and macro are a work around for doing this.
template <typename T>
T FunctionPointerPassThrough(T);
#define TypeOfFunctionPointer(type) TypeOf(FunctionPointerPassThrough(type))

#if _MSC_VER
  #define ZeroHasTypeOf 0
  #if _MSC_VER >= 1600
    #define ZeroHasAuto 1
  #else 
    #define ZeroHasAuto 0
#endif
#else
  #define ZeroHasTypeOf 1
  #define ZeroHasAuto 0
#endif

#if ZeroHasAuto
  #define AutoDeclare(varName, expression) \
    auto varName = expression

  #define AutoDeclareReference(varName, expression) \
    auto& varName = expression
#else
  #define AutoDeclare(varName, expression) \
    TypeOf(expression) varName = expression

  #define AutoDeclareReference(varName, expression) \
    TypeOf(expression)& varName = expression
#endif

#ifdef _MSC_VER
#define SupportsMoveSemantics
#define SupportsStaticAsserts
#elif __cplusplus >= 201103L
#define SupportsMoveSemantics
#define SupportsStaticAsserts
#endif

#ifdef _MSC_VER
  #define ZeroThreadLocal __declspec(thread)
#else
  #define ZeroThreadLocal __thread
#endif

#define ZeroNoImportExport

#ifdef _WIN32
  #define ZeroImport __declspec(dllimport)
  #define ZeroExport __declspec(dllexport)
  #define ZeroExportC extern "C" __declspec(dllexport)

  #if defined(ZeroImportDll)
    #define ZeroShared ZeroImport
  #elif defined(ZeroExportDll)
    #define ZeroShared ZeroExport
  #else
    #define ZeroShared
  #endif
#else
  #define ZeroImport
  #define ZeroExport
  #define ZeroExportC extern "C" __attribute__((visibility("default")))
  #define ZeroShared __attribute__((visibility("default")))
#endif

#if defined(ZeroImportDll)
  #define ZeroSharedTemplate
  #pragma warning(disable: 4251)
#elif defined(ZeroExportDll)
  #define ZeroSharedTemplate ZeroShared
#else
  #define ZeroSharedTemplate
#endif

#ifdef ZeroSharedWarnings
  // We actually want shared warnings (leave this here for debugging)
#else
  #pragma warning(disable: 4251)
  #undef ZeroSharedTemplate
  #define ZeroSharedTemplate
#endif

#include "Utility/Typedefs.hpp"
#include "Diagnostic/Console.hpp"
#include "Utility/Misc.hpp"
#include "Utility/Guid.hpp"
#include "NullPtr.hpp"

// Similar to std::declval
template <typename T>
T& DeclVal()
{
  return *(T*)nullptr;
}
