///////////////////////////////////////////////////////////////////////////////
///
/// \file FpControl.hpp
/// Implementation of the ScpeFpuExceptions, ScoplessFpuExceptions and
/// FpuControlSystem classes.
///
/// Authors: Joshua Davis
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "Platform/FpControl.hpp"

namespace Zero
{
uint FpuControlSystem::DefaultMask = 0;
bool FpuControlSystem::Active = false;

ScopeFpuExceptionsEnabler::ScopeFpuExceptionsEnabler()
{
  Error("Not implemented");
}

ScopeFpuExceptionsEnabler::~ScopeFpuExceptionsEnabler()
{
}

ScopeFpuExceptionsDisabler::ScopeFpuExceptionsDisabler()
{
  Error("Not implemented");
}

ScopeFpuExceptionsDisabler::~ScopeFpuExceptionsDisabler()
{
}

}//namespace Zero
