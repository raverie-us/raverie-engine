///////////////////////////////////////////////////////////////////////////////
///
/// \file OsShared.cpp
/// 
/// 
/// Authors: Chris Peters
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
void InitializeGamepad();

//**************************************************************************************************
void PlatformLibrary::Initialize()
{
  // Initialize platform socket library
  Zero::Status socketLibraryInitStatus;
  Zero::Socket::InitializeSocketLibrary(socketLibraryInitStatus);
  Assert(Zero::Socket::IsSocketLibraryInitialized());
  InitializeGamepad();

  // For all console prints (printf, etc) we want to use UTF8 encoding
  // This only really works if the user sets a console font that has unicode characters
  SetConsoleOutputCP(65001);
}

//**************************************************************************************************
void PlatformLibrary::Shutdown()
{
  // Uninitialize platform socket library
  Zero::Status socketLibraryUninitStatus;
  Zero::Socket::UninitializeSocketLibrary(socketLibraryUninitStatus);
  //Assert(!Zero::Socket::IsSocketLibraryInitialized());
}

//**************************************************************************************************
void StackHandle::Close()
{
  if(mHandle != cInvalidHandle)
    CloseHandle(mHandle);

  mHandle = cInvalidHandle;
}

}//namespace Zero
