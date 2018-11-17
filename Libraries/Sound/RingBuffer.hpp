///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#define RingBufferAtomicType s32

namespace Zero
{

//-------------------------------------------------------------------------------------- Ring Buffer

class RingBuffer
{
public:
  RingBuffer() :
    BufferSize(0),
    Buffer(nullptr),
    WriteIndex(0),
    ReadIndex(0),
    ElementSizeBytes(0),
    SmallMask(0),
    BigMask(0)
  {}

  // The number of elements in the buffer must be a power of 2, returns -1 if it is not
  int Initialize(unsigned elementSizeBytes, unsigned elementCountInBuffer, void* buffer);
  // Resets the indexes. Should only be called when not reading or writing to buffer.
  void ResetBuffer();
  // Returns the number of elements that can be written to
  unsigned GetWriteAvailable();
  // Returns the number of elements that can be read
  unsigned GetReadAvailable();
  // Writes the specified number of elements to the ring buffer. Returns the number of
  // elements that were actually written.
  unsigned Write(const void* data, int elementCount);
  // Reads the specified number of elements from the ring buffer into the provided data buffer.
  // Returns the number of elements that were actually read.
  unsigned Read(void* data, int elementCount);

private:
  // Total number of elements that could fit in the buffer
  unsigned BufferSize;
  // Pointer to the buffer that will be used
  char* Buffer;
  // The size, in bytes, of an element
  unsigned ElementSizeBytes;
  // Index used to find the position to start writing to the buffer
  RingBufferAtomicType WriteIndex;
  // Index used to find the position to start reading from the buffer
  RingBufferAtomicType ReadIndex;
  // Used to wrap indexes to the size of the buffer
  RingBufferAtomicType SmallMask;
  // Used to wrap indexes when advancing and to handle WriteIndex - ReadIndex calculation
  RingBufferAtomicType BigMask;
};

} // namespace Zero
