// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

// Basic handles
// DefineReferenceCountedHandle(ReferenceCountedEmpty);
// ZilchDefineType(ReferenceCountedEmpty, builder, type)
//{
//  ZeroBindHandle();
//}
//
// DefineSafeIdHandle(SafeId32);
// ZilchDefineType(SafeId32, builder, type)
//{
//  ZeroBindHandle();
//}
//
// DefineSafeIdHandle(SafeId64);
// ZilchDefineType(SafeId64, builder, type)
//{
//  ZeroBindHandle();
//}
//
// DefineThreadSafeIdHandle(ThreadSafeId32);
// ZilchDefineType(ThreadSafeId32, builder, type)
//{
//  ZeroBindHandle();
//}
//
// DefineThreadSafeIdHandle(ThreadSafeId64);
// ZilchDefineType(ThreadSafeId64, builder, type)
//{
//  ZeroBindHandle();
//}
//
// DefineReferenceCountedSafeIdHandle(ReferenceCountedSafeId32);
// ZilchDefineType(ReferenceCountedSafeId32, builder, type)
//{
//  ZeroBindHandle();
//}
//
// DefineReferenceCountedSafeIdHandle(ReferenceCountedSafeId64);
// ZilchDefineType(ReferenceCountedSafeId64, builder, type)
//{
//  ZeroBindHandle();
//}
//
// DefineReferenceCountedThreadSafeIdHandle(ReferenceCountedThreadSafeId32);
// ZilchDefineType(ReferenceCountedThreadSafeId32, builder, type)
//{
//  ZeroBindHandle();
//}
//
// DefineReferenceCountedThreadSafeIdHandle(ReferenceCountedThreadSafeId64);
// ZilchDefineType(ReferenceCountedThreadSafeId64, builder, type)
//{
//  ZeroBindHandle();
//}
//
/// Object handles
// DefineReferenceCountedHandle(ReferenceCountedObject);
// ZilchDefineType(ReferenceCountedObject, builder, type)
//{
//  ZeroBindHandle();
//}
//
// DefineSafeIdHandle(SafeId32Object);
// ZilchDefineType(SafeId32Object, builder, type)
//{
//  ZeroBindHandle();
//}
//
// DefineSafeIdHandle(SafeId64Object);
// ZilchDefineType(SafeId64Object, builder, type)
//{
//  ZeroBindHandle();
//}
//
// DefineThreadSafeIdHandle(ThreadSafeId32Object);
// ZilchDefineType(ThreadSafeId32Object, builder, type)
//{
//  ZeroBindHandle();
//}
//
// DefineThreadSafeIdHandle(ThreadSafeId64Object);
// ZilchDefineType(ThreadSafeId64Object, builder, type)
//{
//  ZeroBindHandle();
//}
//
// DefineReferenceCountedSafeIdHandle(ReferenceCountedSafeId32Object);
// ZilchDefineType(ReferenceCountedSafeId32Object, builder, type)
//{
//  ZeroBindHandle();
//}
//
// DefineReferenceCountedSafeIdHandle(ReferenceCountedSafeId64Object);
// ZilchDefineType(ReferenceCountedSafeId64Object, builder, type)
//{
//  ZeroBindHandle();
//}
//
// DefineReferenceCountedThreadSafeIdHandle(ReferenceCountedThreadSafeId32Object);
// ZilchDefineType(ReferenceCountedThreadSafeId32Object, builder, type)
//{
//  ZeroBindHandle();
//}
//
// DefineReferenceCountedThreadSafeIdHandle(ReferenceCountedThreadSafeId64Object);
// ZilchDefineType(ReferenceCountedThreadSafeId64Object, builder, type)
//{
//  ZeroBindHandle();
//}
//
/// EventObject handles
// DefineReferenceCountedHandle(ReferenceCountedEventObject);
// ZilchDefineType(ReferenceCountedEventObject, builder, type)
//{
//  ZeroBindHandle();
//}
//
// DefineSafeIdHandle(SafeId32EventObject);
// ZilchDefineType(SafeId32EventObject, builder, type)
//{
//  ZeroBindHandle();
//}
//
// DefineSafeIdHandle(SafeId64EventObject);
// ZilchDefineType(SafeId64EventObject, builder, type)
//{
//  ZeroBindHandle();
//}
//
// DefineThreadSafeIdHandle(ThreadSafeId32EventObject);
// ZilchDefineType(ThreadSafeId32EventObject, builder, type)
//{
//  ZeroBindHandle();
//}
//
// DefineThreadSafeIdHandle(ThreadSafeId64EventObject);
// ZilchDefineType(ThreadSafeId64EventObject, builder, type)
//{
//  ZeroBindHandle();
//}
//
// DefineReferenceCountedSafeIdHandle(ReferenceCountedSafeId32EventObject);
// ZilchDefineType(ReferenceCountedSafeId32EventObject, builder, type)
//{
//  ZeroBindHandle();
//}
//
// DefineReferenceCountedSafeIdHandle(ReferenceCountedSafeId64EventObject);
// ZilchDefineType(ReferenceCountedSafeId64EventObject, builder, type)
//{
//  ZeroBindHandle();
//}
//
// DefineReferenceCountedThreadSafeIdHandle(ReferenceCountedThreadSafeId32EventObject);
// ZilchDefineType(ReferenceCountedThreadSafeId32EventObject, builder, type)
//{
//  ZeroBindHandle();
//}
//
// DefineReferenceCountedThreadSafeIdHandle(ReferenceCountedThreadSafeId64EventObject);
// ZilchDefineType(ReferenceCountedThreadSafeId64EventObject, builder, type)
//{
//  ZeroBindHandle();
//}

void RegisterCommonHandleManagers()
{
  // Basic handles
  ZeroRegisterHandleManager(ReferenceCountedEmpty);
  ZeroRegisterHandleManager(SafeId32);
  ZeroRegisterHandleManager(SafeId64);
  ZeroRegisterHandleManager(ThreadSafeId32);
  ZeroRegisterHandleManager(ThreadSafeId64);
  ZeroRegisterHandleManager(ReferenceCountedSafeId32);
  ZeroRegisterHandleManager(ReferenceCountedSafeId64);
  ZeroRegisterHandleManager(ReferenceCountedThreadSafeId32);
  ZeroRegisterHandleManager(ReferenceCountedThreadSafeId64);

  // Object handles
  ZeroRegisterHandleManager(ReferenceCountedObject);
  ZeroRegisterHandleManager(SafeId32Object);
  ZeroRegisterHandleManager(SafeId64Object);
  ZeroRegisterHandleManager(ThreadSafeId32Object);
  ZeroRegisterHandleManager(ThreadSafeId64Object);
  ZeroRegisterHandleManager(ReferenceCountedSafeId32Object);
  ZeroRegisterHandleManager(ReferenceCountedSafeId64Object);
  ZeroRegisterHandleManager(ReferenceCountedThreadSafeId32Object);
  ZeroRegisterHandleManager(ReferenceCountedThreadSafeId64Object);

  // EventObject handles
  ZeroRegisterHandleManager(ReferenceCountedEventObject);
  ZeroRegisterHandleManager(SafeId32EventObject);
  ZeroRegisterHandleManager(SafeId64EventObject);
  ZeroRegisterHandleManager(ThreadSafeId32EventObject);
  ZeroRegisterHandleManager(ThreadSafeId64EventObject);
  ZeroRegisterHandleManager(ReferenceCountedSafeId32EventObject);
  ZeroRegisterHandleManager(ReferenceCountedSafeId64EventObject);
  ZeroRegisterHandleManager(ReferenceCountedThreadSafeId32EventObject);
  ZeroRegisterHandleManager(ReferenceCountedThreadSafeId64EventObject);
}

} // namespace Zero
