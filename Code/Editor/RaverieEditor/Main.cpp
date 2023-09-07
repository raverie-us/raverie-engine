// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

using namespace Zero;

static GameOrEditorStartup* startup = nullptr;

extern "C" {
void __wasm_call_ctors();

void ZeroExportNamed(ExportInitialize)() {
  __wasm_call_ctors();
  startup = new GameOrEditorStartup();
}

void ZeroExportNamed(ExportRunIteration)() {
  startup->RunIteration();
}
}

// We don't actually use main since our exeuctable is initialized externally
int main(int argc, char* argv[])
{
  return 0;
}
