// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

using namespace Raverie;

static GameOrEditorStartup* startup = nullptr;

extern "C"
{
  void __wasm_call_ctors();
}

void* RaverieExportNamed(ExportAllocate)(size_t size)
{
  return malloc(size);
}

void RaverieExportNamed(ExportFree)(void* pointer)
{
  free(pointer);
}

void RaverieExportNamed(ExportInitialize)(
    const char* arguments, int32_t clientWidth, int32_t clientHeight, bool focused, byte* projectDataSteal, size_t projectLength, byte* builtContentDataSteal, size_t builtContentLength)
{
  __wasm_call_ctors();
  Shell::sInitialClientSize = IntVec2(clientWidth, clientHeight);
  Shell::sInitialFocused = focused;
  startup = new GameOrEditorStartup();
  startup->mProjectArchive.SetData(projectDataSteal, projectLength, true);
  startup->mBuiltContentArchive.SetData(builtContentDataSteal, builtContentLength, true);
  gCommandLine = arguments;

  // Run until we are ready to update
  for (;;) {
    StartupPhase::Enum nextPhase = startup->RunIteration();
    if (nextPhase == StartupPhase::EngineUpdate) {
      break;
    }
  }
}

void RaverieExportNamed(ExportRunIteration)()
{
  startup->RunIteration();
}

// We don't actually use main since our exeuctable is initialized externally
int main(int argc, char* argv[])
{
  return 0;
}
