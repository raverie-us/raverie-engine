// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

void RaverieConsolePrint(ConsoleEvent* e);

RaverieDefineStaticLibrary(RaverieScriptLibrary)
{
  builder.CreatableInScriptDefault = false;

  RaverieInitializeType(RaverieComponent);
  RaverieInitializeType(RaverieEvent);
  RaverieInitializeType(RaverieObject);
  RaverieInitializeType(RaverieScript);

  MetaLibraryExtensions::AddNativeExtensions(builder);
}

void RaverieScriptLibrary::Initialize()
{
  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());

  InitializeResourceManager(RaverieScriptManager);

  ResourceLibrary::sScriptType = RaverieTypeId(RaverieScript);

  EventConnect(&ScriptConsole::Events, Events::ConsoleWrite, RaverieConsolePrint);
}

void RaverieScriptLibrary::Shutdown()
{
  GetLibrary()->ClearComponents();
}

void RaverieConsolePrint(ConsoleEvent* e)
{
  // Print out the standard formatted error message to the console
  Console::Print(Filter::DefaultFilter, "%s", e->Text.c_str());
}

} // namespace Raverie
