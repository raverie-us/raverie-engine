// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"
#include "SDL_loadso.h"

namespace Zero
{

#if defined(WelderTargetOsEmscripten)
const cstr cExecutableExtensionWithoutDot = ".wasm";
const cstr cSharedLibraryExtensionWithoutDot = "wasm";
const cstr cExecutableExtensionWithDot = ".wasm";
const cstr cSharedLibraryExtensionWithDot = "wasm";
#else
const cstr cExecutableExtensionWithoutDot = "";
const cstr cSharedLibraryExtensionWithoutDot = "";
const cstr cExecutableExtensionWithDot = "";
const cstr cSharedLibraryExtensionWithDot = "";
#endif

ExternalLibrary::ExternalLibrary()
{
  mHandle = nullptr;
  mUnloadOnDestruction = true;
}

ExternalLibrary::~ExternalLibrary()
{
  if (mUnloadOnDestruction)
    Unload();
}

bool ExternalLibrary::IsValid()
{
  return mHandle != nullptr;
}

void ExternalLibrary::Load(Status& status, cstr filePath)
{
  mHandle = (void*)SDL_LoadObject(filePath);
  if (mHandle == nullptr)
    status.SetFailed(SDL_GetError());
}

void ExternalLibrary::Unload()
{
  if (mHandle == nullptr)
    return;

  SDL_UnloadObject(mHandle);
  mHandle = nullptr;
}

void* ExternalLibrary::GetFunctionByName(cstr name)
{
  ReturnIf(mHandle == nullptr, nullptr, "Attempting to get a function from an invalid library");

  return SDL_LoadFunction(mHandle, name);
}

} // namespace Zero
