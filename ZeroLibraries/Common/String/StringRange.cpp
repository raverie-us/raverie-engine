///////////////////////////////////////////////////////////////////////////////
///
/// \file StringRange.cpp
///
/// Authors: Chris Peters, Joshua Davis, Dane Curbow
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------StringRange
const Rune StringRange::InvalidRune = Rune();

StringRange::StringRange()
  : mOriginalString(cEmpty),
    mBegin(cEmpty),
    mEnd(cEmpty)
{
}

StringRange::StringRange(cstr cstring)
  : mOriginalString(cstring)
{
   mBegin = mOriginalString.Data();
   mEnd = mBegin + mOriginalString.SizeInBytes();
}

StringRange::StringRange(StringParam str)
  : mOriginalString(str),
    mBegin(str.Data()),
    mEnd(str.Data() + str.SizeInBytes())
{
}

StringRange::StringRange(StringRangeParam strRange)
  : mOriginalString(strRange.mOriginalString),
    mBegin(strRange.mBegin),
    mEnd(strRange.mEnd)
{
}

StringRange::StringRange(StringIterator pbegin, StringIterator pend)
  : mOriginalString(pbegin.mIteratorRange.mOriginalString),
    mBegin(pbegin.Data()),
    mEnd(pend.Data())
{
}

StringRange::StringRange(StringIterator pbegin, size_t sizeInBytes)
  : mOriginalString(pbegin.mIteratorRange.mOriginalString),
    mBegin(pbegin.Data())
{
  int bytesToEndFromIterator = mOriginalString.SizeInBytes() - (pbegin.Data() - mOriginalString.Data());
  // if someone really messed up bytes to end could be negative, use int.
  if ((int)sizeInBytes > bytesToEndFromIterator)
    mEnd = pbegin.Data() + bytesToEndFromIterator;
  else
    mEnd = pbegin.Data() + sizeInBytes;
}

// Treats begin byte as the beginning of the original string
// This constructor can be very dangerous if you compare this constructed range to 
// any existing ranges/strings
StringRange::StringRange(cstr currentByte, cstr endByte)
  : mOriginalString(currentByte, endByte - currentByte)
{
  // passing in a cstr will allocate a new buffer for the string so we need
  // to properly set up the begin and end values to point at the relevant data
  mBegin = mOriginalString.Data();
  mEnd = mBegin + (endByte - currentByte);
}

StringRange::StringRange(cstr originalBegin, cstr currentByte, cstr endByte)
  : mOriginalString(originalBegin, endByte - currentByte)
{
  // passing in a cstr will allocate a new buffer for the string so we need
  // to properly set up the begin and end values to point at the relevant data
  mBegin = mOriginalString.Data();
  mEnd = mBegin + (endByte - currentByte);
}

//For internal iterator use only
StringRange::StringRange(StringParam orginalStr, cstr currentByte, cstr endByte)
  : mOriginalString(orginalStr),
    mBegin(currentByte),
    mEnd(endByte)
{
}

Zero::StringRange& StringRange::operator=(StringRangeParam rhs)
{
  if (this == &rhs)
    return *this;

  mOriginalString = rhs.mOriginalString;
  mBegin = rhs.mBegin;
  mEnd = rhs.mEnd;

  return *this;
}

Rune StringRange::Front() const
{
  return ReadCurrentRune();
}

Rune StringRange::Back() const
{
  return (End() - 1).ReadCurrentRune();
}

void StringRange::PopFront()
{
  ErrorIf(Empty(), "Popped empty range.");
  IncrementByRune();
}

void StringRange::PopFront(size_t numElements)
{
  ErrorIf(ComputeRuneCount() < numElements, "Popped too many elements.");
  mBegin += numElements;
}

void StringRange::PopBack()
{
  ErrorIf(Empty(), "Popped empty range.");
  DecrementPointerByRune(mEnd);
}

bool StringRange::Contains(StringIterator pos)
{
  return pos.Data() >= mBegin && pos.Data() < mEnd;
}

int StringRange::CompareTo(const StringRange& right) const
{
  size_t leftSize = SizeInBytes();
  size_t rightSize = right.SizeInBytes();

  size_t min = leftSize < rightSize ? leftSize : rightSize;
  int result = strncmp(mBegin, right.mBegin, min);
  if(result != 0)
    return result;

  //if the sizes were actually equal then the strings were equal
  if (leftSize == rightSize)
    return 0;
  //otherwise determine which string is shorter
  else if (leftSize < rightSize)
    return -1;
  return 1;
}

bool StringRange::operator==(const StringRange& right) const
{
  return SizeInBytes() == right.SizeInBytes() &&
         strncmp(mBegin, right.mBegin, SizeInBytes()) == 0;
}

bool StringRange::operator<(const StringRange& right) const
{
  size_t min = SizeInBytes() < right.SizeInBytes() ? SizeInBytes() : right.SizeInBytes();
  int result = strncmp(mBegin, right.mBegin, min);
  if (result == 0)
    return SizeInBytes() < right.SizeInBytes();
  else
    return result < 0;
}

bool StringRange::operator==(Rune rune) const
{
  ErrorIf(Empty(), "No elements in range.");
  return ReadCurrentRune() == rune;
}

bool StringRange::operator==(int i) const
{
  ErrorIf(Empty(), "No elements in range.");
  return operator==(Rune(i));
}

bool StringRange::operator==(char c) const
{
  ErrorIf(Empty(), "No elements in range.");
  return *mBegin == c;
}

bool StringRange::operator!=(Rune rune) const
{
  ErrorIf(Empty(), "No elements in range.");
  return ReadCurrentRune() != rune;
}

bool StringRange::operator!=(int i) const
{
  ErrorIf(Empty(), "No elements in range.");
  return operator!=(Rune(i));
}

bool StringRange::operator!=(char c) const
{
  ErrorIf(Empty(), "No elements in range.");
  return *mBegin != c;
}

StringRange StringRange::FindFirstOf(Rune rune) const
{
  byte buffer[4];
  uint sizeInBytes = UTF8::UnpackUtf8RuneIntoBuffer(rune, buffer);

  return FindFirstByBytes((char*)buffer, sizeInBytes);
}

StringRange StringRange::FindFirstOf(StringRangeParam value) const
{
  return FindFirstByBytes(value.mBegin, value.SizeInBytes());
}

StringRange StringRange::FindLastOf(Rune rune) const
{
  byte buffer[4];
  uint sizeInBytes = UTF8::UnpackUtf8RuneIntoBuffer(rune, buffer);

  return FindLastByBytes((char*)buffer, sizeInBytes);
}

StringRange StringRange::FindLastOf(StringRangeParam value) const
{
  return FindLastByBytes(value.mBegin, value.SizeInBytes());
}

StringRange StringRange::FindFirstByBytes(cstr buffer, uint valueSizeInBytes) const
{
  size_t rangeSize = SizeInBytes();

  if (!valueSizeInBytes || valueSizeInBytes > rangeSize)
    return StringRange();

  uint searchSize = rangeSize - valueSizeInBytes;

  for (uint i = 0; i <= searchSize; i += UTF8::EncodedCodepointLength(mBegin[i]))
  {
    uint j;
    for (j = 0; j < valueSizeInBytes; ++j)
    {
      if (mBegin[i + j] != buffer[j])
        break;
    }

    if (j == valueSizeInBytes)
      return StringRange(mOriginalString, mBegin + i, mBegin + i + j);
  }

  return StringRange();
}

StringRange StringRange::FindLastByBytes(cstr buffer, uint valueSizeInBytes) const
{
  size_t rangeSize = SizeInBytes();

  if (!valueSizeInBytes || valueSizeInBytes > rangeSize)
    return StringRange();

  size_t last = rangeSize - valueSizeInBytes;

  for (size_t i = 0; i <= last; i += UTF8::EncodedCodepointLength(mBegin[i]))
  {
    size_t j;
    for (j = 0; j < valueSizeInBytes; ++j)
    {
      if (mBegin[last - i + j] != buffer[j])
        break;
    }

    if (j == valueSizeInBytes)
      return StringRange(mOriginalString, mBegin + (last - i), mBegin + (last - i + j));
  }

  return StringRange();
}

StringIterator StringRange::FindFirstNonWhitespaceRuneIt() const
{
  StringIterator it = Begin();
  while (it.mIteratorRange.mBegin != mEnd)
  {
    if (!it.IsCurrentRuneWhitespace())
      return it;
    ++it;
  }

  return End();
}

StringIterator StringRange::FindLastNonWhitespaceRuneIt() const
{
  StringIterator it = End() - 1;
  while (it.mIteratorRange.mBegin != mOriginalString.Data())
  {
    if (!it.IsCurrentRuneWhitespace())
    {
      //the current character isn't whitespace, move forward to it
      return it;
    }
    --it;
  }
  //the whole thing is whitespace, will return the begin position and if begin == end then it will
  //be an empty string
  if (!it.IsCurrentRuneWhitespace())
    return it;
  return End();
}

Rune StringRange::FindFirstNonWhitespaceRune() const
{
  StringIterator it = Begin();
  while (it.mIteratorRange.mBegin != mEnd)
  {
    if (!it.IsCurrentRuneWhitespace())
      return it.ReadCurrentRune();
    ++it;
  }

  return InvalidRune;
}

Rune StringRange::FindLastNonWhitespaceRune() const
{
  return *FindLastNonWhitespaceRuneIt();
}

bool StringRange::Contains(StringRangeParam value) const
{
  return FindFirstOf(value).Empty() == false;
}

bool StringRange::StartsWith(StringRangeParam value) const
{
  // if the substring is larger than our entire string, we can't possibly start with it
  size_t bytes = SizeInBytes();
  size_t subStrBytes = value.SizeInBytes();
  if (subStrBytes > bytes)
    return false;

  return strncmp(mBegin, value.Data(), subStrBytes) == 0;
}

bool StringRange::EndsWith(StringRangeParam value) const
{
  // if the substring is larger than our entire string, we can't possibly end with it
  size_t bytes = SizeInBytes();
  size_t subStrBytes = value.SizeInBytes();
  if(subStrBytes > bytes)
    return false;

  return strncmp(mBegin + (bytes - subStrBytes), value.Data(), subStrBytes) == 0;
}

StringRange StringRange::FindRangeExclusive(StringRangeParam startRange, StringRangeParam endRange) const
{
  StringRange inclusiveRange = FindRangeInclusive(startRange, endRange);
  if (!inclusiveRange.Empty())
  {
    //shrink the range in by the size of the search ranges
    inclusiveRange.mBegin += startRange.SizeInBytes();
    inclusiveRange.mEnd   -= endRange.SizeInBytes();
  }
  return inclusiveRange;
}

StringRange StringRange::FindRangeInclusive(StringRangeParam startRange, StringRangeParam endRange) const
{
  StringRange rangeStart = FindFirstOf(startRange);
  if (rangeStart.Empty())
    return rangeStart;

  StringRange subRange(mOriginalString, rangeStart.mEnd, mEnd);
  StringRange rangeEnd = subRange.FindFirstOf(endRange);
  if (rangeEnd.Empty())
    return rangeEnd;

  return StringRange(mOriginalString, rangeStart.mBegin, rangeEnd.mEnd);
}

String StringRange::Replace(StringRangeParam oldValue, StringRangeParam newValue) const
{
  StringBuilder newString;

  StringRange currentRange = *this;
  while (!currentRange.Empty())
  {
    //find the old value in the string
    StringRange valueRange = currentRange.FindFirstOf(oldValue);
    //if we didn't find anything we're done (and we have to Append what was left of the string)
    if(valueRange.Empty())
    {
      newString.Append(currentRange);
      break;
    }
    //add the first part and the new value
    newString.Append(currentRange.mBegin, valueRange.mBegin - currentRange.mBegin);
    newString.Append(newValue);
    //then continue the search with the remaining part of the string
    currentRange.mBegin += valueRange.mEnd - currentRange.mBegin;
  }

  return newString.ToString();
}

StringSplitRange StringRange::Split(StringRangeParam separator) const
{
  return StringSplitRange(*this, separator);
}

StringRange StringRange::Trim() const
{
  StringIterator startIt = FindFirstNonWhitespaceRuneIt();
  if(startIt.mIteratorRange.mBegin == mEnd)
    return StringRange(mOriginalString, mEnd, mEnd);

  StringIterator endIt = FindLastNonWhitespaceRuneIt();
  if (endIt == End())
    return StringRange(endIt, endIt);
  
  ++endIt;
  return StringRange(startIt, endIt);
}

StringRange StringRange::TrimEnd() const
{
  StringIterator endIt = FindLastNonWhitespaceRuneIt();
  if (endIt == End())
    return StringRange(endIt,endIt);
  
  ++endIt;
  return StringRange(mOriginalString, mBegin, endIt.Data());
}

StringRange StringRange::TrimStart() const
{
  StringIterator startIt = FindFirstNonWhitespaceRuneIt();
  if (startIt.Empty())
    return StringRange();
  return StringRange(mOriginalString, startIt.Data(), mEnd);
}

String StringRange::ToUpper() const
{
  StringBuilder builder;
  StringIterator it = Begin();
  StringIterator end = End();
  for (; it != end; ++it)
    builder.Append(UTF8::ToUpper(*it));

  return builder.ToString();
}

String StringRange::ToLower() const
{
  StringBuilder builder;
  StringIterator it = Begin();
  StringIterator end = End();
  for (; it != end; ++it)
    builder.Append(UTF8::ToLower(*it));

  return builder.ToString();
}

bool StringRange::IsAllUpper() const
{
  StringIterator it = Begin();
  while (it.mIteratorRange.mBegin < mEnd)
  {
    if (!it.IsUpper())
      return false;
    ++it;
  }

  return true;
}

bool StringRange::IsAllWhitespace() const
{
  StringIterator it = Begin();
  while (it.mIteratorRange.mBegin < mEnd)
  {
    if (!it.IsCurrentRuneWhitespace())
      return false;
    ++it;
  }

  return true;
}

StringRange StringRange::SubString(StringIterator begin, StringIterator end) const
{
  StringIterator b = begin;

  if (b > End())
  {
    b = End();
  }
  else if (b < begin)
  {
    b = begin;
  }

  StringIterator e = end;

  if (e > End())
  {
    e = End();
  }
  else if (e < begin)
  {
    e = begin;
  }

  if (e < b)
  {
    e = b;
  }

  return StringRange(b, e);
}

StringRange StringRange::SubStringFromByteIndices(size_t startIndex, size_t endIndex) const
{
  cstr begin = mBegin + startIndex;
  cstr end = mBegin + endIndex;
  return StringRange(mOriginalString, begin, end);
}

cstr StringRange::Data() const
{ 
  return mBegin;
}

bool StringRange::Empty() const
{ 
  return mBegin == mEnd; 
}

size_t StringRange::ComputeRuneCount() const
{ 
  cstr tempBegin = mBegin;
  size_t runeCount = 0;
  while (tempBegin < mEnd)
  {
    tempBegin += UTF8::EncodedCodepointLength(*tempBegin);
    ++runeCount;
  }
  return runeCount; 
}

size_t StringRange::SizeInBytes() const
{ 
  return mEnd - mBegin;
}

StringIterator StringRange::Begin() const
{
  return StringIterator(mOriginalString, mBegin);
}

StringIterator StringRange::End() const
{
  return StringIterator(mOriginalString, mEnd);
}

void StringRange::IncrementByRune()
{
  if(ValidateByte(mBegin))
    mBegin += UTF8::EncodedCodepointLength(*mBegin);
}

void StringRange::DecrementPointerByRune(cstr& ptr)
{
  if(ptr != mOriginalString.Data())
  {
    --ptr;
    while (ValidateByte(ptr) && IsContinuationByte(ptr))
      ptr -= 1;
  }
}

// this should never be called on an invalid rune (i.e end iterator itself)
Rune StringRange::ReadCurrentRune() const
{
  //if(ValidateByte(mBegin))
  return UTF8::ReadUtf8Rune((byte*)mBegin);
  //return Rune('\0');
}

bool StringRange::ValidateByte(cstr byte) const
{
  if ((byte >= mOriginalString.Data()) && byte <= mOriginalString.EndData() && !Empty())
    return true;
  return false;
}

bool StringRange::IsValid()
{
  return ValidateByte(mBegin);
}

bool StringRange::IsContinuationByte(cstr byte) const
{
  if ((*byte & Utf8ContinunationByteMask) == Utf8ContinunationByteSignature)
    return true;
  return false;
}

bool StringRange::IsCurrentRuneWhitespace() const
{
  if (UTF8::IsWhiteSpace(ReadCurrentRune()))
    return true;
  return false;
}

bool StringRange::IsCurrentRuneUpper() const
{
  if (UTF8::IsUpper(ReadCurrentRune()))
    return true;
  return false;
}

bool StringRange::IsCurrentRuneLower() const
{
  if (UTF8::IsLower(ReadCurrentRune()))
    return true;
  return false;
}

//------------------------------------------------------------------- String Split Range
StringSplitRange::StringSplitRange(StringRange range, StringRange separator)
{
  mRemainingRange = range;
  mSeparator = separator;

  SkipNext();
}

StringRange StringSplitRange::Front()
{
  return mCurrentRange;
}

void StringSplitRange::PopFront()
{
  SkipNext();
}

bool StringSplitRange::Empty()
{
  return mCurrentRange.Empty() && mRemainingRange.Empty();
}

void StringSplitRange::SkipNext()
{
  // if there's nothing left to search then clear out the current range
  if (mRemainingRange.Empty())
  {
    mCurrentRange = StringRange();
    return;
  }

  // find the first range of the separator
  StringRange separatorRange = mRemainingRange.FindFirstOf(mSeparator);
  // if we didn't find the separator then just set the current range to
  // what was left and clear out the remaining range
  if (separatorRange.Empty())
  {
    mCurrentRange = mRemainingRange;
    mRemainingRange = StringRange();
    return;
  }

  // break the range into the part before and after the separator
  mCurrentRange = StringRange(mRemainingRange.Begin(), separatorRange.Begin());
  mRemainingRange = StringRange(separatorRange.End(), mRemainingRange.End());
}

//------------------------------------------------------------------- String Iterator
StringIterator::StringIterator(StringParam orginalStr, cstr currentByte)
  :  mIteratorRange(orginalStr, currentByte, orginalStr.EndData())
{
}

StringIterator::StringIterator() 
  :  mIteratorRange()
{

}

StringIterator::StringIterator(StringIteratorParam it)
  :  mIteratorRange(it.mIteratorRange)
{
}

StringIterator::StringIterator(StringParam orginalStr)
  :  mIteratorRange(orginalStr)
{

}

StringIterator::~StringIterator()
{

}

Zero::StringIterator& StringIterator::operator=(StringIteratorParam rhs)
{
  if (this == &rhs)
    return *this;

  mIteratorRange = rhs.mIteratorRange;
  return *this;
}

bool StringIterator::operator==(const StringIterator& rhs) const
{
  if (mIteratorRange.mBegin == rhs.mIteratorRange.mBegin)
    return true;
  return false;
}

bool StringIterator::operator!=(StringIteratorParam rhs) const
{
  return !(*this == rhs);
}

bool StringIterator::operator<=(StringIteratorParam rhs) const
{
  if (mIteratorRange.mBegin <= rhs.mIteratorRange.mBegin)
    return true;
  return false;
}

bool StringIterator::operator>=(const StringIterator& rhs) const
{
  if (mIteratorRange.mBegin >= rhs.mIteratorRange.mBegin)
    return true;
  return false;
}

bool StringIterator::operator<(const StringIterator& rhs) const
{
  if (mIteratorRange.mBegin < rhs.mIteratorRange.mBegin)
    return true;
  return false;
}

bool StringIterator::operator>(const StringIterator& rhs) const
{
  if (mIteratorRange.mBegin > rhs.mIteratorRange.mBegin)
    return true;
  return false;
}

StringIterator StringIterator::operator+(uint numElements) const
{
  StringIterator it = *this;
  it += numElements;
  return it;
}

StringIterator StringIterator::operator-(uint numElements) const
{
  StringIterator it = *this;
  it -= numElements;
  return it;
}

int StringIterator::operator-(StringIterator rhs) const
{
  StringIterator it = *this;
  int elements = 0;
  while (rhs.mIteratorRange.mBegin < it.mIteratorRange.mBegin)
  {
   --it;
    ++elements;
  }
  return elements;
}

StringIterator& StringIterator::operator+=(uint numElements)
{
  while (numElements)
  {
    mIteratorRange.IncrementByRune();
    --numElements;
  }
  return *this;
}

StringIterator& StringIterator::operator-=(uint numElements)
{
  while (numElements)
  {
    mIteratorRange.DecrementPointerByRune(mIteratorRange.mBegin);
    --numElements;
  }
  return *this;
}

//Backwards compatibility, should modify later
char StringIterator::operator[](uint numBytes) const
{
  return (char)*(mIteratorRange.mBegin + numBytes);
}

StringIterator& StringIterator::operator++()
{
  mIteratorRange.IncrementByRune();
  return *this;
}

StringIterator& StringIterator::operator--()
{
  mIteratorRange.DecrementPointerByRune(mIteratorRange.mBegin);
  return *this;
}

Rune StringIterator::operator*()
{
  return ReadCurrentRune();
}

Rune StringIterator::operator*() const
{
  return ReadCurrentRune();
}

Rune StringIterator::ReadCurrentRune() const
{
  return mIteratorRange.ReadCurrentRune();
}

bool StringIterator::ValidByte() const
{
  return mIteratorRange.ValidateByte(mIteratorRange.mBegin);
}

bool StringIterator::IsCurrentRuneWhitespace() const
{
  return mIteratorRange.IsCurrentRuneWhitespace();
}

bool StringIterator::IsUpper() const
{
  return mIteratorRange.IsCurrentRuneUpper();
}

bool StringIterator::IsLower() const
{
  return mIteratorRange.IsCurrentRuneLower();
}

bool StringIterator::Empty() const
{
  return mIteratorRange.Empty();
}

size_t StringIterator::GetPosition() const
{
  size_t pos = 0;
  StringIterator original = mIteratorRange.mOriginalString.Begin();
  StringIterator current = mIteratorRange.Begin();
  while (original.Data() != current.Data())
  {
    ++original;
    ++pos;
  }
  return pos;
}

cstr StringIterator::Data() const
{
  return mIteratorRange.mBegin;
}

//----------------------------------------------------------- String Token Range
StringTokenRange::StringTokenRange(StringRange string, Rune delim)
  : internalRange(string),
    mDelim(delim)
{
  PopFront();
}

StringRange StringTokenRange::Front()
{
  return curRange;
}

void StringTokenRange::PopFront()
{
  StringIterator start = internalRange.Begin();
  StringIterator current = start;

  // Scan until delim or end
  while (current != internalRange.End() && *current != mDelim)
  {
    ++current;
    internalRange.PopFront();
  }
  StringIterator end = current;

  // Skip all delimiters if there are multiple in a row
  while (current != internalRange.End() && *current == mDelim)
  {
    ++current;
    internalRange.PopFront();
  }

  curRange = StringRange(start, end);
}

bool StringTokenRange::Empty()
{
  return curRange.Empty() && internalRange.Empty();
}

}// namespace Zero
