// MIT Licensed (see LICENSE.md).
#pragma once

#include "Common/CommonStandard.hpp"
#include "PlatformStandard.hpp"

// Prevent including winsock1.
#define _WINSOCKAPI_

// Only include frequently used elements.
#define WIN32_LEAN_AND_MEAN
#define NOCOMM

// Prevent MIN ans MAX macros from being defined.
#ifndef NOMINMAX
#  define NOMINMAX
#endif

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <Wspiapi.h>
#include <Mmsystem.h>
#include <Regstr.h>
#include <WinBase.h>
#include <windowsx.h>
#include <intrin.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <direct.h>
#include <stdlib.h>
#include <stdio.h>
#include <shlobj.h>
#include <io.h>
#include <crtdbg.h>
#include <winhttp.h>
#include <VersionHelpers.h>
#include <Lmcons.h>
#include <Xinput.h>
#include <iptypes.h>
#include <iphlpapi.h>
#include <setupapi.h>
#include <devguid.h>
#include <commctrl.h>
#include <dlgs.h>
#include <commdlg.h>
#include <shobjidl.h>
#include <dbghelp.h>
#include <Psapi.h>
#include <ws2tcpip.h>
#include <mmreg.h>
#include <Audioclient.h>
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <functiondiscoverykeys.h>
#include <process.h>
#include <avrt.h>
#include <hidsdi.h>

// Include glew before OpenGl
#include <GL/glew.h>
#include <GL/wglew.h>

// Include OpenGl
#include <GL/gl.h>

#ifdef min
#  undef min
#endif
#ifdef max
#  undef max
#endif

#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "Winhttp.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "hid.lib")
#pragma comment(lib, "Setupapi.lib")
#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Avrt.lib")
#pragma comment(lib, "hid.lib")

#pragma comment(linker, "\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// Undef windows defines that overlap with core functions
#undef CopyFile
#undef MoveFile
#undef DeleteFile
#undef CreateDirectory

#include "WindowsError.hpp"
#include "StackWalker.hpp"
#include "ThreadIo.hpp"
#include "WString.hpp"
#include "WinUtility.hpp"

#ifdef RunVld
#  include <vld.h>
#endif
