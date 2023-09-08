// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

using namespace Zero;

static GameOrEditorStartup* startup = nullptr;

extern "C" {
void __wasm_call_ctors();
}

char* ExportInitialize(size_t argumentsLength) {
  __wasm_call_ctors();
  startup = new GameOrEditorStartup();
  gCommandLineBuffer = new char[argumentsLength];
  gCommandLineBufferLength = argumentsLength;
  return gCommandLineBuffer;
}

void ExportRunIteration() {
  startup->RunIteration();
}

// We don't actually use main since our exeuctable is initialized externally
int main(int argc, char* argv[])
{
  return 0;
}
