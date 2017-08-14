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
  bool AtomicCheckEqualityPointer(void* first, void* second)
  {
    void* compareCheck = first;
    AtomicCompareExchangePointer(&compareCheck, nullptr, second);
    return compareCheck == nullptr;
  }
}