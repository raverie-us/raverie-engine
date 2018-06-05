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

}//namespace Zero
