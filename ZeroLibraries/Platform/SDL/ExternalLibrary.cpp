////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg, Dane Curbow
/// Copyright 2018, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "SDL_loadso.h"

namespace Zero
{

ExternalLibrary::ExternalLibrary()
{
  mHandle = nullptr;
  mUnloadOnDestruction = true;
}

ExternalLibrary::~ExternalLibrary()
{
  if(mUnloadOnDestruction)
    Unload();
}

bool ExternalLibrary::IsValid()
{
  return mHandle != nullptr;
}

void ExternalLibrary::Load(Status& status, cstr filePath)
{
  mHandle = (void*)SDL_LoadObject(filePath);
  if(mHandle == nullptr)
  {
    String errorString = SDL_GetError();
    String message = String::Format("Failed to load external library: %s, %s", filePath, errorString.c_str());
    status.SetFailed(message);
  }
}

void ExternalLibrary::Unload()
{
  if(mHandle == nullptr)
    return;

  SDL_UnloadObject(mHandle);
  mHandle = nullptr;
}

void* ExternalLibrary::GetFunctionByName(cstr name)
{
  ReturnIf(mHandle == nullptr, nullptr, "Attempting to get a function from an invalid library");

  return SDL_LoadFunction(mHandle, name);
}

// TODO PLATFORM
void* ExternalLibrary::Patch(void* oldFunctionPointer, void* newFunctionPointer)
{
  return nullptr;
}

}//namespace Zero
