///////////////////////////////////////////////////////////////////////////////
///
/// \file PlatformSelector.hpp
/// 
/// Authors: Trevor sundberg
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

// Detect 32 or 64 bit
// If this is a windows platform...
#if defined(_WIN32) || defined(_WIN64)
  #if defined(_WIN64) || defined(_M_X64) || defined(_M_IA64)
    #define PLATFORM_64 1
  #else
    #define PLATFORM_32 1
  #endif
#else
  // If this is a gcc or clang platforms...
  #if defined(__x86_64__) || defined(__ppc64__)
    #define PLATFORM_64 1
  #else
    #define PLATFORM_32 1
  #endif
#endif

// Detect the Windows platform
#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
  #define PLATFORM_WINDOWS 1
  #define PLATFORM_HARDWARE 1
// Detect all Apple platforms
#elif defined(__APPLE__)
  // This header Contains defines that tell us the current platform
  #include <TargetConditionals.h>
  #if TARGET_IPHONE_SIMULATOR
    #define PLATFORM_IPHONE 1
    #define PLATFORM_VIRTUAL 1
  #elif TARGET_OS_IPHONE
    #define PLATFORM_IPHONE 1
    #define PLATFORM_HARDWARE 1
  #elif TARGET_OS_MAC
    #define PLATFORM_MAC 1
    #define PLATFORM_POSIX 1
    #define PLATFORM_HARDWARE 1
  #else
    #error "Unsupported platform"
  #endif

#elif defined(EMSCRIPTEN)
  #define PLATFORM_EMSCRIPTEN 1
  #define PLATFORM_HARDWARE 1

// Linux, which is a POSIX platform
#elif defined(__linux) || defined(__linux__)
  #define PLATFORM_LINUX 1
  #define PLATFORM_POSIX 1
  #define PLATFORM_HARDWARE 1

// Unix, which is a POSIX platform
#elif defined(__unix) || defined(__unix__)
  #define PLATFORM_UNIX 1
  #define PLATFORM_POSIX 1
  #define PLATFORM_HARDWARE 1

// Catch all for other POSIX compatible platforms
#elif defined(__posix)
  #define PLATFORM_POSIX 1
  #define PLATFORM_HARDWARE 1
#endif

// Detect compilers
#if defined(_MSC_VER)
  #define COMPILER_MICROSOFT 1
#elif defined(__clang__)
  #define COMPILER_CLANG 1
#elif defined(__GNUC__)
  #define COMPILER_GCC 1
#elif defined(__llvm__)
  #define COMPILER_LLVM 1
#endif
