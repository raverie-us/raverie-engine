// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

DeclareEnum3(SeekOrigin, Current, Begin, End);

class Stream
{
public:
  virtual u64 Size() = 0;
  virtual bool Seek(u64 filePosition, SeekOrigin::Enum origin = SeekOrigin::Begin) = 0;
  virtual u64 Tell() = 0;
  virtual size_t Write(byte* data, size_t sizeInBytes) = 0;
  virtual size_t Read(byte* data, size_t sizeInBytes) = 0;
  virtual bool HasData() = 0;
  virtual bool IsEof() = 0;
  virtual void Flush() = 0;

  // Reads data into a block. By default this will just call Read and fill a
  // ByteBufferBlock with owned data. As an optimization, streams like
  // FixedMemoryStream may just return a direct pointer to the data. Note that
  // the Stream should not be written to while the user is holding the
  // ByteBufferBlock as it may invalidate (e.g. ArrayByteMemoryStream...). Note
  // that reading 0 bytes will NOT set status to Failed.
  virtual void ReadMemoryBlock(Status& status, ByteBufferBlock& block, size_t size);

  // Helper non virtual functions
  // These functions assert if they're outside the range (and return 0 / write
  // nothing)
  size_t Peek(byte* data, size_t sizeInBytes);
  void PeekMemoryBlock(Status& status, ByteBufferBlock& block, size_t size);
  byte PeekByte();
  byte PeekByteAt(u64 index);
  byte ReadByte();
  byte ReadByteAt(u64 index);
  void WriteByte(byte value);
  void WriteByteAt(byte value, size_t index);
};

class FixedMemoryStream : public Stream
{
public:
  FixedMemoryStream();
  FixedMemoryStream(byte* data, size_t size);
  FixedMemoryStream(ByteBuffer::Block& block);
  FixedMemoryStream(DataBlock& block);
  FixedMemoryStream(ByteBufferBlock& block);
  FixedMemoryStream(StringParam string);

  u64 Size() override;
  bool Seek(u64 filePosition, SeekOrigin::Enum origin = SeekOrigin::Begin) override;
  u64 Tell() override;
  size_t Write(byte* data, size_t sizeInBytes) override;
  size_t Read(byte* data, size_t sizeInBytes) override;
  bool HasData() override;
  bool IsEof() override;
  void Flush() override;

  // We optimize this case by returning non-owned memory instantly (no memcpy).
  void ReadMemoryBlock(Status& status, ByteBufferBlock& block, size_t sizeInBytes) override;

  // Internal
  byte* mData;
  u64 mSize;
  size_t mCurrent;
};

class ByteBufferMemoryStream : public Stream
{
public:
  ByteBufferMemoryStream();
  ByteBufferMemoryStream(const byte* data, size_t size);
  ByteBufferMemoryStream(const ByteBuffer::Block& block);
  ByteBufferMemoryStream(const DataBlock& block);
  ByteBufferMemoryStream(const ByteBufferBlock& block);
  ByteBufferMemoryStream(const Array<byte>& block);

  u64 Size() override;
  bool Seek(u64 filePosition, SeekOrigin::Enum origin = SeekOrigin::Begin) override;
  u64 Tell() override;
  size_t Write(byte* data, size_t sizeInBytes) override;
  size_t Read(byte* data, size_t sizeInBytes) override;
  bool HasData() override;
  bool IsEof() override;
  void Flush() override;

  // Internal
  ByteBuffer mBuffer;
};

class ArrayByteMemoryStream : public Stream
{
public:
  ArrayByteMemoryStream();
  ArrayByteMemoryStream(const byte* data, size_t size);
  ArrayByteMemoryStream(const ByteBuffer::Block& block);
  ArrayByteMemoryStream(const DataBlock& block);
  ArrayByteMemoryStream(const ByteBufferBlock& block);
  ArrayByteMemoryStream(const Array<byte>& block);

  u64 Size() override;
  bool Seek(u64 filePosition, SeekOrigin::Enum origin = SeekOrigin::Begin) override;
  u64 Tell() override;
  size_t Write(byte* data, size_t sizeInBytes) override;
  size_t Read(byte* data, size_t sizeInBytes) override;
  bool HasData() override;
  bool IsEof() override;
  void Flush() override;

  // We optimize this case by returning non-owned memory instantly (no memcpy).
  void ReadMemoryBlock(Status& status, ByteBufferBlock& block, size_t sizeInBytes) override;

  // Internal
  Array<byte> mBuffer;
  size_t mCurrent;
};

} // namespace Zero
