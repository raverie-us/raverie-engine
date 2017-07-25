///////////////////////////////////////////////////////////////////////////////
///
/// \file String.hpp
/// Declaration of the Referenced counted string class.
///
/// Authors: Chris Peters, Dane Curbow
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Rune.hpp"
#include "Platform/UnicodeUtility.hpp"
#include "Utility/Standard.hpp"
#include "Utility/Atomic.hpp"

#define ZeroStringPooling
//#define ZeroStringStats

namespace Zero
{

class String;
class StringRange;
class StringIterator;
class StringSplitRange;

typedef const String& StringParam;
// Should later be changed to "String&" without const. Commented out for now to prevent/fix wrong usage
//typedef const String& StringRef;
typedef const StringRange& StringRangeParam;

//----------------------------------------------------------------------- StringStats
class StringStats
{
public:
  static StringStats& GetInstance();

  Atomic<size_t> mTotalSize;
  Atomic<size_t> mTotalCount;
};

//----------------------------------------------------------------------- Utility
bool CaseSensitiveCompare(Rune a, Rune b);
bool CaseInsensitiveCompare(Rune a, Rune b);
typedef bool (*RuneComparer)(Rune a, Rune b);

//----------------------------------------------------------------------- StringNode
struct StringNode
{
  typedef char              value_type;
  typedef size_t            size_type;
  typedef s32               count_type;

  // A special flag we use to indicate that the string pool had been destructed, but the node still exists
  // Note this must be the same hash code as the empty / default string
  static const size_type StringPoolFreeHashCode = (size_type)0;

  volatile count_type RefCount;
  size_type Size;
  size_type HashCode;
  value_type Data[1];

  void addRef();

  void release();

  static bool isEqual(StringNode* l, StringNode* r);
};

//----------------------------------------------------------------------- PoolPolicy
class PoolPolicy
{
public:
  size_t operator()(const StringNode* node) const;
  bool Equal(const StringNode* lhs, const StringNode* rhs) const;
};

//----------------------------------------------------------------------- String
class ZeroShared String
{
public:
  ///////Standard typedefs/////////////
  typedef char              value_type;
  typedef value_type*       pointer;
  typedef const value_type* const_pointer;
  typedef value_type&       reference;
  typedef const value_type& const_reference;
  typedef size_t            size_type;
  typedef s32               count_type;
  typedef std::ptrdiff_t    difference_type;
  typedef String            this_type;
  typedef StringRange       range;

  ///////Iterators/////////////////////
  typedef const value_type* iterator;
  typedef const value_type* const_iterator;

  static const uint InvalidIndex = (uint)-1;

  ///////Constructors//////////////////
  String();

  String(StringNode* node);

  //Caution: This is not explicit for ease of use.
  String(const_pointer cstring);
  String(StringRange str);
  String(const_pointer cstring, size_type size);
  String(const_pointer cstart, const_pointer cend);
  String(StringIterator begin, StringIterator end);

  //Copy constructor
  String(const this_type& rhs);
  explicit String(char character);
  explicit String(Rune rune);
  ~String();

  String& operator=(const String& other);

  ///////Data Access Functions/////////
  size_type Hash() const;

  bool Empty() const;
  cstr Data() const;
  cstr EndData() const;
  cstr c_str() const;
  size_type SizeInBytes() const;
  size_t ComputeRuneCount() const;
  void Clear();
  Rune Front() const;
  Rune Back() const;

  bool operator!=(const String& right) const;

  StringRange SubString(StringIterator begin, StringIterator end) const;
  StringRange SubStringFromByteIndices(size_t startIndex, size_t endIndex) const;

  ///////Iteration/////////////////////////

  StringIterator Begin() const;
  StringIterator End() const;
  StringRange All() const;
  static String Format(cstr format, ...);
  static String FormatArgs(cstr format, va_list va);

  // Gets the string node that represents this string (only use in advanced cases)
  StringNode* GetNode() const;

  template<typename type>
  friend struct MoveWithoutDestructionOperator;
  static String ReplaceSub(StringRange source, StringRange text, size_type start, size_type end);

  StringRange FindFirstOf(Rune value) const;
  StringRange FindFirstOf(StringRangeParam value) const;
  StringRange FindLastOf(Rune value) const;
  StringRange FindLastOf(StringRangeParam value) const;
  StringRange FindRangeExclusive(StringRangeParam startRange, StringRangeParam endRange);
  StringRange FindRangeInclusive(StringRangeParam startRange, StringRangeParam endRange);
  Rune FindFirstNonWhitespaceRune() const;
  Rune FindLastNonWhitespaceRune() const;
  
  /// Returns true if all the characters in a string are upper-case
  bool IsAllUpper() const;

  /// Returns true if all the characters in a string are whitespace
  bool IsAllWhitespace() const;

  /// Returns true if the string starts with the given text
  bool StartsWith(StringRange startsWith, RuneComparer compare = CaseSensitiveCompare) const;

  /// Returns true if the string starts with the given text
  static bool StartsWith(StringRange source, StringRange startsWith, RuneComparer compare = CaseSensitiveCompare);
  
  static String Repeat(Rune rune, size_t numberOfTimes);
  bool Contains(StringRangeParam value) const;
  int CompareTo(StringRangeParam value) const;
  bool EndsWith(StringRangeParam value) const;

  static String Join(StringRangeParam separator, StringRangeParam string1, StringRangeParam string2);
  static String Join(StringRangeParam separator, StringRangeParam string1, StringRangeParam string2, StringRangeParam string3);
  static String Join(StringRangeParam separator, StringRangeParam string1, StringRangeParam string2, StringRangeParam string3, StringRangeParam string4);
  static String Join(StringRangeParam separator, const String* strings, size_t stringCount);
  static String JoinInternal(StringRangeParam separator, const StringRange* values, size_t count);

  // A simple policy (to be used with Join below) to convert a the value type of a
  // range of Strings to StringRanges (aka to convert range.Front() to a StringRange)
  struct SimplePolicy
  {
    StringRange ToStringRange(StringParam value);
  };

  // Joins the given range with the provided separator. The range is assumed to be copyable
  // (to get the size of the range) and the policy is expected to have a ToStringRange member
  // function that takes the type of range.Front() and returns a StringRange.
  template <typename RangeType, typename PolicyType>
  static String JoinRange(StringRangeParam separator, RangeType range, PolicyType policy)
  {
    // First we need to know how big the range is, so copy the range and iterate over to count
    RangeType counterRange = range;
    size_t count = 0;
    for (; !counterRange.Empty(); counterRange.PopFront())
      ++count;

    // Now allocate enough pointers for the ranges and copy them over
    StringRange* values = (StringRange*)alloca(sizeof(StringRange) * count);
    size_t i = 0;
    // Fill out the array of StringRanges
    for (; !range.Empty(); range.PopFront())
    {
      new(values + i) StringRange();
      values[i] = policy.ToStringRange(range.Front());
      ++i;
    }

    return JoinInternal(separator, values, count);
  }

  template <typename RangeType>
  static String JoinRange(StringRangeParam separator, RangeType range)
  {
    // Assume range.Front() returns String and use the SimplePolicy
    return JoinRange<RangeType, SimplePolicy>(separator, range, SimplePolicy());
  }

  String Replace(StringRangeParam oldValue, StringRangeParam newValue) const;
  StringSplitRange Split(StringRangeParam separator) const;
  bool StartsWith(StringRangeParam value);

  StringRange TrimStart();
  StringRange TrimEnd();
  StringRange Trim();
  String ToUpper() const;
  String ToLower() const;

  // Internally allocates a node
  // This should never be called by the user except in rare optimization cases
  static StringNode* AllocateNode(size_type size);

  ///////Primary equal operator for strings///////
  friend bool operator==(const String& left, const String& right)
  {
    return StringNode::isEqual(left.mNode, right.mNode);
  }

  ///////Less than operators///////////
  friend bool operator<(const String& left, const String& right)
  {
    return strcmp(left.c_str(), right.c_str()) < 0;
  }

  ///////Greater than operators///////////
  friend bool operator>(const String& left, const String& right)
  {
    return strcmp(right.c_str(), left.c_str()) < 0;
  }

  // This should only ever be used for debugging purposes.
  static bool DebugIsNodePointerInPool(String* node);

private:
  void InitializeCharacter(int character);
  void InitializeCharacterNonPreallocated(int character);

  // This constructor is only used by the preallocating character optimization
  explicit String(int unicodeCharacter, bool);
  static const int cPreAllocatedCharacterCount = 128;
  String* GetPreAllocatedCharacterStrings();

  void initializeToDefault();
  void poolOrDeleteNode(StringNode* node);
  void addRef();
  void release();
  void Assign(const_pointer data, size_type size);
  void Assign(StringNode* node);
  StringNode* mNode;
};

#define DeclareStringConstant(name) extern const String name;
#define DefineStringConstant(name) const String name = #name;

}//namespace Zero
