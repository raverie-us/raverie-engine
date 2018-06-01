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
}

ScopeFpuExceptionsEnabler::~ScopeFpuExceptionsEnabler()
{
}

ScopeFpuExceptionsDisabler::ScopeFpuExceptionsDisabler()
{
}

ScopeFpuExceptionsDisabler::~ScopeFpuExceptionsDisabler()
{
}

}//namespace Zero
