///////////////////////////////////////////////////////////////////////////////
///
/// \file WindowsShellSystemStandard.cpp
/// 
/// Authors: Joshua Claeys
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//**************************************************************************************************
ZilchDefineStaticLibrary(WindowsShellSystemLibrary)
{
  builder.CreatableInScriptDefault = false;
  
  ZilchInitializeType(WindowsOsWindow);

  EngineLibraryExtensions::AddNativeExtensions(builder);
}

//**************************************************************************************************
void WindowsShellSystemLibrary::Initialize()
{
  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());
}

//**************************************************************************************************
void WindowsShellSystemLibrary::Shutdown()
{
  GetLibrary()->ClearComponents();
}

}//namespace Zero
