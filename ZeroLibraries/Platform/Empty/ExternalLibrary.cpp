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

namespace Zero
{

ExternalLibrary::ExternalLibrary()
{
  Error("Not implemented");
}

ExternalLibrary::~ExternalLibrary()
{
  Error("Not implemented");
}

bool ExternalLibrary::IsValid()
{
  Error("Not implemented");
  return false;
}

void ExternalLibrary::Load(Status& status, cstr filePath)
{
  Error("Not implemented");
}

void ExternalLibrary::Unload()
{
  Error("Not implemented");
}

void* ExternalLibrary::GetFunctionByName(cstr name)
{
  Error("Not implemented");
  return nullptr;
}

void* ExternalLibrary::Patch(void* oldFunctionPointer, void* newFunctionPointer)
{
  Error("Not implemented");
  return nullptr;
}

}//namespace Zero
