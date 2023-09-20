// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

void ShaderStreamWriter::Write(uint32 word)
{
  WriteWord(word);
}

void ShaderStreamWriter::Write(uint16 highOrder, uint16 lowOrder)
{
  int32 data;
  data = 0xFFFF0000 & (highOrder << 16);
  data |= 0x0000FFFF & (lowOrder << 0);
  Write(data);
}

void ShaderStreamWriter::Write(uint8 a, uint8 b, uint8 c, uint8 d)
{
  int32 word;
  word = 0xFF000000 & (a << 24);
  word |= 0x00FF0000 & (b << 16);
  word |= 0x0000FF00 & (c << 8);
  word |= 0x000000FF & (d << 0);
  Write(word);
}

void ShaderStreamWriter::WriteInstruction(uint16 size, uint16 instruction)
{
  Write(size, instruction);
}

void ShaderStreamWriter::WriteInstruction(uint16 size, uint16 instruction, uint32 data0)
{
  Write(size, instruction);
  Write(data0);
}

void ShaderStreamWriter::WriteInstruction(uint16 size, uint16 instruction, uint32 data0, uint32 data1)
{
  Write(size, instruction);
  Write(data0);
  Write(data1);
}

void ShaderStreamWriter::WriteInstruction(uint16 size, uint16 instruction, uint32 data0, uint32 data1, uint32 data2)
{
  Write(size, instruction);
  Write(data0);
  Write(data1);
  Write(data2);
}

void ShaderStreamWriter::WriteInstruction(uint16 instruction, Array<uint32>& args)
{
  int16 size = (int16)args.Size() + 1;
  Write(size, instruction);
  for (size_t i = 0; i < args.Size(); ++i)
    Write(args[i]);
}

void ShaderStreamWriter::Write(StringParam text)
{
  size_t byteCount = text.SizeInBytes();
  size_t totalSize = GetPaddedByteCount(text);

  size_t wordCount = byteCount / 4;
  byte* data = (byte*)text.Data();
  size_t i = 0;
  while (i < totalSize)
  {
    int32 word = 0;
    for (size_t j = 0; j < 4 && i + j < byteCount; ++j)
    {
      int32 shiftedData = *(data + i + j) << (j * 8);
      word = word | shiftedData;
    }
    Write(word);
    i += 4;
  }
}

size_t ShaderStreamWriter::GetPaddedByteCount(StringParam text)
{
  size_t byteCount = text.SizeInBytes();
  size_t paddedByteCount = byteCount + 1;
  int totalSize = (paddedByteCount) / 4;
  if ((paddedByteCount % 4) != 0)
    ++totalSize;
  totalSize *= 4;
  return totalSize;
}

void ShaderByteStream::Load(Array<uint32>& words)
{
  size_t byteCount = words.Size() * 4;
  mData.Resize(byteCount);
  memcpy(mData.Data(), (byte*)words.Data(), byteCount);
}

void ShaderByteStream::Load(StringParam str)
{
  mData.Resize(str.SizeInBytes());
  Load((byte*)str.Data(), str.SizeInBytes());
}

void ShaderByteStream::Load(const char* source, size_t sizeInBytes)
{
  Load((byte*)source, sizeInBytes);
}

void ShaderByteStream::Load(const byte* source, size_t sizeInBytes)
{
  mData.Resize(sizeInBytes);
  memcpy(mData.Data(), source, sizeInBytes);
}

void ShaderByteStream::LoadWords(const uint32* data, size_t wordCount)
{
  size_t byteCount = wordCount * 4;
  Load((const byte*)data, byteCount);
}

void ShaderByteStream::SaveTo(Array<uint32>& words)
{
  size_t sizeInBytes = mData.Size();
  size_t wordCount = sizeInBytes / 4;
  words.Resize(wordCount);
  memcpy(words.Data(), (byte*)mData.Data(), sizeInBytes);
}

String ShaderByteStream::ToString()
{
  String str = String((cstr)mData.Data(), mData.Size());
  return str;
}

byte* ShaderByteStream::Data()
{
  return mData.Data();
}

size_t ShaderByteStream::ByteCount() const
{
  size_t byteCount = mData.Size();
  return byteCount;
}

size_t ShaderByteStream::WordCount() const
{
  size_t byteCount = ByteCount();
  size_t wordCount = byteCount / 4;
  return wordCount;
}

ShaderByteStreamWriter::ShaderByteStreamWriter()
{
  mStreamIsOwned = true;
  mByteStream = new ShaderByteStream();
}

ShaderByteStreamWriter::ShaderByteStreamWriter(ShaderByteStream* stream)
{
  mStreamIsOwned = false;
  mByteStream = stream;
}

ShaderByteStreamWriter::~ShaderByteStreamWriter()
{
  if (mStreamIsOwned)
    delete mByteStream;
}

void ShaderByteStreamWriter::WriteWord(uint32 word)
{
  byte* bytes = (byte*)&word;
  for (size_t i = 0; i < 4; ++i)
    mByteStream->mData.PushBack(bytes[i]);
}

byte* ShaderByteStreamWriter::Data()
{
  return mByteStream->mData.Data();
}

size_t ShaderByteStreamWriter::ByteCount() const
{
  return mByteStream->ByteCount();
}

size_t ShaderByteStreamWriter::WordCount() const
{
  return mByteStream->WordCount();
}

} // namespace Raverie
