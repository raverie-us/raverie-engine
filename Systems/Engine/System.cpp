///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Trevor Sundberg
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(System, builder, type)
{
  // Systems exist for the lifetime of any possible scripts that could access them,
  // so it is ok to point directly at them instead of using a safe handle.
  type->HandleManager = ZilchManagerId(PointerManager);
}

}//namespace Zero
