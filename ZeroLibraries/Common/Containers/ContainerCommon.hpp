///////////////////////////////////////////////////////////////////////////////
///
/// \file ContainerCommon.hpp
/// Container Support.
///
/// Authors: Chris Peters, Andrew Colean
/// Copyright 2010-2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "TypeTraits.hpp"

// For placement new
#include <new>
// For size_t
#include <stddef.h>
// For memmove, memcpy
#include <string.h>

namespace Zero
{

// Returns an invalid type that is zeroed out (only used for not-crashing after an assert)
// This places no requirement on default construction or copy construction
template <typename T>
T& GetInvalidObject()
{
  static byte Invalid[sizeof(T)] = { 0 };
  return (T&)*Invalid;
}

/// A special dummy type that we pass in as the first argument to a container's constructor
/// to enable passing in a list of values that we initialize the container with
/// This entirely exists to resolve ambiguities with other constructors (such as Array(size_t), if T was size_t there would be an issue)
class ContainerInitializerDummy
{
public:
};
#define ZeroInit ((Zero::ContainerInitializerDummy*)nullptr)

/// Base class for containers that use allocators.
/// WARNING: This should be a ZeroSharedTemplate, however Release is optimizing out the constructor
/// for an unknown reason and therefore the definition isn't getting exported to the dll
/// We force both sides (dll and us) to generate the definition by forcing it as an export
template<typename AllocatorType>
class ZeroExport AllocationContainer
{
public:
  typedef AllocatorType allocator_type;
  allocator_type& GetAllocator(){return mAllocator;}
  void SetAllocator(const allocator_type& allocator) { mAllocator = allocator; }
protected:
  allocator_type mAllocator;
};

/// Standard swap function
template<typename type> 
inline void Swap(type& a, type& b)
{
  type c(a);
  a = b;
  b = c;
}

/// A Pair of objects.
template<typename type0, typename type1>
struct ZeroSharedTemplate Pair
{
  typedef type0 first_type;
  typedef type1 second_type;
  typedef Pair<type0, type1> this_type;
  type0 first;
  type1 second;

  explicit Pair(const type0& value0)
    : first(value0), second()
  {
  }

  Pair(const type0& value0, const type1& value1)
    : first(value0), second(value1)
  {
  }

  Pair(const type0& value0, MoveReference<type1> value1)
    : first(value0), second(ZeroMove(value1))
  {
  }

  Pair(const Pair& other)
    : first(other.first), second(other.second)
  {
  }

  Pair(MoveReference<Pair> other)
    : first(other->first), second(other->second)
  {
  }

  Pair()
    : first(), second()
  {
  }

  void Swap(Pair& right)
  {
    if(this != &right)
    {
      Swap(first, right.first);
      Swap(second, right.second);
    }
  }

  size_t Hash() const;

  bool operator==(const this_type& rhs) const
  {
    return (first  == rhs.first)
        && (second == rhs.second);
  }

  bool operator!=(const this_type& rhs) const
  {
    return !(*this == rhs);
  }
};

template<typename type0, typename type1>
Pair<type0,type1> MakePair(const type0& first, const type1& second)
{
  return Pair<type0,type1>(first, second);
}

/// Moves the specified type to a different memory destination without calling the destructor on the source instance
/// Used by our containers to speed up shifting operations
/// Provide a template specialization for your own class to define this behavior, else this default will be used
template<typename type>
struct MoveWithoutDestructionOperator
{
  static inline void MoveWithoutDestruction(type* dest, type* source)
  {
    // TODO: Use move constructor if available

    // Use copy constructor
    new(dest) type(*source);
    // Destroy the original
    source->~type();
  }
};

template<typename first, typename second>
struct MoveWithoutDestructionOperator< Pair<first, second> >
{
  static inline void MoveWithoutDestruction(Pair<first, second>* dest, 
                                            Pair<first, second>* source)
  {
    MoveWithoutDestructionOperator<first>::MoveWithoutDestruction(&dest->first, &source->first);
    MoveWithoutDestructionOperator<second>::MoveWithoutDestruction(&dest->second, &source->second);
  }
};

/// Move values from source array to new array using move
template<typename type>
inline void UninitializedMove(type* dest, type* source, size_t size, 
                               false_type /*isPod*/)
{
  type* destEnd = dest + size;
  while(dest != destEnd)
  {
    MoveWithoutDestructionOperator<type>::MoveWithoutDestruction(dest, source);
    ++dest;
    ++source;
  }
}

template<typename type>
inline void UninitializedMove(type* dest, type* source, size_t size, 
                               true_type /*isPod*/)
{
  memmove(dest, source, sizeof(type)*size);
}

template<typename type>
inline void UninitializedMoveRev(type* dest, type* source, size_t size, 
                                  false_type /*isPod*/)
{
  type* destEnd = dest + size;
  type* sourceEnd = source + size;
  while(size != 0)
  {
    --destEnd;
    --sourceEnd;
    MoveWithoutDestructionOperator<type>::MoveWithoutDestruction(destEnd, sourceEnd);
    --size;
  }
}

template<typename type>
inline void UninitializedMoveRev(type* dest, type* source, size_t size, 
                                  true_type /*isPod*/)
{
  memmove(dest, source, sizeof(type)*size);
}

template<typename type>
inline void UninitializedCopy(type* dest, type* source, size_t size, 
                               false_type /*isPod*/)
{
  type* destEnd = dest + size;
  while(dest != destEnd)
  {
    new(dest) type(*source);
    ++dest;
    ++source;
  }
}

template<typename type>
inline void UninitializedCopy(type* dest, type* source, size_t size, 
                               true_type /*isPod*/)
{
  memcpy(dest, source, sizeof(type)*size);
}

template<typename type, typename initType>
inline void ConstructWith(type* elem, const initType& source)
{
  // Copy construct from source
  new(elem) type(source);
}

template<typename type, typename initType>
inline void ConstructWith(type* elem, MoveReference<initType> source)
{
  // Move construct from source
  new(elem) type(ZeroMove(source));
}

template<typename type>
inline void Construct(type* elem, false_type /*isPod*/)
{
  new(elem) type();
}

template<typename type>
inline void Construct(type* /*elem*/, true_type /*isPod*/)
{
  //do nothing for pod
}

template<typename type>
inline void Destroy(type* dest, false_type /*isPod*/)
{
  dest->~type();
}

template<typename type>
inline void Destroy(type* /*dest*/, true_type /*isPod*/)
{
  //do nothing for pod types
}

template<typename type>
inline void UninitializedFill(type* dest, size_t size, const type& source)
{
  type* destEnd = dest+size;
  while(dest!=destEnd)
  {
    new(dest) type(source);
    ++dest;
  }
}

template<typename type>
inline void UninitializedFill(type* dest, size_t size, false_type /*isPod*/)
{
  type* destEnd = dest+size;
  while(dest!=destEnd)
  {
    new(dest) type();
    ++dest;
  }
}

template<typename type>
inline void UninitializedFill(type* /*dest*/, size_t /*size*/, 
                               true_type /*isPod*/)
{
  //do nothing for pod types
}


template<typename type>
inline void DestroyElements(type* begin, size_t size, false_type /*isPod*/)
{
  type* end = begin+size;
  while(begin!=end)
  {
    begin->~type();
    ++begin;
  }
}

template<typename type>
inline void DestroyElements(type* /*begin*/, size_t /*size*/,
                            true_type /*isPod*/)
{
  //do nothing for pod types
}

//Forms a range with iterators.
template<typename containerType>
struct IteratorRange
{
  typedef typename containerType::iterator iterator;
  typedef typename containerType::reference reference;

  IteratorRange(iterator pbegin, iterator pend)
    : Begin(pbegin) , End(pend)
  {
  }

  reference Front() { return *begin; }
  void PopFront()
  {
    ErrorIf(Empty(), "Popped empty range.");
    ++begin;
  }

  bool Empty() { return begin == end; }
  size_t Length() { return end - begin; }

  IteratorRange& All() { return *this; }
  const IteratorRange& All() const { return *this; }

  iterator begin;
  iterator end;
};


//Forms a range with iterators.
template<typename iteratorType>
struct IteratorTypedRange
{
  typedef iteratorType iterator;
  typedef typename iteratorType::reference reference;

  IteratorTypedRange(iterator pbegin, iterator pend)
    : Begin(pbegin), End(pend)
  {
  }

  reference Front() { return *begin; }
  void PopFront()
  {
    ErrorIf(Empty(), "Popped empty range.");
    ++begin;
  }

  bool Empty() { return begin == end; }
  size_t Length() { return end - begin; }

  IteratorTypedRange& All() { return *this; }
  const IteratorTypedRange& All() const { return *this; }

  iterator begin;
  iterator end;
};


// Constant Pointer Range. Forms a range between two pointers.
template<typename type>
struct ConstPointerRange
{
  typedef const type* iterator;

  //Construct a range with two pointers.
  ConstPointerRange(iterator pbegin, iterator pend)
    : begin(pbegin), end(pend)
  {
  }

  //Construct a range with a pointer and a size.
  ConstPointerRange(iterator pbegin, size_t size)
    : begin(pbegin), end(pbegin + size)
  {
  }

  const type& Front() { return *begin; }
  void PopFront()
  {
    ErrorIf(Empty(), "Popped empty range.");
    ++begin;
  }

  bool Empty() { return begin == end; }
  size_t Length() { return end - begin; }

  ConstPointerRange& All() { return *this; }
  const ConstPointerRange& All() const { return *this; }

  iterator begin;
  iterator end;
};

// Pointer Range. Forms a range between two pointers.
template<typename type>
struct PointerRange
{
  typedef type* iterator;

  //Construct a range with two pointers.
  PointerRange(iterator pbegin, iterator pend)
    : begin(pbegin), end(pend)
  {
  }

  //Construct a range with a pointer and a size.
  PointerRange(iterator pbegin, size_t size)
    : begin(pbegin), end(pbegin + size)
  {
  }

  type& Front() { return *begin; }
  void PopFront()
  {
    ErrorIf(Empty(), "Popped empty range.");
    ++begin;
  }

  bool Empty() { return begin == end; }
  size_t Length() { return end - begin; }
  iterator begin;
  iterator end;
};

template<typename iteratorType>
IteratorTypedRange<iteratorType> BuildRange(iteratorType begin, 
                                            iteratorType end)
{
  return IteratorTypedRange<iteratorType>(begin, end);
}

template<typename elementType>
ConstPointerRange<elementType> BuildRange(elementType* begin, elementType* end)
{
  return ConstPointerRange<elementType>(begin, end);
}

template< typename type>
inline void DeleteOp(type* pointer) { delete pointer; }

template<typename keytype, typename type>
inline void DeleteOp(Pair<const keytype,type>& entry) { delete entry.second; }

template<typename keytype, typename type>
inline void DeleteOp(Pair<keytype,type>& entry) { delete entry.second; }


template<typename rangeType>
void DeleteObjectsIn(rangeType range)
{
  for(; !range.Empty(); range.PopFront())
  {
    DeleteOp(range.Front());
  }
}

template<typename containerType>
void DeleteObjectsInContainer(containerType& container)
{
  DeleteObjectsIn(container.All());
  container.Clear();
}

struct DataBlock
{
  DataBlock() : Data(nullptr), Size(0) {};
  DataBlock(byte* data, size_t size):Data(data), Size(size) {};
  operator bool(){return Data != nullptr;}
  byte* Data;
  size_t Size;
};

//
// static_verify_function_signature
//

/// Statically verifies a given function has a specific signature
/// Fails to compile if the given function does not have the exact signature specified
/// First parameter : Function pointer type with desired signature to verify
/// Second parameter: Named function to verify has the desired signature
template <typename Fn, Fn> struct ZeroSharedTemplate static_verify_function_signature
{
  typedef Fn type;
};

//
// has_equality_operator
//

/// has_equality_operator_helper helper class
template<typename T>
struct ZeroSharedTemplate has_equality_operator_helper
{
  template<typename T2>
  static yes Test(int param[sizeof( *((T2*)nullptr) == *((T2*)nullptr) )]);
  template<typename T2>
  static inline no Test(...);

  static const bool value = (sizeof(Test<T>(0)) == sizeof(yes));
};

/// Provides a constant defined as true if T has an equality operator, else defined as false
template<typename T>
struct ZeroSharedTemplate has_equality_operator : public integral_constant<bool, has_equality_operator_helper<T>::value> {};

//
// ComparePolicy
//

/// Policy for how values are tested for equality
/// Allow the containers to be searched by values
/// that are not the same as the stored type
template <typename T, typename Enable = void>
struct ZeroSharedTemplate ComparePolicy
{
};

/// Default compare policy
/// Available for instantiation if T has an accessible self-type equality operator
template <typename T>
struct ZeroSharedTemplate ComparePolicy<T, TC_ENABLE_IF(has_equality_operator<T>::value)>
{
public:
  inline bool Equal(const T& left, const T& right) const
  {
    return left == right;
  }
  template<typename T2>
  inline bool Equal(const T& left, const T2& right) const
  {
    return left == right;
  }
};

/// ComparePolicy for const char* (uses string comparison)
template<>
struct ZeroShared ComparePolicy<const char*>
{
  inline bool Equal(const char* left, const char* right) const
  {
    return strcmp(left, right) == 0;
  }

  template<typename stringType>
  inline bool Equal(const char* left, const stringType& right) const
  {
    // use operator == to other type, usually strings
    return right == left;
  }
};

//
// has_valid_compare_policy
//

/// has_valid_compare_policy_helper helper class
template<typename T>
struct ZeroSharedTemplate has_valid_compare_policy_helper
{
  typedef typename ComparePolicy<T> ComparePolicyT;

  template<typename T2>
  static inline yes Test(static_verify_function_signature< typename bool(ComparePolicyT::*)(const T2&, const T2&) const, &ComparePolicyT::Equal >*);
  template<typename T2>
  static inline no Test(...);

  static const bool value = (sizeof(Test<T>(0)) == sizeof(yes));
};

/// Provides a constant defined as true if T has a valid compare policy, else defined as false
/// A compare policy is valid if it has a function callable as: bool Equal(const T& left, const T& right) const;
template<typename T>
struct ZeroSharedTemplate has_valid_compare_policy : public integral_constant<bool, has_valid_compare_policy_helper<T>::value> {};

} // namespace Zero
