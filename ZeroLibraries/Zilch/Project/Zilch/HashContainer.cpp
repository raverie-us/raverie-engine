/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  AnyHashMap::AnyHashMap() :
    ModifyId(0)
  {
  }
  
  //***************************************************************************
  void AnyHashMap::Modified()
  {
    ++this->ModifyId;
  }
  
  //***************************************************************************
  AnyHashMapRange::AnyHashMapRange() :
    ModifyId(0)
  {
  }

  //***************************************************************************
  HashMapUserData::HashMapUserData() :
    KeyType(nullptr),
    ValueType(nullptr),
    PairRangeType(nullptr),
    ValueRangeType(nullptr),
    KeyRangeType(nullptr)
  {
  }

  //***************************************************************************
  inline HashMapUserData& GetHashMapUserData(Call& call)
  {
    return call.GetFunction()->Owner->ComplexUserData.ReadObject<HashMapUserData>(0);
  }
  
  //***************************************************************************
  inline AnyHashMap* GetHashMapThis(Call& call)
  {
    return (AnyHashMap*)call.GetHandle(Call::This).Dereference();
  }

  //***************************************************************************
  void HashMapGetOrDefault(Call& call, ExceptionReport& report)
  {
    // The user data Contains information about the types used in the hash map
    HashMapUserData& userData = GetHashMapUserData(call);

    // Get ourselves (the hash map)
    AnyHashMap* self = GetHashMapThis(call);

    // The first index is always the key, read that
    byte* keyData = call.GetParameterUnchecked(0);

    // The second index is a default value that gets returned when we fail to find the key
    byte* defaultValueData = call.GetParameterUnchecked(1);

    // Construct an 'any' (we can avoid this step if we use a special finder, but who cares for right now)
    Any key(keyData, userData.KeyType);
    Any* value = (*self).FindPointer(key);

    // Get a pointer to the return value data (on the stack)
    byte* returnValue = call.GetReturnUnchecked();
    call.DisableReturnChecks();
    
    // If we found the value, copy it to the return, otherwise copy the default to the return
    if (value != nullptr)
      value->CopyStoredValueTo(returnValue);
    else
      userData.ValueType->GenericCopyConstruct(returnValue, defaultValueData);
  }

  //***************************************************************************
  void HashMapGetOrDefaultNull(Call& call, ExceptionReport& report)
  {
    // The user data Contains information about the types used in the hash map
    HashMapUserData& userData = GetHashMapUserData(call);

    // Get ourselves (the hash map)
    AnyHashMap* self = GetHashMapThis(call);

    // The first index is always the key, read that
    byte* keyData = call.GetParameterUnchecked(0);

    // Construct an 'any' (we can avoid this step if we use a special finder, but who cares for right now)
    Any key(keyData, userData.KeyType);
    Any* value = (*self).FindPointer(key);

    // Get a pointer to the return value data (on the stack)
    byte* returnValue = call.GetReturnUnchecked();
    call.DisableReturnChecks();
    
    // If we found the value, copy it to the return, otherwise copy the default to the return
    if (value != nullptr)
      value->CopyStoredValueTo(returnValue);
    else
      userData.ValueType->GenericDefaultConstruct(returnValue);
  }

  //***************************************************************************
  void HashMapGetOrError(Call& call, ExceptionReport& report)
  {
    // The user data Contains information about the types used in the hash map
    HashMapUserData& userData = GetHashMapUserData(call);

    // Get ourselves (the hash map)
    AnyHashMap* self = GetHashMapThis(call);

    // The first index is always the key, read that
    byte* keyData = call.GetParameterUnchecked(0);

    // Construct an 'any' (we can avoid this step if we use a special finder, but who cares for right now)
    Any key(keyData, userData.KeyType);

    Any* value = (*self).FindPointer(key);

    if (value == nullptr)
    {
      call.GetState()->ThrowException(report, String::Format("The key '%s' was not found within the map", key.ToString().c_str()));
      return;
    }

    // Get a pointer to the return value data (on the stack)
    byte* returnValue = call.GetReturnUnchecked();
    call.DisableReturnChecks();

    // Generically copy the contained type to the return value
    value->CopyStoredValueTo(returnValue);
  }

  //***************************************************************************
  void HashMapContains(Call& call, ExceptionReport& report)
  {
    // The user data Contains information about the types used in the hash map
    HashMapUserData& userData = GetHashMapUserData(call);

    // Get ourselves (the hash map)
    AnyHashMap* self = GetHashMapThis(call);

    // The first index is always the key, read that
    byte* keyData = call.GetParameterUnchecked(0);

    // Construct an 'any' (we can avoid this step if we use a special finder, but who cares for right now)
    Any key(keyData, userData.KeyType);
    
    bool containsKey = self->ContainsKey(key);
    call.Set(Call::Return, containsKey);
  }

  //***************************************************************************
  // Returns true if it overwrote the value, false otherwise
  void HashMapSetOrOverwrite(Call& call, ExceptionReport& report)
  {
    // The user data Contains information about the types used in the hash map
    HashMapUserData& userData = GetHashMapUserData(call);

    // Get ourselves (the hash map)
    AnyHashMap* self = GetHashMapThis(call);

    // Read the first parameter as the key, and the second as the value
    byte* keyData = call.GetParameterUnchecked(0);
    byte* valueData = call.GetParameterUnchecked(1);

    // Construct an 'any' for the key and value
    Any key(keyData, userData.KeyType);
    Any value(valueData, userData.ValueType);

    bool keyExisted = (self->FindPointer(key) != nullptr);

    // Insert the value into the map under that key
    (*self)[key] = value;

    call.Set<Boolean>(Call::Return, keyExisted);
  }

  //***************************************************************************
  // Returns true if it set the value, false otherwise
  void HashMapSetOrIgnore(Call& call, ExceptionReport& report)
  {
    // The user data Contains information about the types used in the hash map
    HashMapUserData& userData = GetHashMapUserData(call);

    // Get ourselves (the hash map)
    AnyHashMap* self = GetHashMapThis(call);

    // Read the first parameter as the key, and the second as the value
    byte* keyData = call.GetParameterUnchecked(0);
    byte* valueData = call.GetParameterUnchecked(1);

    // Construct an 'any' for the key and value
    Any key(keyData, userData.KeyType);
    Any value(valueData, userData.ValueType);
    
    bool noKeyDoInsert = (self->FindPointer(key) == nullptr);

    // Insert the value into the map under that key only if one doesn't already exist
    if (noKeyDoInsert)
      (*self)[key] = value;

    call.Set<Boolean>(Call::Return, noKeyDoInsert);
  }

  //***************************************************************************
  void HashMapSetOrError(Call& call, ExceptionReport& report)
  {
    // The user data Contains information about the types used in the hash map
    HashMapUserData& userData = GetHashMapUserData(call);

    // Get ourselves (the hash map)
    AnyHashMap* self = GetHashMapThis(call);

    // Read the first parameter as the key, and the second as the value
    byte* keyData = call.GetParameterUnchecked(0);
    byte* valueData = call.GetParameterUnchecked(1);

    // Construct an 'any' for the key and value
    Any key(keyData, userData.KeyType);
    Any value(valueData, userData.ValueType);
    
    // Insert the value into the map under that key only if one doesn't already exist
    if (Any* foundValue = self->FindPointer(key))
    {
      String message = String::Format("The key '%s' already existed within the map with a value of '%s'", key.ToString().c_str(), foundValue->ToString().c_str());
      call.GetState()->ThrowException(report, message);
      return;
    }
    
    // Actually Insert the value into the map
    (*self)[key] = value;
  }

  //***************************************************************************
  void HashMapRemoveOrError(Call& call, ExceptionReport& report)
  {
    // The user data Contains information about the types used in the hash map
    HashMapUserData& userData = GetHashMapUserData(call);

    // Get ourselves (the hash map)
    AnyHashMap* self = GetHashMapThis(call);

    // The first index is always the key, read that
    byte* keyData = call.GetParameterUnchecked(0);

    // Construct an 'any' (we can avoid this step if we use a special finder, but who cares for right now)
    Any key(keyData, userData.KeyType);
 
    Any* value = (*self).FindPointer(key);

    if (value == nullptr)
    {
      call.GetState()->ThrowException(report, String::Format("The key '%s' was not found within the map", key.ToString().c_str()));
      return;
    }

    (*self).Erase(key);
  }

  //***************************************************************************
  // Returns true if the key was removed, false otherwise
  void HashMapRemoveOrIgnore(Call& call, ExceptionReport& report)
  {
    // The user data Contains information about the types used in the hash map
    HashMapUserData& userData = GetHashMapUserData(call);

    // Get ourselves (the hash map)
    AnyHashMap* self = GetHashMapThis(call);

    // The first index is always the key, read that
    byte* keyData = call.GetParameterUnchecked(0);

    // Construct an 'any' (we can avoid this step if we use a special finder, but who cares for right now)
    Any key(keyData, userData.KeyType);
 
    Any* value = (*self).FindPointer(key);

    bool foundValueToRemove = (value != nullptr);

    if (foundValueToRemove)
      (*self).Erase(key);

    call.Set<Boolean>(Call::Return, foundValueToRemove);
  }

  //***************************************************************************
  void HashMapClear(Call& call, ExceptionReport& report)
  {
    // Get ourselves (the hash map)
    AnyHashMap* self = GetHashMapThis(call);

    // Just clear the map, pretty simple!
    self->Clear();
  }

  //***************************************************************************
  void HashMapGetCount(Call& call, ExceptionReport& report)
  {
    // Get ourselves (the hash map)
    AnyHashMap* self = GetHashMapThis(call);

    // Return the size of the hash map (how many elements are in it)
    call.Set(Call::Return, (Integer)self->Size());
  }

  //***************************************************************************
  template <BoundType* HashMapUserData::*RangeMemberType>
  void HashMapAll(Call& call, ExceptionReport& report)
  {
    // Get ourselves (the hash map)
    Handle& selfHandle = call.GetHandle(Call::This);
    AnyHashMap* self = (AnyHashMap*)selfHandle.Dereference();
    
    // The user data Contains information about the types used in the hash map
    HashMapUserData& userData = GetHashMapUserData(call);

    // Create the range type that we will return
    Handle rangeHandle = call.GetState()->AllocateDefaultConstructedHeapObject(userData.*RangeMemberType, report, HeapFlags::ReferenceCounted);

    // If we threw an exception, we need to early out and let the stack unroll
    if (report.HasThrownExceptions())
      return;

    // Get the range's data (should have been constructed!)
    AnyHashMapRange* range = (AnyHashMapRange*)rangeHandle.Dereference();
    range->Range = self->All();
    range->OriginalRange = range->Range;
    range->ModifyId = self->ModifyId;
    range->HashMap = selfHandle;

    // Return the handle to the array range
    call.SetHandle(Call::Return, rangeHandle);
  }

  //***************************************************************************
  BoundType* Core::InstantiateHashMap
  (
    LibraryBuilder& builder,
    StringParam baseName,
    StringParam fullyQualifiedName,
    const Array<Constant>& templateTypes,
    const void* userData
  )
  {
    // Error checking
    ErrorIf(templateTypes.Size() != 2,
      "The HashMap template should only take two template arguments");

    // Get the key and value types for the hash map
    Type* keyType = templateTypes[0].TypeValue;
    Type* valueType = templateTypes[1].TypeValue;

    // We could have put core in our userdata, but no real need
    Core& core = Core::GetInstance();
    
    BoundType* pairRangeType = builder.InstantiateTemplate("HashMapRange", ZilchConstants(keyType, valueType), Module()).Type;
    BoundType* keyRangeType = builder.InstantiateTemplate("HashMapKeyRange", ZilchConstants(keyType), Module()).Type;
    BoundType* valueRangeType = builder.InstantiateTemplate("HashMapValueRange", ZilchConstants(valueType), Module()).Type;

    // Create the array type instance (arrays and any other containers should be reference types!)
    BoundType* containerType = builder.AddBoundType(fullyQualifiedName, TypeCopyMode::ReferenceType, sizeof(AnyHashMap));

    HashMapUserData containerUserData;
    containerUserData.KeyType = keyType;
    containerUserData.ValueType = valueType;
    containerUserData.PairRangeType = pairRangeType;
    containerUserData.KeyRangeType = keyRangeType;
    containerUserData.ValueRangeType = valueRangeType;
    containerType->ComplexUserData.WriteObject(containerUserData);

    // Bind the constructor and destructor of that type
    ZilchFullBindDestructor(builder, containerType, AnyHashMap);
    ZilchFullBindConstructor(builder, containerType, AnyHashMap, nullptr);
    
    ParameterArray keyOnlyParameters = OneParameter(keyType, "key");
    ParameterArray setParameters = TwoParameters(keyType, "key", valueType, "value");

    // Add the constructor
    builder.AddBoundFunction(containerType, "GetOrDefault", HashMapGetOrDefault, TwoParameters(keyType, "key", valueType, "defaultValue"), valueType, FunctionOptions::None);
    builder.AddBoundFunction(containerType, "GetOrDefault", HashMapGetOrDefaultNull, keyOnlyParameters, valueType, FunctionOptions::None);
    builder.AddBoundFunction(containerType, "GetOrError", HashMapGetOrError, keyOnlyParameters, valueType, FunctionOptions::None);

    builder.AddBoundFunction(containerType, "Contains", HashMapContains, keyOnlyParameters, core.BooleanType, FunctionOptions::None);

    builder.AddBoundFunction(containerType, "SetOrOverwrite", HashMapSetOrOverwrite, setParameters, core.BooleanType, FunctionOptions::None);
    builder.AddBoundFunction(containerType, "SetOrIgnore", HashMapSetOrIgnore, setParameters, core.BooleanType, FunctionOptions::None);
    builder.AddBoundFunction(containerType, "SetOrError", HashMapSetOrError, setParameters, core.VoidType, FunctionOptions::None);

    // Operator overloading
    builder.AddBoundFunction(containerType, OperatorInsert, HashMapSetOrError, setParameters, core.VoidType, FunctionOptions::None);
    builder.AddBoundFunction(containerType, OperatorGet, HashMapGetOrError, keyOnlyParameters, valueType, FunctionOptions::None);
    builder.AddBoundFunction(containerType, OperatorSet, HashMapSetOrOverwrite, setParameters, core.BooleanType, FunctionOptions::None);

    builder.AddBoundFunction(containerType, "RemoveOrError", HashMapRemoveOrError, keyOnlyParameters, core.VoidType, FunctionOptions::None);
    builder.AddBoundFunction(containerType, "RemoveOrIgnore", HashMapRemoveOrIgnore, keyOnlyParameters, core.BooleanType, FunctionOptions::None);

    builder.AddBoundFunction(containerType, "Clear", HashMapClear, ParameterArray(), core.VoidType, FunctionOptions::None);

    builder.AddBoundGetterSetter(containerType, "Count", core.IntegerType, nullptr, HashMapGetCount, MemberOptions::None);

    builder.AddBoundGetterSetter(containerType, "All", pairRangeType, nullptr, HashMapAll<&HashMapUserData::PairRangeType>, MemberOptions::None);
    builder.AddBoundGetterSetter(containerType, "Keys", keyRangeType, nullptr, HashMapAll<&HashMapUserData::KeyRangeType>, MemberOptions::None);
    builder.AddBoundGetterSetter(containerType, "Values", valueRangeType, nullptr, HashMapAll<&HashMapUserData::ValueRangeType>, MemberOptions::None);

    // Return the array type we instantiated
    return containerType;
  }
  
  //***************************************************************************
  void AnyHashMapRange::MoveNext()
  {
    ExecutableState* state = ExecutableState::CallingState;

    // If the hash map we originated from is null, then also throw an exception
    if (this->HashMap.IsNull())
    {
      state->ThrowException("The hash map this range referenced is null (or deleted) and cannot be ");
      return;
    }

    // Check if the difference (how much we've gone forward) is past the count
    if (this->IsEmpty())
    {
      // Throw an exception since the range was empty and we called MoveNext
      state->ThrowException("The range reached the end, but then an attempt was made to make it iterate forward more");
      return;
    }

    this->Range.PopFront();
  }
  
  //***************************************************************************
  void AnyHashMapRange::Reset()
  {
    this->Range = this->OriginalRange;
  }
  
  //***************************************************************************
  bool AnyHashMapRange::IsEmpty()
  {
    return this->Range.Empty();
  }
  
  //***************************************************************************
  bool AnyHashMapRange::IsNotEmpty()
  {
    return !this->Range.Empty();
  }
  
  //***************************************************************************
  void HashMapRangeAll(Call& call, ExceptionReport& report)
  {
    call.Set(Call::Return, call.Get<Handle>(Call::This));
  }
  
  //***************************************************************************
  void HashMapRangeCurrent(Call& call, ExceptionReport& report)
  {
    // Read the element size from the current function's user-data
    ExecutableState* state = call.GetState();
    HashMapRangeMode::Enum mode = (HashMapRangeMode::Enum)(size_t)call.GetFunction()->Owner->UserData;

    // Get this object
    AnyHashMapRange* self = (AnyHashMapRange*)call.GetHandle(Call::This).Dereference();
    
    // If the hash map we originated from is null, then also throw an exception
    AnyHashMap* hashMap = (AnyHashMap*)self->HashMap.Dereference();
    if (self->HashMap.IsNull())
    {
      state->ThrowException("The hash map this range referenced is null (or deleted) and cannot be ");
      return;
    }

    // Check if the array was modified
    if (self->ModifyId != hashMap->ModifyId)
    {
      // It was modified, so throw an exception and early out
      state->ThrowException("The collection was modified and therefore the range cannot be used");
      return;
    }

    // If we've already reached the end...
    if (self->IsEmpty())
    {
      // Throw an exception since the range was empty and we called Current
      call.GetState()->ThrowException(report, "The range reached the end and an attempt was made to get the current value");
      return;
    }

    // Grab the key-value pair that we'll be returning (either part of it, or the whole thing)
    AnyKeyValue& keyValue = self->Range.Front();

    // If this is a pair then we need to allocate a KeyValue pair
    switch (mode)
    {
      case HashMapRangeMode::Pair:
      {
        // Create the range type that we will return
        BoundType* keyValueType = (BoundType*)call.GetFunction()->FunctionType->Return;
        Handle keyValueHandle = call.GetState()->AllocateDefaultConstructedHeapObject(keyValueType, report, HeapFlags::ReferenceCounted);

        // If we threw an exception, we need to early out and let the stack unroll
        if (report.HasThrownExceptions())
          return;

        // Copy over the key-value pair (both Any types)
        AnyKeyValue* returnedKeyValue = (AnyKeyValue*)keyValueHandle.Dereference();
        *returnedKeyValue = keyValue;

        // Return the handle to the KeyValue
        call.SetHandle(Call::Return, keyValueHandle);
        break;
      }

      // Otherwise, we just directly return the key or value
      case HashMapRangeMode::Key:
      case HashMapRangeMode::Value:
      {
        // Get a pointer to the return value data (on the stack)
        byte* returnValue = call.GetReturnUnchecked();
        call.DisableReturnChecks();

        // Copy the value at the array to the return type (this properly deals with the Any type)
        if (mode == HashMapRangeMode::Key)
          CopyFromAnyOrActualType(keyValue.first, returnValue);
        else
          CopyFromAnyOrActualType(keyValue.second, returnValue);
        break;
      }
    }
  }

  //***************************************************************************
  BoundType* Core::InstantiateHashMapRange
  (
    LibraryBuilder& builder,
    StringParam baseName,
    StringParam fullyQualifiedName,
    const Array<Constant>& templateTypes,
    const void* userData
  )
  {
    BoundType* rangeType = builder.AddBoundType(fullyQualifiedName, TypeCopyMode::ReferenceType, sizeof(AnyHashMapRange));

    // The user-data will be the 'HashMapRangeMode' enumeration
    rangeType->UserData = userData;
    
    ZilchFullBindDestructor(builder, rangeType, AnyHashMapRange);
    ZilchFullBindConstructor(builder, rangeType, AnyHashMapRange, nullptr);

    Type* currentType = nullptr;
    if (templateTypes.Size() == 2)
    {
      // Get the key and value types for the hash map
      Type* keyType = templateTypes[0].TypeValue;
      Type* valueType = templateTypes[1].TypeValue;

      // The type we return when accessing 'Current' is a KeyValue[Key, Value]
      currentType = builder.InstantiateTemplate("KeyValue", ZilchConstants(keyType, valueType), Module()).Type;
    }
    else
    {
      // The type we return when accessing 'Current' is either the key or value (whatever T was passed in)
      currentType = templateTypes[0].TypeValue;
    }

    ZilchFullBindMethod(builder, rangeType, &AnyHashMapRange::MoveNext, ZilchNoOverload, "MoveNext", ZilchNoNames);
    ZilchFullBindMethod(builder, rangeType, &AnyHashMapRange::Reset, ZilchNoOverload, "Reset", ZilchNoNames);
    ZilchFullBindGetterSetter(builder, rangeType, &AnyHashMapRange::IsEmpty, ZilchNoOverload, ZilchNoSetter, ZilchNoOverload, "IsEmpty");
    ZilchFullBindGetterSetter(builder, rangeType, &AnyHashMapRange::IsNotEmpty, ZilchNoOverload, ZilchNoSetter, ZilchNoOverload, "IsNotEmpty");
    builder.AddBoundGetterSetter(rangeType, "All", rangeType, nullptr, HashMapRangeAll, MemberOptions::None);
    builder.AddBoundGetterSetter(rangeType, "Current", currentType, nullptr, HashMapRangeCurrent, MemberOptions::None);
    return rangeType;
  }

  //***************************************************************************
  class KeyValueUserData
  {
  public:
    KeyValueUserData() :
      KeyType(nullptr),
      ValueType(nullptr)
    {
    }

    Type* KeyType;
    Type* ValueType;
  };
  
  //***************************************************************************
  void KeyValueConstructor(Call& call, ExceptionReport& report)
  {
    // The user data Contains information about the types used in the hash map
    KeyValueUserData& userData = call.GetFunction()->Owner->ComplexUserData.ReadObject<KeyValueUserData>(0);

    // Get ourselves (the key-value pair)
    byte* selfData = call.GetHandle(Call::This).Dereference();

    AnyKeyValue* self = new (selfData) AnyKeyValue();

    self->first.DefaultConstruct(userData.KeyType);
    self->second.DefaultConstruct(userData.ValueType);
  }
  
  //***************************************************************************
  void KeyValueGetKey(Call& call, ExceptionReport& report)
  {
    // The user data Contains information about the types used in the hash map
    KeyValueUserData& userData = call.GetFunction()->Owner->ComplexUserData.ReadObject<KeyValueUserData>(0);

    // Get ourselves (the key-value pair)
    AnyKeyValue* self = (AnyKeyValue*)call.GetHandle(Call::This).Dereference();

    // Get a pointer to the return value data (on the stack)
    byte* returnValue = call.GetReturnUnchecked();
    call.DisableReturnChecks();

    // Copy the key to the stack (in the return place)
    self->first.CopyStoredValueTo(returnValue);
  }
  
  //***************************************************************************
  void KeyValueSetKey(Call& call, ExceptionReport& report)
  {
    // The user data Contains information about the types used in the hash map
    KeyValueUserData& userData = call.GetFunction()->Owner->ComplexUserData.ReadObject<KeyValueUserData>(0);

    // Get ourselves (the key-value pair)
    AnyKeyValue* self = (AnyKeyValue*)call.GetHandle(Call::This).Dereference();

    // Grab the first parameter and set our key to it
    byte* newKeyData = call.GetParameterUnchecked(0);
    self->first.AssignFrom(newKeyData, userData.KeyType);
  }
  
  //***************************************************************************
  void KeyValueGetValue(Call& call, ExceptionReport& report)
  {
    // The user data Contains information about the types used in the hash map
    KeyValueUserData& userData = call.GetFunction()->Owner->ComplexUserData.ReadObject<KeyValueUserData>(0);

    // Get ourselves (the key-value pair)
    AnyKeyValue* self = (AnyKeyValue*)call.GetHandle(Call::This).Dereference();

    // Get a pointer to the return value data (on the stack)
    byte* returnValue = call.GetReturnUnchecked();
    call.DisableReturnChecks();

    // Copy the value to the stack (in the return place)
    self->second.CopyStoredValueTo(returnValue);
  }
  
  //***************************************************************************
  void KeyValueSetValue(Call& call, ExceptionReport& report)
  {
    // The user data Contains information about the types used in the hash map
    KeyValueUserData& userData = call.GetFunction()->Owner->ComplexUserData.ReadObject<KeyValueUserData>(0);

    // Get ourselves (the key-value pair)
    AnyKeyValue* self = (AnyKeyValue*)call.GetHandle(Call::This).Dereference();

    // Grab the first parameter and set our value to it
    byte* newValueData = call.GetParameterUnchecked(0);
    self->second.AssignFrom(newValueData, userData.ValueType);
  }

  //***************************************************************************
  BoundType* Core::InstantiateKeyValue
  (
    LibraryBuilder& builder,
    StringParam baseName,
    StringParam fullyQualifiedName,
    const Array<Constant>& templateTypes,
    const void* userData
  )
  {
    // Error checking
    ErrorIf(templateTypes.Size() != 2,
      "The KeyValue template should only take two template arguments");

    // Get the key and value types for the hash map
    Type* keyType = templateTypes[0].TypeValue;
    Type* valueType = templateTypes[1].TypeValue;

    // Create the pair type instance
    BoundType* keyValueType = builder.AddBoundType(fullyQualifiedName, TypeCopyMode::ReferenceType, sizeof(AnyKeyValue));

    KeyValueUserData newUserData;
    newUserData.KeyType = keyType;
    newUserData.ValueType = valueType;
    keyValueType->ComplexUserData.WriteObject(newUserData);

    // Bind the constructor and destructor of that type
    builder.AddBoundDefaultConstructor(keyValueType, KeyValueConstructor);
    ZilchFullBindDestructor(builder, keyValueType, AnyKeyValue);

    builder.AddBoundGetterSetter(keyValueType, "Key", keyType, KeyValueSetKey, KeyValueGetKey, MemberOptions::None);
    builder.AddBoundGetterSetter(keyValueType, "Value", valueType, KeyValueSetValue, KeyValueGetValue, MemberOptions::None);

    return keyValueType;
  }
}
