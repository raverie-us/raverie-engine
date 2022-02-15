// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

/// by default, we want all of the fpu exceptions apart from inexact
///(inexact happens in lots of odd places...) and underflow
uint FpuControlSystem::DefaultMask = _EM_INEXACT | _EM_UNDERFLOW;
/// Stores that by default floating point exceptions are enabled
bool FpuControlSystem::Active = true;

struct FpuPrivateData
{
  unsigned int mOldState;
};

ScopeFpuExceptionsEnabler::ScopeFpuExceptionsEnabler()
{
  ZeroConstructPrivateData(FpuPrivateData);
  /// only scope change if the fpu control system is active
  if (FpuControlSystem::Active == false)
    return;

  //*NOTE: _clearfp must be called between _controlfp_s calls. If two
  // _controlfp_s calls are made close to each other then an exception
  // will be thrown. Seemingly, _clearfp clears a busy bit on the cpu
  // which will prevent the cpu from thinking the state is being changed
  // while it is busy.

  unsigned int currState;
  // get the old state so we know what to go back to
  _clearfp();
  _controlfp_s(&(self->mOldState), _MCW_EM, _MCW_EM);
  // set the new state
  _clearfp();
  _controlfp_s(&currState, FpuControlSystem::DefaultMask, _MCW_EM);
}

ScopeFpuExceptionsEnabler::~ScopeFpuExceptionsEnabler()
{
  ZeroGetPrivateData(FpuPrivateData);
  /// only scope change if the fpu control system is active
  if (FpuControlSystem::Active == false)
    return;

  // set the old state back
  unsigned int currState;
  _clearfp();
  _controlfp_s(&currState, self->mOldState, _MCW_EM);

  ZeroDestructPrivateData(FpuPrivateData);
}

ScopeFpuExceptionsDisabler::ScopeFpuExceptionsDisabler()
{
  ZeroConstructPrivateData(FpuPrivateData);
  /// only scope change if the fpu control system is active
  if (FpuControlSystem::Active == false)
    return;

  // get the old state
  _controlfp_s(&(self->mOldState), _MCW_EM, _MCW_EM);
  // set all of the exception flags which disables all fp exceptions.
  _controlfp_s(0, _MCW_EM, _MCW_EM);
}

ScopeFpuExceptionsDisabler::~ScopeFpuExceptionsDisabler()
{
  ZeroGetPrivateData(FpuPrivateData);
  /// only scope change if the fpu control system is active
  if (FpuControlSystem::Active == false)
    return;

  // clear any pending fp exceptions otherwise there may be a
  //'deferred crash' as soon as the exceptions are enabled.
  _clearfp();

  // now reset the exceptions to what they were
  _controlfp_s(0, self->mOldState, _MCW_EM);

  ZeroDestructPrivateData(FpuPrivateData);
}

} // namespace Zero
