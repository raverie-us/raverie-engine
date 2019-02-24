// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{
void Stream::ReadMemoryBlock(Status& status, ByteBufferBlock& block, size_t sizeInBytes)
{
  byte* data = (byte*)zAllocate(sizeInBytes);
  if (data == nullptr)
  {
    status.SetFailed("Failed to allocate memory to read the stream");
    return;
  }
  size_t amountRead = Read(data, sizeInBytes);
  block.SetData(data, amountRead, true);
}

byte Stream::ReadByte()
{
  byte value;
  if (Read(&value, 1))
    return value;

  Error("ReadByte failed. Make sure the Stream is within the bounds");
  return 0;
}

byte Stream::ReadByteAt(u64 index)
{
  Seek(index, SeekOrigin::Begin);
  return ReadByte();
}

size_t Stream::Peek(byte* data, size_t sizeInBytes)
{
  u64 position = Tell();
  size_t result = Read(data, sizeInBytes);
  Seek(position);
  return result;
}

void Stream::PeekMemoryBlock(Status& status, ByteBufferBlock& block, size_t size)
{
  u64 position = Tell();
  ReadMemoryBlock(status, block, size);
  Seek(position);
}

byte Stream::PeekByte()
{
  u64 position = Tell();
  byte value = ReadByte();
  Seek(position);
  return value;
}

byte Stream::PeekByteAt(u64 index)
{
  u64 position = Tell();
  byte value = ReadByteAt(index);
  Seek(position);
  return value;
}

void Stream::WriteByte(byte value)
{
  if (Write(&value, 1) == 0)
    Error("WriteByte failed. Make sure the Stream is within the bounds");
}

void Stream::WriteByteAt(byte value, size_t index)
{
  Seek(index, SeekOrigin::Begin);
  return WriteByte(value);
}

FixedMemoryStream::FixedMemoryStream() : FixedMemoryStream(nullptr, 0)
{
}

FixedMemoryStream::FixedMemoryStream(byte* data, size_t size) : mData(data), mSize(size), mCurrent(0)
{
}

FixedMemoryStream::FixedMemoryStream(ByteBuffer::Block& block) : FixedMemoryStream(block.Data, block.Size)
{
}

FixedMemoryStream::FixedMemoryStream(DataBlock& block) : FixedMemoryStream(block.Data, block.Size)
{
}

FixedMemoryStream::FixedMemoryStream(ByteBufferBlock& block) : FixedMemoryStream(block.GetBegin(), block.Size())
{
}

FixedMemoryStream::FixedMemoryStream(StringParam string) : FixedMemoryStream((byte*)string.Data(), string.SizeInBytes())
{
}

u64 FixedMemoryStream::Size()
{
  return mSize;
}

bool FixedMemoryStream::Seek(u64 filePosition, SeekOrigin::Enum origin)
{
  switch (origin)
  {
  case SeekOrigin::Begin:
    mCurrent = (size_t)filePosition;
    break;

  case SeekOrigin::Current:
    mCurrent += (size_t)filePosition;
    break;

  case SeekOrigin::End:
    mCurrent = (size_t)(mSize + filePosition);
    break;

  default:
    Error("Invalid SeekOrigin");
    break;
  }

  if ((u64)mCurrent > mSize)
    mCurrent = (size_t)mSize;

  return true;
}

u64 FixedMemoryStream::Tell()
{
  return mCurrent;
}

size_t FixedMemoryStream::Write(byte* data, size_t sizeInBytes)
{
  if (!HasData())
    return 0;

  size_t amountLeft = (size_t)(mSize - mCurrent);
  if (amountLeft < sizeInBytes)
    sizeInBytes = amountLeft;

  memcpy(mData + mCurrent, data, sizeInBytes);
  mCurrent += sizeInBytes;
  return sizeInBytes;
}

size_t FixedMemoryStream::Read(byte* data, size_t sizeInBytes)
{
  if (!HasData())
    return 0;

  size_t amountLeft = (size_t)(mSize - mCurrent);
  if (amountLeft < sizeInBytes)
    sizeInBytes = amountLeft;

  memcpy(data, mData + mCurrent, sizeInBytes);
  mCurrent += sizeInBytes;
  return sizeInBytes;
}

bool FixedMemoryStream::HasData()
{
  return mCurrent < mSize;
}

bool FixedMemoryStream::IsEof()
{
  return !HasData();
}

void FixedMemoryStream::Flush()
{
}

void FixedMemoryStream::ReadMemoryBlock(Status& status, ByteBufferBlock& block, size_t sizeInBytes)
{
  if (!HasData())
  {
    block.SetData(nullptr, 0, false);
    return;
  }

  size_t amountLeft = (size_t)(mSize - Tell());
  if (amountLeft < sizeInBytes)
    sizeInBytes = amountLeft;

  block.SetData(mData + mCurrent, sizeInBytes, false);
  mCurrent += sizeInBytes;
}

ByteBufferMemoryStream::ByteBufferMemoryStream()
{
}

ByteBufferMemoryStream::ByteBufferMemoryStream(const byte* data, size_t size)
{
  mBuffer.Write(data, size);
}

ByteBufferMemoryStream::ByteBufferMemoryStream(const ByteBuffer::Block& block) :
    ByteBufferMemoryStream(block.Data, block.Size)
{
}

ByteBufferMemoryStream::ByteBufferMemoryStream(const DataBlock& block) : ByteBufferMemoryStream(block.Data, block.Size)
{
}

ByteBufferMemoryStream::ByteBufferMemoryStream(const ByteBufferBlock& block) :
    ByteBufferMemoryStream(const_cast<ByteBufferBlock&>(block).GetBegin(), const_cast<ByteBufferBlock&>(block).Size())
{
}

ByteBufferMemoryStream::ByteBufferMemoryStream(const Array<byte>& block) :
    ByteBufferMemoryStream(block.Data(), block.Size())
{
}

u64 ByteBufferMemoryStream::Size()
{
  return mBuffer.GetSize();
}

bool ByteBufferMemoryStream::Seek(u64 filePosition, SeekOrigin::Enum origin)
{
  Error("Seek is not currently supported on ByteBufferMemoryStream. "
        "ByteBuffer needs to implement a full Seek instead of just Backup.");
  return false;
}

u64 ByteBufferMemoryStream::Tell()
{
  // ByteBuffer is always at the end
  return mBuffer.GetSize();
}

size_t ByteBufferMemoryStream::Write(byte* data, size_t sizeInBytes)
{
  if (!HasData())
    return 0;

  mBuffer.Write(data, sizeInBytes);
  return sizeInBytes;
}

size_t ByteBufferMemoryStream::Read(byte* data, size_t sizeInBytes)
{
  Error("Reading on ByteBufferMemoryStream is not supported");
  return 0;
}

bool ByteBufferMemoryStream::HasData()
{
  return false;
}

bool ByteBufferMemoryStream::IsEof()
{
  return false;
}

void ByteBufferMemoryStream::Flush()
{
}

ArrayByteMemoryStream::ArrayByteMemoryStream()
{
}

ArrayByteMemoryStream::ArrayByteMemoryStream(const byte* data, size_t size) : mCurrent(0)
{
  mBuffer.Insert(mBuffer.Begin(), data, data + size);
}

ArrayByteMemoryStream::ArrayByteMemoryStream(const ByteBuffer::Block& block) :
    ArrayByteMemoryStream(block.Data, block.Size)
{
}

ArrayByteMemoryStream::ArrayByteMemoryStream(const DataBlock& block) : ArrayByteMemoryStream(block.Data, block.Size)
{
}

ArrayByteMemoryStream::ArrayByteMemoryStream(const ByteBufferBlock& block) :
    ArrayByteMemoryStream(const_cast<ByteBufferBlock&>(block).GetBegin(), const_cast<ByteBufferBlock&>(block).Size())
{
}

ArrayByteMemoryStream::ArrayByteMemoryStream(const Array<byte>& block) :
    ArrayByteMemoryStream(block.Data(), block.Size())
{
}

u64 ArrayByteMemoryStream::Size()
{
  return mBuffer.Size();
}

bool ArrayByteMemoryStream::Seek(u64 filePosition, SeekOrigin::Enum origin)
{
  switch (origin)
  {
  case SeekOrigin::Begin:
    mCurrent = (size_t)filePosition;
    break;

  case SeekOrigin::Current:
    mCurrent += (size_t)filePosition;
    break;

  case SeekOrigin::End:
    mCurrent = (size_t)(mBuffer.Size() + filePosition);
    break;

  default:
    Error("Invalid SeekOrigin");
    break;
  }

  if (mCurrent > mBuffer.Size())
    mCurrent = mBuffer.Size();

  return true;
}

u64 ArrayByteMemoryStream::Tell()
{
  return mCurrent;
}

size_t ArrayByteMemoryStream::Write(byte* data, size_t sizeInBytes)
{
  if (!HasData())
    return 0;

  size_t amountLeft = mBuffer.Size() - mCurrent;
  if (amountLeft < sizeInBytes)
    sizeInBytes = amountLeft;

  memcpy(mBuffer.Data() + mCurrent, data, sizeInBytes);
  mCurrent += sizeInBytes;
  return sizeInBytes;
}

size_t ArrayByteMemoryStream::Read(byte* data, size_t sizeInBytes)
{
  if (!HasData())
    return 0;

  size_t amountLeft = mBuffer.Size() - mCurrent;
  if (amountLeft < sizeInBytes)
    sizeInBytes = amountLeft;

  memcpy(data, mBuffer.Data() + mCurrent, sizeInBytes);
  mCurrent += sizeInBytes;
  return sizeInBytes;
}

bool ArrayByteMemoryStream::HasData()
{
  return mCurrent < mBuffer.Size();
}

bool ArrayByteMemoryStream::IsEof()
{
  return !HasData();
}

void ArrayByteMemoryStream::Flush()
{
}

void ArrayByteMemoryStream::ReadMemoryBlock(Status& status, ByteBufferBlock& block, size_t sizeInBytes)
{
  if (!HasData())
  {
    block.SetData(nullptr, 0, false);
    return;
  }

  size_t amountLeft = mBuffer.Size() - mCurrent;
  if (amountLeft < sizeInBytes)
    sizeInBytes = amountLeft;

  block.SetData(mBuffer.Data() + mCurrent, sizeInBytes, false);
  mCurrent += sizeInBytes;
}

} // namespace Zero
