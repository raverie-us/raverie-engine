// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

/// Typedefs
typedef HashSet<CogId> CogHashSet;

//                               CogHashSetRange //

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

} // namespace Raverie
