// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{
const cstr cExecutableExtensionWithoutDot = "";
const cstr cSharedLibraryExtensionWithoutDot = "";
const cstr cExecutableExtensionWithDot = "";
const cstr cSharedLibraryExtensionWithDot = "";

ExternalLibrary::ExternalLibrary()
{
}

ExternalLibrary::~ExternalLibrary()
{
}

bool ExternalLibrary::IsValid()
{
  return false;
}

void ExternalLibrary::Load(Status& status, cstr filePath)
{
  status.SetFailed("ExternalLibrary not implemented");
}

void ExternalLibrary::Unload()
{
}

void* ExternalLibrary::GetFunctionByName(cstr name)
{
  return nullptr;
}

} // namespace Zero
