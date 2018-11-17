/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_DESTRUCTiBLE_BUFFER_HPP
#define ZILCH_DESTRUCTiBLE_BUFFER_HPP

namespace Zilch
{
  // Aligns a number to a machine byte boundary (either the same or a larger value)
  ZeroShared size_t AlignToBusWidth(size_t value);

  // A function that will invoke the destructor on an object (does not call delete or free!)
  typedef void (*DestructFn) (void* object);

  // A function that will invoke the copy constructor from one object to another uninitialized place in memory
  typedef void (*CopyConstructFn) (const void* from, void* to);

  // A function for wrapping a destructor of any object in the above signature
  template <typename T>
  ZeroSharedTemplate void DestructorWrapper(void* object)
  {
    // Cast the object pointer we got into our own object type
    T* objectTyped = (T*)object;

    // Invoke the destructor on the object
    // Note: We do not call free or delete on the object memory, it isn't ours!
    objectTyped->~T();
  }

  // Get the destructor for a type, or null for pod types...
  template <typename T>
  ZeroSharedTemplate DestructFn GetDestructor()
  {
    // If the type is considered to be pod, ignore the destructor
    if (StandardTraits<T>::is_pod_::value)
    {
      return nullptr;
    }
    else
    {
      // Otherwise we wrap the destructor into our own signature
      return DestructorWrapper<T>;
    }
  }

  // A function for wrapping a copy constructor of any object in the above signature
  template <typename T>
  ZeroSharedTemplate void CopyConstructorWrapper(const void* from, void* to)
  {
    // Use placement new to construct the object at the 'to' location (but invoke the copy constructor using the from object reference)
    new (to) T(*(T*)from);
  }

  // Get the copy constructor for a type, or null for pod types...
  template <typename T>
  ZeroSharedTemplate CopyConstructFn GetCopyConstructor()
  {
    // If the type is considered to be pod, ignore the destructor
    if (StandardTraits<T>::is_pod_::value)
    {
      return nullptr;
    }
    else
    {
      // Otherwise we wrap the copy constructor into our own signature
      return CopyConstructorWrapper<T>;
    }
  }

  // Used to write arbitrary data to a buffer where some of the data could be destructed
  class ZeroShared DestructibleBuffer
  {
  public:

    // The block size we use for allocations
    static const size_t BlockSize = 512;

    // Default constructor (empty)
    DestructibleBuffer();

    // Copy constructor (copies all internally written elements properly)
    DestructibleBuffer(const DestructibleBuffer& from);
    
    // Destructor
    ~DestructibleBuffer();

    // Assignment from one to another
    DestructibleBuffer& operator=(const DestructibleBuffer& from);

    // Get the absolute size of the data
    // Note: Do NOT use this as an index in 'Read' or 'GetElement'
    size_t GetSize();

    // Clears the entire buffer, invoking all the destructors as needed
    void Clear();

    // Allocates writable memory that the user can work with
    // If the destructor passed in is null, then no destructor will be pushed
    byte* Allocate(size_t size, DestructFn destructor = nullptr, CopyConstructFn copyConstructor = nullptr, size_t* positionOut = nullptr);

    // Writes memory directly to the buffer
    // If the destructor passed in is null, then no destructor will be pushed
    byte* WriteMemory(void* source, size_t size, DestructFn destructor = nullptr);

    // Writes an object directly to the buffer
    // If the object is POD, no destructor will be pushed
    template <typename T>
    T& WriteObject(const T& value, size_t* positionOut = nullptr)
    {
      // Allocate the memory for the object and push a wrapper around it's destructor
      byte* newData = this->Allocate(sizeof(T), GetDestructor<T>(), GetCopyConstructor<T>(), positionOut);

      // Copy the object we got into the allocated data and return it
      return *new (newData) T(value);
    }

    // Creates an object with a default constructor directly
    // to the buffer and pushes the destructor
    template <typename T>
    T& CreateObject(size_t* positionOut = nullptr)
    {
      // Allocate the memory for the object and push a wrapper around it's destructor
      byte* newData = this->Allocate(sizeof(T), GetDestructor<T>(), GetCopyConstructor<T>(), positionOut);

      // Default construct the type into the allocated data and return it
      return *new (newData) T();
    }

    // Reads an amount of data starting from the given position
    // and can optionally return the next position to read from
    byte* Read(size_t position, size_t length, size_t* nextPositionOut = nullptr);

    // Reads an object from the data and moves the position forward by its size
    template <typename T>
    T& ReadObject(size_t position, size_t* nextPositionOut = nullptr)
    {
      // Read the data using the size of the given class
      return *(T*)this->Read(position, sizeof(T), nextPositionOut);
    }

    // Get the element at the given position
    byte* GetElement(size_t position);

  private:

    // Represents any bit of destructible data in our buffer
    class ZeroShared Entry
    {
    public:
      // The position in the buffer where the object lives
      size_t AbsolutePosition;

      // The destructor (typically wrapped) to call on the object's memory
      // This destrcutor should never call free or delete, just the class destructor
      DestructFn Destructor;

      // When we copy construct the entire buffer, we'll walk through and copy construct its entries
      CopyConstructFn CopyConstructor;
    };

    // All the bits of data in the buffer that must be destructed/copied
    PodArray<Entry> Entries;

    // The internal data
    UntypedBlockArray<BlockSize> Data;
  };
}

#endif
