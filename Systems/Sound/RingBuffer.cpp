///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------------------------- Ring Buffer

//**************************************************************************************************
int RingBuffer::Initialize(unsigned elementSizeBytes, unsigned elementCountInBuffer, void* buffer)
{
  // Check if the number of elements is a power of 2
  if (((elementCountInBuffer - 1) & elementCountInBuffer) != 0)
    return -1;

  // Buffer size is the number of elements in the buffer
  BufferSize = elementCountInBuffer;
  // Save the size of the elements in bytes
  ElementSizeBytes = elementSizeBytes;
  // Set the buffer pointer
  Buffer = (char*)buffer;
  // Reset write and read indexes to 0
  WriteIndex = ReadIndex = 0;
  // Set the mask used for wrapping indexes to size of buffer
  SmallMask = elementCountInBuffer - 1;
  // Set the mask used for advancing indexes and difference calculation
  BigMask = (elementCountInBuffer * 2) - 1;

  return 0;
}

//**************************************************************************************************
void RingBuffer::ResetBuffer()
{
  // Reset write and read indexes to 0
  WriteIndex = ReadIndex = 0;
}

//**************************************************************************************************
unsigned RingBuffer::GetWriteAvailable()
{
  // Elements available for writing is the buffer size minus elements available for reading,
  // limiting the elements written to no more than the buffer size.
  return BufferSize - GetReadAvailable();
}

//**************************************************************************************************
unsigned RingBuffer::GetReadAvailable()
{
  // Because of the limit in GetWriteAvailable, WriteIndex will never be more than BufferSize
  // greater than ReadIndex, so this will never return more than the BufferSize. 
  // If WriteIndex - ReadIndex is negative, since the BigMask value is (BufferSize * 2) - 1,
  // the indexes wrap using BigMask, and BufferSize is a power of 2, the mask will turn the value 
  // into the corresponding positive number, which will be less than BufferSize.
  return (WriteIndex - ReadIndex) & BigMask;
}

//**************************************************************************************************
unsigned RingBuffer::Write(const void* data, int elementCount)
{
  int available = GetWriteAvailable();

  // No available space to write, just return
  if (available == 0)
    return 0;

  // Make sure we don't write more elements than there is space for
  if (elementCount > available)
    elementCount = available;

  // Wrap the WriteIndex at the BufferSize
  unsigned index = WriteIndex & SmallMask;

  // Check to see if write is not contiguous
  if (index + elementCount > BufferSize)
  {
    // Get the size of the first write section
    unsigned size1 = BufferSize - index;

    // Copy first section, from index position to end of buffer
    memcpy(Buffer + (index * ElementSizeBytes), data, size1 * ElementSizeBytes);
    // Copy second section, from beginning of buffer to end of elements
    memcpy(Buffer, (char*)data + (size1 * ElementSizeBytes), (elementCount - size1) * ElementSizeBytes);
  }
  else
  {
    // Write all elements at once
    memcpy(Buffer + (index * ElementSizeBytes), data, elementCount * ElementSizeBytes);
  }

  // Advance the write index, wrapping at 2 * BufferSize
  AtomicStore(&WriteIndex, (WriteIndex + elementCount) & BigMask);

  // Return the number of elements actually written
  return elementCount;
}

//**************************************************************************************************
unsigned RingBuffer::Read(void* data, int elementCount)
{
  int available = GetReadAvailable();

  // No available space to read, just return
  if (available == 0)
    return 0;

  // Make sure we don't read more elements than there is space for
  if (elementCount > available)
    elementCount = available;

  // Wrap the ReadIndex at the BufferSize
  unsigned index = ReadIndex & SmallMask;

  // Check to see if read is not contiguous
  if (index + elementCount > BufferSize)
  {
    // Get the size of the first read section
    unsigned size1 = BufferSize - index;

    // Copy first section, from index position to end of buffer
    memcpy(data, Buffer + (index * ElementSizeBytes), size1 * ElementSizeBytes);
    // Copy second section, from beginning of buffer to end of elements
    memcpy((char*)data + (size1 * ElementSizeBytes), Buffer, (elementCount - size1) * ElementSizeBytes);
  }
  else
  {
    // Read all elements at once
    memcpy(data, Buffer + (index * ElementSizeBytes), elementCount * ElementSizeBytes);
  }

  // Advance the read index, wrapping at 2 * BufferSize
  AtomicStore(&ReadIndex, (ReadIndex + elementCount) & BigMask);

  // Return the number of elements actually read
  return elementCount;
}

} // namespace Zero
