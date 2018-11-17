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

//**************************************************************************************************
extern void InitializeGamepad();
void PlatformLibrary::Initialize()
{
  // For all console prints (printf, etc) we want to use UTF8 encoding
  // This only really works if the user sets a console font that has unicode characters
  SetConsoleOutputCP(65001);

  InitializeGamepad();
}

//**************************************************************************************************
void PlatformLibrary::Shutdown()
{
}

//**************************************************************************************************
void StackHandle::Close()
{
  if(mHandle != cInvalidHandle)
    CloseHandle(mHandle);

  mHandle = cInvalidHandle;
}

}//namespace Zero
