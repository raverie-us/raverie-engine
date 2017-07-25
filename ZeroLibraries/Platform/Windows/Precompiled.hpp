///////////////////////////////////////////////////////////////////////////////
///
/// \file Precompiled.hpp
/// Precompiled header for windows library.
/// 
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Common/CommonStandard.hpp"
#include "Platform/PlatformStandard.hpp"

#ifdef _MSC_VER
#ifdef UNICODE
#define ZeroCStringCopyW(dest, destSize, source, sourceSize) wcsncpy_s(dest, (destSize), source, sourceSize);
#endif
#define ZeroCStringCopy(dest, destSize, source, sourceSize) strncpy_s(dest, (destSize), source, sourceSize);
#else 
#define ZeroCStringCopy(dest, destSize, source, sourceSize) strncpy(dest, source, sourceSize);
#endif

//Include the windows header.
#include "Windows.hpp"
#include "WindowsError.hpp"
#include <shellapi.h>
#include <shlwapi.h>
#include <direct.h>
#include <stdlib.h>
#include <stdio.h>
#include <shlobj.h>
#include <io.h>

#include "StackWalker.hpp"
#include "ThreadIo.hpp"

#include "WString.hpp"
