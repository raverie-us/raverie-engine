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
    if (sizeof(destination) == 4)
      return (void*)InterlockedCompareExchange((long*)destination, (long)exchange, (long)comperand);
    else if (sizeof(destination) == 8)
      return (void*)InterlockedCompareExchange64((long long*)(&destination), (long long)exchange, 
        (long long)comperand);
#endif

    return destination;
  }

  //************************************************************************************************
  void AtomicSetPointer(void** target, void* value)
  {
#ifdef _MSC_VER
    if (sizeof(target) == 4)
      InterlockedExchange((long*)target, (long)value);
    else if (sizeof(target) == 8)
      InterlockedExchange64((long long*)target, (long long)value);
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