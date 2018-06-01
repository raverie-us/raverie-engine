///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

bool GetExecutableResource(const char* name, const char* type, ByteBufferBlock& output)
{
  return false;
}

ExecutableResourceUpdater::ExecutableResourceUpdater(Status& status, const char* fileName)
{
  status.SetFailed("Not implemented");
}

ExecutableResourceUpdater::~ExecutableResourceUpdater()
{
}

void ExecutableResourceUpdater::Update(const char* name, const char* type, const byte* data, size_t size)
{
}

void ExecutableResourceUpdater::UpdateIcon(const byte* buffer, size_t size)
{
}

}//namespace Zero
