///////////////////////////////////////////////////////////////////////////////
///
/// \file String.cpp
/// Implementation of the StringBuilder and ByteBuffer.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
char* AppendRange(char* bufferPos, char* bufferEnd, StringRange& b)
{
  ZeroCStringCopy(bufferPos, bufferEnd-bufferPos, b.mBegin, b.SizeInBytes());
  bufferPos+=b.SizeInBytes();
  return bufferPos;
}

String BuildString(StringRange a, StringRange b)
{
  //+1 extra for null terminator
  const size_t bufferSize = a.SizeInBytes() + b.SizeInBytes() + 1;
  char* buffer = (char*)alloca(bufferSize);
  char* bufferPos = buffer;
  char* bufferEnd = buffer + bufferSize;

  bufferPos = AppendRange(bufferPos, bufferEnd, a);
  bufferPos = AppendRange(bufferPos, bufferEnd, b);
  *bufferPos = '\0';

  return String(buffer);
}

String BuildString(StringRange a, StringRange b, StringRange c)
{
  StringRange* strings[] = {&a, &b, &c};
  return BuildString(strings, 3);
}

String BuildString(StringRange a, StringRange b, StringRange c, StringRange d)
{
  StringRange* strings[] = {&a, &b, &c, &d};
  return BuildString(strings, 4);
}

String BuildString(StringRange a, StringRange b, StringRange c, StringRange d, StringRange e)
{
  StringRange* strings[] = {&a, &b, &c, &d, &e};
  return BuildString(strings, 5);
}

String BuildString(StringRange a, StringRange b, StringRange c, StringRange d, StringRange e, StringRange f)
{
  StringRange* strings[] = { &a, &b, &c, &d, &e, &f };
  return BuildString(strings, 6);
}

String BuildString(StringRange** ranges, uint count)
{
  //+1 extra for null terminator
  size_t bufferSize = 1;
  for(size_t i=0;i<count;++i)
    bufferSize += ranges[i]->SizeInBytes();

  char* buffer = (char*)alloca(bufferSize);
  char* bufferPos = buffer;
  char* bufferEnd = buffer + bufferSize;

  for(size_t i=0;i<count;++i)
    bufferPos = AppendRange(bufferPos, bufferEnd, *ranges[i]);

  *bufferPos = '\0';

  return String(buffer);
}

String StringJoin(Array<String>& strings, StringParam joinToken)
{
  //join together all of the strings with the join token in between
  if(strings.Size() == 0)
    return String();

  if(strings.Size() == 1)
    return strings[0];

  StringBuilder builder;
  builder.Append(strings[0]);
  for(uint i = 1; i < strings.Size(); ++i)
  {
    builder.Append(joinToken);
    builder.Append(strings[i]);
  }
  return builder.ToString();
}


void StringBuilder::Append(StringRange range)
{
  ByteBuffer::Append((byteType*)range.Data(), range.SizeInBytes());
}

void StringBuilder::Append(char character)
{
  ByteBuffer::Append((const byte*)&character, sizeof(character));
}

void StringBuilder::AppendFormat(cstr format, ...)
{
  va_list va;
  va_start(va, format);
  String str = String::FormatArgs(format, va);
  Append(str.All());
  va_end(va);
}

void StringBuilder::Append(cstr begin, uint sizeInBytes)
{
  ByteBuffer::Append((byteType*)begin, sizeInBytes);
}

void StringBuilder::Append(Rune rune)
{
  byte utf8Bytes[4];
  int bytesRead = UTF8::UnpackUtf8RuneIntoBuffer(rune, utf8Bytes);
  ByteBuffer::Append(utf8Bytes, bytesRead);
}

char& StringBuilder::operator[](size_t index)
{
  ErrorIf(index >= mTotalSize, "Index is out of bounds");

  size_t blockIndex = index / mBlockSize;

  byteType* block = mBlocks[blockIndex];

  ErrorIf(block == nullptr, "The block should be filled out");

  return (char&)block[index %  mBlockSize];
}

void StringBuilder::Repeat(size_t count, StringParam str)
{
  for(size_t i = 0; i < count; ++i)
    Append(str);
}

}//namespace Zero
