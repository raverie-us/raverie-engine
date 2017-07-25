///////////////////////////////////////////////////////////////////////////////
///
/// \file StringRange.hpp
///
/// Authors: Chris Peters, Joshua Davis, Dane Curbow
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Containers/Hashing.hpp"
#include "String/String.hpp"

namespace Zero
{

const char* const cEmpty = "";
const int Utf8ContinunationByteSignature = 0x80; //10000000
const int Utf8ContinunationByteMask = 0xC0; //11000000

class ZeroShared StringRange
{
public:
  typedef Rune value_type;
  static const Rune InvalidRune;

  StringRange();
  StringRange(cstr cstring);
  StringRange(StringParam str);
  StringRange(StringRangeParam strRange);
  StringRange(StringIterator pbegin, StringIterator pend);
  StringRange(StringIterator pbegin, size_t sizeInBytes);
  StringRange(cstr currentByte, cstr endByte);
  StringRange(cstr originalBegin, cstr currentByte, cstr endByte);
  //For internal iterator use only and SubStringFromByteIndices
  explicit StringRange(StringParam orginalStr, cstr currentByte, cstr endByte);
  
  StringRange& operator=(StringRangeParam rhs);

  Rune Front() const;
  Rune Back() const;
  void PopFront();
  void PopFront(size_t n);
  void PopBack();
  bool Contains(StringIterator pos);
  int CompareTo(const StringRange& right) const;

  bool operator==(const StringRange& right) const;
  bool operator<(const StringRange& right) const;
  bool operator==(Rune rune) const;
  bool operator==(int i) const;
  bool operator==(char c) const;
  bool operator!=(Rune rune) const;
  bool operator!=(int i) const;
  bool operator!=(char c) const;

  StringRange FindFirstOf(Rune rune) const;
  StringRange FindFirstOf(StringRangeParam value) const;
  StringRange FindLastOf(Rune rune) const;
  StringRange FindLastOf(StringRangeParam value) const;

  StringRange FindFirstByBytes(cstr buffer, uint valueSizeInBytes) const;
  StringRange FindLastByBytes(cstr buffer, uint valueSizeInBytes) const;

  StringIterator FindFirstNonWhitespaceRuneIt() const;
  StringIterator FindLastNonWhitespaceRuneIt() const;
  Rune FindFirstNonWhitespaceRune() const;
  Rune FindLastNonWhitespaceRune() const;

  bool Contains(StringRangeParam value) const;
  bool StartsWith(StringRangeParam value) const;
  bool EndsWith(StringRangeParam value) const;
  StringRange FindRangeExclusive(StringRangeParam startRange, StringRangeParam endRange) const;
  StringRange FindRangeInclusive(StringRangeParam startRange, StringRangeParam endRange) const;
  String Replace(StringRangeParam oldValue, StringRangeParam newValue) const;
  StringSplitRange Split(StringRangeParam separator) const;
  StringRange Trim() const;
  StringRange TrimEnd() const;
  StringRange TrimStart() const;
  String ToUpper() const;
  String ToLower() const;

  bool IsAllUpper() const;
  bool IsAllWhitespace() const;
  StringRange SubString(StringIterator begin, StringIterator end) const;
  StringRange SubStringFromByteIndices(size_t startIndex, size_t endIndex) const;

  cstr Data() const;
  bool Empty() const;
  size_t ComputeRuneCount() const;
  size_t SizeInBytes() const;

  StringIterator Begin() const;
  StringIterator End() const;

  void IncrementByRune();
  void DecrementPointerByRune(cstr& ptr);
  Rune  ReadCurrentRune() const;

  bool ValidateByte(cstr byte) const;
  bool IsValid();
  
  bool IsContinuationByte(cstr byte) const;
  bool IsCurrentRuneWhitespace() const;
  bool IsCurrentRuneUpper() const;
  bool IsCurrentRuneLower() const;

  StringRange& All() { return *this; }
  const StringRange& All() const { return *this; }

  String mOriginalString;
  cstr mBegin;
  cstr mEnd;
};

//------------------------------------------------------------------- String Split Range
/// A range that splits a StringRange based upon a separator range.
class ZeroShared StringSplitRange
{
public:
  StringSplitRange(StringRange range, StringRange separator);
  StringRange Front();
  void PopFront();
  bool Empty();

  StringSplitRange& All() { return *this; }
  const StringSplitRange& All() const { return *this; }

  void SkipNext();

  StringRange mSeparator;
  StringRange mCurrentRange;
  StringRange mRemainingRange;
};

// String range vs cstr
inline bool operator==(const StringRange& left, cstr right);
inline bool operator==(cstr left, const StringRange& right);
inline bool operator!=(const StringRange& left, cstr right);
inline bool operator<(const StringRange& left, cstr right);
inline bool operator<(cstr left, const StringRange& right);

//----------------------------------------------------------- String Iterator
// An iterator for StringRange to make moving backwards in the StringRange safe
class StringIterator
{
public:
  typedef const StringIterator& StringIteratorParam;

  StringIterator();
  explicit StringIterator(StringParam orginalStr);
  StringIterator(StringParam orginalStr, cstr currentByte);
  StringIterator(StringIteratorParam it);
  ~StringIterator();

  StringIterator& operator=(StringIteratorParam rhs);

  bool operator==(StringIteratorParam rhs) const;
  bool operator!=(StringIteratorParam rhs) const;
  bool operator<=(StringIteratorParam rhs) const;
  bool operator>=(StringIteratorParam rhs) const;
  bool operator<(StringIteratorParam rhs) const;
  bool operator>(StringIteratorParam rhs) const;

  StringIterator operator+(uint numElements) const;
  StringIterator operator-(uint numElements) const;
  int operator-(StringIterator rhs) const;
  StringIterator& operator+=(uint numElements);
  StringIterator& operator-=(uint numElements);
  char operator[](uint numBytes) const;
  
  StringIterator& operator++();
  StringIterator& operator--();
  Rune operator*();
  Rune operator*() const;

  Rune  ReadCurrentRune() const;

  bool ValidByte() const;
  bool IsCurrentRuneWhitespace() const;
  bool IsUpper() const;
  bool IsLower() const;
  bool Empty() const;
  size_t GetPosition() const;

  cstr Data() const;

  StringRange mIteratorRange;
};

//----------------------------------------------------------- String Token Range
class StringTokenRange
{
public:
  StringTokenRange(StringRange stringRange, Rune delim);

  StringRange Front();
  void PopFront();
  bool Empty();

  StringRange curRange;
  StringRange internalRange;
  Rune mDelim;
};

inline bool operator==(const String& left, cstr right)
{
  return strcmp(left.Data(), right) == 0;
}

inline bool operator!=(const String& left, cstr right)
{
  return strcmp(left.Data(), right) != 0;
}

inline bool operator<(const String& left, cstr right)
{
  return strcmp(left.Data(), right) < 0;
}

//String vs other StringRange
inline bool operator==(const String& left, const StringRange& right)
{
  return left.SizeInBytes() == right.SizeInBytes() &&
    strncmp(left.Data(), right.Data(), right.SizeInBytes()) == 0;
}

inline bool operator==(const StringRange& left, const String& right)
{
  return left.SizeInBytes() == right.SizeInBytes() &&
    strncmp(left.Data(), right.Data(), left.SizeInBytes()) == 0;
}

inline bool operator<(const String& left, const StringRange& right)
{
  return strncmp(left.Data(), right.Data(), right.SizeInBytes()) < 0;
}

inline bool operator<(const StringRange& left, const String& right)
{
  return strncmp(left.Data(), right.Data(), left.SizeInBytes()) < 0;
}

//-------------------------------------------------------------------String range vs cstr
inline bool operator==(const StringRange& left, cstr right)
{
  return left.SizeInBytes() == strlen(right) &&
    strncmp(left.Data(), right, left.SizeInBytes()) == 0;
}

inline bool operator==(cstr left, const StringRange& right)
{
  return strlen(left) == right.SizeInBytes() &&
    strncmp(left, right.Data(), right.SizeInBytes()) == 0;
}

inline bool operator!=(const StringRange& left, cstr right)
{
  return !(left == right);
}

inline bool operator<(const StringRange& left, cstr right)
{
  return strncmp(left.Data(), right, left.SizeInBytes()) < 0;
}

inline bool operator<(cstr left, const StringRange& right)
{
  return strncmp(left, right.Data(), right.SizeInBytes()) < 0;
}

//--------------------------------------------------- Hash Policy (String Range)
//Hash policy for string range
template<>
struct ZeroShared HashPolicy<StringRange>
{
  inline size_t operator () (const StringRange& value) const
  {
    return HashString(value.mBegin, value.SizeInBytes());
  }

  inline bool Equal(const StringRange& left, const StringRange& right) const
  {
    return left == right;
  }

  template<typename stringType>
  inline bool Equal(const StringRange& left, const stringType& right) const
  {
    // use operator == to other type, usually strings
    return right == left;
  }
};

//--------------------------------------------------------- Hash Policy (String)
//Hash policy for String class.
template<>
struct ZeroShared HashPolicy<String>
{
  inline size_t operator () (const String& value) const
  {
    return value.Hash();
  }

  inline bool Equal(const String& left, const String& right) const
  {
    return left == right;
  }
};

//------------------------------------------------------- Move Operator (String)
template<>
struct MoveWithoutDestructionOperator<String>
{
  static inline void MoveWithoutDestruction(String* dest, String* source)
  {
    dest->mNode = source->mNode;
  }
};

//------------------------------------------------------------- Global Functions
// Wraps text input on space boundaries (does not add -)
String WordWrap(StringRange input, size_t maxLineLength);

} // namespace Zero
