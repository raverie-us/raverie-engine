///////////////////////////////////////////////////////////////////////////////
///
/// \file Precompiled.hpp
/// Precompiled header for windows system library.
/// 
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "WindowsShellSystemStandard.hpp"

#include "Platform/Windows/Precompiled.hpp"

#include <setupapi.h>
#include <devguid.h>
#include <commctrl.h>
#include <dlgs.h>
#include <commdlg.h>
#include <Shellapi.h>
#include <shobjidl.h>

extern "C"
{
#include "WinHid/include/hidsdi.h"
}

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma comment(lib, "hid.lib")
#pragma comment(lib, "Setupapi.lib")

#include "WinUtility.hpp"
#include "ComPort.hpp"
#include "WindowsSystem.hpp"
#include "ResourceDef.hpp"
#include "Launcher.hpp"
