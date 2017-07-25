///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineEnum(LauncherStartupArguments);

//**************************************************************************************************
ZilchDefineStaticLibrary(LauncherDllLibrary)
{
  builder.CreatableInScriptDefault = false;
  
  // Enums
  ZilchInitializeEnum(LauncherStartupArguments);

  // Events
  ZilchInitializeType(LauncherBuildEvent);

  // Components
  ZilchInitializeType(ZeroBuildContent);
  ZilchInitializeType(ZeroBuildReleaseNotes);
  ZilchInitializeType(ZeroBuildDeprecated);
  ZilchInitializeType(ZeroTemplate);
  ZilchInitializeType(LauncherProjectInfo);
}

void LauncherDllLibrary::Initialize()
{
  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());
}

void LauncherDllLibrary::Shutdown()
{

}

}//namespace Zero
