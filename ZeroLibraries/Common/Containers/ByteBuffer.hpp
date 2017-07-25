///////////////////////////////////////////////////////////////////////////////
///
/// \file ByteBuffer.hpp
/// Definition of ByteBuffer.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Containers/Array.hpp"
#include "String/String.hpp"
#include "Common/Utility/Status.hpp"

namespace Zero
{

class ByteBufferBlock;

///ByteBuffer used for efficient appending. Structured as an array of blocks.
class ZeroShared ByteBuffer
{
public:
  typedef unsigned char byteType;

  //Block of data in the ByteBuffer
  struct Block
  {
    size_t Size;
    byteType* Data;
  };

  //Block range is used to iterate
  //through all blocks on the buffer.
  struct BlockRange
  {
    BlockRange(ByteBuffer* buffer);

    const Block& Front();
    bool Empty();
    void PopFront();
    BlockRange& All() { return *this; }
    const BlockRange& All() const { return *this; }
  private:
    void LoadBlock();
    size_t mBlockSize;
    size_t mLastBlockSize;
    byteType** mBlock;
    byteType** mLast;
    Block mCurrent;
  };

  ByteBuffer(size_t blockSize=512);
  ~ByteBuffer();

  //Get current position of writing
  size_t Tell();

  //Write bytes to the buffer.
  void Write(const byteType* data, size_t sizeInBytes);

  //Add Data to end of buffer.
  void Append(const byteType* data, size_t sizeInBytes);

  //Backs up by a certain number of bytes
  void Backup(size_t sizeInBytes);

  byte operator[](size_t index) const;
  byte& operator[](size_t index);

  //Get size of buffer.
  size_t GetSize() const {return mTotalSize;}

  //Get buffer as a string
  String ToString() const;

  //Extract into a raw memory buffer
  void ExtractInto(byteType* buffer, size_t bufferSizeInBytes) const;

  //Extract into a ByteBufferReader
  void ExtractInto(ByteBufferBlock& buffer) const;

  //Deallocate memory of blocks
  void Deallocate();

  //Get range of all blocks
  BlockRange Blocks();

private:
  ByteBuffer(const ByteBuffer&) {};
  void operator=(const ByteBuffer&) {};

protected:

  Array<byteType*> mBlocks;
  size_t mBlockSize;
  size_t mTotalSize;
  size_t mCurBlockSize;
  byteType* mCurBlockBuffer;
};

///ByteBufferBlock is a simple wrapper around a single contiguous block of memory.
///has same interface as a file.
class ByteBufferBlock
{
public:
  ByteBufferBlock();
  ~ByteBufferBlock();

  //Create buffer with the given size
  ByteBufferBlock(size_t size);

  //Create a buffer from existing data. If data is owned
  //buffer will deallocate data on destruction.
  ByteBufferBlock(byte* data, size_t size, bool owned);

  //Set data to use for this byte buffer
  void SetData(byte* data, size_t size, bool owned);

  //Set data block to use for this byte buffer
  void SetBlock(DataBlock block);

  //Read data from buffer.
  size_t Read(Status& status, byte* data, size_t sizeInBytes);

  //Write data to the buffer
  size_t Write(byte* data, size_t sizeInBytes);
  
  //Write a single byte of data
  size_t Write(byte value);

  //Seek to position
  void Seek(int offset, uint origin);

  //Pointer to current position in buffer.
  byte* GetCurrent();

  //Size of a buffer.
  size_t Size();

  //Return current position.
  size_t Tell();
  
  //Get start of block
  byte* GetBegin();

  //Deallocate memory if owned.
  void Deallocate();

private:
  byte* mData;
  size_t mSize;
  byte* mCurrent;
  bool mOwnsData;
  friend class ByteBuffer;
};

}
