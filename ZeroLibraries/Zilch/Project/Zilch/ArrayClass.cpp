/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  // Unfortunately because there's some sort of bug in the MSVC linker, we have to make a bunch of non-inlined comparison functions
  ZilchNoInline bool LinkerEquals(Boolean         a, Boolean          b) { return a == b; }
  ZilchNoInline bool LinkerEquals(Boolean2Param   a, Boolean2Param    b) { return a == b; }
  ZilchNoInline bool LinkerEquals(Boolean3Param   a, Boolean3Param    b) { return a == b; }
  ZilchNoInline bool LinkerEquals(Boolean4Param   a, Boolean4Param    b) { return a == b; }
  ZilchNoInline bool LinkerEquals(Integer         a, Integer          b) { return a == b; }
  ZilchNoInline bool LinkerEquals(Integer2Param   a, Integer2Param    b) { return a == b; }
  ZilchNoInline bool LinkerEquals(Integer3Param   a, Integer3Param    b) { return a == b; }
  ZilchNoInline bool LinkerEquals(Integer4Param   a, Integer4Param    b) { return a == b; }
  ZilchNoInline bool LinkerEquals(Real            a, Real             b) { return a == b; }
  ZilchNoInline bool LinkerEquals(Real2Param      a, Real2Param       b) { return a == b; }
  ZilchNoInline bool LinkerEquals(Real3Param      a, Real3Param       b) { return a == b; }
  ZilchNoInline bool LinkerEquals(Real4Param      a, Real4Param       b) { return a == b; }
  ZilchNoInline bool LinkerEquals(QuaternionParam a, QuaternionParam  b) { return a == b; }
  ZilchNoInline bool LinkerEquals(DoubleInteger   a, DoubleInteger    b) { return a == b; }
  ZilchNoInline bool LinkerEquals(DoubleReal      a, DoubleReal       b) { return a == b; }
  ZilchNoInline bool LinkerEquals(const Handle&   a, const Handle&    b) { return a == b; }
  ZilchNoInline bool LinkerEquals(const Delegate& a, const Delegate&  b) { return a == b; }
  ZilchNoInline bool LinkerEquals(const Any&      a, const Any&       b) { return a == b; }
  
  //***************************************************************************
  ArrayUserData::ArrayUserData() :
    ContainedType(nullptr),
    RangeType(nullptr),
    SelfType(nullptr)
  {
  }
  
  //***************************************************************************
  // Enum comparison mode for array sorting
  DeclareEnum2(ComparisonMode, BoolMode, CompareMode);

  //***************************************************************************
  // Forward declaration of the range template
  template <typename T>
  class ArrayRangeTemplate;
  
  //***************************************************************************
  // The template layout we use for arrays
  // As an optimization, the array can be instantiated for some known data types
  // For all other unknown types (such as structs created in Zilch) we use the 'Any' type
  template <typename T>
  class ArrayTemplate : public ArrayClass<T>
  {
  public:
    // Get the number of elements in the array
    Integer GetCount()
    {
      return (Integer)this->NativeArray.Size();
    }

    //***************************************************************************
    static String ArrayToString(const BoundType* type, const byte* data)
    {
      // Read the element size from the current function's user-data
      ArrayUserData& userData = type->ComplexUserData.ReadObject<ArrayUserData>(0);

      // Create a string builder to generate the array entries
      StringBuilder builder;
      builder.Append(Grammar::GetKeywordOrSymbol(Grammar::BeginInitializer));

      // Grab the generic data as our own self template
      ArrayTemplate* self = (ArrayTemplate*)data;
    
      // Loop through all entries in the array
      for (size_t i = 0; i < self->NativeArray.Size(); ++i)
      {
        // Get a pointer to the value at the given index (as a byte*)
        byte* valuePointer = (byte*)&self->NativeArray[i];

        // Convert that value to a string generically
        String valueString = userData.ContainedType->GenericToString(valuePointer);

        // Append the stringified value to the builder
        builder.Append(valueString);

        // If we're not the last element, add a comma (argument separator) and a space
        bool isNotLastItem = (self->NativeArray.Size() - 1 != i);
        if (isNotLastItem)
        {
          builder.Append(Grammar::GetKeywordOrSymbol(Grammar::ArgumentSeparator));
          builder.Append(" ");
        }
      }

      builder.Append(Grammar::GetKeywordOrSymbol(Grammar::EndInitializer));

      String result = builder.ToString();
      return result;
    }

    //***************************************************************************
    // Store 'Any' value as return value
    static void ArrayCopyReturnValue(Call& call, const T& value)
    {
      // Get a pointer to the return value data (on the stack)
      byte* returnValue = call.GetReturnUnchecked();
      call.DisableReturnChecks();

      // Generically copy the contained type to the return value
      CopyFromAnyOrActualType<T>(value, returnValue);
    }

    //***************************************************************************
    static T ArrayReadValue(Call& call, ArrayTemplate* self, Integer parameter)
    {
      // Read the element size from the current function's user-data
      ArrayUserData& userData = call.GetFunction()->ComplexUserData.ReadObject<ArrayUserData>(0);

      // Get the value
      byte* valueData = call.GetParameterUnchecked(parameter);

      // Grab the data out generically (if this is an Any type, we handle that properly)
      return CopyToAnyOrActualType<T>(valueData, userData.ContainedType);
    }

    //***************************************************************************
    static void ArrayGet(Call& call, ExceptionReport& report)
    {
      // Read the element size from the current function's user-data
      ArrayUserData& userData = call.GetFunction()->ComplexUserData.ReadObject<ArrayUserData>(0);

      // Get ourselves (the array)
      ArrayTemplate* self = (ArrayTemplate*)call.GetHandle(Call::This).Dereference();

      // Read the array index
      Integer index = call.Get<Integer>(0);

      // Check the array index
      if (index < 0 || index >= self->GetCount())
      {
        call.GetState()->ThrowException(report, "Array index was out of bounds");
        return;
      }

      // Return the value
      ArrayCopyReturnValue(call, self->NativeArray[index]);
    }

    //***************************************************************************
    static void ArraySet(Call& call, ExceptionReport& report)
    {
      // Read the element size from the current function's user-data
      ArrayUserData& userData = call.GetFunction()->ComplexUserData.ReadObject<ArrayUserData>(0);

      // Get ourselves (the array)
      ArrayTemplate* self = (ArrayTemplate*)call.GetHandle(Call::This).Dereference();

      // Read the array index
      Integer index = call.Get<Integer>(0);
    
      // Check the array index
      if (index < 0 || index >= self->GetCount())
      {
        call.GetState()->ThrowException(report, "Array index was out of bounds");
        return;
      }

      // Place in array
      self->NativeArray[index] = ArrayReadValue(call, self, 1);
      self->Modified();
    }

    //***************************************************************************
    static void ArrayPush(Call& call, ExceptionReport& report)
    {
      // Get ourselves (the array)
      ArrayTemplate* self = (ArrayTemplate*)call.GetHandle(Call::This).Dereference();

      // Place in array
      self->NativeArray.PushBack(ArrayReadValue(call, self, 0));
      self->Modified();
    }

    //***************************************************************************
    static void ArrayInsert(Call& call, ExceptionReport& report)
    {
      // Get ourselves (the array)
      ArrayTemplate* self = (ArrayTemplate*)call.GetHandle(Call::This).Dereference();

      // Read the array index
      Integer index = call.Get<Integer>(0);

      // Check the array index, inserting at count is valid
      if (index < 0 || index > self->GetCount())
      {
        call.GetState()->ThrowException(report, "Array index was out of bounds");
        return;
      }

      // Place in array
      self->NativeArray.InsertAt(index, ArrayReadValue(call, self, 1));
      self->Modified();
    }

    //***************************************************************************
    static void ArrayPop(Call& call, ExceptionReport& report)
    {
      // Read the element size from the current function's user-data
      ArrayUserData& userData = call.GetFunction()->ComplexUserData.ReadObject<ArrayUserData>(0);

      // Get ourselves (the array)
      ArrayTemplate* self = (ArrayTemplate*)call.GetHandle(Call::This).Dereference();

      // Check that the array has elements
      if (self->GetCount() == 0)
      {
        call.GetState()->ThrowException(report, "Cannot pop from an empty array");
        return;
      }

      // This function returns the popped value so copy it to return value before removal
      ArrayCopyReturnValue(call, self->NativeArray.Back());

      //  Remove the element
      self->NativeArray.PopBack();
      self->Modified();
    }

    //***************************************************************************
    static void ArrayRemoveAt(Call& call, ExceptionReport& report)
    {
      // Get ourselves (the array)
      ArrayTemplate* self = (ArrayTemplate*)call.GetHandle(Call::This).Dereference();

      // Get the index we're trying to remove at
      Integer index = call.Get<Integer>(0);

      // Check the array index
      if (index < 0 || index >= self->GetCount())
      {
        call.GetState()->ThrowException(report, "Array index was out of bounds");
        return;
      }

      // Erase the value
      self->NativeArray.EraseAt(index);
      self->Modified();
    }

    //***************************************************************************
    static void ArrayRemoveSwap(Call& call, ExceptionReport& report)
    {
      // Get ourselves (the array)
      ArrayTemplate* self = (ArrayTemplate*)call.GetHandle(Call::This).Dereference();

      // Get the index we're trying to remove at
      Integer index = call.Get<Integer>(0);

      // Check the array index
      if (index < 0 || index >= self->GetCount())
      {
        call.GetState()->ThrowException(report, "Array index was out of bounds");
        return;
      }

      RemoveSwap(self->NativeArray, index);
      self->Modified();
    }

    //***************************************************************************
    static void ArrayClear(Call& call, ExceptionReport& report)
    {
      // Get ourselves (the array)
      ArrayTemplate* self = (ArrayTemplate*)call.GetHandle(Call::This).Dereference();

      self->NativeArray.Clear();
      self->Modified();
    }

    //***************************************************************************
    static void ArrayCopy(Call& call, ExceptionReport& report)
    {
      // Get ourselves (the array)
      ArrayTemplate* self = (ArrayTemplate*)call.GetHandle(Call::This).Dereference();

      ArrayUserData& userData = call.GetFunction()->ComplexUserData.ReadObject<ArrayUserData>(0);

      // Create the new array
      Handle arrayHandle = call.GetState()->AllocateDefaultConstructedHeapObject(userData.SelfType, report, HeapFlags::ReferenceCounted);

      // If we threw an exception, we need to early out and let the stack unroll
      if (report.HasThrownExceptions())
        return;

      // Get pointer to the new array
      ArrayTemplate* newArray = (ArrayTemplate*)arrayHandle.Dereference();

      // Copy array data
      newArray->NativeArray = self->NativeArray;

      // Write out the count
      call.SetHandle(Call::Return, arrayHandle);
    }

    //***************************************************************************
    static void ArrayCount(Call& call, ExceptionReport& report)
    {
      // Get ourselves (the array)
      ArrayTemplate* self = (ArrayTemplate*)call.GetHandle(Call::This).Dereference();
    
      // Write out the count
      call.Set(Call::Return, self->GetCount());
    }

    //***************************************************************************
    static void ArrayCapacity(Call& call, ExceptionReport& report)
    {
      // Get ourselves (the array)
      ArrayTemplate* self = (ArrayTemplate*)call.GetHandle(Call::This).Dereference();
    
      // Write out the capacity
      call.Set(Call::Return, (Integer)self->NativeArray.capacity());
    }

    //***************************************************************************
    static void ArrayReserve(Call& call, ExceptionReport& report)
    {
      // Get ourselves (the array)
      ArrayTemplate* self = (ArrayTemplate*)call.GetHandle(Call::This).Dereference();
    
      // Get the first argument, capacity
      Integer capacity = call.Get<Integer>(0);

      // Reserve space on the array (setting to anything smaller than the current capacity is ignored)
      self->NativeArray.Reserve(capacity);
    }

    //***************************************************************************
    static void ArrayResizeHelper(Call& call, ExceptionReport& report, byte* defaultValue)
    {
      // Get the user data, because we need to know the template types
      ArrayUserData& userData = call.GetFunction()->ComplexUserData.ReadObject<ArrayUserData>(0);

      // Get ourselves (the array)
      ArrayTemplate* self = (ArrayTemplate*)call.GetHandle(Call::This).Dereference();
    
      // Get the new size of the array (first argument)
      Integer newSize = call.Get<Integer>(0);

      // If the user attempted to pass in a negative value...
      if (newSize < 0)
      {
        call.GetState()->ThrowException(report, "Cannot resize the array to a negative size");
        return;
      }

      // Reserve space on the array (setting to anything smaller than the current capacity is ignored)
      // Normally we'd like to just invoke 'resize', however because this is an array of any types,
      // each element needs to be default constructed to the contained value type
      if (newSize > self->GetCount())
      {
        // First start by reserving space
        self->NativeArray.Reserve((size_t)newSize);

        // Loop until we've filled the array
        while (self->GetCount() < newSize)
        {
          // Add each element one by one and construct it to be the element type
          T& element = self->NativeArray.PushBack();

          // If no default value was provided, use default construction (otherwise use the given default value)
          element = CopyToAnyOrActualType<T>(defaultValue, userData.ContainedType);
        }
      }
      else
      {
        // Just resize the array, this will auto destruct elements
        self->NativeArray.Resize((size_t)newSize);
      }
    }

    //***************************************************************************
    static void ArrayResizeConstructorHelper(Call& call, ExceptionReport& report, byte* defaultValue)
    {
      // Read the element size from the current function's user-data
      ArrayUserData& userData = call.GetFunction()->ComplexUserData.ReadObject<ArrayUserData>(0);

      // Get ourselves (the array)
      byte* memory = call.GetHandle(Call::This).Dereference();

      // Construct the array
      ArrayTemplate* self = new (memory) ArrayTemplate();
    
      // Resize with no default value
      ArrayResizeHelper(call, report, defaultValue);
    }

    //***************************************************************************
    static void ArrayConstructorResize(Call& call, ExceptionReport& report)
    {
      // Construct the array and resize the number of elements, with no default value
      ArrayResizeConstructorHelper(call, report, nullptr);
    }

    //***************************************************************************
    static void ArrayConstructorResizeDefault(Call& call, ExceptionReport& report)
    {
      // The second argument should be the default value we'd like to initialize elements with
      byte* defaultValue = call.GetParameterUnchecked(1);

      // Construct the array and resize the number of elements, with no default value
      ArrayResizeConstructorHelper(call, report, defaultValue);
    }

    //***************************************************************************
    static void ArrayResize(Call& call, ExceptionReport& report)
    {
      // Resize with no default value
      ArrayResizeHelper(call, report, nullptr);
    }

    //***************************************************************************
    static void ArrayResizeDefault(Call& call, ExceptionReport& report)
    {
      // The second argument should be the default value we'd like to initialize elements with
      byte* defaultValue = call.GetParameterUnchecked(1);
    
      // Resize with the given default value
      ArrayResizeHelper(call, report, defaultValue);
    }

    //***************************************************************************
    static void ArrayLastIndex(Call& call, ExceptionReport& report)
    {
      // Get ourselves (the array)
      ArrayTemplate* self = (ArrayTemplate*)call.GetHandle(Call::This).Dereference();
    
      // Write out the count
      call.Set(Call::Return, self->GetCount() - 1);
    }

    //***************************************************************************
    static void ArrayFindFirstIndex(Call& call, ExceptionReport& report)
    {
      // Get ourselves (the array)
      ArrayTemplate* self = (ArrayTemplate*)call.GetHandle(Call::This).Dereference();

      // Read value to find
      T value = ArrayReadValue(call, self, 0);

      // Loop through the entire array to find the value
      for (size_t i = 0; i < self->NativeArray.Size(); ++i)
      {
        // If we found the value....
        const T& temp = self->NativeArray[i];
        if (LinkerEquals(value, temp))
        {
          // Return the index at which we found the value
          return call.Set(Call::Return, (Integer)i);
        }
      }

      // We didn't find the first index, just return -1 to indicate it was not found
      call.Set(Call::Return, -1);
    }

    //***************************************************************************
    static void ArrayRemoveFirst(Call& call, ExceptionReport& report)
    {
      // Get ourselves (the array)
      ArrayTemplate* self = (ArrayTemplate*)call.GetHandle(Call::This).Dereference();

      ArrayUserData& userData = call.GetFunction()->ComplexUserData.ReadObject<ArrayUserData>(0);

      // Read the value to find
      T value = ArrayReadValue(call, self, 0);

      // Loop through all the values in the native array
      for (size_t i = 0; i < self->NativeArray.Size(); ++i)
      {
        // If we found the value...
        const T& temp = self->NativeArray[i];
        if (LinkerEquals(value, temp))
        {
          // Erase the value at that index and mark the container as modified
          self->NativeArray.EraseAt(i);
          self->Modified();

          // Return that we removed a value
          return call.Set<Boolean>(Call::Return, true);
        }
      }

      // Return that we did not remove a value
      call.Set<Boolean>(Call::Return, false);
    }

    //***************************************************************************
    static void ArrayRemoveAll(Call& call, ExceptionReport& report)
    {
      // Get ourselves (the array)
      ArrayTemplate* self = (ArrayTemplate*)call.GetHandle(Call::This).Dereference();

      // Read the value that we want to remove
      T testValue = ArrayReadValue(call, self, 0);

      // Remove all the elements if they are equal, and get how many were removed
      size_t removeCount = RemoveAll(self->NativeArray, Zero::EqualTo<T>(testValue));

      // Return the amount we removed
      call.Set<Integer>(Call::Return, (Integer)removeCount);
    }

    //***************************************************************************
    static void SetParameter(Call& call, size_t index, T& value)
    {
      byte* dest = call.GetParameterUnchecked(index);
      CopyFromAnyOrActualType(value, dest);
    }

    //***************************************************************************
    template <ComparisonMode::Enum comparisonMode>
    class DelegateCompare
    {
    public:
      DelegateCompare(ExecutableState* state, ExceptionReport& report, Delegate& comparer) :
        State(state),
        Report(&report),
        Comparer(&comparer)
      {
      }

      ExecutableState* State;
      ExceptionReport* Report;
      Delegate* Comparer;

      bool operator()(T& left, T& right)
      {
        // Ideally we would have exited out of the algorithm, but sort has no mechanism for that
        // Just check if an exception was set upon coming in here, if so ignore it
        if (this->Report->HasThrownExceptions())
          return false;

        // Call the delegate with the left and right values
        Zilch::Call call(*this->Comparer, this->State);
        SetParameter(call, 0, left);
        SetParameter(call, 1, right);
        call.DisableParameterChecks();
        call.Invoke(*this->Report);

        // If the recent invocation threw an exception, then it means no return value was placed on the stack
        if (this->Report->HasThrownExceptions())
          return false;

        //do a different compare depending on if this is a boolean compare function or a int compare function
        if(comparisonMode == ComparisonMode::BoolMode)
          return call.Get<bool>(Call::Return);
        else if(comparisonMode == ComparisonMode::CompareMode)
          return call.Get<int>(Call::Return) < 0;
        return false;
      }
    };

    //***************************************************************************
    static void ArraySortDelegate(Call& call, ExceptionReport& report)
    {
      Delegate& comparer = call.GetDelegate(0);
      ArrayTemplate* self = (ArrayTemplate*)call.GetHandle(Call::This).Dereference();
      Sort(self->NativeArray.All(), DelegateCompare<ComparisonMode::BoolMode>(call.GetState(), report, comparer) );
    }

    //***************************************************************************
    static void ArraySortCompareToDelegate(Call& call, ExceptionReport& report)
    {
      Delegate& comparer = call.GetDelegate(0);
      ArrayTemplate* self = (ArrayTemplate*)call.GetHandle(Call::This).Dereference();
      Sort(self->NativeArray.All(), DelegateCompare<ComparisonMode::CompareMode>(call.GetState(), report, comparer) );
    }

    //***************************************************************************
    static void ArrayReturnIndexedRange(Call& call, ExceptionReport& report, ArrayTemplate* self, Integer start, Integer count)
    {
      // Read the element size from the current function's user-data
      ArrayUserData& userData = call.GetFunction()->ComplexUserData.ReadObject<ArrayUserData>(0);

      // Create the range type that we will return
      Handle rangeHandle = call.GetState()->AllocateDefaultConstructedHeapObject(userData.RangeType, report, HeapFlags::ReferenceCounted);

      // If we threw an exception, we need to early out and let the stack unroll
      if (report.HasThrownExceptions())
        return;

      // Get the range's data (should have been constructed!)
      ArrayRangeTemplate<T>* range = (ArrayRangeTemplate<T>*)rangeHandle.Dereference();

      // Setup the range to be returned
      range->Array = call.GetHandle(Call::This);
      range->Count = count;
      range->Start = start;
      range->Current = start;
      range->ModifyId = self->ModifyId;

      // Return the handle to the array range
      call.SetHandle(Call::Return, rangeHandle);
    }


    //***************************************************************************
    static void ArrayAll(Call& call, ExceptionReport& report)
    {
      // Get ourselves (the array)
      ArrayTemplate* self = (ArrayTemplate*)call.GetHandle(Call::This).Dereference();

      // Range of all elements
      ArrayReturnIndexedRange(call, report, self, 0, self->GetCount());
    }

    //***************************************************************************
    static void ArrayRange(Call& call, ExceptionReport& report)
    {
      // Get ourselves (the array)
      ArrayTemplate* self = (ArrayTemplate*)call.GetHandle(Call::This).Dereference();

      // Get the index we're trying to remove at
      Integer begin = call.Get<Integer>(0);
      Integer count = call.Get<Integer>(1);

      if (begin < 0 || begin >= self->GetCount())
      {
        call.GetState()->ThrowException(report, "Array range was out of bounds");
        return;
      }

      Integer end = begin + count;
      if (count < 0 || end > self->GetCount())
      {
        call.GetState()->ThrowException(report, "Array range was out of bounds");
        return;
      }

      // Range of all elements
      ArrayReturnIndexedRange(call, report, self, begin, count);
    }

  };

  // To iterate through arrays using 'foreach', we use the range type
  // Ranges are also much safer than iterators
  template <typename T>
  class ArrayRangeTemplate
  {
  public:

    // Constructor
    ArrayRangeTemplate() :
      Current(0),
      Start(0),
      Count(0),
      ModifyId(0)
    {
    }

    // Check if the range is empty
    Boolean IsEmpty()
    {
      return (this->Current - this->Start) == this->Count;
    }

    // Check if the range is not empty
    Boolean IsNotEmpty()
    {
      return this->IsEmpty() == false;
    }

    // A handle back to the source container that our data belongs to
    Handle Array;

    // The current index that we're iterating through
    Integer Current;

    // Where we started (so we can reset the range)
    Integer Start;

    // How many elements are in the range (so we know the end)
    Integer Count;

    // The id that the container had when we were created from it
    Integer ModifyId;

    //***************************************************************************
    static void ArrayRangeConstructor(Call& call, ExceptionReport& report)
    {
      // Get ourselves (the range)
      Handle& selfHandle = call.GetHandle(Call::This);
      byte* selfData = selfHandle.Dereference();

      // Call our default constructor on the memory
      new (selfData) ArrayRangeTemplate();
    }

    //***************************************************************************
    static void ArrayRangeDestructor(Call& call, ExceptionReport& report)
    {
      // Get ourselves (the range)
      ArrayRangeTemplate* self = (ArrayRangeTemplate*)call.GetHandle(Call::This).Dereference();
    
      // Directly invoke the destructor
      self->~ArrayRangeTemplate();
    }

    //***************************************************************************
    static void ArrayRangeReset(Call& call, ExceptionReport& report)
    {
      // Get ourselves (the range)
      ArrayRangeTemplate* self = (ArrayRangeTemplate*)call.GetHandle(Call::This).Dereference();
    
      // Move the range back to the start
      self->Current = self->Start;
    }

    //***************************************************************************
    static void ArrayRangeMoveNext(Call& call, ExceptionReport& report)
    {
      // Get ourselves (the range)
      ArrayRangeTemplate* self = (ArrayRangeTemplate*)call.GetHandle(Call::This).Dereference();
    
      // Check if the difference (how much we've gone forward) is past the count
      if (self->IsEmpty())
      {
        // Throw an exception since the range was empty and we called MoveNext
        call.GetState()->ThrowException(report, "The range reached the end, but then an attempt was made to make it iterate forward more");
        return;
      }
      else
      {
        // Move the range forward
        ++self->Current;
      }
    }

    //***************************************************************************
    static void ArrayRangeIsEmpty(Call& call, ExceptionReport& report)
    {
      // Get ourselves (the range)
      ArrayRangeTemplate* self = (ArrayRangeTemplate*)call.GetHandle(Call::This).Dereference();

      // Compute and return whether or not he range is empty
      Boolean isEmpty = self->IsEmpty();
      call.Set(Call::Return, isEmpty);
    }

    //***************************************************************************
    static void ArrayRangeIsNotEmpty(Call& call, ExceptionReport& report)
    {
      // Get ourselves (the range)
      ArrayRangeTemplate* self = (ArrayRangeTemplate*)call.GetHandle(Call::This).Dereference();

      // Compute and return whether or not he range is empty
      Boolean isNotEmpty = self->IsNotEmpty();
      call.Set(Call::Return, isNotEmpty);
    }
  
    //***************************************************************************
    static void ArrayRangeAll(Call& call, ExceptionReport& report)
    {
      // Grab our self handle and return it (we just return ourselves)
      Handle& selfHandle = call.GetHandle(Call::This);
      call.SetHandle(Call::Return, selfHandle);
    }

    //***************************************************************************
    static void ArrayRangeCurrent(Call& call, ExceptionReport& report)
    {
      // Read the element size from the current function's user-data
      ArrayUserData& userData = call.GetFunction()->ComplexUserData.ReadObject<ArrayUserData>(0);

      // Get this object
      ArrayRangeTemplate* self = (ArrayRangeTemplate*)call.GetHandle(Call::This).Dereference();
    
      // Get the array that we look at
      ArrayTemplate<T>* array = (ArrayTemplate<T>*)self->Array.Dereference();

      // Check if the array was modified
      if (self->ModifyId != array->ModifyId)
      {
        // It was modified, so throw an exception and early out
        call.GetState()->ThrowException(report,
          "The collection was modified and therefore the range cannot be used");
        return;
      }

      // If we've already reached the end...
      if (self->IsEmpty())
      {
        // Throw an exception since the range was empty and we called Current
        call.GetState()->ThrowException(report, "The range reached the end and an attempt was made to get the current value");
        return;
      }
      else
      {
        // Get a pointer to the return value data (on the stack)
        byte* returnValue = call.GetReturnUnchecked();
        call.DisableReturnChecks();

        // Copy the value at the array to the return type (this properly deals with the Any type)
        CopyFromAnyOrActualType(array->NativeArray[self->Current], returnValue);
      }
    }
  };

  //***************************************************************************
  template <typename T>
  BoundType* InstantiateArray
  (
    LibraryBuilder& builder,
    StringParam baseName,
    StringParam fullyQualifiedName,
    const Array<Constant>& templateTypes,
    const void* userData
  )
  {
    // Error checking
    ErrorIf(templateTypes.Size() != 1,
      "The Array template should only take one template argument");

    // Get the type we're instantiating
    Type* containedType = templateTypes.Front().TypeValue;

    // We could have put core in our userdata, but no real need
    Core& core = Core::GetInstance();

    StringBuilder rangeName;
    rangeName.Append("ArrayRange[");
    rangeName.Append(containedType->ToString());
    rangeName.Append("]");

    String fullyQualifiedRangeName = rangeName.ToString();
    
    ZilchTodo("The range type must have a valid destructor the decrements the reference count on the 'array' handle");
    BoundType* rangeType = builder.AddBoundType(fullyQualifiedRangeName, TypeCopyMode::ReferenceType, sizeof(ArrayRangeTemplate<T>));

    // Create the array type instance (arrays and any other containers should be reference types!)
    BoundType* arrayType = builder.AddBoundType(fullyQualifiedName, TypeCopyMode::ReferenceType, sizeof(ArrayTemplate<T>));

    arrayType->ToStringFunction = ArrayTemplate<T>::ArrayToString;

    Function* f = nullptr;
    Property* p = nullptr;

    ArrayUserData arrayUserData;
    arrayUserData.ContainedType = containedType;
    arrayUserData.RangeType = rangeType;
    arrayUserData.SelfType = arrayType;
    arrayType->ComplexUserData.WriteObject(arrayUserData);
    
    ZilchFullBindDestructor(builder, arrayType, ArrayTemplate<T>);
    ZilchFullBindConstructor(builder, arrayType, ArrayTemplate<T>, ZilchNoNames);

    f = builder.AddBoundConstructor(arrayType, ArrayTemplate<T>::ArrayConstructorResize, OneParameter(core.IntegerType, "size"));
    f->ComplexUserData.WriteObject(arrayUserData);

    f = builder.AddBoundConstructor(arrayType, ArrayTemplate<T>::ArrayConstructorResizeDefault, TwoParameters(core.IntegerType, "size", containedType, "defaultValue"));
    f->ComplexUserData.WriteObject(arrayUserData);

    f = builder.AddBoundFunction(arrayType, OperatorGet, ArrayTemplate<T>::ArrayGet, OneParameter(core.IntegerType, "index"), containedType, FunctionOptions::None);
    f->ComplexUserData.WriteObject(arrayUserData);

    f = builder.AddBoundFunction(arrayType, OperatorSet, ArrayTemplate<T>::ArraySet, TwoParameters(core.IntegerType, "index", containedType, "value"), core.VoidType, FunctionOptions::None);
    f->ComplexUserData.WriteObject(arrayUserData);

    f = builder.AddBoundFunction(arrayType, OperatorInsert, ArrayTemplate<T>::ArrayPush, OneParameter(containedType), core.VoidType, FunctionOptions::None);
    f->ComplexUserData.WriteObject(arrayUserData);

    f = builder.AddBoundFunction(arrayType, "Reserve", ArrayTemplate<T>::ArrayReserve, OneParameter(core.IntegerType, "capacity"), core.VoidType, FunctionOptions::None);
    f->ComplexUserData.WriteObject(arrayUserData);

    f = builder.AddBoundFunction(arrayType, "Resize", ArrayTemplate<T>::ArrayResize, OneParameter(core.IntegerType, "size"), core.VoidType, FunctionOptions::None);
    f->ComplexUserData.WriteObject(arrayUserData);

    f = builder.AddBoundFunction(arrayType, "Resize", ArrayTemplate<T>::ArrayResizeDefault, TwoParameters(core.IntegerType, "size", containedType, "defaultValue"), core.VoidType, FunctionOptions::None);
    f->ComplexUserData.WriteObject(arrayUserData);

    f = builder.AddBoundFunction(arrayType, "Push", ArrayTemplate<T>::ArrayPush, OneParameter(containedType), core.VoidType, FunctionOptions::None);
    f->ComplexUserData.WriteObject(arrayUserData);

    f = builder.AddBoundFunction(arrayType, "Pop", ArrayTemplate<T>::ArrayPop, ParameterArray(), containedType, FunctionOptions::None);
    f->ComplexUserData.WriteObject(arrayUserData);

    f = builder.AddBoundFunction(arrayType, "Insert", ArrayTemplate<T>::ArrayInsert, TwoParameters(core.IntegerType, "index", containedType, "value"), core.VoidType, FunctionOptions::None);
    f->ComplexUserData.WriteObject(arrayUserData);

    f = builder.AddBoundFunction(arrayType, "RemoveAt", ArrayTemplate<T>::ArrayRemoveAt, OneParameter(core.IntegerType, "index"), core.VoidType, FunctionOptions::None);
    f->ComplexUserData.WriteObject(arrayUserData);

    f = builder.AddBoundFunction(arrayType, "RemoveFirst", ArrayTemplate<T>::ArrayRemoveFirst, OneParameter(containedType, "value"), core.BooleanType, FunctionOptions::None);
    f->ComplexUserData.WriteObject(arrayUserData);

    f = builder.AddBoundFunction(arrayType, "RemoveAll", ArrayTemplate<T>::ArrayRemoveAll, OneParameter(containedType, "value"), core.IntegerType, FunctionOptions::None);
    f->ComplexUserData.WriteObject(arrayUserData);

    f = builder.AddBoundFunction(arrayType, "RemoveSwap", ArrayTemplate<T>::ArrayRemoveSwap, OneParameter(core.IntegerType, "index"), core.VoidType, FunctionOptions::None);
    f->ComplexUserData.WriteObject(arrayUserData);

    f = builder.AddBoundFunction(arrayType, "Range", ArrayTemplate<T>::ArrayRange, TwoParameters(core.IntegerType, "start", core.IntegerType, "count"), rangeType, FunctionOptions::None);
    f->ComplexUserData.WriteObject(arrayUserData);

    f = builder.AddBoundFunction(arrayType, "Copy", ArrayTemplate<T>::ArrayCopy, ParameterArray(), arrayType, FunctionOptions::None);
    f->ComplexUserData.WriteObject(arrayUserData);

    f = builder.AddBoundFunction(arrayType, "Clear", ArrayTemplate<T>::ArrayClear, ParameterArray(), core.VoidType, FunctionOptions::None);
    f->ComplexUserData.WriteObject(arrayUserData);

    f = builder.AddBoundFunction(arrayType, "FindFirstIndex", ArrayTemplate<T>::ArrayFindFirstIndex, OneParameter(containedType, "value"), core.IntegerType, FunctionOptions::None);
    f->ComplexUserData.WriteObject(arrayUserData);

    DelegateType* binaryCompare = builder.GetDelegateType(TwoParameters(containedType, "left", containedType, "right"), core.BooleanType);
    f = builder.AddBoundFunction(arrayType, "Sort", ArrayTemplate<T>::ArraySortDelegate, OneParameter(binaryCompare, "compare"), core.VoidType, FunctionOptions::None);
    f->ComplexUserData.WriteObject(arrayUserData);

    DelegateType* binaryCompareTo = builder.GetDelegateType(TwoParameters(containedType, "left", containedType, "right"), core.IntegerType);
    f = builder.AddBoundFunction(arrayType, "Sort", ArrayTemplate<T>::ArraySortCompareToDelegate, OneParameter(binaryCompareTo, "compare"), core.VoidType, FunctionOptions::None);
    f->ComplexUserData.WriteObject(arrayUserData);

    builder.AddBoundGetterSetter(arrayType, "Count", core.IntegerType, nullptr, ArrayTemplate<T>::ArrayCount, MemberOptions::None);
    builder.AddBoundGetterSetter(arrayType, "Capacity", core.IntegerType, nullptr, ArrayTemplate<T>::ArrayCapacity, MemberOptions::None);
    builder.AddBoundGetterSetter(arrayType, "LastIndex", core.IntegerType, nullptr, ArrayTemplate<T>::ArrayLastIndex, MemberOptions::None);

    p = builder.AddBoundGetterSetter(arrayType, "All", rangeType, nullptr, ArrayTemplate<T>::ArrayAll, MemberOptions::None);
    p->Get->ComplexUserData.WriteObject(arrayUserData);

    builder.AddBoundConstructor(rangeType, ArrayRangeTemplate<T>::ArrayRangeConstructor, ParameterArray());
    builder.AddBoundDestructor(rangeType, ArrayRangeTemplate<T>::ArrayRangeDestructor);

    builder.AddBoundFunction(rangeType, "MoveNext", ArrayRangeTemplate<T>::ArrayRangeMoveNext, ParameterArray(), core.VoidType, FunctionOptions::None);
    builder.AddBoundFunction(rangeType, "Reset", ArrayRangeTemplate<T>::ArrayRangeReset, ParameterArray(), core.VoidType, FunctionOptions::None);

    p = builder.AddBoundGetterSetter(rangeType, "Current", containedType, nullptr, ArrayRangeTemplate<T>::ArrayRangeCurrent, MemberOptions::None);
    p->Get->ComplexUserData.WriteObject(arrayUserData);

    builder.AddBoundGetterSetter(rangeType, "IsEmpty", core.BooleanType, nullptr, ArrayRangeTemplate<T>::ArrayRangeIsEmpty, MemberOptions::None);
    builder.AddBoundGetterSetter(rangeType, "IsNotEmpty", core.BooleanType, nullptr, ArrayRangeTemplate<T>::ArrayRangeIsNotEmpty, MemberOptions::None);
    builder.AddBoundGetterSetter(rangeType, "All", rangeType, nullptr, ArrayRangeTemplate<T>::ArrayRangeAll, MemberOptions::None);
    
    // Return the array type we instantiated
    return arrayType;
  }

  //***************************************************************************
  BoundType* InstantiateArray
  (
    LibraryBuilder& builder,
    StringParam baseName,
    StringParam fullyQualifiedName,
    const Array<Constant>& templateTypes,
    const void* userData
  )
  {
    // Get the type our array is containing
    Type* containedType = templateTypes.Front().TypeValue;

    if (Type::IsHandleType(containedType))
    {
      return InstantiateArray<Handle>(builder, baseName, fullyQualifiedName, templateTypes, userData);
    }
    else if (Type::IsDelegateType(containedType))
    {
      return InstantiateArray<Delegate>(builder, baseName, fullyQualifiedName, templateTypes, userData);
    }
    else if (Type::IsSame(containedType, ZilchTypeId(Boolean)))
    {
      return InstantiateArray<Boolean>(builder, baseName, fullyQualifiedName, templateTypes, userData);
    }
    else if (Type::IsSame(containedType, ZilchTypeId(Boolean2)))
    {
      return InstantiateArray<Boolean2>(builder, baseName, fullyQualifiedName, templateTypes, userData);
    }
    else if (Type::IsSame(containedType, ZilchTypeId(Boolean3)))
    {
      return InstantiateArray<Boolean3>(builder, baseName, fullyQualifiedName, templateTypes, userData);
    }
    else if (Type::IsSame(containedType, ZilchTypeId(Boolean4)))
    {
      return InstantiateArray<Boolean4>(builder, baseName, fullyQualifiedName, templateTypes, userData);
    }
    else if (Type::IsSame(containedType, ZilchTypeId(Byte)))
    {
      return InstantiateArray<Byte>(builder, baseName, fullyQualifiedName, templateTypes, userData);
    }
    else if (Type::IsSame(containedType, ZilchTypeId(Integer)) || Type::IsEnumOrFlagsType(containedType))
    {
      return InstantiateArray<Integer>(builder, baseName, fullyQualifiedName, templateTypes, userData);
    }
    else if (Type::IsSame(containedType, ZilchTypeId(Integer2)))
    {
      return InstantiateArray<Integer2>(builder, baseName, fullyQualifiedName, templateTypes, userData);
    }
    else if (Type::IsSame(containedType, ZilchTypeId(Integer3)))
    {
      return InstantiateArray<Integer3>(builder, baseName, fullyQualifiedName, templateTypes, userData);
    }
    else if (Type::IsSame(containedType, ZilchTypeId(Integer4)))
    {
      return InstantiateArray<Integer4>(builder, baseName, fullyQualifiedName, templateTypes, userData);
    }
    else if (Type::IsSame(containedType, ZilchTypeId(Real)))
    {
      return InstantiateArray<Real>(builder, baseName, fullyQualifiedName, templateTypes, userData);
    }
    else if (Type::IsSame(containedType, ZilchTypeId(Real2)))
    {
      return InstantiateArray<Real2>(builder, baseName, fullyQualifiedName, templateTypes, userData);
    }
    else if (Type::IsSame(containedType, ZilchTypeId(Real3)))
    {
      return InstantiateArray<Real3>(builder, baseName, fullyQualifiedName, templateTypes, userData);
    }
    else if (Type::IsSame(containedType, ZilchTypeId(Real4)))
    {
      return InstantiateArray<Real4>(builder, baseName, fullyQualifiedName, templateTypes, userData);
    }
    else if (Type::IsSame(containedType, ZilchTypeId(Quaternion)))
    {
      return InstantiateArray<Quaternion>(builder, baseName, fullyQualifiedName, templateTypes, userData);
    }
    else if (Type::IsSame(containedType, ZilchTypeId(DoubleInteger)))
    {
      return InstantiateArray<DoubleInteger>(builder, baseName, fullyQualifiedName, templateTypes, userData);
    }
    else if (Type::IsSame(containedType, ZilchTypeId(DoubleReal)))
    {
      return InstantiateArray<DoubleReal>(builder, baseName, fullyQualifiedName, templateTypes, userData);
    }
    else
    {
      return InstantiateArray<Any>(builder, baseName, fullyQualifiedName, templateTypes, userData);
    }
  }

  // Make sure the size of ArrayClass is the same as ArrayTemplate
  ZilchStaticAssert(sizeof(ArrayClass<Byte>) == sizeof(ArrayTemplate<Byte>),
    "The array base 'ArrayClass' and 'ArrayTemplate' should be binary compatible with each other",
    ArrayClassAndArrayTemplateShouldBeBinaryCompatable1);
  ZilchStaticAssert(sizeof(ArrayClass<Any>) == sizeof(ArrayTemplate<Any>),
    "The array base 'ArrayClass' and 'ArrayTemplate' should be binary compatible with each other",
    ArrayClassAndArrayTemplateShouldBeBinaryCompatable2);
}
