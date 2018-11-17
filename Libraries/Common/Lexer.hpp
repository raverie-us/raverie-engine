///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
//***************************************************************************
class ReplacementNode;
class CaptureExpressionNode;
class CharacterStream;
template <typename TokenType>
class Capture;
template <typename TokenType>
class GrammarSet;

//***************************************************************************
template <typename TokenType>
class GrammarRange
{
public:
  TokenType mStartInclusive;
  TokenType mEndInclusive;
};

//***************************************************************************
// Adds the \ in front of common ascii codes (\n, \r, \0, etc)
ZeroShared String EscapeString(StringRange input);
ZeroShared String EscapeCharacter(int unicodeCharacter);

//***************************************************************************
DeclareEnum11(GrammarNodeType, Epsilon, Rule, Or, Concatenate, ZeroOrMore, OneOrMore, Optional, RangeSet, NotRangeSet, Terminate, Capture);
template <typename TokenType>
class GrammarNode
{
public:
  friend class GrammarSet<TokenType>;
  GrammarNode();
  ~GrammarNode();

  GrammarNode& operator|(GrammarNode& rhs);
  GrammarNode& operator<<(GrammarNode& rhs);
  GrammarNode& operator*();
  GrammarNode& operator+();
  GrammarNode& operator~();

  GrammarNode& operator|=(GrammarNode& rhs);
  GrammarNode& operator%=(GrammarNode& rhs);

  GrammarNodeType::Enum mType;

  // RangeSet
  Array<GrammarRange<TokenType> > mRanges;
  Array<TokenType> mSingleTokens;

  // Binary (Or, Concatenate, Replacement's Find/Reparse)
  GrammarNode* mLhs;
  GrammarNode* mRhs;

  // Unary (ZeroOrMore, OneOrMore, Optional, Capture)
  GrammarNode* mOperand;

  // Rule / Capture
  String mName;

  // Rule (the grammar set we came from)
  GrammarSet<TokenType>* mGrammarSet;

  // Rule (the replacements that we run before running our tokens)
  typedef Pair<GrammarNode<TokenType>*, ReplacementNode*> ReplacementPair;
  typedef Array<ReplacementPair> ReplacementArray;
  ReplacementArray mReplacements;

  // Rule (the order of declaration, only used in range comparisons)
  int mOrderId;

  // Get a range string
  void GetRangeSetString(StringBuilder& builder);
  String GetRangeSetString();

protected:
  // Deletes a node if it is valid and not a rule reference
  static void DeleteNode(GrammarNode* node);

private:

  static GrammarNode& MakeBinary(GrammarNode& lhs, GrammarNode& rhs, GrammarNodeType::Enum type);
  static GrammarNode& MakeUnary(GrammarNode& operand, GrammarNodeType::Enum type);
};

//***************************************************************************
template <typename TokenType>
class GrammarRule : public GrammarNode<TokenType>
{
public:
  GrammarRule();

  GrammarRule& operator|=(GrammarNode<TokenType>& rhs);
  GrammarRule& operator%=(GrammarNode<TokenType>& rhs);
};

//***************************************************************************
class Character
{
public:
  Character();
  Character(int character);

  // LexerToken interface
  bool IsEnd() const;
  bool IsValid() const;
  String ToEscapedString() const;
  bool operator==(const Character& rhs) const;
  bool operator!=(const Character& rhs) const;
  bool operator< (const Character& rhs) const;
  bool operator<=(const Character& rhs) const;
  bool operator> (const Character& rhs) const;
  bool operator>=(const Character& rhs) const;

  int mCharacter;
};

//***************************************************************************
class Token
{
public:
  Token();
  Token(GrammarRule<Character>* rule);

  // LexerToken interface
  bool IsEnd() const;
  bool IsValid() const;
  String ToEscapedString() const;
  bool operator==(const Token& rhs) const;
  bool operator!=(const Token& rhs) const;
  bool operator< (const Token& rhs) const;
  bool operator<=(const Token& rhs) const;
  bool operator> (const Token& rhs) const;
  bool operator>=(const Token& rhs) const;

  operator String();

  bool mEnd;
  bool mError;
  GrammarRule<Character>* mRule;
  String mString;
  size_t mStartInclusive;
  size_t mEndExclusive;
  CharacterStream* mStream;
};

//***************************************************************************
GrammarNode<Character>& T();
GrammarNode<Character>& T(int character);
GrammarNode<Character>& T(StringRange rangeSet);
GrammarNode<Character>& T(int startInclusive, int endInclusive);
GrammarNode<Character>& T(StringParam captureName, GrammarNode<Character>& capture);

//***************************************************************************
GrammarNode<Token>& P();
GrammarNode<Token>& P(GrammarRule<Character>& tokenRule);
GrammarNode<Token>& P(StringParam captureName, GrammarNode<Token>& capture);

//***************************************************************************
DeclareEnum4(ReplacementNodeType, Text, JoinCapture, ForeachCapture, Concatenate);
class ReplacementNode
{
public:
  ReplacementNode();
  ~ReplacementNode();
  ReplacementNode& operator<<(ReplacementNode& rhs);

  ReplacementNodeType::Enum mType;

  // Text
  String mText;

  // JoinCapture, ForeachCapture
  CaptureExpressionNode* mCaptureReference;

  // Binary (Concatenate), ForeachCapture (rhs), JoinCapture (rhs) for inbetweens
  ReplacementNode* mLhs;
  ReplacementNode* mRhs;
};

//***************************************************************************
ReplacementNode& R(StringParam text);
ReplacementNode& R(CaptureExpressionNode& capture);
ReplacementNode& R(CaptureExpressionNode& capture, ReplacementNode& replaceEach);
ReplacementNode& R(CaptureExpressionNode& capture, StringParam name, ReplacementNode& replaceEach);

//***************************************************************************
// Capture nodes always result in a forest of captures (which can be linearized by replacement nodes)
DeclareEnum6(CaptureExpressionNodeType, NamedCapture, NestedCapture, IndexRange, Exists, Not, Union);
class CaptureExpressionNode
{
public:
  CaptureExpressionNode();
  ~CaptureExpressionNode();
  CaptureExpressionNode& operator+(CaptureExpressionNode& rhs);
  CaptureExpressionNode& operator[](int index);
  CaptureExpressionNode& operator[](StringParam nestedCaptureName);
  CaptureExpressionNode& operator!();
  CaptureExpressionNode& operator~();

  CaptureExpressionNodeType::Enum mType;

  // NamedCapture / NestedCapture
  String mName;

  // NestedCapture / Index / Exists / Union
  CaptureExpressionNode* mLhs;
  CaptureExpressionNode* mRhs;

  // IndexRange
  int mStartIndexInclusive;
  int mEndIndexInclusive;
};

//***************************************************************************
CaptureExpressionNode& C(StringParam captureName);
CaptureExpressionNode& C(CaptureExpressionNode& parent, int startIndexInclusive, int endIndexInclusive);

//***************************************************************************
template <typename TokenType>
class ParseNodeInfo
{
public:
  ParseNodeInfo();
  ~ParseNodeInfo();

  TokenType GetFirstCapturedToken(StringParam name, const TokenType& failResult = TokenType());

  GrammarRule<TokenType>* mRule;
  TokenType mToken;
  size_t mStartInclusive;
  size_t mEndExclusive;
  bool mAccepted;
  bool mFailed;
  Capture<TokenType>* mCapture;
};

//***************************************************************************
template <typename TokenType>
class GrammarSet
{
public:
  GrammarSet();
  ~GrammarSet();
  GrammarRule<TokenType>& operator[](StringParam name);

  // If the exact keyword string is parsed, we accept it as if it were this rule
  void AddKeyword(StringParam keyword, StringParam ruleName);
  void AddKeyword(StringParam keyword, GrammarRule<TokenType>& rule);

  // We explicitly ignore this rule by name
  void AddIgnore(StringParam ruleName);
  void AddIgnore(GrammarRule<TokenType>& rule);

  // Whenever we parse this exact string, we accept it as this keyword (only used by when tokenizing)
  HashMap<String, GrammarRule<TokenType>*> mKeywords;

  // Whenever we parse this exact string, we accept it as this keyword (only used by when tokenizing)
  HashSet<GrammarRule<TokenType>*> mIgnore;

private:
  // This counter assigns ids to rules so that they may be used in range comparisons
  int mOrderIdCounter;
  HashMap<String, GrammarRule<TokenType>*> mRules;
};

//***************************************************************************
// We use 1 as a base for both values because almost every text editor starts at 1
// The algorithm we use to count lines attempts to match many common text editor techniques
// By default, we initialize the line and character to 0 (so you can tell if its valid or not)
class CharacterLocation
{
public:
  CharacterLocation();
  CharacterLocation(size_t line, size_t character);

  // Compute the line and character from and index and input string
  static CharacterLocation FromIndex(StringRange input, size_t index);

  // Get the a string index via the line and character number
  static size_t ToIndex(StringRange input, CharacterLocation location);

  // 1 based line index
  size_t mLine;

  // 1 based character (or column) index
  size_t mCharacter;

private:
  static void Compute(StringRange input, CharacterLocation* outLocation, size_t* outIndex, bool computeIndex);
};

//***************************************************************************
class CharacterStream
{
public:
  // LexerStream
  Character GetToken(size_t index);
  StringRange GetText(size_t startInclusive, size_t endExclusive);
  void Replace(StringParam text, size_t startInclusive, size_t endExclusive);

  String mText;
};

//***************************************************************************
class AutoIncrement
{
public:
  AutoIncrement(size_t* increment);
  ~AutoIncrement();
private:
  size_t* mIncrement;
};

//***************************************************************************
template <typename T>
class AutoPush
{
public:
  AutoPush(Array<T>& array, const T& value);
  ~AutoPush();
private:
  Array<T>* mArray;
};

//***************************************************************************
template <typename TokenType>
class Capture
{
public:
  Capture();
  ~Capture();

  TokenType GetFirstToken(StringParam name, const TokenType& failResult = TokenType());

  String mName;
  Capture* mParent;
  size_t mStartInclusive;
  size_t mEndExclusive;
  Array<TokenType> mTokens;

  Array<Capture*> mNestedCaptures;
  HashMap<String, Array<Capture*> > mNestedCapturesByName;
};

//***************************************************************************
template <typename TokenType>
class ParseNode : public ParseNodeInfo<TokenType>
{
public:
  ParseNode();
  ParseNode(const ParseNodeInfo<TokenType>& info);
  ~ParseNode();
  ParseNode& operator=(const ParseNodeInfo<TokenType>& rhs);

  String GetDebugRepresentation();
  String GetGraphRepresentation();

  ParseNode* mParent;
  Array<ParseNode*> mChildren;

private:
  void OutputNode(StringBuilder& builder);
  void GetDebugRepresentation(size_t depth, StringBuilder& builder);
  void GetGraphRepresentation(size_t& index, StringBuilder& builder);
  static void GetGraphRepresentation(size_t& index, StringBuilder& builder, Capture<TokenType>* capture);
};

//***************************************************************************
template <typename TokenType>
class EmptyParseHandler
{
public:
  // ParseHandler interface
  void StartRule(GrammarRule<TokenType>* rule);
  void EndRule(ParseNodeInfo<TokenType>* info);
  void TokenParsed(ParseNodeInfo<TokenType>* info);
  void StartParsing();
  void EndParsing();
};

//***************************************************************************
template <typename TokenType>
class ParseTreeBuilder
{
public:
  // The parser should send the rule and token parsed events
  ParseTreeBuilder();
  ~ParseTreeBuilder();

  // Creates a root node that all future nodes will be attached to
  // (erases any node currently existing)
  // This is useful for debugging a tokenizer, since the tokenizer runs
  // many times and creates a 'forest' of parse trees
  // This single node will encapsulate the  forest
  void CreateRootNode();

  // Deletes the current tree and clears it to null
  void Restart();

  // ParseHandler interface
  void StartRule(GrammarRule<TokenType>* rule);
  void EndRule(ParseNodeInfo<TokenType>* info);
  void TokenParsed(ParseNodeInfo<TokenType>* info);
  void StartParsing();
  void EndParsing();

  // The tree that should be accessible after the parser has run (or null)
  ParseNode<TokenType>* mTree;
};

//***************************************************************************
template <typename TokenType>
class CaptureVariable
{
public:
  CaptureVariable();
  String mName;
  Capture<TokenType>* mCapture;
};

//***************************************************************************
template <typename TokenType, typename StreamType>
class ParseError
{
public:
  ParseError();
  Array<GrammarRule<TokenType>*>* mStack;
  StreamType* mStream;
  size_t mFailedIndex;
};

//***************************************************************************
template <typename TokenType, typename StreamType, typename ParseHandlerType = EmptyParseHandler<TokenType> >
class RecursiveDescentParser
{
public:
  RecursiveDescentParser();
  ~RecursiveDescentParser();
  RecursiveDescentParser(const RecursiveDescentParser& rhs);

  void Parse();
  void ReplaceAndBackup(StringParam text, size_t startInclusive, size_t endExclusive);

  static const size_t MaxRecursionDepth = 500;
  static const size_t MaxReplacementStringLength = 256 * 256;

  // Inputs
  bool mTokenMode;
  bool mDebug;
  GrammarRule<TokenType>* mStartRule;
  StreamType* mStream;
  ParseHandlerType* mParseHandler;

  // Outputs
  GrammarRule<TokenType>* mAcceptedRule;
  size_t mStartIndex;
  size_t mAcceptedIndex;
  bool mEnd;
  StatusContext<ParseError<TokenType, StreamType> > mStatus;
  StringBuilder mDebugOutput;

private:

  // Returns whether or not anything was parsed (in all cases, even epsilon)
  bool EvaluateGrammar(GrammarNode<TokenType>* node);
  void EvaluateReplacement(ReplacementNode* node, StringBuilder& builder);
  void EvaluateCapture(CaptureExpressionNode* node, Array<Capture<TokenType>*>& capturesOut);

  bool ShouldInvokeParseHandler();
  bool IsInReplacement();

  void CheckRecursionDepth();

  void SetError(StringParam error);
  void SetErrorAndBackout(StringParam error);

  // This will only ever be Token, RangeSet, or NotRangeSet
  // This is the last character or token node we attempted to accept that failed (used for error reporting)
  GrammarNode<TokenType>* mLastAttemptedAccept;

  // A stack of capture variable names
  Array<CaptureVariable<TokenType> > mCaptureVariableStack;

  size_t mIndex;

  size_t mDepth;
  size_t mReplacementDepth;

  // We put all captures here
  // Captures are based on the rules being parsed
  Array<Capture<TokenType>*> mCaptureStack;

  // As we enter rules, we need to know which ones have been started so that we can tell the user
  // This is only used when a ParseHandler is present (so we know not to call StartRule unless we actually read a token
  // This must be a stack because we can enter multiple rules without reading a token (Start -> Object -> Name, then token)
  // Once we start a rule, we'll set the value to null
  Array<GrammarRule<TokenType>*> mNonStartedRules;
  // This index represents the first non started rule in the list (non-null)
  // This is so that we can efficiently loop through the rules that haven't started
  // If the index is larger than the number of started rules, then its not valid
  size_t mNonStartedRuleIndex;

  // This is a stack that only exists for reporting error information
  Array<GrammarRule<TokenType>*> mRuleStack;

  bool mBackout;

  // We read one token or character ahead since this is
  // a predictive parser (and we never need to backtrack)
  TokenType mNextToken;
};

//***************************************************************************
template <typename ParseHandlerType = EmptyParseHandler<Character> >
class TokenRange
{
public:
  TokenRange();
  TokenRange(GrammarSet<Character>& set, GrammarRule<Character>& startRule, StringParam input, bool debug = false);
  TokenRange(const TokenRange& rhs);
  TokenRange& operator=(const TokenRange& rhs);

  Token Front();
  bool Empty();
  void PopFront();
  TokenRange& All() { return *this; }
  const TokenRange& All() const { return *this; }

  RecursiveDescentParser<Character, CharacterStream, ParseHandlerType> mParser;
  CharacterStream mStream;

private:
  GrammarSet<Character>* mSet;
  bool mHasRunFirstIteration;
};

//***************************************************************************
template <typename ParseHandlerType = EmptyParseHandler<Character> >
class TokenStream
{
public:
  // LexerStream
  Token GetToken(size_t index);
  StringRange GetText(size_t startInclusive, size_t endExclusive);
  void Replace(StringParam text, size_t startInclusive, size_t endExclusive);

  // The token stream reads from the character stream at the same time
  // If a replacement occurs, we backup and perform the replacement on
  // the original character stream then re-tokenize the characters
  TokenRange<ParseHandlerType> mRange;
  Array<Token> mTokens;
};
}

#include "Lexer.inl"
