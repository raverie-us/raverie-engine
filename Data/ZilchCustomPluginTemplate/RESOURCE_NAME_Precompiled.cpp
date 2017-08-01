#include "RESOURCE_NAME_Precompiled.hpp"

//***************************************************************************
ZilchDefineStaticLibraryAndPlugin(RESOURCE_NAME_Library, RESOURCE_NAME_Plugin, ZilchDependencyStub(Core) ZilchDependencyStub(ZeroEngine))
{
  ZilchInitializeType(RESOURCE_NAME_);
  ZilchInitializeType(RESOURCE_NAME_Event);
  // Auto Initialize (used by Visual Studio plugins, do not remove this line)
}

//***************************************************************************
void RESOURCE_NAME_Plugin::Initialize()
{
  // One time startup logic goes here
  // This runs after our plugin library/reflection is built
  Zilch::Console::WriteLine("RESOURCE_NAME_Plugin::Initialize");
}

//***************************************************************************
void RESOURCE_NAME_Plugin::Uninitialize()
{
  // One time shutdown logic goes here
  Zilch::Console::WriteLine("RESOURCE_NAME_Plugin::Uninitialize");
}
