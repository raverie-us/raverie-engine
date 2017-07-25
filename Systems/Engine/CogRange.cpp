///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//---------------------------------------------------------------------------------//
//                               CogHashSetRange                                   //
//---------------------------------------------------------------------------------//

CogHashSetRange::CogHashSetRange()
  : CogHashSet::range()
{
}
CogHashSetRange::CogHashSetRange(const CogHashSetRange::range& rhs)
  : CogHashSet::range(rhs)
{
}

Cog* CogHashSetRange::Front()
{
  return CogHashSet::range::Front();
}

} // namespace Zero
