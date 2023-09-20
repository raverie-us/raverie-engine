// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

void RegisterCommonHandleManagers()
{
  // Basic handles
  RaverieRegisterHandleManager(ReferenceCountedEmpty);
  RaverieRegisterHandleManager(SafeId32);
  RaverieRegisterHandleManager(SafeId64);
  RaverieRegisterHandleManager(ThreadSafeId32);
  RaverieRegisterHandleManager(ThreadSafeId64);
  RaverieRegisterHandleManager(ReferenceCountedSafeId32);
  RaverieRegisterHandleManager(ReferenceCountedSafeId64);
  RaverieRegisterHandleManager(ReferenceCountedThreadSafeId32);
  RaverieRegisterHandleManager(ReferenceCountedThreadSafeId64);

  // Object handles
  RaverieRegisterHandleManager(ReferenceCountedObject);
  RaverieRegisterHandleManager(SafeId32Object);
  RaverieRegisterHandleManager(SafeId64Object);
  RaverieRegisterHandleManager(ThreadSafeId32Object);
  RaverieRegisterHandleManager(ThreadSafeId64Object);
  RaverieRegisterHandleManager(ReferenceCountedSafeId32Object);
  RaverieRegisterHandleManager(ReferenceCountedSafeId64Object);
  RaverieRegisterHandleManager(ReferenceCountedThreadSafeId32Object);
  RaverieRegisterHandleManager(ReferenceCountedThreadSafeId64Object);

  // EventObject handles
  RaverieRegisterHandleManager(ReferenceCountedEventObject);
  RaverieRegisterHandleManager(SafeId32EventObject);
  RaverieRegisterHandleManager(SafeId64EventObject);
  RaverieRegisterHandleManager(ThreadSafeId32EventObject);
  RaverieRegisterHandleManager(ThreadSafeId64EventObject);
  RaverieRegisterHandleManager(ReferenceCountedSafeId32EventObject);
  RaverieRegisterHandleManager(ReferenceCountedSafeId64EventObject);
  RaverieRegisterHandleManager(ReferenceCountedThreadSafeId32EventObject);
  RaverieRegisterHandleManager(ReferenceCountedThreadSafeId64EventObject);
}

} // namespace Raverie
