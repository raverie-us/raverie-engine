///////////////////////////////////////////////////////////////////////////////
///
/// \file OsShared.hpp
/// 
/// 
/// Authors: Chris Peters
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Common/CommonStandard.hpp"

namespace Zero
{

// Engine library
class ZeroNoImportExport PlatformLibrary
{
public:
  static void Initialize();
  static void Shutdown();
};

}//namespace Zero

#include "PlatformSelector.hpp"
#include "PrivateImplementation.hpp"
#include "Utilities.hpp"
#include "OsHandle.hpp"
#include "Thread.hpp"
#include "ThreadSync.hpp"
#include "CrashHandler.hpp"
#include "Debug.hpp"
#include "DebugSymbolInformation.hpp"
#include "ExternalLibrary.hpp"
#include "File.hpp"
#include "FileEvents.hpp"
#include "FilePath.hpp"
#include "FileSystem.hpp"
#include "FpControl.hpp"
#include "IpAddress.hpp"
#include "Lock.hpp"
#include "PlatformSelector.hpp"
#include "Process.hpp"
#include "Registry.hpp"
#include "Resolution.hpp"
#include "Socket.hpp"
#include "SocketConstants.hpp"
#include "SocketEnums.hpp"
#include "Timer.hpp"
#include "TimerBlock.hpp"
#include "DirectoryWatcher.hpp"
#include "ConsoleListeners.hpp"
