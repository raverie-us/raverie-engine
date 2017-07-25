///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

// Includes
#include "BitMath.hpp"

namespace Zero
{

/// Unsigned integer of N bits
template <Bits N, bool WrapAware = false>
class UintN
{
public:
  /// Typedefs
  typedef NEAREST_UINT(N) value_type;

  /// Constants
  static const Bits       bits    = N;
  static const Bytes      bytes   = BITS_TO_BYTES(bits);
  static const value_type min     = 0U;
  static const value_type max     = POW2(bits) - 1;
  static const value_type halfmax = max / 2;

  /// Default Constructor
  UintN(void)                     : mValue(min)         {}
  /// Copy Constructor
  UintN(const UintN& rhs)         : mValue(rhs.mValue)  {}
  /// Move Constructor
  UintN(MoveReference<UintN> rhs) : mValue(rhs->mValue) {}
  /// Conversion Constructor
  UintN(value_type rhs)           : mValue(Clamp(rhs))  {}

  /// Copy Assignment Operator
  UintN& operator=(const UintN& rhs)         { mValue = rhs.mValue; return *this;  }
  /// Move Assignment Operator
  UintN& operator=(MoveReference<UintN> rhs) { mValue = rhs->mValue; return *this; }
  /// Assignment Operator
  UintN& operator=(value_type rhs)           { mValue = Clamp(rhs); return *this;  }

  /// Arithmetic Operators
  UintN operator+(const UintN& rhs) const { return mValue + rhs.mValue;                }
  UintN operator-(const UintN& rhs) const { return mValue - rhs.mValue;                }
  UintN operator*(const UintN& rhs) const { return mValue * rhs.mValue;                }
  UintN operator/(const UintN& rhs) const { return mValue / rhs.mValue;                }
  UintN operator%(const UintN& rhs) const { return mValue % rhs.mValue;                }
  UintN operator++(void)           { mValue = Clamp(++mValue); return *this;    }
  UintN operator++(int)            { UintN temp(*this); ++(*this); return temp; }
  UintN operator--(void)           { mValue = Clamp(--mValue); return *this;    }
  UintN operator--(int)            { UintN temp(*this); --(*this); return temp; }

  /// Bitwise Operators
  UintN operator~(void) const       { return ~mValue;              }
  UintN operator&(const UintN& rhs) const  { return mValue & rhs.mValue;  }
  UintN operator|(const UintN& rhs) const  { return mValue | rhs.mValue;  }
  UintN operator^(const UintN& rhs) const  { return mValue ^ rhs.mValue;  }
  UintN operator<<(const UintN& rhs) const { return mValue << rhs.mValue; }
  UintN operator>>(const UintN& rhs) const { return mValue >> rhs.mValue; }

  /// Compound Assignment Operators
  UintN& operator+=(const UintN& rhs)  { mValue = Clamp(mValue + rhs.mValue); return *this;  }
  UintN& operator-=(const UintN& rhs)  { mValue = Clamp(mValue - rhs.mValue); return *this;  }
  UintN& operator*=(const UintN& rhs)  { mValue = Clamp(mValue * rhs.mValue); return *this;  }
  UintN& operator/=(const UintN& rhs)  { mValue = Clamp(mValue / rhs.mValue); return *this;  }
  UintN& operator%=(const UintN& rhs)  { mValue = Clamp(mValue % rhs.mValue); return *this;  }
  UintN& operator&=(const UintN& rhs)  { mValue = Clamp(mValue & rhs.mValue); return *this;  }
  UintN& operator|=(const UintN& rhs)  { mValue = Clamp(mValue | rhs.mValue); return *this;  }
  UintN& operator^=(const UintN& rhs)  { mValue = Clamp(mValue ^ rhs.mValue); return *this;  }
  UintN& operator<<=(const UintN& rhs) { mValue = Clamp(mValue << rhs.mValue); return *this; }
  UintN& operator>>=(const UintN& rhs) { mValue = Clamp(mValue >> rhs.mValue); return *this; }

  /// Comparison Operators
  bool operator==(const UintN& rhs) const { return mValue == rhs.mValue;          }
  bool operator!=(const UintN& rhs) const { return mValue != rhs.mValue;          }
  bool operator>(const UintN& rhs) const  { return IsGreaterThan<WrapAware>(rhs); }
  bool operator<(const UintN& rhs) const  { return IsLessThan<WrapAware>(rhs);    }
  bool operator>=(const UintN& rhs) const { return *this > rhs || *this == rhs;   }
  bool operator<=(const UintN& rhs) const { return *this < rhs || *this == rhs;   }

  /// Logical Operators
  bool operator!(void) const            { return !mValue;              }
  bool operator&&(value_type rhs) const { return mValue && rhs.mValue; }
  bool operator||(value_type rhs) const { return mValue || rhs.mValue; }

  /// Returns the underlying value
  value_type& value(void)       { return mValue; }
  value_type  value(void) const { return mValue; }

private:
  /// Clamps a value to fit within the representable bit range
  static inline value_type Clamp(value_type mValue)
  {
    return (mValue <= max) ? mValue : ((mValue - max) - 1);
  }

  /// Wrap-aware greater-than comparison operator
  /// Returns true if rhs is greater, else false
  template<bool UseWrapAware>
  inline bool IsGreaterThan(const UintN& rhs) const
  {
    if(UseWrapAware)
      return (mValue == rhs.mValue) ? false : ((rhs - *this).mValue > halfmax);
    else
      return mValue > rhs.mValue;
  }

  /// Wrap-aware less-than comparison operator
  /// Returns true if rhs is lesser, else false
  template<bool UseWrapAware>
  inline bool IsLessThan(const UintN& rhs) const
  {
    if(UseWrapAware)
      return (mValue == rhs.mValue) ? false : ((rhs - *this).mValue < halfmax);
    else
      return mValue < rhs.mValue;
  }

  /// Underlying value
  value_type mValue;
};

} // namespace Zero
