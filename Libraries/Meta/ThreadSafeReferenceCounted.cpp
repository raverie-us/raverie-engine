// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

DefineThreadSafeReferenceCountedHandle(ThreadSafeReferenceCounted);

ZilchDefineType(ThreadSafeReferenceCounted, builder, type)
{
  ZeroBindThreadSafeReferenceCountedHandle();
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

} // namespace Zero
