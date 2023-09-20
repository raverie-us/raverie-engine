// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{
void InitializeKeyboard();

void CommonLibrary::Initialize()
{
  Thread::MainThreadId = Thread::GetCurrentThreadId();

  // Start the memory system used for all systems and containers.
  Memory::Root::Initialize();

  WebRequest::Initialize();

  // Initialize platform socket library
  Raverie::Status socketLibraryInitStatus;
  Raverie::Socket::InitializeSocketLibrary(socketLibraryInitStatus);

  // Setup keyboard enumerations
  InitializeKeyboard();
}

void CommonLibrary::Shutdown()
{
  WebRequest::Shutdown();

  // Uninitialize platform socket library
  Raverie::Status socketLibraryUninitStatus;
  Raverie::Socket::UninitializeSocketLibrary(socketLibraryUninitStatus);

  Memory::Shutdown();
}

} // namespace Raverie
