/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_ANY_HPP
#define ZILCH_ANY_HPP

namespace Zilch
{
  // Returns an invalid type that is zeroed out (only used for not-crashing after an assert)
  // This places no requirement on default construction or copy construction
  template <typename T>
  T& GetInvalid()
  {
    static byte Invalid[sizeof(T)] = { 0 };
    return (T&)*Invalid;
  }

  // Stores any type of object (handles, delegates, or even value types)
  class ZeroShared Any
  {
  public:

    // Constructor that initializes the Any to null (a handle, set to NullType)
    Any();
    Any(nullptr_t);

    Any(Zero::MoveReference<Any> rhs)
    {
      Error("Not implemented.");
    }

    template <typename T>
    Any(const T& value, ExecutableState* state = nullptr, P_DISABLE_IF((Zero::is_base_of<Type, typename Zero::remove_pointer<typename Zero::remove_const_and_volatile<T>::type>::type>::value)))
    {
      typedef typename TypeBinding::StripQualifiers<T>::Type UnqualifiedType;
      const UnqualifiedType* pointer = TypeBinding::ReferenceCast<T&, const UnqualifiedType*>::Cast((T&)value);
      BoundType* type = ZilchVirtualTypeId(pointer);
      type->IsInitializedAssert();
      ZilchTypeId(T)->IsInitializedAssert();

      // Get how big the copyable size of the object is (size of a handle, or the entire value size)
      size_t copyableSize = type->GetCopyableSize();

      // Allocate room to store this type (may store locally and not actually allocate)
      byte* destination = this->AllocateData(copyableSize);

      // Store the type and copy construct the data into us
      this->StoredType = type;

      // If the type is a reference type... (this is always a handle)
      if (type->CopyMode == TypeCopyMode::ReferenceType)
      {
        if (state == nullptr)
          state = ExecutableState::CallingState;

        InternalWriteRef<T>(value, destination, state);
      }
      // Otherwise it must be a value type...
      else
      {
        InternalWriteValue<T>(value, destination);
      }
    }

    template <typename T>
    T Get(GetOptions::Enum options = GetOptions::ReturnDefaultOrNull) const
    {
      if (this->StoredType == nullptr)
      {
        ErrorIf(options == GetOptions::AssertOnNull,
          "The value inside the Any was null");
        return GetInvalid<T>();
      }

      // Check if we can directly convert the stored type into the requested type
      // This supports derived -> base class casting (but not visa versa), enums to integers, etc
      BoundType* toType = ZilchTypeId(T);
      if (this->StoredType->IsRawCastableTo(toType) == false)
      {
        ErrorIf(options == GetOptions::AssertOnNull,
          "There was a value inside the Any of type '%s' but it cannot be converted to '%s'",
          this->StoredType->ToString().c_str(),
          toType->Name.c_str());
        return GetInvalid<T>();
      }

      byte* data = (byte*)this->GetData();

      // If the type is a reference type... (this is always a handle)
      if (toType->CopyMode == TypeCopyMode::ReferenceType)
      {
        return InternalReadRef<T>(data);
      }
      // Otherwise it must be a value type...
      else
      {
        return InternalReadValue<T>(data);
      }
    }

    template <typename T>
    bool Is() const
    {
      BoundType* checkType = ZilchTypeId(T);
      return this->StoredType->IsRawCastableTo(checkType);
    }

    // Copying will properly reference count handles, delegates this handle, and memcpy value types
    Any(const Any& other);

    // Creates an any from a handle and reference counts
    Any(const Handle& other);

    // Creates an any from a delegate and reference counts
    Any(const Delegate& other);

    // Constructor that initializes to the given data and type (the data is copied in using GenericCopyConstruct)
    Any(const byte* data, Type* type);

    // Construct a default instance of a particular type (equivalent of default(T))
    explicit Any(Type* type);

    // Destructor that decrements reference counts and properly handles stored data
    ~Any();
    
    // Copying will properly reference count handles, delegates this handle, and memcpy value types
    Any& operator=(const Any& rhs);

    // Checks if the internal handle/delegate/value is the same
    bool operator==(const Any& rhs) const;
    bool operator==(Zero::NullPointerType) const;
    
    // Checks if the internal handle/delegate/value is the different
    bool operator!=(const Any& rhs) const;
    bool operator!=(Zero::NullPointerType) const;

    // Hashes a handle (generally used by hashable containers)
    size_t Hash() const;

    // Checks if the any itself is holding no value, or if the value stored within the any
    // is null. This specifically checks Handles and Delegates for a null value
    bool IsNull() const;
    bool IsNotNull() const;

    // Converts the internal value to string (used for debugging)
    String ToString() const;

    // Converts the internal value to a Handle. If it does not store a handle type, it will
    // return an empty Handle.
    Handle ToHandle() const;

    // If the type stored internally is a Handle then this will invoke Dereference on the handle
    // Otherwise this will return the same value as GetData
    byte* Dereference() const;

    // Destruct any data stored by the any
    // This also clears the entire any out to zero
    void Clear();

    // Allocates data if the size goes past the sizeof(this->Data), or returns a pointer to this->Data
    byte* AllocateData(size_t size);

    // Get the raw type data that we point at (may be our internal Data, or may be allocated)
    const byte* GetData() const;

    // Much like the copy constructor or assignment of an any, except it avoids
    // creating an extra 'any' in cases where we just have the memory and the type
    void AssignFrom(const byte* data, Type* type);

    // Replaces our stored definition with a default constructed version of the given type (equivalent of default(T))
    // Typically makes handles null, delegates null, and value types cleared to 0
    void DefaultConstruct(Type* type);

    // Generically copies the value of this any to another location
    // This will NOT copy the 'Any' but rather the stored type
    // Make sure the size and type of destination matches!
    void CopyStoredValueTo(byte* to) const;

    // Checks if the any is currently holding a value
    // Note that the value MAY be null, which is still technically a value stored within the any
    // If you wish to check for null for various stored types, use IsNull
    bool IsHoldingValue() const;

  public:

    // We want to store the largest type (the delegate, handle, etc)
    // The delegate stores the handle, so we know delegate is the biggest
    // If the size of the type is bigger then can fit here, then we allocate a pointer instead
    byte Data[sizeof(Delegate)];

    // The type that we're storing inside the data
    Type* StoredType;
  };

  // Type defines for ease of use
  typedef const Any& AnyParam;

  // Given a type we know natively, return a value pointed at by a data pointer
  // If the data is not given, this will default construct the type
  // This is specialized by the Any type to return an Any that encapsulates the value
  template <typename T>
  T CopyToAnyOrActualType(byte* data, Type* dataType)
  {
    // If no data was provided, then return the default value for T
    if (data == nullptr)
    {
      // Not all primitive constructors zero out the memory, so do that first
      byte memory[sizeof(T)] = {0};
      new (memory) T();
      return *(T*)memory;
    }

    // Otherwise just cast data into the T type
    return *(T*)data;
  }

  // Specialization for the Any type, which will copy the value into an Any
  template <>
  ZeroShared Any CopyToAnyOrActualType<Any>(byte* data, Type* dataType);

  // Given a type we know natively, just directly copy it to a location
  // This is specialized by the Any type to only copy its inner value
  template <typename T>
  ZeroSharedTemplate void CopyFromAnyOrActualType(const T& value, byte* to)
  {
    // Just directly construct the value
    new (to) T(value);
  }

  // Specialization for the Any type, which will copy the value out of an Any
  template <>
  ZeroShared void CopyFromAnyOrActualType<Any>(const Any& any, byte* to);
}

#endif
