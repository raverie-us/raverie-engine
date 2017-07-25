///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

// Includes
#include "Diagnostic/Diagnostic.hpp"
#include "Containers/TypeTraits.hpp"
#include "BitTypes.hpp"
#include "Log2.hpp"

namespace Zero
{

/// Verify platform byte length
StaticAssert(PlatformByteLength, CHAR_BIT == 8, "Platform byte length must be 8 bits");

/// Converts Bits to Bytes
#define BITS_TO_BYTES(b) (((b) + 7) >> 3)
/// Converts Bytes to Bits
#define BYTES_TO_BITS(B) ((B) * 8)

/// Unsigned integer bit size
#define UINT_BITS    (BYTES_TO_BITS(sizeof(uint)))
/// Maximum unsigned integer bit size
#define UINTMAX_BITS (BYTES_TO_BITS(sizeof(uintmax)))

/// Right-justified bit N
#define RBIT(N) (uint8(1) << (N))
/// Left-justified bit N
#define LBIT(N) (uint8(128) >> (N))

/// Sets the right-justified bit N of Byte if Condition is true, else clears it
#define ASSIGN_RBIT(Condition, Byte, N) ((*(uint8*)Byte) ^= ((-(bool)(Condition) ^ ((uint8)*Byte)) & RBIT(N)))
/// Sets the left-justified bit N of Byte if Condition is true, else clears it
#define ASSIGN_LBIT(Condition, Byte, N) ((*(uint8*)Byte) ^= ((-(bool)(Condition) ^ ((uint8)*Byte)) & LBIT(N)))

/// Equivalent to (X / 8)
#define DIV8(X) ((X) >> 3)
/// Equivalent to (X % 8)
#define MOD8(X) ((X) & 7)
/// Rounds X up to the next multiple of 8
#define ROUND_UP_8(X) ( MOD8(X) == 0 ? (X) : (X) + (8 - MOD8(X)) )

/// Equivalent to (2 ^ X)
#define POW2(X) (uintmax(1) << (X))

#define ROUND_UP_POW2_32(X) ((X) | (X) >> 32)
#define ROUND_UP_POW2_16(X) ((X) | (X) >> 16)
#define ROUND_UP_POW2_8(X)  ((X) | (X) >> 8)
#define ROUND_UP_POW2_4(X)  ((X) | (X) >> 4)
#define ROUND_UP_POW2_2(X)  ((X) | (X) >> 2)
#define ROUND_UP_POW2_1(X)  ((X) | (X) >> 1)

/// Rounds X up to the next highest power of 2
#define ROUND_UP_POW2(X) (ROUND_UP_POW2_32(ROUND_UP_POW2_16(ROUND_UP_POW2_8(ROUND_UP_POW2_4(ROUND_UP_POW2_2(ROUND_UP_POW2_1((X) - 1)))))) + 1)

/// Returns the number of bits needed to represent X
#define BITS_NEEDED_TO_REPRESENT(X) (LOG2(X) + 1)

/// Rounds X up to the next highest integer
#define ROUND_UP(X) (intmax(X) + (1 - intmax(intmax((X) + 1) - (X))))

/// Returns the exact uint type of N bits, else void
#define EXACT_UINT(N) typename conditional<(N) == 8,  uint8,                                   \
                      typename conditional<(N) == 16, uint16,                                  \
                      typename conditional<(N) == 32, uint32,                                  \
                      typename conditional<(N) == 64, uint64, void>::type>::type>::type>::type

/// Returns the nearest uint type of N bits, else uintmax
#define NEAREST_UINT(N) typename conditional<(N) <= 8,  uint8,                                      \
                        typename conditional<(N) <= 16, uint16,                                     \
                        typename conditional<(N) <= 32, uint32,                                     \
                        typename conditional<(N) <= 64, uint64, uintmax>::type>::type>::type>::type

/// Returns the fastest uint type of at least N bits, else uintmax
#define FASTEST_UINT(N) typename conditional<(N) <= 8,  uintfast8,                                      \
                        typename conditional<(N) <= 16, uintfast16,                                     \
                        typename conditional<(N) <= 32, uintfast32,                                     \
                        typename conditional<(N) <= 64, uintfast64, uintmax>::type>::type>::type>::type

/// Returns the exact int type of N bits, else void
#define EXACT_INT(N) typename conditional<(N) == 8,  int8,                                   \
                     typename conditional<(N) == 16, int16,                                  \
                     typename conditional<(N) == 32, int32,                                  \
                     typename conditional<(N) == 64, int64, void>::type>::type>::type>::type

/// Returns the nearest int type of N bits, else intmax
#define NEAREST_INT(N) typename conditional<(N) <= 8,  int8,                                     \
                       typename conditional<(N) <= 16, int16,                                    \
                       typename conditional<(N) <= 32, int32,                                    \
                       typename conditional<(N) <= 64, int64, intmax>::type>::type>::type>::type

/// Returns the fastest int type of at least N bits, else intmax
#define FASTEST_INT(N) typename conditional<(N) <= 8,  intfast8,                                     \
                       typename conditional<(N) <= 16, intfast16,                                    \
                       typename conditional<(N) <= 32, intfast32,                                    \
                       typename conditional<(N) <= 64, intfast64, intmax>::type>::type>::type>::type

/// Smallest unsigned integer capable of storing a pointer
typedef typename NEAREST_UINT(BYTES_TO_BITS(sizeof(void*))) uintptr;

/// Returns the minimum of a and b
#define MIN(a, b) ( (a) < (b) ? (a) : (b) )

/// Returns the maximum of a and b
#define MAX(a, b) ( (a) > (b) ? (a) : (b) )

/// Endianness (Byte Order)
namespace Endianness
{
  enum Enum
  {
    Little = 0x04030201UL,
    Big    = 0x01020304UL,
    PDP    = 0x02010403UL
  };
  typedef uint32 Type;
}
static const union { uint8 bytes[4]; Endianness::Enum value; } platformEndianness = { 0x01, 0x02, 0x03, 0x04 };

/// Returns the platform endianness
inline Endianness::Enum PlatformEndianness(void)
{
  return platformEndianness.value;
}

/// Returns true if the platform is in network byte order
inline bool IsPlatformNetworkByteOrder(void)
{
  return PlatformEndianness() == Endianness::Big;
}

/// Returns the endian flipped equivalent of the specified value
template<typename T>
inline R_ENABLE_IF(sizeof(T) <= 1, T) EndianFlip(T value)
{
  // Nothing to flip
  return value;
}

template<typename T>
inline R_ENABLE_IF(sizeof(T) > 1, T) EndianFlip(T value)
{
  // Reverse byte order
  T flipped = T();
  for(Bytes i = 0; i < sizeof(value); ++i)
    ((byte*)&flipped)[i] = ((byte*)&value)[sizeof(value) - 1 - i];

  return flipped;
}

/// Performs an endian flip on little endian platforms
template<typename T>
inline T NetworkFlip(T value)
{
  return (PlatformEndianness() == Endianness::Little) ? EndianFlip(value) : value;
}

/// Returns the number of bits needed to represent the unsigned integral value
template<typename T>
inline R_ENABLE_IF(is_integral<T>::value && is_unsigned<T>::value, Bits) BitsNeededToRepresent(T value)
{
  return BITS_NEEDED_TO_REPRESENT(value);
}

/// Returns the number of bits needed to represent the signed integral value
template<typename T>
inline R_ENABLE_IF(is_integral<T>::value && is_signed<T>::value, Bits) BitsNeededToRepresent(T value)
{
  typedef typename make_unsigned<T>::type UT;

  // Reinterpret cast to unsigned type, then determine bits needed
  return BitsNeededToRepresent(*reinterpret_cast<UT*>(&value));
}

/// Returns true if the fixed-size buffer is completely zeroed, else false
/// (Note: Incurs static memory overhead equal to specified buffer size. Optimized for speed.)
template<size_t N>
inline bool BufferIsZeroed(const byte (&buffer)[N])
{
  // Statically allocate zero-initialized buffer of same size
  static const byte zeroBuffer[N] = {};

  // Given buffer is identical to zero buffer?
  return (memcmp(buffer, zeroBuffer, N) == 0);
}

/// Returns true if memA and memB are overlapping
inline bool MemoryIsOverlapping(const byte* memA, Bytes memASize, const byte* memB, Bytes memBSize)
{
  if((memA - memB <= 0 && memB - (memA + memASize) <= 0)                            //    memB starting point is within memA?
  || (memA - (memB + memBSize) <= 0 && (memB + memBSize) - (memA + memASize) <= 0)) // OR memB ending point is within memA?
    return true;  // There is overlap
  else
    return false; // No overlap
}

/// Clamps value to within the specified inclusive range
template<typename T>
inline const T& Clamp(const T& value, const T& minValue, const T& maxValue)
{
  return std::max(std::min(value, maxValue), minValue);
}

/// Returns the interpolated value between a and b at alpha [0, 1], where 0 is a and 1 is b
template <typename T, typename F>
inline T Interpolate(T a, T b, F alpha)
{
  return a + ((b - a) * alpha);
}

/// Returns the average of previous and current, given currentWeight [0, 1]
template <typename T, typename F>
inline T Average(T previous, T current, F currentWeight)
{
  return T((previous * (F(1) - currentWeight)) + (current * currentWeight));
}

/// Returns the specified floating-point value rounded to the nearest integer value (represented in the same floating-point type)
template<typename T>
inline R_ENABLE_IF(is_floating_point<T>::value, T) Round(T value)
{
  return std::floor(value + T(0.5));
}

/// Divides the numerator by the denominator and ceils the result without branching or casting to intermediary types
template<typename T>
inline R_ENABLE_IF(is_integral<T>::value, T) DivCeil(T numerator, T denominator)
{
  // Formula from StackOverflow answer ( http://stackoverflow.com/a/17005764 ), Ben Voigt, June 9th, 2013
  return (numerator / denominator)
       + T(((numerator < 0) ^ (denominator > 0)) && (numerator % denominator));
}
template<typename T>
inline R_ENABLE_IF(is_floating_point<T>::value, T) DivCeil(T numerator, T denominator)
{
  return std::ceil(numerator / denominator);
}

} // namespace Zero

// Using directives
//using Zero::uintptr;
