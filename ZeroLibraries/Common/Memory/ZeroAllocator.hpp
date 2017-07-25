///////////////////////////////////////////////////////////////////////////////
///
/// \file ZeroAllocator.hpp
/// Allocator that can be used in STL containers.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

///Allocator to bind Zero memory management to STL.
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

  template <class U> struct rebind
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

  Zallocator() { }
  ~Zallocator() { }
  Zallocator(const Zallocator&) { }
  Zallocator& operator=(const Zallocator&) { return *this; }
  template <class U> Zallocator(const Zallocator<U>&) { }

  //Max number of elements
  size_type MaxSize() const
  {
    return std::numeric_limits<std::size_t>::max() / sizeof(T);
  }

  // This allocates the memory for num elements of type T
  pointer allocate(size_type num)
  {
    if (num == 0) { return nullptr; }
    if (num > MaxSize()) 
    {
      throw std::length_error("Zallocator Too Many Elements");
    }
    void* ptr = Zero::zAllocate(num * sizeof(T));
    if (!ptr)
    {
      throw std::bad_alloc();
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
    new(static_cast<void*>(p)) T(value);
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
bool operator!= (const Zallocator<T1>&, const Zallocator<T2>&)
{
  return false;
}
