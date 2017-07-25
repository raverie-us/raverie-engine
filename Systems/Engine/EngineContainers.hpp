///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

DeclareEnum2(Verbosity, Minimal, Detailed);

typedef HashMap<String, String> StringMap;

//----------------------------------------------------------- Cast Range Adapter
template<typename rangeType, typename castType>
class CastRangeAdapter
{
public:
  typedef castType FrontResult;

  CastRangeAdapter(rangeType r)
  {
    mRange = r;
  }

  CastRangeAdapter(const CastRangeAdapter& rhs)
  {
    mRange = rhs.mRange;
  }

  void PopFront()
  {
    mRange.PopFront();
  }

  castType Front()
  {
    return static_cast<castType>(mRange.Front());
  }

  bool Empty()
  {
    return mRange.Empty();
  }

  rangeType mRange;
};

//-------------------------------------------------------------------- ArrayMultiMap
template<typename KeyType, typename DataType>
class ArrayMultiMap
{
public:
  template<typename pairType>
  struct SortByFirst
  {
    bool operator()(const pairType& left, const pairType& right)
    {
      return left.first < right.first;
    }
  };
  
  typedef KeyType key_type;
  typedef DataType data_type;
  typedef ArrayMultiMap<KeyType, DataType> this_type;
  typedef Pair<KeyType, DataType> value_type;
  typedef Array<value_type> ArrayType;
  typedef size_t size_type;
  typedef data_type& reference;
  typedef typename Array<value_type>::range irange;
  
  class range
  {
  public:
    typedef value_type& FrontResult;

    range(irange r) : mR(r) {}
    irange mR;
    value_type& Front() { return mR.Front(); }
    void PopFront() { mR.PopFront(); }
    bool Empty() { return mR.Empty(); }
  };
  
  class valueRange
  {
  public:
    // Used for binding the range to script
    typedef data_type value_type;
    typedef data_type& FrontResult;

    valueRange(irange r = irange()) : mR(r) {}
    irange mR;
    data_type& Front() { return mR.Front().second; }
    void PopFront() { mR.PopFront(); }
    bool Empty() { return mR.Empty(); }
  };
  
  value_type& operator[](uint index)
  {
    return mArray[index];
  }
  
  range All()
  {
    return range(mArray.All());
  }
  
  valueRange AllValues()
  {
    return valueRange(mArray.All());
  }
  
  void Insert(const value_type& datapair)
  {
    mArray.PushBack(datapair);
    Sort(mArray.All(), SortByFirst<value_type>());
  }
  
  void Insert(const KeyType& key, const DataType& value)
  {
    Insert(value_type(key, value));
  }
  
  void EraseEqualValues(const DataType& value)
  {
    for(uint i = 0; i < mArray.Size();)
    {
      if(mArray[i].second == value)
        mArray.Erase(&mArray[i]);
      else
        ++i;
    }
  
    Sort(mArray.All(), SortByFirst<value_type>() );
  }
  
  void Reserve(size_t size)
  {
    mArray.Reserve(size);
  }
  
  void Erase(const KeyType& key)
  {
    size_t index = FindApproxIndex(key);
  
    if((index < mArray.Size()) && (mArray[index].first == key))
    {
      mArray.EraseAt(index);
    }
    else
    {
      Error("Trying to erase something that is not there.");
    }
  }
  
  bool Empty()
  {
    return mArray.Empty();
  }
  
  void Clear()
  {
    mArray.Clear();
  }
  
  template<typename searchKeyType>
  DataType FindValue(const searchKeyType& key, const data_type& valueIfNotFound)
  {
    DataType* val = FindPointer(key);
    if(val != nullptr)
      return *val;
    return valueIfNotFound;
  }
  
  //Returns a pointer to the value if found, or null if not found
  template<typename searchKeyType>
  DataType* FindPointer(const searchKeyType& key)
  {
    size_t index = FindApproxIndex(key);
  
    if((index < mArray.Size()) && (mArray[index].first == key))
      return &mArray[index].second;
  
    return nullptr;
  }

  template<typename searchKeyType>
  valueRange FindAll(const searchKeyType& key)
  {
    // Get the lower bound
    size_t lowerBound = FindLowerBound(key);
    
    if(lowerBound == mArray.Size())
      return valueRange();

    // Get the upper bound
    size_t upperBound = GetUpperBoundFromLower(lowerBound, key);

    return valueRange(mArray.SubRange(lowerBound, upperBound - lowerBound));
  }

  template<typename searchKeyType>
  size_t FindLowerBound(const searchKeyType& key)
  {
    size_t approxIndex = FindApproxIndex(key);
  
    if(approxIndex < mArray.Size() && mArray[approxIndex].first != key)
      return mArray.Size();
  
    return approxIndex;
  }
  
  template<typename searchKeyType>
  size_t GetUpperBoundFromLower(const size_t lowerBound, const searchKeyType& key)
  {
    // Walk right to get the upper bound
    size_t upperBound = lowerBound;
    while(upperBound < mArray.Size() && mArray[upperBound].first == key)
      ++upperBound;
  
    return upperBound;
  }
  
  template<typename searchKeyType>
  size_t FindUpperBound(const searchKeyType& key)
  {
    size_t lowerBound = FindLowerBound(key);
    return GetUpperBoundFromLower(lowerBound);
  }
  
  template<typename searchKeyType>
  size_t FindApproxIndex(const searchKeyType& key)
  {
    int begin = 0;
    int end = (int)mArray.Size();
  
    while(begin < end)
    {
      size_t mid = (begin+end) / 2;
      if(mArray[mid].first < key)
      {
        begin = mid + 1;
      }
      else
      {
        end = mid;
      }
    }
  
    return begin;
  }
  
  size_t FindIndex(const DataType& value)
  {
    for(uint i = 0; i < mArray.Size(); ++i)
    {
      if(mArray[i].second == value)
        return i;
    }
  
    return Array<value_type>::InvalidIndex;
  }

  bool Contains(const DataType& value)
  {
    return FindIndex(value) != Array<value_type>::InvalidIndex;
  }
  
  uint Size()
  {
    return mArray.Size();
  }
  
  value_type& Front()
  {
    return mArray.Front();
  }
  
  value_type& Back()
  {
    return mArray.Back();
  }
  
  ArrayType mArray;
};

namespace Serialization
{

template<typename keytype, typename valuetype>
struct Trait< ArrayMultiMap<keytype, valuetype> >
{
  enum {Type=StructureType::Array};
  static inline cstr TypeName() { return "Array"; }
};

template<typename keytype, typename valuetype>
struct Policy< ArrayMultiMap<keytype, valuetype> >
{
  typedef ArrayMultiMap<keytype, valuetype> containertype;
  static inline bool Serialize(Serializer& serializer, cstr fieldName, 
                               containertype& container)
  {
    return SerializeSequenceInsert(serializer, fieldName, container);
  }
};

}//namespace Serialization

}//namespace Zero
