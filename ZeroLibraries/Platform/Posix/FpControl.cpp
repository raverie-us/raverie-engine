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
#include "Utility/Typedefs.hpp"
#include "Platform/FpControl.hpp"
#include <float.h>


namespace Zero
{

///the mask is 0 in the generic system since we aren't running any fp exceptions.
uint FpuControlSystem::DefaultMask = 0;
///The system is not active because we're on some random platform
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
