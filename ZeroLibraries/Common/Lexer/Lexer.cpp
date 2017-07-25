///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//***************************************************************************
String EscapeString(StringRange input)
{
  StringBuilder builder;

  // Walk through all the input characters
  StringIterator end = input.End();
  for (StringIterator it = input.Begin(); it < end; ++it)
  {
    // Grab the current character
    Rune r = *it;

    // Based on the escaped character type...
    switch (r.value)
    {
      // All these characters just get directly inserted as themselves
    case '"':
      builder.Append("\\\"");
      break;
    case '`':
      builder.Append("\\`");
      break;
    case '\\':
      builder.Append("\\\\");
      break;

      // These characters have special meanings or ascii values
    case '\0':
      builder.Append("\\0");
      break;
    case '\a':
      builder.Append("\\a");
      break;
    case '\b':
      builder.Append("\\b");
      break;
    case '\f':
      builder.Append("\\f");
      break;
    case '\n':
      builder.Append("\\n");
      break;
    case '\r':
      builder.Append("\\r");
      break;
    case '\t':
      builder.Append("\\t");
      break;
    case '\v':
      builder.Append("\\v");
      break;
    default:
      builder.Append(r);
      break;
    }
  }

  // Return the resulting string
  return builder.ToString();
}

//***************************************************************************
String EscapeCharacter(int unicodeCharacter)
{
  // Based on the escaped character type...
  switch (unicodeCharacter)
  {
    // All these characters just get directly inserted as themselves
  case '"':
    return "\\\"";
  case '`':
    return "\\`";
  case '\\':
    return "\\\\";

    // These characters have special meanings or ascii values
  case '\0':
    return "\\0";
  case '\a':
    return "\\a";
  case '\b':
    return "\\b";
  case '\f':
    return "\\f";
  case '\n':
    return "\\n";
    break;
  case '\r':
    return "\\r";
  case '\t':
    return "\\t";
  case '\v':
    return "\\v";
  default:
    return String(unicodeCharacter);
  }
}

//***************************************************************************
Character::Character() :
  mCharacter(0)
{
}

//***************************************************************************
Character::Character(int character)
{
  this->mCharacter = character;
}

//***************************************************************************
bool Character::IsEnd() const
{
  return this->mCharacter == 0;
}

//***************************************************************************
bool Character::IsValid() const
{
  return this->mCharacter != 0;
}

//***************************************************************************
String Character::ToEscapedString() const
{
  return EscapeCharacter(this->mCharacter);
}

//***************************************************************************
bool Character::operator==(const Character& rhs) const
{
  return this->mCharacter == rhs.mCharacter;
}

//***************************************************************************
bool Character::operator!=(const Character& rhs) const
{
  return this->mCharacter != rhs.mCharacter;
}

//***************************************************************************
bool Character::operator< (const Character& rhs) const
{
  return this->mCharacter < rhs.mCharacter;
}

//***************************************************************************
bool Character::operator<=(const Character& rhs) const
{
  return this->mCharacter <= rhs.mCharacter;
}

//***************************************************************************
bool Character::operator> (const Character& rhs) const
{
  return this->mCharacter > rhs.mCharacter;
}

//***************************************************************************
bool Character::operator>=(const Character& rhs) const
{
  return this->mCharacter >= rhs.mCharacter;
}

//***************************************************************************
Token::Token() :
  mRule(nullptr),
  mStartInclusive(0),
  mEndExclusive(0),
  mStream(nullptr),
  mEnd(true),
  mError(false)
{
}

//***************************************************************************
Token::Token(GrammarRule<Character>* rule) :
  mRule(rule),
  mStartInclusive(0),
  mEndExclusive(0),
  mStream(nullptr),
  mEnd(false),
  mError(false)
{
}

//***************************************************************************
Token::operator String()
{
  return this->mString;
}

//***************************************************************************
bool Token::IsEnd() const
{
  return this->mEnd;
}

//***************************************************************************
bool Token::IsValid() const
{
  return (mRule != nullptr);
}

//***************************************************************************
String Token::ToEscapedString() const
{
  if (this->mEnd)
  {
    static String Eof("EndOfFile");
    return Eof;
  }


  if (this->mString.Empty())
  {
    if (this->mRule != nullptr)
      return this->mRule->mName;
  }

  return EscapeString(this->mString);
}

//***************************************************************************
bool Token::operator==(const Token& rhs) const
{
  return this->mRule == rhs.mRule;
}

//***************************************************************************
bool Token::operator!=(const Token& rhs) const
{
  return this->mRule != rhs.mRule;
}

//***************************************************************************
bool Token::operator< (const Token& rhs) const
{
  return this->mRule->mOrderId < rhs.mRule->mOrderId;
}

//***************************************************************************
bool Token::operator<=(const Token& rhs) const
{
  return this->mRule->mOrderId <= rhs.mRule->mOrderId;
}

//***************************************************************************
bool Token::operator> (const Token& rhs) const
{
  return this->mRule->mOrderId > rhs.mRule->mOrderId;
}

//***************************************************************************
bool Token::operator>=(const Token& rhs) const
{
  return this->mRule->mOrderId >= rhs.mRule->mOrderId;
}

//***************************************************************************
ReplacementNode::ReplacementNode() :
  mType(ReplacementNodeType::Text),
  mCaptureReference(nullptr),
  mLhs(nullptr),
  mRhs(nullptr)
{
}

//***************************************************************************
ReplacementNode::~ReplacementNode()
{
  delete this->mLhs;
  delete this->mRhs;
  delete this->mCaptureReference;
}

//***************************************************************************
ReplacementNode& ReplacementNode::operator<<(ReplacementNode& rhs)
{
  ReplacementNode& node = *new ReplacementNode();
  node.mType = ReplacementNodeType::Concatenate;
  node.mLhs = this;
  node.mRhs = &rhs;
  return node;
}

//***************************************************************************
ReplacementNode& R(StringParam text)
{
  ReplacementNode& node = *new ReplacementNode();
  node.mType = ReplacementNodeType::Text;
  node.mText = text;
  return node;
}

//***************************************************************************
ReplacementNode& R(CaptureExpressionNode& capture)
{
  ReplacementNode& node = *new ReplacementNode();
  node.mType = ReplacementNodeType::JoinCapture;
  node.mCaptureReference = &capture;
  return node;
}

//***************************************************************************
ReplacementNode& R(CaptureExpressionNode& capture, ReplacementNode& replaceEach)
{
  ReplacementNode& node = *new ReplacementNode();
  node.mType = ReplacementNodeType::ForeachCapture;
  node.mCaptureReference = &capture;
  node.mRhs = &replaceEach;
  return node;
}

//***************************************************************************
ReplacementNode& R(CaptureExpressionNode& capture, StringParam name, ReplacementNode& replaceEach)
{
  ReplacementNode& node = *new ReplacementNode();
  node.mType = ReplacementNodeType::ForeachCapture;
  node.mCaptureReference = &capture;
  node.mRhs = &replaceEach;
  node.mText = name;
  return node;
}

//***************************************************************************
GrammarNode<Character>& T()
{
  GrammarNode<Character>& node = *new GrammarNode<Character>();
  node.mType = GrammarNodeType::Terminate;
  return node;
}

//***************************************************************************
GrammarNode<Character>& T(int character)
{
  GrammarNode<Character>& node = *new GrammarNode<Character>();
  node.mType = GrammarNodeType::RangeSet;
  node.mSingleTokens.PushBack(character);
  return node;
}

//***************************************************************************
GrammarNode<Character>& T(StringRange rangeSet)
{
  GrammarNode<Character>& node = *new GrammarNode<Character>();

  if (rangeSet.Empty())
  {
    node.mType = GrammarNodeType::Epsilon;
    return node;
  }

  node.mType = GrammarNodeType::RangeSet;

  bool firstCharacter = true;

  // If this is an 'inverse' range
  if (rangeSet.Front() == '^')
  {
    node.mType = GrammarNodeType::NotRangeSet;
    rangeSet.PopFront();
  }

  char rangeStart = '\0';
  bool range = false;

  for (; !rangeSet.Empty(); rangeSet.PopFront())
  {
    Rune rune = rangeSet.Front();

    // If we're ending a range (could even be a -)
    if (range == true)
    {
      GrammarRange<Character>& newRange = node.mRanges.PushBack();
      newRange.mStartInclusive = rangeStart;
      newRange.mEndInclusive = rune.value;

      // If the user did a-z-A we would actually include the second - as a character
      rangeStart = '\0';
      range = false;
    }
    // If we're attempting to start a range (unless its the first character)
    else if (rune == '-' && rangeStart != '\0')
    {
      // We pre-maturely added the last character to our character list
      // when really, it was the start of range
      node.mSingleTokens.PopBack();

      range = true;
    }
    // Otherwise its just a normal character
    else
    {
      rangeStart = rune.value;
      node.mSingleTokens.PushBack(rune.value);
    }
  }

  // If we started a range, but it had no end, then the - character is included as a character
  // Note that this WILL NOT be hit when it is a single dash for the entire string
  if (range)
  {
    node.mSingleTokens.PushBack(rangeStart);
    node.mSingleTokens.PushBack('-');
  }
  return node;
}

//***************************************************************************
GrammarNode<Character>& T(int startInclusive, int endInclusive)
{
  GrammarNode<Character>& node = *new GrammarNode<Character>();
  GrammarRange<Character>& range = node.mRanges.PushBack();
  range.mStartInclusive = startInclusive;
  range.mEndInclusive = endInclusive;
  node.mType = GrammarNodeType::RangeSet;
  return node;
}

//***************************************************************************
GrammarNode<Character>& T(StringParam captureName, GrammarNode<Character>& capture)
{
  GrammarNode<Character>& node = *new GrammarNode<Character>();
  node.mType = GrammarNodeType::Capture;
  node.mName = captureName;
  node.mOperand = &capture;
  return node;
}

//***************************************************************************
GrammarNode<Token>& P()
{
  GrammarNode<Token>& node = *new GrammarNode<Token>();
  node.mType = GrammarNodeType::Terminate;
  return node;
}

//***************************************************************************
GrammarNode<Token>& P(GrammarRule<Character>& tokenRule)
{
  GrammarNode<Token>& node = *new GrammarNode<Token>();
  node.mType = GrammarNodeType::RangeSet;
  node.mSingleTokens.PushBack(&tokenRule);
  return node;
}

//***************************************************************************
GrammarNode<Token>& P(StringParam captureName, GrammarNode<Token>& capture)
{
  GrammarNode<Token>& node = *new GrammarNode<Token>();
  node.mType = GrammarNodeType::Capture;
  node.mName = captureName;
  node.mOperand = &capture;
  return node;
}

//***************************************************************************
Character CharacterStream::GetToken(size_t index)
{
  char character = this->mText.Data()[index];
  return character;
}

//***************************************************************************
StringRange CharacterStream::GetText(size_t startInclusive, size_t endExclusive)
{
  StringIterator start = this->mText.Begin() + startInclusive;
  StringIterator end = start + (endExclusive - startInclusive);
  StringRange subString = this->mText.SubString(start, end);
  return subString;
}

//***************************************************************************
void CharacterStream::Replace(StringParam text, size_t startInclusive, size_t endExclusive)
{
  this->mText = String::ReplaceSub(this->mText, text, startInclusive, endExclusive);
}

//***************************************************************************
AutoIncrement::AutoIncrement(size_t* increment) :
  mIncrement(increment)
{
  ++*this->mIncrement;
}

//***************************************************************************
AutoIncrement::~AutoIncrement()
{
  --*this->mIncrement;
}

//***************************************************************************
CaptureExpressionNode::CaptureExpressionNode() :
  mType(CaptureExpressionNodeType::NamedCapture),
  mLhs(nullptr),
  mRhs(nullptr),
  mStartIndexInclusive(0),
  mEndIndexInclusive(0)
{
}

//***************************************************************************
CaptureExpressionNode::~CaptureExpressionNode()
{
  delete this->mLhs;
  delete this->mRhs;
}

//***************************************************************************
CaptureExpressionNode& CaptureExpressionNode::operator+(CaptureExpressionNode& rhs)
{
  CaptureExpressionNode& node = *new CaptureExpressionNode();
  node.mType = CaptureExpressionNodeType::Union;
  node.mLhs = this;
  node.mRhs = &rhs;
  return node;
}

//***************************************************************************
CaptureExpressionNode& CaptureExpressionNode::operator[](int index)
{
  CaptureExpressionNode& node = *new CaptureExpressionNode();
  node.mType = CaptureExpressionNodeType::IndexRange;
  node.mLhs = this;
  node.mStartIndexInclusive = index;
  node.mEndIndexInclusive = index;
  return node;
}

//***************************************************************************
CaptureExpressionNode& CaptureExpressionNode::operator[](StringParam nestedCaptureName)
{
  CaptureExpressionNode& node = *new CaptureExpressionNode();
  node.mType = CaptureExpressionNodeType::NestedCapture;
  node.mLhs = this;
  node.mName = nestedCaptureName;
  return node;
}

//***************************************************************************
CaptureExpressionNode& CaptureExpressionNode::operator!()
{
  CaptureExpressionNode& node = *new CaptureExpressionNode();
  node.mType = CaptureExpressionNodeType::Not;
  node.mRhs = this;
  return node;
}

//***************************************************************************
CaptureExpressionNode& CaptureExpressionNode::operator~()
{
  CaptureExpressionNode& node = *new CaptureExpressionNode();
  node.mType = CaptureExpressionNodeType::Exists;
  node.mRhs = this;
  return node;
}

//***************************************************************************
CaptureExpressionNode& C(StringParam captureName)
{
  CaptureExpressionNode& node = *new CaptureExpressionNode();
  node.mType = CaptureExpressionNodeType::NamedCapture;
  node.mName = captureName;
  return node;
}

//***************************************************************************
CaptureExpressionNode& C(CaptureExpressionNode& parent, int startIndexInclusive, int endIndexInclusive)
{
  CaptureExpressionNode& node = *new CaptureExpressionNode();
  node.mType = CaptureExpressionNodeType::IndexRange;
  node.mLhs = &parent;
  node.mStartIndexInclusive = startIndexInclusive;
  node.mEndIndexInclusive = endIndexInclusive;
  return node;
}

//***************************************************************************
CharacterLocation::CharacterLocation() :
  mLine(0),
  mCharacter(0)
{
}

//***************************************************************************
CharacterLocation::CharacterLocation(size_t line, size_t character) :
  mLine(line),
  mCharacter(character)
{
}

//***************************************************************************
CharacterLocation CharacterLocation::FromIndex(StringRange input, size_t index)
{
  CharacterLocation location;
  Compute(input, &location, &index, true);
  return location;
}

//***************************************************************************
size_t CharacterLocation::ToIndex(StringRange input, CharacterLocation location)
{
  size_t index = 0;
  Compute(input, &location, &index, true);
  return index;
}

//***************************************************************************
void CharacterLocation::Compute(StringRange input, CharacterLocation* outLocation, size_t* outIndex, bool computeIndex)
{
  CharacterLocation location;
  size_t currentIndex = 0;
  bool wasCariageReturn = false;

  for (; !input.Empty(); input.PopFront())
  {
    // If we're computing locations from an index, and the current index 
    if (computeIndex == false && currentIndex >= *outIndex)
      break;

    // Read the character from the range (the range converts utf8 to rune int)
    Rune rune = input.Front();

    // Increment the character count for the current line
    ++location.mCharacter;

    // Check if the character matches that of the newline or carriage return
    if (rune == '\n' || rune == '\r')
    {
      // As long as the last character wasn't a carriage return and this character isn't a newline (CRLF)...
      // Note: If this was a full CRLF, we already incremented the line on the first CR, no need to do it again!
      if ((wasCariageReturn && rune == '\n') == false)
      {
        // Increment the line count since we hit a line
        ++location.mLine;
      }

      // If the character is a carriage return then set it
      wasCariageReturn = (rune == '\r');

      // For this new line, we start out on character one
      location.mCharacter = 1;
    }

    // If we're computing the index from the location, then check if the line and character matches
    if (computeIndex && location.mLine == outLocation->mLine && location.mCharacter == outLocation->mCharacter)
      break;

    ++currentIndex;
  }

  // At the end we return the index or the location
  if (computeIndex)
    *outIndex = currentIndex;
  else
    *outLocation = location;
}
}
