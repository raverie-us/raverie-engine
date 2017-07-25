///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

// Includes
#include "Utility/Typedefs.hpp"

// Fixed-Width Signed Integral Types
typedef smax intmax;
typedef s64  int64;
typedef s32  int32;
typedef s16  int16;
typedef s8   int8;

// Fixed-Width Unsigned Integral Types
typedef umax uintmax;
typedef u64  uint64;
typedef u32  uint32;
typedef u16  uint16;
typedef u8   uint8;

// Fastest Signed Integral Types (minimum width specified)
typedef s64 intfast64;
typedef s32 intfast32;
typedef s32 intfast16;
typedef s8  intfast8;

// Fastest Unsigned Integral Types (minimum width specified)
typedef u64 uintfast64;
typedef u32 uintfast32;
typedef u32 uintfast16;
typedef u8  uintfast8;

// Other Types
typedef uint32 Bits;     // 1 Bit
typedef uint32 Bytes;    // 8 Bits
typedef uint32 Kilobits; // 1000 Bits
typedef double Kbps;     // Kilobits per second
