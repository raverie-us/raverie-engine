/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  Any::Any() :
    StoredType(nullptr)
  {
    ZilchErrorIfNotStarted(Any);

    // Memset the area to empty, so any handles or primitives we store won't get messed up by exceptions
    memset(this->Data, 0, sizeof(this->Data));
  }

  //***************************************************************************
  Any::Any(nullptr_t) :
    StoredType(nullptr)
  {
    ZilchErrorIfNotStarted(Any);

    // Memset the area to empty, so any handles or primitives we store won't get messed up by exceptions
    memset(this->Data, 0, sizeof(this->Data));
  }

  //***************************************************************************
  Any::Any(Type* type)
  {
    ZilchErrorIfNotStarted(Any);

    // Get how big the copyable size of the object is (size of a handle, or the entire value size)
    size_t copyableSize = type->GetCopyableSize();

    // Allocate room to store this type (may store locally and not actually allocate)
    byte* destination = this->AllocateData(copyableSize);

    // Store the type and default construct the data into us
    this->StoredType = type;
    type->GenericDefaultConstruct(destination);
  }

  //***************************************************************************
  Any::Any(const byte* data, Type* type)
  {
    ZilchErrorIfNotStarted(Any);

    // If we're trying to construct an any from an any, just directly copy it
    if (Type::IsAnyType(type))
    {
      // Memset the area to empty, so any handles or primitives we store won't get messed up by exceptions
      this->StoredType = nullptr;
      memset(this->Data, 0, sizeof(this->Data));
      
      // Now Assign our any to the other any
      *this = *(Any*)data;
    }
    else
    {
      // When storing any type within an any, we should always store the most derived type
      type = type->GenericGetVirtualType(data);

      // Get how big the copyable size of the object is (size of a handle, or the entire value size)
      size_t copyableSize = type->GetCopyableSize();

      // Allocate room to store this type (may store locally and not actually allocate)
      byte* destination = this->AllocateData(copyableSize);

      // Store the type and copy construct the data into us
      this->StoredType = type;
      type->GenericCopyConstruct(destination, data);
    }
  }

  //***************************************************************************
  Any::Any(const Any& other)
  {
    // Change the stored type to their same type
    this->StoredType = other.StoredType;

    // If we're copying from an any that actually Contains data...
    if (this->StoredType != nullptr)
    {
      // Get how big the copyable size of the object is (size of a handle, or the entire value size)
      size_t copyableSize = this->StoredType->GetCopyableSize();

      // Allocate room to store this type (may store locally and not actually allocate)
      byte* destination = this->AllocateData(copyableSize);

      // Copy the right hand data into our data
      this->StoredType->GenericCopyConstruct(destination, other.GetData());
    }
    else
    {
      // Memset the area to empty, so any handles or primitives we store won't get messed up by exceptions
      memset(this->Data, 0, sizeof(this->Data));
    }
  }
  
  //***************************************************************************
  Any::Any(const Handle& other)
  {
    // Clear the entire any
    memset(this, 0, sizeof(*this));

    // Get the real type that this handle is referencing
    Type* type = other.GetBoundOrIndirectType();
    if (type != nullptr)
      this->AssignFrom((const byte*)&other, type);
  }

  //***************************************************************************
  Any::Any(const Delegate& other)
  {
    // Clear the entire any
    memset(this, 0, sizeof(*this));

    // Copy the delegate if its valid
    if (other.BoundFunction != nullptr)
    {
      Type* type = other.BoundFunction->FunctionType;
      this->AssignFrom((const byte*)&other, type);
    }
  }

  //***************************************************************************
  Any::~Any()
  {
    // Clear the any, even though it's a bit redundant (could be optimized)
    this->Clear();
  }

  //***************************************************************************
  byte* Any::AllocateData(size_t size)
  {
    // We assume that the size of the object will fit within our data section
    byte* result = this->Data;

    // If the type is bigger then we can store... (note than storing an 'any' inside an 'any' will always hit this case!)
    if (size > sizeof(this->Data))
    {
      // Allocate memory to store the data
      result = new byte[size];

      // Store a pointer to our allocation inside the data field
      *((byte**)this->Data) = result;
    }

    // Return data that will be large enough to store the object
    return result;
  }

  //***************************************************************************
  const byte* Any::GetData() const
  {
    if (this->StoredType == nullptr)
      return nullptr;

    // Get the size of the handle, delegate, or entire value type (copyable size)
    size_t copyableSize = this->StoredType->GetCopyableSize();

    // If the type is bigger then we can store... (note than storing an 'any' inside an 'any' will always hit this case!)
    if (copyableSize > sizeof(this->Data))
    {
      // The size of the object was large, which meant we must be storing it by pointer instead
      return *((byte**)this->Data);
    }

    // Otherwise, it was small enough so we just stored it in our fixed data field
    return this->Data;
  }

  //***************************************************************************
  void Any::Clear()
  {
    // If we're storing anything...
    if (this->StoredType != nullptr)
    {
      // Get the size of the handle, delegate, or entire value type (copyable size)
      size_t copyableSize = this->StoredType->GetCopyableSize();

      // Memory that we need to free
      byte* toBeDeleted = nullptr;

      // Where we store the memory that needs to be destructed (via GenericDestruct)
      byte* data = this->Data;

      // If the type is bigger then we can store... (note than storing an 'any' inside an 'any' will always hit this case!)
      if (copyableSize > sizeof(this->Data))
      {
        // The size of the object was large, which meant we must be storing it by pointer instead
        data = *((byte**)this->Data);

        // Since we're clearing, we also want to free the data
        toBeDeleted = data;
      }

      // Generically destruct the data we store first
      this->StoredType->GenericDestruct(data);

      // Delete the memory (could be null!)
      delete[] toBeDeleted;

      // Memset the area to empty, so any handles or primitives we store won't get messed up by exceptions
      memset(this->Data, 0, sizeof(this->Data));

      // Clear the stored type
      this->StoredType = nullptr;
    }
  }
    
  //***************************************************************************
  Any& Any::operator=(const Any& other)
  {
    // Avoid self assignment
    if (this == &other)
      return *this;

    // Clear ourself out, which destructs any data we were storing
    this->Clear();

    // Change the stored type to their same type
    this->StoredType = other.StoredType;

    // If we're copying from an any that actually Contains data...
    if (this->StoredType != nullptr)
    {
      // Get how big the copyable size of the object is (size of a handle, or the entire value size)
      size_t copyableSize = this->StoredType->GetCopyableSize();

      // Allocate room to store this type (may store locally and not actually allocate)
      byte* destination = this->AllocateData(copyableSize);

      // Copy the right hand data into our data
      this->StoredType->GenericCopyConstruct(destination, other.GetData());
    }

    // Return ourself for chaining... which I don't like ;)
    return *this;
  }

  //***************************************************************************
  bool Any::operator==(const Any& rhs) const
  {
    // The types must compare the same for the values to be the same
    // Remember that the types stored are the MOST derived type in terms of inheritance
    // Example:
    // var derviedClass = new Cat();
    // var baseClass : Animal = derviedClass;
    // var any : Any = baseClass;
    // The 'StoredType' in Any will be 'Cat', not 'Animal' (most derived)
    if (this->StoredType != rhs.StoredType)
      return false;

    // If the types are both null, return true (we already know they are the same based on the above check)
    if (this->StoredType == nullptr)
      return true;

    // Generically compare the type with each other (we know they are the same type!)
    return this->StoredType->GenericEquals(this->GetData(), rhs.GetData());
  }

  //***************************************************************************
  bool Any::operator==(Zero::NullPointerType) const
  {
    return this->IsNull();
  }
  
  //***************************************************************************
  bool Any::operator!=(const Any& rhs) const
  {
    // Just invert the comparison
    return !((*this) == rhs);
  }

  //***************************************************************************
  bool Any::operator!=(Zero::NullPointerType) const
  {
    return this->IsNull();
  }

  //***************************************************************************
  size_t Any::Hash() const
  {
    // The hash of an empty any is always 0
    if (this->StoredType == nullptr)
      return 0;

    // Generically hash our stored value
    return this->StoredType->GenericHash(this->GetData());
  }

  //***************************************************************************
  String Any::ToString() const
  {
    // Return an empty string if we store nothing
    static String NullString("null");
    if (this->StoredType == nullptr)
      return NullString;

    // Generically stringify our stored value
    return this->StoredType->GenericToString(this->GetData());
  }

  //***************************************************************************
  Handle Any::ToHandle() const
  {
    if(this->StoredType->IsHandle())
    {
      Handle* handle = (Handle*)GetData();
      return *handle;
    }

    return Handle();
  }

  //***************************************************************************
  byte* Any::Dereference() const
  {
    // If the value is a handle, then dereference it
    const byte* data = this->GetData();
    if (this->StoredType->IsHandle())
    {
      Handle& handle = *(Handle*)data;
      return handle.Dereference();
    }

    // Otherwise just return the data directly
    return (byte*)data;
  }

  //***************************************************************************
  void Any::AssignFrom(const byte* data, Type* type)
  {
    ErrorIf(type == nullptr, "Cannot Assign the 'Any' to a null type, use Clear instead");

    // Avoid self assignment
    if (this->GetData() == data)
      return;

    // Clear ourself out, which destructs any data we were storing
    this->Clear();

    // Get the copyable size (size of the handle, delegate, or value type, etc)
    size_t copyableSize = type->GetCopyableSize();

    // Allocate room to store this type (may store locally and not actually allocate)
    byte* destination = this->AllocateData(copyableSize);

    // Copy the right hand data into our data
    type->GenericCopyConstruct(destination, data);

    // Change the stored type to their same type
    this->StoredType = type;
  }

  //***************************************************************************
  void Any::DefaultConstruct(Type* type)
  {
    ErrorIf(type == nullptr, "Cannot Assign the 'Any' to a null type, use Clear instead");

    // Destruct any memory we are currently holding
    this->Clear();

    // Change the stored type to their same type
    this->StoredType = type;

    // Get the copyable size (size of the handle, delegate, or value type, etc)
    size_t copyableSize = type->GetCopyableSize();

    // Allocate room to store this type (may store locally and not actually allocate)
    byte* destination = this->AllocateData(copyableSize);

    // Default construct the value into our data
    // Typically makes handles null, delegates null, and value types cleared to 0
    this->StoredType->GenericDefaultConstruct(destination);
  }

  //***************************************************************************
  void Any::CopyStoredValueTo(byte* to) const
  {
    ErrorIf(this->StoredType == nullptr, "The any does not contain a type!");

    // Get the size of the handle, delegate, or entire value type (copyable size)
    size_t copyableSize = this->StoredType->GetCopyableSize();

    // Where we store the memory that needs to be copied from
    const byte* data = this->GetData();

    // Generically copy the stored value
    this->StoredType->GenericCopyConstruct(to, data);
  }

  //***************************************************************************
  bool Any::IsHoldingValue() const
  {
    return this->StoredType != nullptr;
  }

  //***************************************************************************
  bool Any::IsNull() const
  {
    if (this->StoredType == nullptr)
      return true;

    if (this->StoredType->IsHandle())
    {
      Handle* handle = (Handle*)GetData();
      return handle->IsNull();
    }
    else if (this->StoredType->IsDelegate())
    {
      Delegate* delegate = (Delegate*)GetData();
      return delegate->IsNull();
    }

    return false;
  }

  //***************************************************************************
  bool Any::IsNotNull() const
  {
    return !IsNull();
  }

  //***************************************************************************
  template <>
  Any CopyToAnyOrActualType<Any>(byte* data, Type* dataType)
  {
    // If no data was provided, then default construct the type into the any
    if (data == nullptr)
      return Any(dataType);

    // Construct the any from the given data
    return Any(data, dataType);
  }

  //***************************************************************************
  template <>
  void CopyFromAnyOrActualType<Any>(const Any& any, byte* to)
  {
    // Generically copy the contained type to the destination
    any.CopyStoredValueTo(to);
  }
}
