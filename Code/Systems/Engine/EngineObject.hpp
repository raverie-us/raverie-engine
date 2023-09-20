// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

/// Base class for common engine/system objects. Provides overloaded new/delete
/// to track memory allocations.
class EngineObject : public EventObject
{
public:
  static Memory::Heap* sEngineHeap;
  static void* operator new(size_t size);
  static void operator delete(void* pMem, size_t size);

  virtual ~EngineObject();
};

} // namespace Raverie
