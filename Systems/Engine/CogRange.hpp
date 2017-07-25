///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Typedefs
typedef HashSet<CogId> CogHashSet;

//---------------------------------------------------------------------------------//
//                               CogHashSetRange                                   //
//---------------------------------------------------------------------------------//

/// CogId HashSet Range
struct CogHashSetRange : public CogHashSet::range
{
  typedef Cog* value_type;
  typedef Cog* return_type;
  typedef Cog* FrontResult;

  CogHashSetRange();
  CogHashSetRange(const CogHashSetRange::range& rhs);
  Cog* Front();
};

} // namespace Zero
