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

namespace Zero
{

// If this is the Microsoft compiler
#ifdef _MSC_VER

///by default, we want all of the fpu exceptions apart from inexact
///(inexact happens in lots of odd places...) and underflow
uint FpuControlSystem::DefaultMask = _EM_INEXACT | _EM_UNDERFLOW;
///Stores that by default floating point exceptions are enabled
bool FpuControlSystem::Active = true;

ScopeFpuExceptionsEnabler::ScopeFpuExceptionsEnabler()
{
  ///only scope change if the fpu control system is active
  if(FpuControlSystem::Active == false)
    return;

  //*NOTE: _clearfp must be called between _controlfp_s calls. If two
  // _controlfp_s calls are made close to each other then an exception
  //will be thrown. Seemingly, _clearfp clears a busy bit on the cpu
  //which will prevent the cpu from thinking the state is being changed
  //while it is busy.

  unsigned int currState;
  //get the old state so we know what to go back to
  _clearfp();
  _controlfp_s(&mOldState, _MCW_EM, _MCW_EM);
  //set the new state
  _clearfp();
  _controlfp_s(&currState,FpuControlSystem::DefaultMask, _MCW_EM);
}

ScopeFpuExceptionsEnabler::~ScopeFpuExceptionsEnabler()
{
  ///only scope change if the fpu control system is active
  if(FpuControlSystem::Active == false)
    return;

  //set the old state back
  unsigned int currState;
  _clearfp();
  _controlfp_s(&currState,mOldState, _MCW_EM);
}

ScopeFpuExceptionsDisabler::ScopeFpuExceptionsDisabler()
{
  ///only scope change if the fpu control system is active
  if(FpuControlSystem::Active == false)
    return;

  //get the old state
  _controlfp_s(&mOldState, _MCW_EM, _MCW_EM);
  //set all of the exception flags which disables all fp exceptions.
  _controlfp_s(0, _MCW_EM, _MCW_EM);
}

ScopeFpuExceptionsDisabler::~ScopeFpuExceptionsDisabler()
{
  ///only scope change if the fpu control system is active
  if(FpuControlSystem::Active == false)
    return;

  //clear any pending fp exceptions otherwise there may be a
  //'deferred crash' as soon as the exceptions are enabled.
  _clearfp();

  //now reset the exceptions to what they were
  _controlfp_s(0, mOldState, _MCW_EM);
}

// Any other compiler
#else

///the mask is 0 in the generic system since we aren't funning any fp exceptions.
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

#endif

}//namespace Zero
