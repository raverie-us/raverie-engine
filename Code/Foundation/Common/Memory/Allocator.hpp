// MIT Licensed (see LICENSE.md).

#pragma once
#ifdef WelderExceptions
#  include <stdexcept>
#endif

#ifndef UseMemoryGraph
#  define UseMemoryGraph 1
#endif

#if UseMemoryGraph

#  include "Graph.hpp"
#  include "Heap.hpp"

#else

namespace Zero
{

void* zAllocate(size_t numberOfBytes);
void zDeallocate(void*);

class ZeroShared StandardMemory
{
public:
  static inline void MemCopy(void* dest, void* source, size_t numberOfBytes)
  {
    memcpy(dest, source, numberOfBytes);
  }

  static inline void MemMove(void* dest, void* source, size_t numberOfBytes)
  {
    memmove(dest, source, numberOfBytes);
  }
};

// Default allocator of Standard Memory. This allocator
// is used by default for all the containers.
class ZeroShared DefaultAllocator : public StandardMemory
{
public:
  enum
  {
    cAlignment = 4
  };
  void* Allocate(size_t numberOfBytes)
  {
    return zAllocate(numberOfBytes);
  };
  void Deallocate(void* ptr, size_t numberOfBytes)
  {
    zDeallocate(ptr);
  }
};

} // namespace Zero

#endif // UseMemoryGraph

/// Allocator to bind memory management to STL.
template <class T>
class Zallocator
{
public:
  typedef T value_type;
  typedef T* pointer;
  typedef const T* const_pointer;
  typedef T& reference;
  typedef const T& const_reference;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;

  template <class U>
  struct rebind
  {
    typedef Zallocator<U> other;
  };

  pointer Address(reference value) const
  {
    return &value;
  }

  const_pointer Address(const_reference value) const
  {
    return &value;
  }

  Zallocator()
  {
  }
  ~Zallocator()
  {
  }
  Zallocator(const Zallocator&)
  {
  }
  Zallocator& operator=(const Zallocator&)
  {
    return *this;
  }
  template <class U>
  Zallocator(const Zallocator<U>&)
  {
  }

  // Max number of elements
  size_type MaxSize() const
  {
    return std::numeric_limits<std::size_t>::max() / sizeof(T);
  }

  // This allocates the memory for num elements of type T
  pointer allocate(size_type num)
  {
    if (num == 0)
    {
      return nullptr;
    }
    if (num > MaxSize())
    {
#ifdef WelderExceptions
      throw std::length_error("Zallocator Too Many Elements");
#else
      return nullptr;
#endif
    }
    void* ptr = Zero::zAllocate(num * sizeof(T));
    if (!ptr)
    {
#ifdef WelderExceptions
      throw std::bad_alloc();
#else
      return nullptr;
#endif
    }
    return static_cast<pointer>(ptr);
  }

  pointer allocate(const size_t n, const void*)
  {
    return allocate(n);
  }

  void deallocate(pointer p, size_type)
  {
    Zero::zDeallocate(p);
  }

  void Construct(pointer p, const T& value)
  {
    new (static_cast<void*>(p)) T(value);
  }

  void Destroy(pointer p)
  {
    p->~T();
  }
};

template <class T1, class T2>
bool operator==(const Zallocator<T1>&, const Zallocator<T2>&)
{
  return true;
}

template <class T1, class T2>
bool operator!=(const Zallocator<T1>&, const Zallocator<T2>&)
{
  return false;
}
