#include "%NAME%Precompiled.hpp"

//***************************************************************************
ZilchDefineStaticLibraryAndPlugin(%NAME%Library, %NAME%Plugin, ZilchDependencyStub(Core) ZilchDependencyStub(ZeroEngine))
{
  ZilchInitializeType(%NAME%);
  ZilchInitializeType(%NAME%Event);
  // Auto Initialize (used by Visual Studio plugins, do not remove this line)
}

//***************************************************************************
void %NAME%Plugin::Initialize(Zilch::BuildEvent* event)
{
  // One time startup logic goes here
  // This runs after our plugin library/reflection is built
  Zilch::Console::WriteLine("%NAME%Plugin::Initialize");
}

//***************************************************************************
void %NAME%Plugin::Uninitialize()
{
  // One time shutdown logic goes here
  Zilch::Console::WriteLine("%NAME%Plugin::Uninitialize");
}
