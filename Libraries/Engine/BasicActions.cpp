///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(ActionDelay, builder, type)
{
  ZeroBindDocumented();

  ZilchBindField(mTimeLeft);
}

} // namespace Zero
