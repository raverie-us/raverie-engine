///////////////////////////////////////////////////////////////////////////////
///
/// \file BitField.hpp
/// Declaration of the BitField class.
///
/// Authors: Joshua Claeys
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Utility/Typedefs.hpp"

//*NOTE - Includes a library dependency and should be moved.

namespace Zero
{
class Serializer;

template <typename EnumType>
class BitField
{
public:

  BitField() { Clear(); }
  BitField(u32 field) { U32Field = field; }

  bool operator==(const BitField& rhs) { return U32Field == rhs.U32Field; }
  inline void SetFlag(u32 flag) {U32Field |= flag;}
  inline void ClearFlag(u32 flag) {U32Field &= ~flag;}
  inline void ToggleFlag(u32 flag) {U32Field ^= flag;}
  inline bool IsSet(u32 flag) const {return (U32Field & flag) > 0;}
  inline void SetState(u32 flag, bool state) { if(state) SetFlag(flag); else ClearFlag(flag); }
  inline void Clear() {U32Field = 0;}
  inline void SetAll() {U32Field = (u32)(-1);}

  union
  {
    u32 U32Field;
    EnumType Field;
  };
};

}//namespace Zero
