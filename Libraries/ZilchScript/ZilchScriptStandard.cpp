///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2016 DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

void ZilchConsolePrint(ConsoleEvent* e);

//**************************************************************************************************
ZilchDefineStaticLibrary(ZilchScriptLibrary)
{
  builder.CreatableInScriptDefault = false;
  
  ZilchInitializeType(ZilchComponent);
  ZilchInitializeType(ZilchEvent);
  ZilchInitializeType(ZilchObject);
  ZilchInitializeType(ZilchScript);

  ZilchInitializeType(ZilchPluginSource);
  ZilchInitializeType(ZilchPluginLibrary);

  MetaLibraryExtensions::AddNativeExtensions(builder);
}

//**************************************************************************************************
void ZilchScriptLibrary::Initialize()
{
  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());

  InitializeResourceManager(ZilchScriptManager);
  InitializeResourceManager(ZilchPluginLibraryManager);
  InitializeResourceManager(ZilchPluginSourceManager);

  ResourceLibrary::sScriptType = ZilchTypeId(ZilchScript);

  EventConnect(&Zilch::Console::Events, Zilch::Events::ConsoleWrite, ZilchConsolePrint);
}

//**************************************************************************************************
void ZilchScriptLibrary::Shutdown()
{
  GetLibrary()->ClearComponents();
}

//**************************************************************************************************
void ZilchConsolePrint(ConsoleEvent* e)
{
  // Print out the standard formatted error message to the console
  Console::Print(Filter::DefaultFilter, "%s", e->Text.c_str());
}

}//namespace Zero
