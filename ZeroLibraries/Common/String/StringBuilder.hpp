///////////////////////////////////////////////////////////////////////////////
///
/// \file StringBuilder.hpp
/// Declaration of StringBuilder.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Containers/ByteBuffer.hpp"

namespace Zero
{

//Simple String Builder
ZeroShared String BuildString(StringRange a, StringRange b);
ZeroShared String BuildString(StringRange a, StringRange b, StringRange c);
ZeroShared String BuildString(StringRange a, StringRange b, StringRange c, StringRange d);
ZeroShared String BuildString(StringRange a, StringRange b, StringRange c, StringRange d, StringRange e);
ZeroShared String BuildString(StringRange a, StringRange b, StringRange c, StringRange d, StringRange e, StringRange f);
ZeroShared String BuildString(StringRange** ranges, uint count);

ZeroShared String StringJoin(Array<String>& strings, StringParam joinToken);


///Extension of ByteBuffer for building strings. Has
///stream operators overloaded so it can act as a replacement
///for io streams.
class ZeroShared StringBuilder : public ByteBuffer
{
public:
  template<typename type>
  friend struct MoveWithoutDestructionOperator;

  StringBuilder(){};
  ~StringBuilder(){};
  void operator+=(StringRange adapter){Append(adapter);}
  void operator+=(Rune rune){Append(rune);}
  void Append(StringRange adapter);
  void Append(char character);
  void AppendFormat(cstr format, ...);
  void Append(Rune rune);
  void Append(cstr begin, uint sizeInBytes);
  char& operator[](size_t index);

  void Repeat(size_t count, StringParam str);

private:
  StringBuilder(const StringBuilder&) {};
  void operator=(const StringBuilder&) {};
};

template<>
struct MoveWithoutDestructionOperator<StringBuilder>
{
  static inline void MoveWithoutDestruction(StringBuilder* dest, StringBuilder* source)
  {
    MoveWithoutDestructionOperator< Array<ByteBuffer::byteType*> >::MoveWithoutDestruction(&dest->mBlocks, &source->mBlocks);
    
    dest->mBlockSize = source->mBlockSize;
    dest->mTotalSize = source->mTotalSize;
    dest->mCurBlockSize = source->mCurBlockSize;
    dest->mCurBlockBuffer = source->mCurBlockBuffer;
  }
};

inline StringBuilder& operator<<(StringBuilder& builder, StringRange range)
{
  builder.Append(range);
  return builder;
}

inline StringBuilder& operator<<(StringBuilder& builder, cstr text)
{
  builder.Append(text);
  return builder;
}

inline StringBuilder& operator<<(StringBuilder& builder, char character)
{
  builder.Append(character);
  return builder;
}

inline StringBuilder& operator<<(StringBuilder& builder, Rune rune)
{
  builder.Append(rune);
  return builder;
}

inline StringBuilder& operator<<(StringBuilder& builder, StringParam str)
{
  builder.Append(str);
  return builder;
}

//Generic templated
template<typename type>
inline StringBuilder& operator<<(StringBuilder& builder, const type& value)
{
  const uint bufferSize = 128;
  char buffer[bufferSize];
  uint size = ToBuffer(buffer, bufferSize, value);
  builder.Append(String(buffer, buffer+size));
  return builder;
}


}//namespace Zero
