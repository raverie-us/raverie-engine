// MIT Licensed (see LICENSE.md).
#pragma once
#include "Containers/ByteBuffer.hpp"

namespace Raverie
{

// Simple String Builder
String BuildString(StringRange a, StringRange b);
String BuildString(StringRange a, StringRange b, StringRange c);
String BuildString(StringRange a, StringRange b, StringRange c, StringRange d);
String BuildString(StringRange a, StringRange b, StringRange c, StringRange d, StringRange e);
String BuildString(StringRange a, StringRange b, StringRange c, StringRange d, StringRange e, StringRange f);
String
BuildString(StringRange a, StringRange b, StringRange c, StringRange d, StringRange e, StringRange f, StringRange g);
String BuildString(StringRange** ranges, uint count);

String StringJoin(Array<String>& strings, StringParam joinToken);

/// Extension of ByteBuffer for building strings. Has
/// stream operators overloaded so it can act as a replacement
/// for io streams.
class StringBuilder : public ByteBuffer
{
public:
  template <typename type>
  friend struct MoveWithoutDestructionOperator;

  StringBuilder(){};
  ~StringBuilder(){};
  void operator+=(StringRange adapter)
  {
    Append(adapter);
  }
  void operator+=(Rune rune)
  {
    Append(rune);
  }
  void Append(StringRange adapter);
  void Append(char character);
  void AppendFormat(cstr format, ...);
  void Append(Rune rune);
  void Append(cstr begin, uint sizeInBytes);
  char& operator[](size_t index);

  void Repeat(size_t count, StringParam str);

private:
  StringBuilder(const StringBuilder&){};
  void operator=(const StringBuilder&){};
};

template <>
struct MoveWithoutDestructionOperator<StringBuilder>
{
  static inline void MoveWithoutDestruction(StringBuilder* dest, StringBuilder* source)
  {
    MoveWithoutDestructionOperator<Array<ByteBuffer::byteType*>>::MoveWithoutDestruction(&dest->mBlocks,
                                                                                         &source->mBlocks);

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

// Generic templated
template <typename type>
inline StringBuilder& operator<<(StringBuilder& builder, const type& value)
{
  const uint bufferSize = 128;
  char buffer[bufferSize];
  uint size = ToBuffer(buffer, bufferSize, value);
  builder.Append(String(buffer, buffer + size));
  return builder;
}

template <typename T>
inline String GetBinaryString(const T& value)
{
  StringBuilder result;
  byte* valueCursor = (byte*)&value;

  // Read every byte (from left to right)
  for (uint i = 0; i < sizeof(value); ++i)
  {
    // Read every bit in the byte (from left to right)
    for (uint j = 0; j < 8; ++j)
      result += *valueCursor & LBIT(j) ? '1' : '0';

    // Next byte
    result += ' ';
    ++valueCursor;
  }

  return result.ToString();
}

} // namespace Raverie
