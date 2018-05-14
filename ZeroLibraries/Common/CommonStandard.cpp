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
void InitializeGamepad();
void InitializeKeyboard();

//**************************************************************************************************
void CommonLibrary::Initialize()
{
  //Start the memory system used for all systems and containers.
  Memory::Root::Initialize();

  // Initialize platform socket library
  Zero::Status socketLibraryInitStatus;
  Zero::Socket::InitializeSocketLibrary(socketLibraryInitStatus);
  Assert(Zero::Socket::IsSocketLibraryInitialized());

  // Setup input devices
  InitializeGamepad();
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
