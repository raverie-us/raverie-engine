///////////////////////////////////////////////////////////////////////////////
///
/// \file ExternalLibrary.cpp
/// Implementation of the ExternalLibrary class.
/// 
/// Authors: Trevor Sundberg
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "Platform/ExternalLibrary.hpp"

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

void ExternalLibrary::Load(cstr filePath)
{
  Error("Not implemented");
}

void ExternalLibrary::Unload()
{
  if(mHandle == nullptr)
    return;
  Error("Not implemented");
  mHandle = nullptr;
}

void* ExternalLibrary::GetFunctionByName(cstr name)
{
  ReturnIf(mHandle == nullptr, nullptr, "Attempting to get a function from an invalid library");
  Error("Not implemented");
  return nullptr;
}

}//namespace Zero
