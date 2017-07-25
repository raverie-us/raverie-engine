///////////////////////////////////////////////////////////////////////////////
///
/// \file Windows.hpp
/// Includes Windows Header
/// 
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifdef ZERO
  #pragma message("Including Windows...")
  #ifdef _WINDOWS_
  #error Windows has already been included!
  #else
  #define STRICT
  #endif
#endif

//Prevent including winsock1.
#define _WINSOCKAPI_

//Only include frequently used elements.
#define WIN32_LEAN_AND_MEAN
#define NOCOMM

//Minimum version is Windows XP.
//#define WINVER 0x0501
//#define _WIN32_WINNT 0x0501

//Prevent MIN ans MAX macros from being defined.
#ifndef NOMINMAX
#define NOMINMAX
#endif

// Get rid of an annoying gcc warning that is incorrect
// (complains about winsock being included after windows, it's not)
#ifndef _MSC_VER
#ifdef _WINSOCKAPI_
#undef _WINSOCKAPI_
#endif
#endif
#include <winsock2.h> // Includes windows.h (we get a warning in gcc otherwise)
#include <Ws2tcpip.h>
#include <Wspiapi.h>
#include <Mmsystem.h>
#include <Regstr.h>
#include <WinBase.h>

//Undef windows defines that overlap with core functions
#undef CopyFile
#undef MoveFile
#undef DeleteFile
#undef CreateDirectory
