///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
//******************************************************************************
void SpinLock::Lock()
{
  while (!mLocked.CompareExchange(true, false));
}

//******************************************************************************
void SpinLock::Unlock()
{
  mLocked = false;
}
}
