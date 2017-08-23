///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.h"

#ifdef _MSC_VER
#include "Windows.h"
#endif

namespace Audio
{
  //************************************************************************************************
  void* AtomicCompareExchangePointer(void** destination, void* exchange, void* comperand)
  {
#ifdef _MSC_VER
    return InterlockedCompareExchangePointer(destination, exchange, comperand);
#endif

    return destination;
  }

  //************************************************************************************************
  void AtomicSetPointer(void** target, void* value)
  {
#ifdef _MSC_VER
    InterlockedExchangePointer(target, value);
#endif
  }

  //************************************************************************************************
  Type32Bit AtomicDecrement32(Type32Bit* value)
  {
#ifdef _MSC_VER
    return InterlockedDecrement(value);
#endif

    return *value;
  }

  //************************************************************************************************
  Type32Bit AtomicIncrement32(Type32Bit* value)
  {
#ifdef _MSC_VER
    return InterlockedIncrement(value);
#endif

    return *value;
  }

  //************************************************************************************************
  Type32Bit AtomicSet32(Type32Bit* target, Type32Bit value)
  {
#ifdef _MSC_VER
    return InterlockedExchange(target, value);
#endif

    return *target;
  }

  //************************************************************************************************
  Type32Bit AtomicCompareExchange32(Type32Bit* destination, Type32Bit exchange, Type32Bit comperand)
  {
#ifdef _MSC_VER
    return InterlockedCompareExchange(destination, exchange, comperand);
#endif

    return *destination;
  }

}