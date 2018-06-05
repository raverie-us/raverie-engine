///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
//**************************************************************************************************
void InitializeKeyboard();

//**************************************************************************************************
void CommonLibrary::Initialize()
{
  Thread::MainThreadId = Thread::GetCurrentThreadId();

  //Start the memory system used for all systems and containers.
  Memory::Root::Initialize();

  // Initialize platform socket library
  Zero::Status socketLibraryInitStatus;
  Zero::Socket::InitializeSocketLibrary(socketLibraryInitStatus);

  // Setup keyboard enumerations
  InitializeKeyboard();

}

//**************************************************************************************************
void CommonLibrary::Shutdown()
{
  // Uninitialize platform socket library
  Zero::Status socketLibraryUninitStatus;
  Zero::Socket::UninitializeSocketLibrary(socketLibraryUninitStatus);

  Memory::Shutdown();
}

}//namespace Zero
