// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

using namespace Zero;

static GameOrEditorStartup* startup = nullptr;

extern "C" {
void __wasm_call_ctors();
}

void* ZeroExportNamed(ExportAllocate)(size_t size) {
  return malloc(size);
}

void ZeroExportNamed(ExportFree)(void* pointer) {
  free(pointer);
}

void ZeroExportNamed(ExportInitialize)(const char* arguments, int32_t clientWidth, int32_t clientHeight) {
  __wasm_call_ctors();
  Shell::sInitialClientSize = IntVec2(clientWidth, clientHeight);
  startup = new GameOrEditorStartup();
  gCommandLine = arguments;
}

void ZeroExportNamed(ExportRunIteration)() {
  startup->RunIteration();
}

// We don't actually use main since our exeuctable is initialized externally
int main(int argc, char* argv[])
{
  return 0;
}
