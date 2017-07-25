///////////////////////////////////////////////////////////////////////////////
///
/// \file Array.hpp
/// Declaration of the Array container.
///
/// Authors: Chris Peters, Andrew Colean
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

// Includes
#include "ContainerCommon.hpp"
#include "Allocator.hpp"

namespace Zero
{
/// Generic dynamic array class / Linear Sequence. Equivalent to std::vector.
/// Stores objects in contiguous blocks of dynamically allocated 
/// memory allowing random access.
/// Constant time insertion and removal at the end. 
/// Linear time for insertion and removal at beginning or middle.
/// This array is also optimized to use pod conventions (memcpy, no destructors)
/// on fundamental types or types with proper type traits.(see TypeTraits).
template< typename ValueType,
          typename Allocator = DefaultAllocator, 
          typename value_tt  = StandardTraits<ValueType> >
class ZeroSharedTemplate Array : public AllocationContainer<Allocator>
{
public:
  /// Standard Typedefs
  typedef ValueType                             value_type;
  typedef value_type*                           pointer;
  typedef const value_type*                     const_pointer;
  typedef value_type&                           reference;
  typedef const value_type&                     const_reference;
  typedef pointer                               iterator;
  typedef const_pointer                         const_iterator;
  typedef size_t                                size_type;
  typedef ptrdiff_t                             difference_type;
  typedef Array<ValueType, Allocator, value_tt> this_type;
  typedef AllocationContainer<Allocator>        base_type;
  using base_type::mAllocator;

  /// Type Traits
  typedef value_tt                                             value_type_traits;
  typedef typename value_type_traits::is_pod_                  typeIsPod;
  typedef typename value_type_traits::has_trivial_copy_        typePodCopy;
  typedef typename value_type_traits::has_trivial_destructor_  typePodDes;
  typedef typename value_type_traits::has_trivial_constructor_ typePodCon;
  typedef typename value_type_traits::is_pod_                  typePodMove;

  /// Constants
  static const size_type InvalidIndex = size_type(-1);

  /// Provides access to a range of elements contained between two iterators
  struct range
  {
    /// Typedefs
    typedef typename this_type::value_type      value_type;
    typedef typename this_type::pointer         pointer;
    typedef typename this_type::const_pointer   const_pointer;
    typedef typename this_type::reference       reference;
    typedef typename this_type::const_reference const_reference;
    typedef typename this_type::iterator        iterator;
    typedef typename this_type::const_iterator  const_iterator;
    typedef typename this_type::size_type       size_type;
    typedef typename this_type::difference_type difference_type;
    typedef value_type                          contiguousRangeType;
    typedef reference                           FrontResult;

    /// Member Functions
    range()                             : mBegin(nullptr), mEnd(nullptr) {}
    range(iterator begin, iterator end) : mBegin(begin), mEnd(end) {}

    /// Data Access
    iterator        Begin()                           { return mBegin;         }
    iterator        End()                             { return mEnd;           }
    void            PopFront()                        { ++mBegin;              }
    void            PopBack()                         { --mEnd;                }
    reference       Front()                           { return *mBegin;        }
    reference       Back()                            { return *(mEnd - 1);    }
    bool            Empty() const                     { return mBegin == mEnd; }
    size_type       Length() const                    { return mEnd - mBegin;  }
    size_type       Size() const                      { return Length();       }
    reference       operator[](size_type index)       { return mBegin[index];  }
    const_reference operator[](size_type index) const { return mBegin[index];  }
    range&          All()                             { return *this;          }

    /// Iterators
    iterator mBegin;
    iterator mEnd;
  };

  //
  // Member Functions
  //

  /// Default Constructor
  Array()
    : mData(nullptr),
      mCapacity(0),
      mSize(0)
  {
  }

  /// Copy Constructor
  Array(const this_type& other)
    : base_type(other),
      mData(nullptr),
      mCapacity(other.mSize),
      mSize(other.mSize)
  {
    if(other.mSize != 0)
    {
      mData = (value_type*)mAllocator.Allocate(mSize*sizeof(value_type));
      UninitializedCopy(mData, other.mData, mSize, typePodCopy());
    }
  }

  /// Move Constructor
  Array(MoveReference<this_type> other)
    : base_type(*other),
      mData(other->mData),
      mCapacity(other->mCapacity),
      mSize(other->mSize)
  {
    other->ReleaseData();
  }

  /// Constructs an array of the specified size with default constructed values
  explicit Array(size_type size)
    : mData(nullptr),
      mCapacity(0),
      mSize(0)
  {
    ChangeCapacity(size);
    UninitializedFill(mData, size, typePodCon());
    mSize = size;
  }

  /// Constructs an array of the specified size with copy constructed values
  Array(size_type size, const_reference fillType)
    : mData(nullptr),
      mCapacity(0),
      mSize(0)
  {
    ChangeCapacity(size);
    UninitializedFill(mData, size, fillType);
    mSize = size;
  }
  Array(ContainerInitializerDummy*, const_reference p0)
    : mData(nullptr),
      mCapacity(0),
      mSize(0)
  {
    Reserve(1);
    PushBack(p0);
  }
  Array(ContainerInitializerDummy*, const_reference p0, const_reference p1)
    : mData(nullptr),
      mCapacity(0),
      mSize(0)
  {
    Reserve(2);
    PushBack(p0);
    PushBack(p1);
  }
  Array(ContainerInitializerDummy*, const_reference p0, const_reference p1, const_reference p2)
    : mData(nullptr),
      mCapacity(0),
      mSize(0)
  {
    Reserve(3);
    PushBack(p0);
    PushBack(p1);
    PushBack(p2);
  }
  Array(ContainerInitializerDummy*, const_reference p0, const_reference p1, const_reference p2, const_reference p3)
    : mData(nullptr),
      mCapacity(0),
      mSize(0)
  {
    Reserve(4);
    PushBack(p0);
    PushBack(p1);
    PushBack(p2);
    PushBack(p3);
  }
  Array(ContainerInitializerDummy*, const_reference p0, const_reference p1, const_reference p2, const_reference p3, const_reference p4)
    : mData(nullptr),
      mCapacity(0),
      mSize(0)
  {
    Reserve(5);
    PushBack(p0);
    PushBack(p1);
    PushBack(p2);
    PushBack(p3);
    PushBack(p4);
  }
  Array(ContainerInitializerDummy*, const_reference p0, const_reference p1, const_reference p2, const_reference p3, const_reference p4, const_reference p5)
    : mData(nullptr),
      mCapacity(0),
      mSize(0)
  {
    Reserve(6);
    PushBack(p0);
    PushBack(p1);
    PushBack(p2);
    PushBack(p3);
    PushBack(p4);
    PushBack(p5);
  }

  /// Destructor
  ~Array()
  {
    Deallocate();
  }

  /// Copy Assignment Operator
  void operator=(const this_type& other)
  {
    if(&other == this)
      return;

    Assign(const_cast<this_type*>(&other)->All());
  }

  /// Move Assignment Operator
  void operator=(MoveReference<this_type> other)
  {
    if(&other == this)
      return;

    Deallocate();

    mData     = other->mData;
    mCapacity = other->mCapacity;
    mSize     = other->mSize;

    other->ReleaseData();
  }

  /// Comparison Operators
  bool operator ==(const this_type& rhs) const
  {
    return Equal(this->All(), rhs.All());
  }
  bool operator !=(const this_type& rhs) const
  {
    return !(*this == rhs);
  }

  //
  // Element Information
  //

  /// Maximum number of elements before the internal buffer must be reallocated
  /// Adjusted automatically on insertion or manually with reserve and set_capacity
  size_type capacity() const { return mCapacity; }

  /// Returns true if the array Contains no elements, else false
  bool Empty() const { return mSize == 0; }

  /// Returns the number of elements in the array
  size_type Size() const { return mSize; }

  //
  // Element Access
  //

  /// Returns an iterator to the beginning of the array
  iterator       Begin()       { return mData; }
  const_iterator Begin() const { return mData; }

  /// Returns an iterator to the end of the array
  iterator       End()       { return mData + mSize; }
  const_iterator End() const { return mData + mSize; }

  /// Returns a range of all elements in the array
  range All()       { return range(Begin(), End());           }
  range All() const { return const_cast<Array*>(this)->All(); }

  /// Returns a pointer to the underlying data array
  pointer       Data()       { return mData; }
  const_pointer Data() const { return mData; }

  /// Returns the range of elements from index to index + length
  range SubRange(size_type index, size_type length)
  {
    ErrorIf(index + length > mSize, "Accessed array out of bounds.");
    return range(mData + index, mData + index + length);
  }
  range SubRange(size_type index, size_type length) const
  {
    return const_cast<Array*>(this)->SubRange(index, length);
  }

  /// Returns a reference to the element at the specified index
  reference operator[](size_type index)
  {
    ErrorIf(index >= mSize, "Accessed array out of bounds.");
    return mData[index];
  }
  const_reference operator[](size_type index) const
  {
    ErrorIf(index >= mSize, "Accessed array out of bounds.");
    return mData[index];
  }

  /// Returns a reference to the element at the front of the array
  reference Front()
  {
    ErrorIf(mSize == 0, "Empty array, no front element.");
    return mData[0];
  }
  const_reference Front() const
  {
    ErrorIf(mSize == 0, "Empty array, no front element.");
    return mData[0];
  }

  /// Returns a reference to the element at the back of the array
  reference Back()
  {
    ErrorIf(mSize == 0, "Empty array, no back element.");
    return mData[mSize - 1];
  }
  const_reference Back() const
  {
    ErrorIf(mSize == 0, "Empty array, no back element.");
    return mData[mSize - 1];
  }

  //
  // Element Modification
  //

  /// Swaps this array's internal data with another array
  void Swap(this_type& other)
  {
    Zero::Swap(mData, other.mData);
    Zero::Swap(mSize, other.mSize);
    Zero::Swap(mCapacity, other.mCapacity);
  }

  /// Constructs an element at the back of the array
  reference PushBack()
  {
    ExpandToNewSize(mSize+1);
    Construct(mData+mSize, typeIsPod());
    ++mSize;
    return *(mData+(mSize-1));
  }

  /// Copies an element to the back of the array
  void PushBack(const_reference item)
  {
    const_pointer toBeAdded = &item;
    if(mCapacity < mSize + 1)
    {
      //Is the item from this array.
      //This prevents errors with array.PushBack(array[0]);
      if(toBeAdded >= Begin() && toBeAdded < End())
      {
        size_type index = toBeAdded - Begin();
        ExpandToNewSize(mSize + 1);
        //mData is now the newly allocated array.
        //Find the value in the new array
        toBeAdded = mData + index;
      }
      else
      {
        ExpandToNewSize(mSize + 1);
      }
    }
    ConstructWith(mData + mSize, *toBeAdded);
    ++mSize;
  }

  /// Moves an element to the back of the array
  void PushBack(MoveReference<value_type> item)
  {
    pointer toBeAdded = &item;
    if(mCapacity < mSize + 1)
    {
      //Is the item from this array.
      //This prevents errors with array.PushBack(array[0]);
      if(toBeAdded >= Begin() && toBeAdded < End())
      {
        size_type index = toBeAdded - Begin();
        ExpandToNewSize(mSize + 1);
        //mData is now the newly allocated array.
        //Find the value in the new array
        toBeAdded = mData + index;
      }
      else
      {
        ExpandToNewSize(mSize + 1);
      }
    }
    ConstructWith(mData + mSize, ZeroMove(*toBeAdded));
    ++mSize;
  }

  /// Removes the element at the front of the array
  void PopFront()
  {
    Erase(mData);
  }

  /// Removes the element at the back of the array
  void PopBack()
  {
    ErrorIf(mSize == 0, "Empty array, can not pop back element.");
    Destroy(mData + mSize - 1, typePodDes());
    --mSize;
  }

  /// Changes the number of elements stored in the array
  /// Removes or default constructs elements at the back of the array as necessary
  void Resize(size_type newSize)
  {
    //Check to see if the size did not change
    if(mSize == newSize)
      return;
    else if(mSize < newSize)
    {
      //Expand memory for array
      Reserve(newSize);

      //Only construct on new elements
      //Elements from current size to new size
      UninitializedFill(mData + mSize, newSize - mSize, typePodCon());
    }
    else
    {
      //array shrinking 
      //Destroy elements from new size to old size
      DestroyElements(mData + newSize, mSize - newSize, typePodDes());
    }
    mSize = newSize;
  }

  /// Changes the number of elements stored in the array
  /// Removes or copy constructs elements at the back of the array as necessary
  void Resize(size_type newSize, const_reference defaultValue)
  {
    //Check to see if the size did not change
    if(mSize == newSize)
      return;
    else if(mSize < newSize)
    {
      //Expand memory for array
      Reserve(newSize);

      //Only construct on new elements
      //Elements from current size to new size
      UninitializedFill(mData + mSize, newSize - mSize, defaultValue);
    }
    else
    {
      //array shrinking 
      //Destroy elements from new size to old size
      DestroyElements(mData + newSize, mSize - newSize, typePodDes());
    }
    mSize = newSize;
  }

  /// Clears all elements from the array
  /// Effectively resizes the array to zero
  void Clear()
  {
    DestroyElements(mData, mSize, typePodDes());
    mSize = 0;
  }

  /// Reserves at least the specified element capacity
  void Reserve(size_type newCapacity)
  {
    if(mCapacity < newCapacity)
      ChangeCapacity(newCapacity);
  }

  /// Reserves exactly the specified element capacity
  void SetCapacity(size_type newCapacity)
  {
    if(newCapacity != mCapacity)
      ChangeCapacity(newCapacity);
  }

  /// Destroys all elements and frees reserved memory
  void Deallocate()
  {
    DestroyElements(mData, mSize, typePodDes());
    if(mCapacity)
      mAllocator.Deallocate(mData, sizeof(value_type)*mCapacity);
    mSize = 0;
    mCapacity = 0;
  }

  /// Takes control of a block of memory and treats it as a valid array
  void SetData(pointer memory, size_type size)
  {
    Deallocate();
    mSize = size;
    mCapacity = size;
    mData = memory;
  }

  /// Clears all data members, does not free reserved memory
  /// Allows external users to take control of the underlying array
  void ReleaseData()
  {
    mSize = 0;
    mCapacity = 0;
    mData = nullptr;
  }

  //
  // Insertion Operations
  //

  /// Clears the array and inserts a range of elements
  template<typename iteratorType>
  void Assign(iteratorType begin, iteratorType end)
  {
    Clear();
    Insert(0, BuildRange(begin,end));
  }
  template<typename inputRangeType>
  void Assign(inputRangeType range)
  {
    Clear();
    Insert(0, range);
  }
  template<typename inputRangeType>
  void Assign(MoveReference<inputRangeType> range)
  {
    Clear();
    Insert(0, ZeroMove(range));
  }

  /// Appends a range of elements to the end of the array
  template<typename inputRangeType>
  void Append(inputRangeType inputRange)
  {
    Insert(mData + mSize, inputRange);
  }
  template<typename inputRangeType>
  void Append(MoveReference<inputRangeType> inputRange)
  {
    Insert(mData + mSize, ZeroMove(inputRange));
  }

  /// Inserts a range of elements before the specified position in the array
  template<typename iteratorType>
  void Insert(pointer where, iteratorType begin, iteratorType end)
  {
    Insert(where, BuildRange(begin,end));
  }
  template<typename inputRangeType>
  void Insert(pointer where, inputRangeType inputRange)
  {
    if(where == nullptr && mSize == 0)
    {
      //Insert on empty container
      ChangeCapacity(inputRange.Length());
      where = mData;
    }

    ErrorIf(where < mData || where > mData + mSize, 
            "Access array out of bounds.");

    size_type elementsToInsert = inputRange.Length();

    //Expand this container to the new size
    pointer buffer = InsertExpandCapacity(where, elementsToInsert);

    for(; !inputRange.Empty(); inputRange.PopFront())
    {
      ConstructWith(buffer, inputRange.Front());
      ++buffer;
    }

    mSize += elementsToInsert;
  }
  template<typename inputRangeType>
  void Insert(pointer where, MoveReference<inputRangeType> inputRange)
  {
    if(where == nullptr && mSize == 0)
    {
      //Insert on empty container
      ChangeCapacity(inputRange->Length());
      where = mData;
    }

    ErrorIf(where < mData || where > mData + mSize, 
            "Access array out of bounds.");

    size_type elementsToInsert = inputRange->Length();

    //Expand this container to the new size
    pointer buffer = InsertExpandCapacity(where, elementsToInsert);

    for(; !inputRange->Empty(); inputRange->PopFront())
    {
      ConstructWith(buffer, ZeroMove(inputRange->Front()));
      ++buffer;
    }

    mSize += elementsToInsert;
  }
  template<typename inputRangeType>
  void InsertAt(size_type index, inputRangeType inputRange)
  {
    //for insertion the index can be mSize
    ErrorIf(index > mSize, "Access array out of bounds.");
    Insert(mData + index, inputRange);
  }
  template<typename inputRangeType>
  void InsertAt(size_type index, MoveReference<inputRangeType> inputRange)
  {
    //for insertion the index can be mSize
    ErrorIf(index > mSize, "Access array out of bounds.");
    Insert(mData + index, ZeroMove(inputRange));
  }

  /// Inserts an element before the specified position in the array
  void Insert(pointer where, const_reference value)
  {
    ErrorIf(where < mData || where > mData + mSize, 
            "Access array out of bounds.");
    ConstPointerRange<value_type> singleValue(&value, &value+1);
    Insert(where, singleValue);
  }
  void Insert(pointer where, MoveReference<value_type> value)
  {
    ErrorIf(where < mData || where > mData + mSize, 
            "Access array out of bounds.");
    PointerRange<value_type> singleValue(&value, &value+1);
    Insert(where, ZeroMove(singleValue));
  }
  void InsertAt(size_type index, const_reference value)
  {
    //for insertion the index can be mSize
    ErrorIf(index > mSize, "Access array out of bounds.");
    if(index == mSize)
      PushBack(value);
    else
      Insert(mData + index, value);
  }
  void InsertAt(size_type index, MoveReference<value_type> value)
  {
    //for insertion the index can be mSize
    ErrorIf(index > mSize, "Access array out of bounds.");
    if(index == mSize)
      PushBack(ZeroMove(value));
    else
      Insert(mData + index, ZeroMove(value));
  }

  //
  // Removal Operations
  //

  /// Removes the element at the specified position in the array
  void EraseAt(size_type index)
  {
    ErrorIf(index >= mSize, "Access array out of bounds.");
    EraseElements(mData + index, 1);
  }
  pointer Erase(pointer where)
  {
    ErrorIf(where < mData || where > mData + mSize, 
            "Access array out of bounds.");
    EraseElements(where, 1);
    if(!Empty())
      return where;//safe memory is not reallocated
    else
      return End();
  }

  /// Removes the sub-range of elements from their positions in the array
  void Erase(range elements)
  {
    EraseElements(elements.Begin(), elements.Length());
  }

  /// Removes the first equivalent element from the array
  /// Returns true if successful, else false (an equivalent element was not found)
  template<typename CompareType>
  bool EraseValue(const CompareType& value)
  {
    // Find the first equivalent element
    size_type index = FindIndex(value);
    if(index == InvalidIndex) // Unable?
      return false;

    // Erase the first equivalent element
    Erase(mData + index);
    return true;
  }
  /// Removes the first equivalent element from the array
  /// Generates an error if an equivalent element was not found
  template<typename CompareType>
  void EraseValueError(const CompareType& value)
  {
    // Remove the first equivalent element from the array
    bool result = EraseValue(value);
    if(!result) // Unable?
      Warn("Value not found in the array");
  }

  //
  // Search Operations
  //

  /// Returns the index of the first equivalent element from the array, else InvalidIndex
  template<typename CompareType>
  size_type FindIndex(const CompareType& value) const
  {
    for (size_type i = 0; i < mSize; ++i)
    {
      if (mData[i] == value)
        return i;
    }

    return InvalidIndex;
  }

  /// Returns an iterator to the first equivalent element in the array, else End()
  template<typename CompareType>
  iterator FindIterator(const CompareType& value)
  {
    // Find first instance
    size_type index = FindIndex(value);
    if(index == InvalidIndex) // Unable?
      return End();
    else
      return mData + index;
  }
  template<typename CompareType>
  const_iterator FindIterator(const CompareType& value) const
  {
    return const_cast<this_type*>(this)->FindIterator(value);
  }

  /// Returns a pointer to the first equivalent element in the array, else nullptr
  template<typename CompareType>
  pointer FindPointer(const CompareType& value) const
  {
    // Find first instance
    size_type index = FindIndex(value);
    if(index == InvalidIndex) // Unable?
      return nullptr;
    else
      return mData + index;
  }

  /// Returns true if the array Contains an equivalent element, else false
  template<typename CompareType>
  bool Contains(const CompareType& value) const
  {
    return FindIndex(value) != InvalidIndex;
  }

protected:
  /// Expand the array at where by numberOf elements and return a 
  /// pointer to where the new data will be inserted.
  /// When a large amount of data is inserted the array will
  /// often have to reallocated. Using the standard resize function will
  /// copy all the old elements over to the new array then Insert have to
  /// move them again. This function allocates new memory if necessary
  /// and copies the memory in front of and back of where in place. 
  /// It then returns the pointer for the block to Insert the new elements.
  pointer InsertExpandCapacity(pointer where, size_type numberOfElements)
  {
    size_type neededCapacity = numberOfElements + mSize ;
    if(neededCapacity > mCapacity)
    {
      //Need more room expand capacity
      size_type newCapacity = mCapacity + (mCapacity / 2);
      if(neededCapacity > newCapacity)
      {
        //Needed size is bigger then %50 expansion
        //just use the new size
        newCapacity = neededCapacity;
      }

      pointer newData = (pointer)mAllocator.Allocate(newCapacity*sizeof(value_type));
      size_type firstBlockSize = where - mData;
      size_type secondBlockSize = End() -  where;
      pointer startOfSecondBlock = newData+firstBlockSize+numberOfElements;
      pointer startOfFirstBlock = newData;

      if(mSize != 0)
      {
        //Copy over first block of data
        if(firstBlockSize != 0)
          UninitializedMove(startOfFirstBlock, mData, firstBlockSize, typePodMove());


        //Copy over second block of data
        if(secondBlockSize != 0)
        {
          UninitializedMove(startOfSecondBlock, mData+firstBlockSize, 
                             secondBlockSize, typePodMove());
        }
      }

      //Deallocate old memory
      if(mCapacity != 0)
        mAllocator.Deallocate(mData, sizeof(value_type)*mCapacity);


      mCapacity = newCapacity;
      mData = newData;

      return newData + firstBlockSize;

    }
    else
    {
      //There is enough room just move the elements after where outwards
      size_type elementsToMove = End() - where;

      //Watch out we have an overlapped write
      if(elementsToMove != 0)
      {
        UninitializedMoveRev(where + numberOfElements, where, elementsToMove, 
                              typePodMove());
      }
      return where;
    }
  }

  /// Removes the specified number of elements from the array starting at the given position
  void EraseElements(pointer where, size_type numberOfElements)
  {
    size_type elementsToMove = (End() - where) - numberOfElements;

    DestroyElements(where, numberOfElements, typeIsPod());

    if(elementsToMove > 0)
    {
      UninitializedMove(where, where + numberOfElements, elementsToMove, 
                         typePodMove());
    }

    mSize -= numberOfElements;
  }

  /// Expands the underlying array capacity as necessary to fit the specified new size
  void ExpandToNewSize(size_type newSize)
  {
    //Do not expand if new size fits in current capacity
    if(mCapacity < newSize)
    {
      //If capacity is not zero expand by %50 of capacity
      //if it is zero set it to 2
      size_type expandedSize = mCapacity != 0 ? mCapacity + (mCapacity / 2) : 2;
      if(expandedSize < newSize)
      {
        //%50 expansion was not enough space just expand to the new size
        ChangeCapacity(newSize);
      }
      else
      {
        //use the expanded size
        ChangeCapacity(expandedSize);
      }
    }
  }

  /// This function directly changes the capacity
  /// If the capacity is lowered it will destroy the old elements using resize
  void ChangeCapacity(size_type newCapacity)
  {
    //delete extra elements if necessary
    if(newCapacity < mSize)
      Resize(newCapacity);

    if(newCapacity == 0)
      return;

    //Allocate the new buffer size
    pointer newData = (pointer)mAllocator.Allocate(newCapacity*sizeof(value_type));

    //copy everything in mSize above. resize call above shrinks if necessary.
    if(mSize != 0)
      UninitializedMove(newData, mData, mSize, typePodMove());

    //Deallocate old memory if any allocated
    if(mCapacity != 0)
      mAllocator.Deallocate(mData, sizeof(value_type)*mCapacity);

    mCapacity = newCapacity;
    mData = newData;
  }

  /// Underlying array
  pointer   mData;
  /// Element capacity
  size_type mCapacity;
  /// Number of elements in the array
  size_type mSize;

  /// Friends
  template<typename T>
  friend struct MoveWithoutDestructionOperator;
};

/// PodArray is the same as Array except Pod conventions are forced on
template<typename ValueType, typename Allocator = DefaultAllocator>
class ZeroSharedTemplate PodArray : public Array<ValueType, Allocator, PodOverride>
{
public:
  typedef Array<ValueType, Allocator, PodOverride> base_type;
  typedef typename base_type::size_type size_type;

  PodArray()
    : base_type()
  {
  }

  explicit PodArray(size_type size)
    : base_type(size)
  {
  }
};

/// Array Move-Without-Destruction Operator
template<typename ValueType, typename Allocator, typename tt_traits>
struct MoveWithoutDestructionOperator<Array<ValueType, Allocator, tt_traits> >
{
  static inline void MoveWithoutDestruction(Array<ValueType, Allocator, tt_traits>* dest, 
                                            Array<ValueType, Allocator, tt_traits>* source)
  {
    dest->mData = source->mData;
    dest->mCapacity = source->mCapacity;
    dest->mSize = source->mSize;

    new (&dest->mAllocator) Allocator(source->mAllocator);
  }
};

} // namespace Zero
