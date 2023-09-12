// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

void ZilchConsolePrint(ConsoleEvent* e);

ZilchDefineStaticLibrary(ZilchScriptLibrary)
{
  builder.CreatableInScriptDefault = false;

  ZilchInitializeType(ZilchComponent);
  ZilchInitializeType(ZilchEvent);
  ZilchInitializeType(ZilchObject);
  ZilchInitializeType(ZilchScript);

  MetaLibraryExtensions::AddNativeExtensions(builder);
}

void ZilchScriptLibrary::Initialize()
{
  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());

  InitializeResourceManager(ZilchScriptManager);

  ResourceLibrary::sScriptType = ZilchTypeId(ZilchScript);

  EventConnect(&Zilch::Console::Events, Zilch::Events::ConsoleWrite, ZilchConsolePrint);
}

void ZilchScriptLibrary::Shutdown()
{
  GetLibrary()->ClearComponents();
}

void ZilchConsolePrint(ConsoleEvent* e)
{
  // Print out the standard formatted error message to the console
  Console::Print(Filter::DefaultFilter, "%s", e->Text.c_str());
}

} // namespace Zero
