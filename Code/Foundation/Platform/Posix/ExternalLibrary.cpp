// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"
#include "Common/ExternalLibrary.hpp"

namespace Zero
{
const cstr cExecutableExtensionWithoutDot = "";
const cstr cSharedLibraryExtensionWithoutDot = "";
const cstr cExecutableExtensionWithDot = "";
const cstr cSharedLibraryExtensionWithDot = "";

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

void ExternalLibrary::Load(cstr filePath)
{
  mHandle = dlopen(filePath, RTLD_LAZY);
}

void ExternalLibrary::Unload()
{
  if (mHandle == nullptr)
    return;
  dlclose(mHandle);
  mHandle = nullptr;
}

void* ExternalLibrary::GetFunctionByName(cstr name)
{
  ReturnIf(mHandle == nullptr, nullptr, "Attempting to get a function from an invalid library");
  return dlsym(mHandle, name);
}

} // namespace Zero
