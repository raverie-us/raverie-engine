///////////////////////////////////////////////////////////////////////////////
///
/// \file String.cpp
/// Implementation of the referenced string class.
///
/// Authors: Chris Peters, Dane Curbow
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "Platform/ThreadSync.hpp"

#if defined(ZeroStringStats)
  #undef ZeroStringStats
  #define ZeroStringStats(X) X
#else
  #define ZeroStringStats(X)
#endif

namespace Zero
{

//----------------------------------------------------------------------- StringPool
#if defined(ZeroStringPooling)
class StringPool
{
public:
  StringPool();
  ~StringPool();
  static StringPool& GetInstance();

  // The empty node is an optimization for empty strings, but it still has to be in the pool
  StringNode& GetEmptyNode();

  ThreadLock mLock;
  HashSet<StringNode*, PoolPolicy> mPool;
};

StringPool::StringPool()
{
  mPool.Insert(&GetEmptyNode());
}

StringPool::~StringPool()
{
  // It is possible that the StringPool can be freed before the last few Strings
  // This only occurs post main (due to pre-main allocated strings)
  // We handle this by setting a special flag on the StringNode
  forRange(StringNode* node, mPool.All())
  {
    node->HashCode = StringNode::StringPoolFreeHashCode;
  }
}

StringNode& StringPool::GetEmptyNode()
{
  static StringNode node = { 1, 0, 0,{ 0 } };
  return node;
}

StringPool& StringPool::GetInstance()
{
  static StringPool pool;
  return pool;
}
#endif

StringStats& StringStats::GetInstance()
{
  static StringStats stats;
  return stats;
}

size_t PoolPolicy::operator()(const StringNode* node) const
{
  return node->HashCode;
}

bool PoolPolicy::Equal(const StringNode* lhs, const StringNode* rhs) const
{
  return StringNode::isEqual((StringNode*)lhs, (StringNode*)rhs);
}

size_t HashString(const char* str, size_t l)
{
  size_t h = l;
  // if string is too long, don't hash all its chars
  size_t step = (l >> 5) + 1;
  size_t l1;
  for (l1 = l; l1 >= step; l1 -= step)  /* compute hash */
    h = h ^ ((h << 5) + (h >> 2) + byte(str[l1 - 1]));
  return h;
}

bool CaseSensitiveCompare(Rune a, Rune b)
{
  return a == b;
}

bool CaseInsensitiveCompare(Rune a, Rune b)
{
  return UTF8::ToLower(a) == UTF8::ToLower(b);
}

//----------------------------------------------------------------------- String

String::String()
{
  initializeToDefault();
}

String::String(const_pointer cstring)
{
  if (cstring != nullptr)
  {
    size_t length = strlen(cstring);
    if (length == 0)
      initializeToDefault();
    else
      Assign(cstring, length);
  }
  else
  {
    //Error("Construction of a string from a NULL pointer is not valid");
    initializeToDefault();
  }
}

String::String(StringRange str)
{
  Assign(str.mBegin, str.SizeInBytes());
}

String::String(const_pointer cstring, size_type size)
{
  Assign(cstring, size);
}

String::String(const_pointer cstart, const_pointer cend)
{
  Assign(cstart, cend - cstart);
}

String::String(const this_type& rhs)
{
  Assign(rhs.mNode);
}

String::String(char character)
{
  char str[2] = { character, '\0' };
  Assign(str, 1);
}

String::String(Rune rune)
{
  byte utf8Bytes[4];
  int bytesRead = UTF8::UnpackUtf8RuneIntoBuffer(rune, utf8Bytes);

  Assign((char*)utf8Bytes, bytesRead);
}

String::String(StringIterator begin, StringIterator end)
{
  Assign(begin.Data(), end.Data() - begin.Data());
}

String::~String()
{
  release();
}

String::size_type String::Hash() const
{
  return mNode->HashCode;
}

bool String::Empty() const
{
  return mNode->Size == 0;
}

cstr String::Data() const
{
  return mNode->Data;
}

cstr String::EndData() const
{
  return mNode->Data + SizeInBytes();
}

cstr String::c_str() const
{
  return mNode->Data;
}

String::size_type String::SizeInBytes() const
{
  return mNode->Size;
}

size_t String::ComputeRuneCount() const
{
  return All().ComputeRuneCount();
}

void String::Clear()
{
  *this = String();
}

Rune String::Front() const
{
  return All().Front();
}

Rune String::Back() const
{
  return All().Back();
}

bool String::operator!=(const String& right) const
{
  return !((*this) == right);
}

StringRange String::SubString(StringIterator begin, StringIterator end) const
{
  return All().SubString(begin, end);
}

StringRange String::SubStringFromByteIndices(size_t startIndex, size_t endIndex) const
{
  return All().SubStringFromByteIndices(startIndex, endIndex);
}

StringIterator String::Begin() const
{
  return All().Begin();
}

StringIterator String::End() const
{
  return All().End();
}

StringRange String::All() const
{
  return StringRange(*this);
}

String& String::operator=(const String& other)
{
  // Instead of checking if we're doing self assignment with the string, its much more useful to check if
  // we're self assigning the node (because there could be two different String objects with the same StringNode)
  if (this->mNode == other.mNode)
    return *this;

  //release this strings data
  release();
  //Assign and add a reference
  Assign(other.mNode);
  return *this;
}

void String::initializeToDefault()
{
#if defined(ZeroStringPooling)
  StringPool& pool = StringPool::GetInstance();
  StringNode& node = pool.GetEmptyNode();
#else
  static StringNode node = { 1, 0, 0,{ 0 } };
#endif
  Assign(&node);
}

void String::InitializeCharacter(int character)
{
  // This is an optimization where we keep pre-allocated character strings around for ascii characters
  if (character < cPreAllocatedCharacterCount)
  {
    Assign(GetPreAllocatedCharacterStrings()[character].mNode);
    return;
  }

  InitializeCharacterNonPreallocated(character);
}

void String::InitializeCharacterNonPreallocated(int character)
{
  // Until we actually do unicode encoding, just call the character version
  char str[2] = { (char)character, '\0' };
  Assign(str, 1);
}

String::String(int unicodeCharacter, bool)
{
  InitializeCharacterNonPreallocated(unicodeCharacter);
}

String* String::GetPreAllocatedCharacterStrings()
{
  // We do this all in one line to take advantage of compiler generated thread safety
  #define S(value) String(value, true)
  static String ascii[] = { S(0), S(1), S(2), S(3), S(4), S(5), S(6), S(7), S(8), S(9), S(10), S(11), S(12), S(13), S(14), S(15), S(16), S(17), S(18), S(19), S(20), S(21), S(22), S(23), S(24), S(25), S(26), S(27), S(28), S(29), S(30), S(31), S(32), S(33), S(34), S(35), S(36), S(37), S(38), S(39), S(40), S(41), S(42), S(43), S(44), S(45), S(46), S(47), S(48), S(49), S(50), S(51), S(52), S(53), S(54), S(55), S(56), S(57), S(58), S(59), S(60), S(61), S(62), S(63), S(64), S(65), S(66), S(67), S(68), S(69), S(70), S(71), S(72), S(73), S(74), S(75), S(76), S(77), S(78), S(79), S(80), S(81), S(82), S(83), S(84), S(85), S(86), S(87), S(88), S(89), S(90), S(91), S(92), S(93), S(94), S(95), S(96), S(97), S(98), S(99), S(100), S(101), S(102), S(103), S(104), S(105), S(106), S(107), S(108), S(109), S(110), S(111), S(112), S(113), S(114), S(115), S(116), S(117), S(118), S(119), S(120), S(121), S(122), S(123), S(124), S(125), S(126), S(127) };
  #undef S
  return ascii;
}

String::String(StringNode* node)
{
  // This node should be unique here
  ErrorIf(node->RefCount != 1, "Bad string node creation");
  node->HashCode = HashString(node->Data, node->Size);
  mNode = node;
  poolOrDeleteNode(node);
}

void String::Assign(const_pointer data, size_type size)
{
  StringNode* node = AllocateNode(size);
  memcpy(node->Data, data, size);
  node->HashCode = HashString(data, size);
  mNode = node;
  poolOrDeleteNode(node);
}

void String::Assign(StringNode* node)
{
  mNode = node;
  addRef();
}


void StringNode::addRef()
{
  AtomicPreIncrement(&RefCount);
}

bool String::DebugIsNodePointerInPool(String* str)
{
  if(str == nullptr)
    return false;

  bool isInPool = false;
#if defined(ZeroStringPooling)
  StringPool& pool = StringPool::GetInstance();
  pool.mLock.Lock();
  {
    isInPool = pool.mPool.Contains(str->mNode);
  }
  pool.mLock.Unlock();
#endif
  return isInPool;
}

void String::poolOrDeleteNode(StringNode* node)
{
  ZeroStringStats(StringStats& stats = StringStats::GetInstance());
#if defined(ZeroStringPooling)
  StringPool& pool = StringPool::GetInstance();
  pool.mLock.Lock();
  {
    StringNode* existingNode = pool.mPool.FindValue(node, nullptr);
    if (existingNode == nullptr)
    {
      pool.mPool.InsertOrError(node);
      ZeroStringStats(++stats.mTotalCount);
      ZeroStringStats(stats.mTotalSize += node->Size);
    }
    else
    {
      // Note: It is OK if the existingNode's RefCount is 0 here
      // This happens in the case where we're creating the SAME string on one thread
      // but it's literally being released on another thread at the same time (already ref count 0)
      zDeallocate(node);
      Assign(existingNode);
    }
  }
  pool.mLock.Unlock();
#else
  ZeroStringStats(++stats.mTotalCount);
  ZeroStringStats(stats.mTotalSize += node->Size);
#endif
}

void StringNode::release()
{
#if defined(ZeroStringPooling)
  // It is possible that the StringPool can be freed before the last few Strings
  // This only occurs post main (due to pre-main allocated strings)
  // We handle this by setting a special flag on the StringNode
  if (HashCode == StringPoolFreeHashCode)
  {
    if (AtomicPreDecrement(&RefCount) == 0)
    {
      ZeroStringStats(--stats.mTotalCount);
      ZeroStringStats(stats.mTotalSize -= Size);
      zDeallocate(this);
    }
    return;
  }

  StringPool& pool = StringPool::GetInstance();
  pool.mLock.Lock();
#endif

  if (AtomicPreDecrement(&RefCount) == 0)
  {
    ZeroStringStats(StringStats& stats = StringStats::GetInstance());

#if defined(ZeroStringPooling)
    ErrorIf(pool.mPool.FindValue(this, nullptr) == nullptr, "Did not find node in pool");
    pool.mPool.Erase(this);
    ZeroStringStats(--stats.mTotalCount);
    ZeroStringStats(stats.mTotalSize -= Size);
#endif
    zDeallocate(this);
  }


#if defined(ZeroStringPooling)
  pool.mLock.Unlock();
#endif
}

bool StringNode::isEqual(StringNode* l, StringNode* r)
{
  if(!(l == r))
  {
    if(l->Size == r->Size &&
        l->HashCode == r->HashCode && 
        strcmp(l->Data, r->Data) == 0)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return true;
  }
}

void String::addRef()
{
  mNode->addRef();
}

void String::release()
{
  if(mNode)
    mNode->release();
}

StringNode* String::AllocateNode(size_type size)
{
  const size_type nodeSize = 
                             sizeof(StringNode) //size of the string node
                           - sizeof(value_type);//remove the extra

  //size of buffer is string size plus once extra buffer
  //for null terminator '\0'
  const size_type bufferSize = size + sizeof(value_type);

  //Make new string node
  StringNode* newNode = (StringNode*)zAllocate(nodeSize + bufferSize);
  newNode->RefCount = 1;
  newNode->Size = size;
  newNode->HashCode = 0;
  newNode->Data[size] = '\0';

  return newNode;
}

//--------------------------------------------------------------- String Helpers
String String::Format(cstr format, ...)
{
  va_list va;
  va_start(va, format);
  String result = String::FormatArgs(format, va);
  va_end(va);
  return result;
}

String String::FormatArgs(cstr format, va_list args)
{
  //Get the number of characters needed for message
  int bufferSize;
  ZeroVSPrintfCount(format, args, 1, bufferSize);
  char* stringBuffer = (char*)alloca((bufferSize+1)*sizeof(char));
  stringBuffer[bufferSize] = '\0';
  ZeroVSPrintf(stringBuffer, bufferSize, format, args);
  return String(stringBuffer);
}

StringNode* String::GetNode() const
{
  return mNode;
}
// Should be fine left as bytes despite UTF8
// only used by Recursive Descent Parser working with bytes.
String String::ReplaceSub(StringRange source, StringRange text,
                                size_type start, size_type end)
{
  size_type sizeToRemove = end - start;
  size_type sizeToAdd = text.SizeInBytes();

  //time to build the new string
  size_type newSize = source.SizeInBytes() - sizeToRemove + sizeToAdd;

  StringNode* node = String::AllocateNode(newSize);
  char* bufferPos = node->Data;
  char* bufferEnd = bufferPos + newSize + 1;

  ErrorIf(start > source.SizeInBytes(), "Start was outside the string range");
  ErrorIf(end > source.SizeInBytes(), "End was outside the string range");

  //Copy over the front if there is anything to copy
  if(start > 0)
  {
    ZeroCStringCopy(bufferPos, bufferEnd - bufferPos, source.Data(), start);
    bufferPos += start;
  }

  if(sizeToAdd != 0)
  {
    ZeroCStringCopy(bufferPos, bufferEnd - bufferPos, text.Data(), sizeToAdd);
    bufferPos += sizeToAdd;
  }

  size_type sizeOfEndText = source.SizeInBytes() - end;
  if(sizeOfEndText)
  {
    ZeroCStringCopy(bufferPos, bufferEnd - bufferPos, source.Data() + end, 
                sizeOfEndText);
    bufferPos += sizeOfEndText;
  }

  bufferPos[0] = '\0';

  return String(node);
}

StringRange String::FindFirstOf(Rune value) const
{
  return All().FindFirstOf(value);
}

StringRange String::FindFirstOf(const StringRange& value) const
{
  return All().FindFirstOf(value);
}

StringRange String::FindLastOf(Rune value) const
{
  return All().FindLastOf(value);
}

StringRange String::FindLastOf(const StringRange& value) const
{
  return All().FindLastOf(value);
}

StringRange String::FindRangeExclusive(StringRangeParam startRange, StringRangeParam endRange)
{
  return All().FindRangeExclusive(startRange, endRange);
}

StringRange String::FindRangeInclusive(StringRangeParam startRange, StringRangeParam endRange)
{
  return All().FindRangeInclusive(startRange, endRange);
}

Rune String::FindFirstNonWhitespaceRune() const
{
  return All().FindFirstNonWhitespaceRune();
}

Rune String::FindLastNonWhitespaceRune() const
{
  return All().FindLastNonWhitespaceRune();
}

bool String::IsAllUpper() const
{
  return All().IsAllUpper();
}

bool String::IsAllWhitespace() const
{
  return All().IsAllWhitespace();
}

String String::Join(StringRangeParam separator, StringRangeParam string1, StringRangeParam string2)
{
  StringRange values[2] = {string1, string2};
  return JoinInternal(separator, values, 2);
}

String String::Join(StringRangeParam separator, StringRangeParam string1, StringRangeParam string2, StringRangeParam string3)
{
  StringRange values[3] = {string1, string2, string3};
  return JoinInternal(separator, values, 3);
}

String String::Join(StringRangeParam separator, StringRangeParam string1, StringRangeParam string2, StringRangeParam string3, StringRangeParam string4)
{
  StringRange values[4] = {string1, string2, string3, string4};
  return JoinInternal(separator, values, 4);
}

String String::Join(StringRangeParam separator, const String* strings, size_t stringCount)
{
  size_t bytesToAlloc = sizeof(StringRange) * stringCount;
  Array<StringRange> values;
  values.Resize(stringCount);

  // String range Contains string and garbage data causes string to attempt to release 
  // the string data
  for(size_t i = 0; i < stringCount; ++i)
    values[i] = strings[i].All();
  
  return JoinInternal(separator, values.Begin(), stringCount);
}

String String::JoinInternal(StringRangeParam separator, const StringRange* values, size_t count)
{
  if(count == 0)
    return String();

  //count the total size needed (don't include an extra for the null as the string constructor adds the null)
  size_t separatorSize = separator.SizeInBytes();
  size_t totalSize = separatorSize * (count - 1);
  for(size_t i = 0; i < count; ++i)
    totalSize += values[i].SizeInBytes();

  //allocate the entire buffer
  char* data = (char*)alloca(totalSize);
  char* current = data;
  //Append item + separator for all but the last item
  for(size_t i = 0; i < count - 1; ++i)
  {
    StringRangeParam value = values[i];
    size_t valueSize = value.SizeInBytes();
    memcpy(current, value.Data(), valueSize);
    current += valueSize;

    memcpy(current, separator.Data(), separatorSize);
    current += separatorSize;
  }

  //add the last item
  StringRangeParam lastValue = values[count - 1];
  size_t lastValueSize = lastValue.SizeInBytes();
  memcpy(current, lastValue.Data(), lastValueSize);
  current += lastValueSize;

  return String(data, totalSize);
}

String String::Replace(StringRangeParam oldValue, StringRangeParam newValue) const
{
  return All().Replace(oldValue, newValue);
}

StringSplitRange String::Split(StringRangeParam separator) const
{
  return All().Split(separator);
}

bool String::StartsWith(StringRange startsWith, RuneComparer compare) const
{
  return StartsWith(All(), startsWith, compare);
}

bool String::StartsWith(StringRange source, StringRange startsWith, RuneComparer compare)
{
  // If the string we're matching is larger than our string, then we don't match
  if (startsWith.SizeInBytes() > source.SizeInBytes())
    return false;

  for (; !startsWith.Empty(); startsWith.PopFront(), source.PopFront())
  {
    Rune compareChar = startsWith.Front();
    Rune ourChar = source.Front();

    if (!compare(compareChar, ourChar))
    {
      return false;
    }
  }
  return true;
}

bool String::StartsWith(StringRangeParam value)
{
  return All().StartsWith(value);
}

StringRange String::TrimStart()
{
  return All().TrimStart();
}

StringRange String::TrimEnd()
{
  return All().TrimEnd();
}

StringRange String::Trim()
{
  return All().Trim();
}

String String::ToUpper() const
{
  return All().ToUpper();
}

String String::ToLower() const
{
  return All().ToLower();
}

String String::Repeat(Rune rune, size_t numberOfTimes)
{
  byte utf8Bytes[4];
  int bytesRead = UTF8::UnpackUtf8RuneIntoBuffer(rune, utf8Bytes);
  // Create a temporary memory buffer that Contains the character repeated over and over
  size_t bufferSize = numberOfTimes * bytesRead;
  char* buffer = (char*)alloca(bufferSize);
  // Advance the buffer by the amount by the amount of bytes written each copy
  for (size_t i = 0; i < bufferSize; i += bytesRead)
    memcpy(buffer + i, utf8Bytes, bytesRead);

  // Return a string made out of the buffer
  return String(buffer, bufferSize);
}

bool String::Contains(StringRangeParam value) const
{
  return All().Contains(value);
}

int String::CompareTo(StringRangeParam value) const
{
  return All().CompareTo(value);
}

bool String::EndsWith(StringRangeParam value) const
{
  return All().EndsWith(value);
}

//------------------------------------------------------------- Global Functions
StringIterator GetNextWhitespace(StringRange input)
{
  StringIterator it = input.Begin();
  while (!it.Empty())
  {
    if (it.IsCurrentRuneWhitespace())
      return it;
    ++it;
  }
  return it;
}

String WordWrap(StringRange input, size_t maxLineLength)
{
  StringBuilder builder;
  
  size_t lineLength = 0;
  
  while(!input.Empty())
  {
    Rune r = input.Front();
    input.PopFront();
    
    ++lineLength;
    
    if(r == '\n' || r == '\r')
    {
      lineLength = 0;
      builder.Append(r);
      continue;
    }
    else if(!UTF8::IsWhiteSpace(r))
    {
      StringIterator nextWhiteSpace = GetNextWhitespace(input);
      uint wordLength = nextWhiteSpace - input.Begin();
      bool isWordShort = wordLength < maxLineLength;
      bool doesWordMakeLineTooLong = lineLength + wordLength >= maxLineLength;
      if(isWordShort && doesWordMakeLineTooLong)
      {
        lineLength = 0;
        builder.Append('\n');
      }
    }
    
    if(lineLength >= maxLineLength)
    {
      lineLength = 0;
      builder.Append('\n');
    }

    // Eat any whitespace at the beginning of the line
    if(lineLength == 0 && UTF8::IsWhiteSpace(r))
      continue;
    
    builder.Append(r);
  }

  return builder.ToString();
}

//----------------------------------------------------------- String SimplePolicy
StringRange String::SimplePolicy::ToStringRange(StringParam value)
{
  return value.All();
}

}//namespace Zero
