// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

DefineThreadSafeReferenceCountedHandle(ThreadSafeReferenceCounted);

RaverieDefineType(ThreadSafeReferenceCounted, builder, type)
{
  RaverieBindThreadSafeReferenceCountedHandle();
}

ThreadSafeReferenceCounted::ThreadSafeReferenceCounted()
{
  ConstructThreadSafeReferenceCountedHandle();
}

ThreadSafeReferenceCounted::ThreadSafeReferenceCounted(const ThreadSafeReferenceCounted&)
{
  ConstructThreadSafeReferenceCountedHandle();
}

ThreadSafeReferenceCounted::~ThreadSafeReferenceCounted()
{
  DestructThreadSafeReferenceCountedHandle();
}

} // namespace Raverie
