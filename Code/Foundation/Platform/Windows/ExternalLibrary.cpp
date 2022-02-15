// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{
const cstr cExecutableExtensionWithoutDot = "exe";
const cstr cSharedLibraryExtensionWithoutDot = "dll";
const cstr cExecutableExtensionWithDot = ".exe";
const cstr cSharedLibraryExtensionWithDot = ".dll";

ExternalLibrary::ExternalLibrary()
{
  mHandle = NULL;
  mUnloadOnDestruction = true;
}

ExternalLibrary::~ExternalLibrary()
{
  if (mUnloadOnDestruction)
    Unload();
}

bool ExternalLibrary::IsValid()
{
  return mHandle != NULL;
}

void ExternalLibrary::Load(Status& status, cstr filePath)
{
  mHandle = (void*)LoadLibraryA(filePath);
  if (mHandle == NULL)
    FillWindowsErrorStatus(status);
}

void ExternalLibrary::Unload()
{
  if (mHandle == NULL)
    return;
  FreeLibrary((HMODULE)mHandle);
  mHandle = NULL;
}

void* ExternalLibrary::GetFunctionByName(cstr name)
{
  ReturnIf(mHandle == NULL, NULL, "Attempting to get a function from an invalid library");

  return (void*)GetProcAddress((HMODULE)mHandle, name);
}

} // namespace Zero
