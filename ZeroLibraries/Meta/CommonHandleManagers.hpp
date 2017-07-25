////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Claeys
/// Copyright 2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// These are commonly used handle types to fit most use cases. If your type needs to derive from
// another type, you will have to declare meta for the custom templated handle type. See below
// and in CommonHandleManagers.cpp for examples.

//------------------------------------------------------------------------------------ Basic handles
typedef ReferenceCounted<EmptyClass>                   ReferenceCountedEmpty;
typedef SafeId<u32, EmptyClass>                        SafeId32;
typedef SafeId<u64, EmptyClass>                        SafeId64;
typedef ThreadSafeId<u32, EmptyClass>                  ThreadSafeId32;
typedef ThreadSafeId<u64, EmptyClass>                  ThreadSafeId64;
typedef ReferenceCountedSafeId<u32, EmptyClass>        ReferenceCountedSafeId32;
typedef ReferenceCountedSafeId<u64, EmptyClass>        ReferenceCountedSafeId64;
typedef ReferenceCountedThreadSafeId<u32, EmptyClass>  ReferenceCountedThreadSafeId32;
typedef ReferenceCountedThreadSafeId<u64, EmptyClass>  ReferenceCountedThreadSafeId64;

//----------------------------------------------------------------------------------- Object handles
typedef ReferenceCounted<Object>                       ReferenceCountedObject;
typedef SafeId<u32, Object>                            SafeId32Object;
typedef SafeId<u64, Object>                            SafeId64Object;
typedef ThreadSafeId<u32, Object>                      ThreadSafeId32Object;
typedef ThreadSafeId<u64, Object>                      ThreadSafeId64Object;
typedef ReferenceCountedSafeId<u32, Object>            ReferenceCountedSafeId32Object;
typedef ReferenceCountedSafeId<u64, Object>            ReferenceCountedSafeId64Object;
typedef ReferenceCountedThreadSafeId<u32, Object>      ReferenceCountedThreadSafeId32Object;
typedef ReferenceCountedThreadSafeId<u64, Object>      ReferenceCountedThreadSafeId64Object;

//------------------------------------------------------------------------------ EventObject handles
typedef ReferenceCounted<EventObject>                  ReferenceCountedEventObject;
typedef SafeId<u32, EventObject>                       SafeId32EventObject;
typedef SafeId<u64, EventObject>                       SafeId64EventObject;
typedef ThreadSafeId<u32, EventObject>                 ThreadSafeId32EventObject;
typedef ThreadSafeId<u64, EventObject>                 ThreadSafeId64EventObject;
typedef ReferenceCountedSafeId<u32, EventObject>       ReferenceCountedSafeId32EventObject;
typedef ReferenceCountedSafeId<u64, EventObject>       ReferenceCountedSafeId64EventObject;
typedef ReferenceCountedThreadSafeId<u32, EventObject> ReferenceCountedThreadSafeId32EventObject;
typedef ReferenceCountedThreadSafeId<u64, EventObject> ReferenceCountedThreadSafeId64EventObject;

} // namespace Zero
