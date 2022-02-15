// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

// To make Windows compile (will not run properly).
extern "C" void* __imp___RTDynamicCast(void* input, ...)
{
  return input;
}
