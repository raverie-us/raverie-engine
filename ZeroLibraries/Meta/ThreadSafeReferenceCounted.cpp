///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Nathan Carlson
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
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
