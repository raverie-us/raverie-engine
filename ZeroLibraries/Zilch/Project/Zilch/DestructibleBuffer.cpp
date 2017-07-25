/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  size_t AlignToBusWidth(size_t value)
  {
    // We need to properly impelment struct alignment
    return value;
  }

  //***************************************************************************
  DestructibleBuffer::DestructibleBuffer()
  {
  }
  
  //***************************************************************************
  DestructibleBuffer::DestructibleBuffer(const DestructibleBuffer& from) :
    Data(from.Data),
    Entries(from.Entries)
  {
    // We already directly copied over all the block array data (handed by its copy constructor)
    // Note: Untyped block array memcopies all its data over, but in truth we need to copy construct each of them
    for (size_t i = 0; i < this->Entries.Size(); ++i)
    {
      // Grab the current entry
      Entry& entry = this->Entries[i];

      // Skip this entry (already memcopied) if no copy constructor was provided
      if (entry.CopyConstructor == nullptr)
        continue;

      // Copy construct this element from its old memory to the new memory
      // (does not destruct the old or anything weird like that)
      DestructibleBuffer& fromNonConst = const_cast<DestructibleBuffer&>(from);
      byte* fromElement = fromNonConst.Data.GetAbsoluteElement(entry.AbsolutePosition);
      byte* toElement = this->Data.GetAbsoluteElement(entry.AbsolutePosition);
      entry.CopyConstructor(fromElement, toElement);
    }
  }

  //***************************************************************************
  DestructibleBuffer::~DestructibleBuffer()
  {
    // Clear and invoke all the destructors on our data
    this->Clear();
  }
  
  //***************************************************************************
  DestructibleBuffer& DestructibleBuffer::operator=(const DestructibleBuffer& from)
  {
    // Destruct ourselves and copy construct ourself again from the other one
    this->~DestructibleBuffer();
    new (this) DestructibleBuffer(from);
    return *this;
  }

  //**************************************************************************
  size_t DestructibleBuffer::GetSize()
  {
    return this->Data.AbsoluteSize();
  }

  //**************************************************************************
  void DestructibleBuffer::Clear()
  {
    // Loop through all the destructors
    for (size_t i = 0; i < this->Entries.Size(); ++i)
    {
      // Get the current destructible object
      Entry& entry = this->Entries[i];

      // Get the pointer to where the object exists in our memory
      byte* object = this->Data.GetAbsoluteElement(entry.AbsolutePosition);

      // Invoke its destructor since we're being erased
      if (entry.Destructor != nullptr)
        entry.Destructor(object);
    }

    // Clear the destructible objects as well as the data
    this->Entries.Clear();
    this->Data.Clear();
  }

  //***************************************************************************
  byte* DestructibleBuffer::Allocate(size_t size, DestructFn destructor, CopyConstructFn copyConstructor, size_t* positionOut)
  {
    // Make sure the size aligns with the bus for efficiency
    size = AlignToBusWidth(size);

    // Get a pointer directly to the new data
    size_t absolutePosition = 0;
    byte* newData = this->Data.RequestElementOfSize(size, &absolutePosition);

    // If the user also wants the position...
    if (positionOut != nullptr)
    {
      // The position is our absolute position
      *positionOut = absolutePosition;
    }

    // Clear out the new data, just for safety (this also helps when we align data)
    memset(newData, 0, size);

    // If we were given a destructor
    if (destructor != nullptr || copyConstructor != nullptr)
    {
      // Add a new destructible component
      Entry& entry = this->Entries.PushBack();

      // Setup the destructible so that it destroys
      // the object at the new memory position
      entry.Destructor = destructor;
      entry.CopyConstructor = copyConstructor;
      entry.AbsolutePosition = absolutePosition;
    }

    // Return the writable data
    return newData;
  }

  //***************************************************************************
  byte* DestructibleBuffer::WriteMemory(void* source, size_t size, DestructFn destructor)
  {
    // Allocate data and use the given destructor (it may be null)
    byte* newData = this->Allocate(size, destructor);

    // Copy the user's data over the new data
    memcpy(newData, source, size);

    // Finally, return the new writable data
    return newData;
  }

  //***************************************************************************
  byte* DestructibleBuffer::Read(size_t position, size_t length, size_t* nextPositionOut)
  {
    // Make sure we only jump by full aligned blocks
    length = AlignToBusWidth(length);

    // Get the position of the memory the user is reading
    byte* memory = this->Data.GetAbsoluteElement(position);

    // Error checking
    ErrorIf(position != 0 && nextPositionOut == nullptr,
      "In cases where the position is not 0 (the front), you "
      "must get the next position via 'nextPositionOut'");

    // If the user needs us to output the next position
    if (nextPositionOut != nullptr)
    {
      // If the read causes a lapse in length...
      size_t newBlockPosition = (position % BlockSize) + length;
      if (newBlockPosition >= BlockSize)
      {
        // Move the position to the next block
        *nextPositionOut = position + (newBlockPosition - BlockSize);
      }
      else
      {
        // Move the position forward by the length
        *nextPositionOut = position + length;
      }
    }

    // Now return the memory
    return memory;
  }

  //***************************************************************************
  byte* DestructibleBuffer::GetElement(size_t position)
  {
    return this->Data.GetAbsoluteElement(position);
  }
}