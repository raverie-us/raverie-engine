// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

BinaryBufferSaver::BinaryBufferSaver()
{
}

BinaryBufferSaver::~BinaryBufferSaver()
{
}

void BinaryBufferSaver::Open()
{
}

void BinaryBufferSaver::Data(byte* data, uint sizeInBytes)
{
  mBuffer.Append(data, sizeInBytes);
}

uint BinaryBufferSaver::GetSize()
{
  return mBuffer.GetSize();
}

void BinaryBufferSaver::ExtractInto(DataBlock& block)
{
  mBuffer.ExtractInto(block.Data, block.Size);
}

void BinaryBufferSaver::ExtractInto(byte* data, uint size)
{
  mBuffer.ExtractInto(data, size);
}

DataBlock BinaryBufferSaver::ExtractAsDataBlock()
{
  uint size = mBuffer.GetSize();
  DataBlock dataBlock;
  dataBlock.Data = (byte*)zAllocate(size);
  dataBlock.Size = size;
  ExtractInto(dataBlock);
  return dataBlock;
}

void BinaryBufferSaver::Deallocate()
{
  mBuffer.Deallocate();
}

void BinaryBufferLoader::SetBuffer(byte* data, uint size)
{
  mBufferSize = size;
  mCurrentPosition = data;
  mBuffer = data;
}

void BinaryBufferLoader::SetBlock(DataBlock block)
{
  mBufferSize = block.Size;
  mCurrentPosition = block.Data;
  mBuffer = block.Data;
}

bool BinaryBufferLoader::TestForObjectEnd(BoundType** data)
{
  *data = nullptr;

  size_t bytesToRead = sizeof(u32);
  byte* endOfBuffer = mBuffer + mBufferSize;
  byte* ifMoved = mCurrentPosition + bytesToRead;
  if (ifMoved < endOfBuffer)
  {
    //*data = *(BoundType**)mCurrentPosition;
    if (*(u32*)mCurrentPosition == BinaryEndSignature)
    {
      return false;
    }
    else
    {
      mCurrentPosition += bytesToRead;
      return true;
    }
  }
  else
  {
    // end of buffer
    return false;
  }
}

void BinaryBufferLoader::Data(byte* data, uint sizeInBytes)
{
  const bool bufferOverrun = mCurrentPosition + sizeInBytes > mBuffer + mBufferSize;
  ErrorIf(bufferOverrun, "Access buffer out of range.");

  if (!bufferOverrun)
  {
    memcpy(data, mCurrentPosition, sizeInBytes);
    mCurrentPosition += sizeInBytes;
  }
}

bool BinaryBufferLoader::StringField(cstr typeName, cstr fieldName, StringRange& stringRange)
{
  u32 size = 0;
  Data((byte*)&size, sizeof(size));

  byte* start = mCurrentPosition;
  mCurrentPosition += size;
  byte* end = mCurrentPosition;

  stringRange = StringRange((char*)start, (char*)end);
  return true;
}

bool BinaryFileLoader::OpenFile(Status& status, cstr filename)
{
  return mFile.Open(filename, FileMode::Read, FileAccessPattern::Sequential);
}

void BinaryFileLoader::Close()
{
  mFile.Close();
}

bool BinaryFileLoader::TestForObjectEnd(BoundType** data)
{
  *data = nullptr;

  size_t bytesToRead = sizeof(u32);
  FilePosition fp = mFile.Tell();
  if (fp + bytesToRead < mFile.Size())
  {
    u32 end = BinaryEndSignature;
    Data((byte*)&end, bytesToRead);
    if (end == BinaryEndSignature)
    {
      // End of object
      // Move back to prevent reading the end tag
      mFile.Seek(-(int)bytesToRead, SeekOrigin::Current);
      return false;
    }
    return true;
  }
  else
  {
    // End of file
    return false;
  }
}

void BinaryFileLoader::Data(byte* data, uint sizeInBytes)
{
  Status status;
  mFile.Read(status, data, sizeInBytes);
}

bool BinaryFileLoader::StringField(cstr typeName, cstr fieldName, StringRange& stringRange)
{
  Status status;
  u32 size = 0;
  mFile.Read(status, (byte*)&size, sizeof(size));

  if (size < 512)
  {
    mFile.Read(status, (byte*)mTempSpace, size);
    stringRange = StringRange(mTempSpace, mTempSpace, mTempSpace + size);
  }

  return true;
}

bool BinaryFileSaver::Open(Status& status, cstr filename)
{
  return mFile.Open(filename, FileMode::Write, FileAccessPattern::Sequential);
}

void BinaryFileSaver::Close()
{
  mFile.Close();
}

void BinaryFileSaver::Data(byte* data, uint sizeInBytes)
{
  mFile.Write(data, sizeInBytes);
}

} // namespace Raverie
