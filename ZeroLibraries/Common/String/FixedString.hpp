///////////////////////////////////////////////////////////////////////////////
///
/// \file FixedString.hpp
/// Declaration of FixedArray, FixedString, and String Range.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "StringRange.hpp"

namespace Zero
{

template<typename type, size_t maxSize>
class FixedArray
{
public:
  typedef type value_type;
  typedef value_type* pointer;
  typedef const value_type* const_pointer;
  typedef value_type& reference;
  typedef const value_type& const_reference;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef FixedArray<type, maxSize> this_type;
  typedef ConstPointerRange<type> range;

  explicit FixedArray()
    : mSize(0)
  {}

  explicit FixedArray(size_type size)
    : mSize(size)
  {}

  FixedArray(pointer data, size_type size)
    : mSize(size)
  {
    if(mSize > maxSize) mSize = maxSize;
    UninitializedCopy(mData, data, mSize, true_type());
  }

  range All() { return range(mData, mSize); }
  bool Empty()const { return mSize == 0; }
  const_pointer Data() const { return mData; }
  size_type Size() const { return mSize; }
  size_type capacity() const { return maxSize; }

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

  void PushBack(type& element)
  {
    ErrorIf(mSize>=maxSize, "Array at max size");
    mData[mSize] = element;
    ++mSize;
  }

  type& PushBack()
  {
    ErrorIf(mSize>=maxSize, "Array at max size");
    ++mSize;
    return *(mData+(mSize-1));
  }

  //Removes the last element in the array.
  void PopBack()
  {
    ErrorIf(mSize == 0,"Empty array, can not pop back element.");
    --mSize;
  }

  void EraseIndex(uint index)
  {
    ErrorIf(index>mSize, "Index out of bounds.");
    //Shift over values
    uint lastValid = mSize-1;
    for(uint i=index;index<lastValid;++i)
      mData[i] = mData[i+1];
    //decrement size
    --mSize;
  }

  //simple erase value
  void EraseValue(type& value)
  {
    for(uint i=0;i<mSize;++i)
    {
      if(mData[i] == value)
      {
        //value found remove it
        eraseIndex(i);
        return;
      }
    }
  }

protected:
  type mData[maxSize];
  size_type mSize;
};

template<size_t maxSize>
class ZeroSharedTemplate FixedString : public FixedArray<char, maxSize>
{
public:
  typedef FixedArray<char, maxSize> base_type;
  typedef FixedString<maxSize> this_type;
  typedef typename base_type::const_pointer const_pointer;
  typedef typename base_type::size_type size_type;
  
  typedef StringRange range;
  range All()
  { 
    return StringRange(this->mData, this->mData, this->mData + this->mSize);
  }

  FixedString()
    : base_type(0)
  {
    memset(this->mData, 0, maxSize);
  }

  explicit FixedString(const_pointer cstring)
  {
    Assign(cstring, strlen(cstring));
  }

  explicit FixedString(range r)
  {
    Assign(r.Data(), r.SizeInBytes());
  }

  template<size_t otherSize>
  explicit FixedString(const FixedString<otherSize>& fstring)
  {
    Assign(fstring.Data(), fstring.Size());
  }

  void operator=(const_pointer cstring)
  {
    Assign(cstring, strlen(cstring));
  }

  void operator=(range r)
  {
    Assign(r.Data(), r.SizeInBytes());
  }

  void Assign(const_pointer cstring, size_type size)
  {
    memset(this->mData, 0, maxSize);

    if (size > maxSize - 1)
    {
      Error("String is not large enough for range.");
      size = maxSize - 1;
    }
    
    this->mSize = size;
    //need room for null terminator
    if(this->mSize > (maxSize - 1))
      this->mSize = maxSize - 1;

    memcpy(this->mData, cstring, this->mSize);
    this->mData[this->mSize] = '\0';
  }

  template<size_t otherSize>
  friend inline bool operator==(const this_type& left,
                                const FixedString<otherSize>& right)
  {
    return strcmp(left.Data(), right.Data()) == 0;
  }

  template<size_t otherSize>
  friend inline bool operator!=(const this_type& left,
    const FixedString<otherSize>& right)
  {
    return !(left == right);
  }

  template<size_t otherSize>
  friend inline bool operator<(const this_type& left, 
                               const FixedString<otherSize>& right)
  {
    return strcmp(left.Data(), right.Data()) < 0;
  }

  const_pointer c_str()const{return this->mData;}
};

template<size_t s>
inline bool operator==(const FixedString<s>& left, cstr right)
{
  return strcmp(left.Data(), right) == 0;
}

template<size_t s>
inline bool operator!=(const FixedString<s>& left, cstr right)
{
  return strcmp(left.Data(), right) != 0;
}

template<size_t s>
inline bool operator==(const FixedString<s>& left, const String& right)
{
  return strcmp(left.Data(), right.Data()) == 0;
}

template<size_t s>
inline bool operator==(const String& left, const FixedString<s>& right)
{
  return strcmp(left.Data(), right.Data()) == 0;
}

template<size_t s>
inline bool operator==(const FixedString<s>& left, const StringRange& right)
{
  return strcmp(left.Data(), right.Data()) == 0;
}

template<size_t s>
inline bool operator==(const StringRange& left, const FixedString<s>& right)
{
  return strcmp(left.Data(), right.Data()) == 0;
}

}//namespace Zero
