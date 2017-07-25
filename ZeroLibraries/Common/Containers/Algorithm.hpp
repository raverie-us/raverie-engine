///////////////////////////////////////////////////////////////////////////////
///
/// \file Algorithm.hpp
/// Declaration of the basic algorithms for Containers.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "ContainerCommon.hpp"

namespace Zero
{

template<typename argument, typename result>
struct unary_function
{
  typedef argument argument_type;
  typedef result result_type;
};

template<typename argument0, typename argument1, typename result>
struct binary_function
{
  typedef argument0 first_argument_type;
  typedef argument1 second_argument_type;
  typedef result result_type;
};

#define StandardFunctor(name, operation)                    \
template<typename type>                                     \
struct name : public binary_function<type,type,bool>{       \
bool operator()(const type& left, const type& right) const{ \
return (left operation right);}};

StandardFunctor(equal_to, ==);
StandardFunctor(not_equal_to, !=);
StandardFunctor(less, <);
StandardFunctor(greater, >);
StandardFunctor(logical_and, &&);
StandardFunctor(logical_or, ||);

#undef StandardFunctor

// Linearly searches the input range and returns the range starting with either 
// the (value == searchValue) or an empty version of input range if value is not
// found.
template<typename inputRange, typename serachValueType>
inputRange Find(inputRange input, const serachValueType& searchValue)
{
  while(!input.Empty())
  {
    if(input.Front() == searchValue)
      return input;
    input.PopFront();
  }
  return input;
}

// Linearly searches the input range and returns the range starting with either 
// the Predicate(value) == true or an empty version of input range if value is not 
// found.
template<typename inputRange, typename Predicate>
inputRange FindIf(inputRange input, Predicate predicate)
{
  while(!input.Empty())
  {
    if(predicate(input.Front()))
      return input;
    input.PopFront();
  }
  return input;
}

template<typename inputRange, typename testInputRange>
inputRange FindFirstOf(inputRange input, testInputRange testInput)
{
  while(!input.Empty())
  {
    for(testInputRange test = testInput; !test.Empty(); test.PopFront())
    {
      if(input.Front() == test.Front())
        return input;
    }
    input.PopFront();
  }
  return input;
}


// Performs the unary operation on all elements in the range.
template<typename inputRange, typename unaryOperator>
void ForEach(inputRange input, unaryOperator op)
{
  while(!input.Empty())
  {
    op(input.Front());
    input.PopFront();
  }
}

// Transform all elements in input range using the unaryOperator and then place
// the result of the operation in outputRange.
template<typename inputRange, typename outputRange, typename unaryOperator>
void TransformRange(inputRange input, outputRange output, unaryOperator op)
{
  ErrorIf(output.Length() < input.Length(), "Output range is smaller than input range.");
  while(!input.Empty())
  {
    output.Front() = op(input.Front());
    output.PopFront();
    input.PopFront();
  }
}

// Copy values from one range to another
template<typename inputRange, typename outputRange>
void Copy(inputRange input, outputRange output)
{
  ErrorIf(output.Length() < input.Length(), "Output range is smaller than input range.");
  while(!input.Empty())
  {
    output.Front() = input.Front();
    output.PopFront();
    input.PopFront();
  }
}

// Set all values in range to source value
template<typename inputRange, typename fillType>
inline void Fill(inputRange range, const fillType& sourceValue)
{
  while(!range.Empty())
  {
    range.Front() = sourceValue;
    range.PopFront();
  }
}

// Check if two ranges are equal
template<typename inputRange0, typename inputRange1>
bool Equal(inputRange0 range0, inputRange1 range1)
{
  while(!range0.Empty() && !range1.Empty())
  {
    if(!(range0.Front() == range1.Front()))
      return false;
    range0.PopFront();
    range1.PopFront();
  }

  if(range0.Empty() && range1.Empty())
    return true;
  else
    return false;
}

// Insertion sort
template<typename iterator, typename Comparer, typename type>
void InsertionSort(iterator begin, iterator end, Comparer comparer, type* /*dummy*/)
{
  if(begin != end)
  {
    iterator iter = begin;
    ++iter;
    //Insert each element
    for(;iter != end; ++iter)
    {
      iterator Insert = iter;
      type test = *Insert;
      iterator prev = Insert;
      --prev;

      while(comparer(test, *prev))
      {
        //Move previous up
        *Insert = *prev;

        if(prev == begin)
        {
          --Insert;
          break;
        }

        --prev;
        --Insert;
      }

      //copy element into position
      *Insert = test;
    }
  }
}

// partition used in quick sort
template<typename iterator, typename Comparer, typename type> 
inline Pair<iterator, iterator> Partition(iterator first, iterator last, 
                                          Comparer comparer, type* /*dummy*/)
{
  size_t size = last - first;

  //Make last point to the last valid
  //element instead of the end
  --last;

  //make a pointer to the pivot
  iterator pivot = first;

  //Swap the pivot value to the front
  Zero::Swap(*pivot, *(first + size / 2));

  //Move first to the first valid element
  ++first;

  //Make sure the pivot is the median value (middle value) 
  //of the first, middle, and last elements. This prevents N^2 
  //performance on a already sorted list
  if(comparer(*last, *first))  Zero::Swap(*last, *first);
  if(comparer(*pivot, *first)) Zero::Swap(*pivot, *first);
  if(comparer(*last, *pivot))  Zero::Swap(*last, *pivot);

  //store the pivot value on the stack
  type pivotValue = *pivot;

  //keep looping while the iterators have not crossed
  while (first < last) 
  {
    //search from the left for a value greater
    //than the pivot
    do ++first; 
    while(comparer(*first, pivotValue));

    //search from the right for a value
    //less than the pivot
    do --last; 
    while(comparer(pivotValue, *last));

    //If the two value are not the middle
    if (first < last) 
    {
      Zero::Swap(*first, *last);
    }
  };

  //Move pivot back into position
  Zero::Swap(*pivot, *last);

  //return the last and first of the two new ranges.
  return Pair<iterator, iterator>(last, first);
}

const size_t cSortMaxStackSize = 40;
const size_t cSortLimit = 32;

// primary quick sort algorithm
template<typename iterator, typename Comparer, typename type>
void QuickSort(iterator begin, iterator end, type* dummy, Comparer comparer)
{
  size_t stackSize = 0;

  // Instead of using recursion quick sort uses a static array
  // for performance
  Pair<iterator, iterator> rangeStack[cSortMaxStackSize];

  for(;;)
  {
    size_t rangeSize = end - begin;

    // Use quick sort for larger arrays
    if(rangeSize > cSortLimit)
    {
      Pair<iterator, iterator> division = Partition(begin, end, comparer, dummy);

      // push the larger division for later
      // make the smaller division the current begin and end
      if(division.first - begin > end - division.second)
      {
        // left side larger
        rangeStack[stackSize] = MakePair(begin, division.first);
        ++stackSize;

        begin = division.second;
        // end stays the same
      }
      else
      {
        // right side larger
        rangeStack[stackSize] = MakePair(division.second, end);
        ++stackSize;

        // begin stays the same
        end = division.first;
      }

    }
    else
    {
      // Perform insertion sort on smaller range

      // if range is one element skip sorting
      if(rangeSize > 1)
      {
        InsertionSort(begin, end, comparer, dummy);
      }

      // Pop the current range moving back
      if(stackSize > 0)
      {
        --stackSize;
        begin = rangeStack[stackSize].first;
        end = rangeStack[stackSize ].second;
      }
      else
      {
        // no more ranges sort is complete
        return;
      }
    }
  }
}

template<typename range,typename type>
void SortWithLess(range r, type* dummy)
{
  QuickSort(r.Begin(), r.End(), dummy, less<type>());
}

template<typename range>
void Sort(range r)
{
  //if you see an error on this line, odds are you passed in an array instead of array.All()
  size_t temp = sizeof(typename range::contiguousRangeType);
  SortWithLess(r, &r.Front());
}

template<typename range, typename Comparer>
void Sort(range r, Comparer comparer)
{
  //if you see an error on this line, odds are you passed in an array instead of array.All()
  size_t temp = sizeof(typename range::contiguousRangeType);
  UnusedParameter(temp);
  QuickSort(r.Begin(), r.End(), &r.Front(), comparer);
}

template<typename iterator>
void Reverse(iterator start, iterator end)
{
  --end;
  while(start<end)
  {
    Zero::Swap(*start, *end);
    ++start;
    --end;
  }
}

template<typename rangeType>
bool IsSorted(rangeType range)
{
  if(!range.Empty())
  {
    rangeType r0 = range;
    rangeType r1 = range;
    r1.PopFront();
    while(!r1.Empty())
    {
      if(r1.Front()  < r0.Front())
        return false;
      r0.PopFront();
      r1.PopFront();
    }
  }
  return true;
}


template<typename rangeType, typename searchType, typename valueType>
valueType& BinarySearch(rangeType& range, const searchType& searchValue, valueType& valueIfNotFound)
{
  size_t begin = 0;
  size_t end = range.Size();

  while(begin < end)
  {
    size_t mid = (begin+end) / 2;
    if(range[mid] < searchValue)
    {
      begin = mid + 1;
    }
    else
    {
      end = mid;
    }
  }

  if((begin < range.Size()) && (range[begin] == searchValue))
  {
    return range[begin];
  }
  else
  {
    return valueIfNotFound;
  }
}

/// Performs a binary search to find the lower bound of the specified value in the range
/// Returns an iterator to the first value in the range that is greater-or-equal (predicate true) to the specified value, else end
template<typename rangeType, typename valueType, typename predicate>
rangeType LowerBound(rangeType r, const valueType& value, predicate pred)
{
  rangeType newRange = r;
  int count = newRange.Length();
  while(count > 0)
  {
    int step = count / 2;
    typename rangeType::reference newValue = newRange[step];
    if( pred(newValue, value) )
    {
      newRange.mBegin = newRange.mBegin + step + 1;
      count -= step + 1;
    }
    else
      count = step;
  }
  return newRange;
}
template<typename iteratorType, typename valueType, typename predicate>
iteratorType LowerBound(iteratorType begin, iteratorType end, const valueType& value, predicate pred)
{
  return LowerBound(BuildRange(begin, end), value, pred).Begin();
}

/// Performs a binary search to find the upper bound of the specified value in the range
/// Returns an iterator to the first value in the range that is greater (predicate false) than the specified value, else end
template<typename rangeType, typename valueType, typename predicate>
rangeType UpperBound(rangeType r, const valueType& value, predicate pred)
{
  rangeType newRange = r;
  int count = newRange.Length();
  while(count > 0)
  {
    int step = count / 2;
    typename rangeType::reference newValue = newRange[step];
    if( !pred(value, newValue) )
    {
      newRange.mBegin = newRange.mBegin + step + 1;
      count -= step + 1;
    }
    else
      count = step;
  }
  return newRange;
}
template<typename iteratorType, typename valueType, typename predicate>
iteratorType UpperBound(iteratorType begin, iteratorType end, const valueType& value, predicate pred)
{
  return UpperBound(BuildRange(begin, end), value, pred).Begin();
}

/// Returns the lower and upper bound of the specified value in the range
template<typename rangeType, typename valueType, typename predicate>
rangeType LowerAndUpperBound(rangeType r, const valueType& value, predicate pred)
{
  return BuildRange( LowerBound(r, value, pred).Begin(),
                     UpperBound(r, value, pred).Begin() );
}

template<typename rangeType, typename Predicate>
rangeType Search(rangeType searchRange, rangeType toFind, Predicate predicate)
{
  size_t findSize = toFind.SizeInBytes();
  while(searchRange.SizeInBytes() >= findSize)
  {
    rangeType r0 = searchRange;
    rangeType find = toFind;
    while(!r0.Empty() && predicate(r0.Front(), find.Front()))
    {
      r0.PopFront();
      find.PopFront();

      if(find.Empty())
        return searchRange;
    }

    searchRange.PopFront();
  }

  //make the range empty
  searchRange.PopFront(searchRange.ComputeRuneCount());

  return searchRange;
}

template<typename rangeType> 
rangeType Search(rangeType searchRange, rangeType toFind)
{
  return Search(searchRange, toFind, equal_to<typename rangeType::value_type>());
}

// Inserts a value into a sorted list
// return true if value was inserted false if
// value was already in the list
template<typename containertype, typename type>
bool SortedInsert(containertype& container, type& value)
{
  typename containertype::iterator cur = container.Begin();
  typename containertype::iterator end = container.End();
  while(cur != end)
  {
    type& currentValue = *cur;
    if(value < currentValue)
    {
      // value is less than current value
      // Insert the value at this location
      container.Insert(cur, value);
      return true;
    }
    else if(value == currentValue)
    {
      // already in list
      // failed to Insert
      return false;
    }
    ++cur;
  }

  //Insert at end
  container.PushBack(value);
  return true;
}


//Array Algorithms


template<typename desttype, typename range>
void PushAll(desttype& a, range inputRange )
{
  for(;!inputRange.Empty();inputRange.PopFront())
  {
    a.PushBack(inputRange.Front());
  }
}


template<typename type>
struct EqualTo
{
  EqualTo(type& value)
    :TestValue(value)
  {
  }

  type TestValue;

  bool operator()(type& any)
  {
    return TestValue == any;
  }
};

const size_t NotFoundIndex = (size_t)-1;

// Find the index of the first value that passes the predicate
// or NotFoundIndex when not found
template<typename ArrayType, typename Predicate>
size_t FindFirstIndex(ArrayType& array, Predicate predicate)
{
  // Loop through elements
  size_t size = array.Size();
  for (size_t i = 0; i < size; ++i)
  {
    // Found value return
    if(predicate(array[i]))
      return i;
  }
  return NotFoundIndex;
}

// Remove all elements that do not pass Predicate
template<typename ArrayType, typename Predicate>
size_t RemoveAll(ArrayType& array, Predicate predicate)
{
  // Loop through all elements in the array
  // Copying over valid elements 
  size_t sourceIndex = 0;
  size_t destIndex = 0;
  size_t size = array.Size();
  size_t removeCount = 0;

  for(; sourceIndex < size;)
  {
    if(predicate(array[sourceIndex]))
    {
      // If not valid just move forward source
      ++sourceIndex;
      ++removeCount;
    }
    else
    {
      // Need to keep value in array

      // Do not copy if already in place
      if(sourceIndex != destIndex)
      {
        array[destIndex] = array[sourceIndex];
      }

      // Move both forward
      ++destIndex;
      ++sourceIndex;
    }
  }

  // get rid of unused elements at the end
  array.Resize(destIndex);
  return removeCount;
}

// Remove a element placing last element in its place
// to prevent extra copying
template<typename ArrayType>
void RemoveSwap(ArrayType& array, size_t index)
{
  array[index] = array.Back();
  array.PopBack();
}

}//namespace Zero
