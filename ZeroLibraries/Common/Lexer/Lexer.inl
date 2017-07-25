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
template <typename TokenType>
GrammarNode<TokenType>::GrammarNode() :
  mType(GrammarNodeType::Epsilon),
  mLhs(nullptr),
  mRhs(nullptr),
  mOperand(nullptr),
  mGrammarSet(nullptr),
  mOrderId(0)
{
}

//***************************************************************************
template <typename TokenType>
GrammarNode<TokenType>::~GrammarNode()
{
  DeleteNode(this->mLhs);
  DeleteNode(this->mRhs);
  DeleteNode(this->mOperand);
}

//***************************************************************************
template <typename TokenType>
GrammarNode<TokenType>& GrammarNode<TokenType>::MakeBinary(GrammarNode& lhs, GrammarNode& rhs, GrammarNodeType::Enum type)
{
  GrammarNode& node = *new GrammarNode();
  node.mType = type;
  node.mLhs = &lhs;
  node.mRhs = &rhs;
  return node;
}

//***************************************************************************
template <typename TokenType>
GrammarNode<TokenType>& GrammarNode<TokenType>::MakeUnary(GrammarNode& operand, GrammarNodeType::Enum type)
{
  GrammarNode& node = *new GrammarNode();
  node.mType = type;
  node.mOperand = &operand;
  return node;
}

//***************************************************************************
template <typename TokenType>
GrammarNode<TokenType>& GrammarNode<TokenType>::operator|(GrammarNode& rhs)
{
  return MakeBinary(*this, rhs, GrammarNodeType::Or);
}

//***************************************************************************
template <typename TokenType>
GrammarNode<TokenType>& GrammarNode<TokenType>::operator<<(GrammarNode& rhs)
{
  return MakeBinary(*this, rhs, GrammarNodeType::Concatenate);
}

//***************************************************************************
template <typename TokenType>
GrammarNode<TokenType>& GrammarNode<TokenType>::operator*()
{
  return MakeUnary(*this, GrammarNodeType::ZeroOrMore);
}

//***************************************************************************
template <typename TokenType>
GrammarNode<TokenType>& GrammarNode<TokenType>::operator+()
{
  return MakeUnary(*this, GrammarNodeType::OneOrMore);
}

//***************************************************************************
template <typename TokenType>
GrammarNode<TokenType>& GrammarNode<TokenType>::operator~()
{
  return MakeUnary(*this, GrammarNodeType::Optional);
}

//***************************************************************************
template <typename TokenType>
void GrammarNode<TokenType>::GetRangeSetString(StringBuilder& builder)
{
  bool hasEmitted = false;

  if (this->mType == GrammarNodeType::NotRangeSet)
  {
    if (this->mRanges.Empty() && this->mSingleTokens.Empty())
      builder.Append("anything");
    else
      builder.Append("anything but ");
  }

  forRange (GrammarRange<TokenType>& range, this->mRanges.All())
  {
    if (hasEmitted)
      builder.Append(", ");
    else
      hasEmitted = true;

    builder.Append(range.mStartInclusive.ToEscapedString());
    builder.Append('-');
    builder.Append(range.mEndInclusive.ToEscapedString());
  }

  forRange (TokenType& token, this->mSingleTokens.All())
  {
    if (hasEmitted)
      builder.Append(", ");
    else
      hasEmitted = true;

    builder.Append(token.ToEscapedString());
  }
}

//***************************************************************************
template <typename TokenType>
String GrammarNode<TokenType>::GetRangeSetString()
{
  StringBuilder builder;
  this->GetRangeSetString(builder);
  return builder.ToString();
}

//***************************************************************************
template <typename TokenType>
void GrammarNode<TokenType>::DeleteNode(GrammarNode* node)
{
  if (node != nullptr && node->mType != GrammarNodeType::Rule)
    delete node;
}

//***************************************************************************
template <typename TokenType>
GrammarRule<TokenType>::GrammarRule()
{
  this->mType = GrammarNodeType::Rule;
}

//***************************************************************************
template <typename TokenType>
GrammarRule<TokenType>& GrammarRule<TokenType>::operator |=(GrammarNode<TokenType>& rhs)
{
  if (this->mOperand == nullptr)
    this->mOperand = &rhs;
  else
    this->mOperand = &((*this->mOperand) | rhs);

  return *this;
}

//***************************************************************************
template <typename TokenType>
GrammarRule<TokenType>& GrammarRule<TokenType>::operator%=(GrammarNode<TokenType>& rhs)
{
  DeleteNode(this->mOperand);
  this->mOperand = &rhs;
  return *this;
}

//***************************************************************************
template <typename TokenType>
GrammarSet<TokenType>::GrammarSet() :
  mOrderIdCounter(0)
{
}

//***************************************************************************
template <typename TokenType>
GrammarSet<TokenType>::~GrammarSet()
{
  // We have to delete all the rule children before the rules themselves
  // Because the children can point back at the rule (and try to dereference it during destruction)
  forRange (GrammarRule<TokenType>* rule, this->mRules.Values())
  {
    GrammarNode<TokenType>::DeleteNode(rule->mOperand);

    forRange(typename GrammarNode<TokenType>::ReplacementPair& pair, rule->mReplacements.All())
    {
      GrammarNode<TokenType>::DeleteNode(pair.first);
      delete pair.second;
    }

    rule->mReplacements.Clear();
    rule->mOperand = nullptr;
  }

  forRange (GrammarRule<TokenType>* rule, this->mRules.Values())
  {
    delete rule;
  }
}

//***************************************************************************
template <typename TokenType>
GrammarRule<TokenType>& GrammarSet<TokenType>::operator[](StringParam name)
{
  GrammarRule<TokenType>*& rule = this->mRules[name];
  if (rule == nullptr)
  {
    rule = new GrammarRule<TokenType>();
    rule->mGrammarSet = this;
    rule->mName = name;
    rule->mOrderId = this->mOrderIdCounter++;
  }
  return *rule;
}

//***************************************************************************
template <typename TokenType>
void GrammarSet<TokenType>::AddKeyword(StringParam keyword, StringParam ruleName)
{
  this->mKeywords[keyword] = &(*this)[ruleName];
}

//***************************************************************************
template <typename TokenType>
void GrammarSet<TokenType>::AddKeyword(StringParam keyword, GrammarRule<TokenType>& rule)
{
  this->mKeywords[keyword] = &rule;
}

//***************************************************************************
template <typename TokenType>
void GrammarSet<TokenType>::AddIgnore(StringParam ruleName)
{
  this->mIgnore.Insert(&(*this)[ruleName]);
}

//***************************************************************************
template <typename TokenType>
void GrammarSet<TokenType>::AddIgnore(GrammarRule<TokenType>& rule)
{
  this->mIgnore.Insert(&rule);
}

//***************************************************************************
template <typename T>
AutoPush<T>::AutoPush(Array<T>& array, const T& value)
{
  array.PushBack(value);
  this->mArray = &array;
}

//***************************************************************************
template <typename T>
AutoPush<T>::~AutoPush()
{
  this->mArray->PopBack();
}

//***************************************************************************
template <typename TokenType>
Capture<TokenType>::Capture() :
  mParent(nullptr),
  mStartInclusive(0),
  mEndExclusive(0)
{
}

//***************************************************************************
template <typename TokenType>
Capture<TokenType>::~Capture()
{
  forRange (Capture* nestedCapture, this->mNestedCaptures.All())
  {
    delete nestedCapture;
  }
}
//***************************************************************************
template <typename TokenType>
TokenType Capture<TokenType>::GetFirstToken(StringParam name, const TokenType& failResult)
{
  Array<Capture*>* namedCaptures = mNestedCapturesByName.FindPointer(name, nullptr);
  if(namedCaptures == nullptr || namedCaptures->Empty())
    return failResult;

  Capture* firstNamedCapture = (*namedCaptures)[0];

  if(firstNamedCapture->mTokens.Empty())
    return failResult;

  return firstNamedCapture->mTokens[0];
}

//***************************************************************************
template <typename TokenType>
ParseNodeInfo<TokenType>::ParseNodeInfo() :
  mStartInclusive(0),
  mEndExclusive(0),
  mRule(nullptr),
  mAccepted(false),
  mFailed(false),
  mCapture(nullptr)
{
}

//***************************************************************************
template <typename TokenType>
ParseNodeInfo<TokenType>::~ParseNodeInfo()
{
  delete this->mCapture;
}

//***************************************************************************
template <typename TokenType>
TokenType ParseNodeInfo<TokenType>::GetFirstCapturedToken(StringParam name, const TokenType& failResult)
{
  if(mCapture == nullptr)
    return failResult;
  return mCapture->GetFirstToken(name, failResult);
}

//***************************************************************************
template <typename TokenType>
ParseNode<TokenType>::ParseNode() :
  mParent(nullptr)
{
}

//***************************************************************************
template <typename TokenType>
ParseNode<TokenType>::ParseNode(const ParseNodeInfo<TokenType>& info) :
  ParseNodeInfo(info),
  mParent(nullptr)
{
}

//***************************************************************************
template <typename TokenType>
ParseNode<TokenType>::~ParseNode()
{
  forRange (ParseNode* node, mChildren.All())
    delete node;
}

//***************************************************************************
template <typename TokenType>
ParseNode<TokenType>& ParseNode<TokenType>::operator=(const ParseNodeInfo<TokenType>& rhs)
{
  *static_cast<ParseNodeInfo*>(this) = rhs;
  return *this;
}

//***************************************************************************
template <typename TokenType>
String ParseNode<TokenType>::GetDebugRepresentation()
{
  StringBuilder builder;
  this->GetDebugRepresentation(0, builder);
  return builder.ToString();
}

//***************************************************************************
template <typename TokenType>
void ParseNode<TokenType>::OutputNode(StringBuilder& builder)
{
  if (this->mRule != nullptr)
  {
    builder.Append(this->mRule->mName);

    if (this->mAccepted)
      builder.Append('*');
  }
  else
  {
    builder.Append(this->mToken.ToEscapedString());
  }
}

//***************************************************************************
template <typename TokenType>
void ParseNode<TokenType>::GetDebugRepresentation(size_t depth, StringBuilder& builder)
{
  static const String DebugBar("| ");
  builder.Repeat(depth, DebugBar);
  this->OutputNode(builder);
  builder.Append("\n");

  forRange (ParseNode* child, this->mChildren.All())
  {
    child->GetDebugRepresentation(depth + 1, builder);
  }
}

//***************************************************************************
template <typename TokenType>
String ParseNode<TokenType>::GetGraphRepresentation()
{
  StringBuilder builder;
  builder.Append("digraph G\n");
  builder.Append("{\n");
  size_t index = 0;
  this->GetGraphRepresentation(index, builder);
  builder.Append("}\n");
  return builder.ToString();
}

//***************************************************************************
template <typename TokenType>
void ParseNode<TokenType>::GetGraphRepresentation(size_t& index, StringBuilder& builder)
{
  size_t myIndex = index;
  ++index;

  forRange (ParseNode* node, this->mChildren.All())
  {
    builder.Append(String::Format("  %d -> %d;\n", myIndex, index));
    node->GetGraphRepresentation(index, builder);
  }

  builder.Append(String::Format("  %d [label=\"", myIndex));
  this->OutputNode(builder);
  builder.Append("\"];\n");

  if (this->mCapture != nullptr)
  {
    forRange (Capture<TokenType>* nestedCapture, this->mCapture->mNestedCaptures.All())
    {
      builder.Append(String::Format("  %d -> %d;\n", myIndex, index));
      GetGraphRepresentation(index, builder, nestedCapture);
    }
  }
}

//***************************************************************************
template <typename TokenType>
void ParseNode<TokenType>::GetGraphRepresentation(size_t& index, StringBuilder& builder, Capture<TokenType>* capture)
{
  size_t myIndex = index;
  ++index;

  forRange (Capture<TokenType>* nestedCapture, capture->mNestedCaptures.All())
  {
    builder.Append(String::Format("  %d -> %d;\n", myIndex, index));
    GetGraphRepresentation(index, builder, nestedCapture);
  }

  bool first = true;
  builder.Append(String::Format("  %d [label=\"%s(", myIndex, capture->mName.c_str()));
  forRange (TokenType& capturedToken, capture->mTokens.All())
  {
    if (first == false)
      builder.Append(' ');

    first = true;
    builder.Append(capturedToken.ToEscapedString());
  }
  builder.Append(")\"];\n");
}

//***************************************************************************
template <typename TokenType>
void EmptyParseHandler<TokenType>::StartRule(GrammarRule<TokenType>* rule)
{
}

//***************************************************************************
template <typename TokenType>
void EmptyParseHandler<TokenType>::EndRule(ParseNodeInfo<TokenType>* info)
{
}

//***************************************************************************
template <typename TokenType>
void EmptyParseHandler<TokenType>::TokenParsed(ParseNodeInfo<TokenType>* info)
{
}

//***************************************************************************
template <typename TokenType>
void EmptyParseHandler<TokenType>::StartParsing()
{
}

//***************************************************************************
template <typename TokenType>
void EmptyParseHandler<TokenType>::EndParsing()
{
}

//***************************************************************************
template <typename TokenType>
ParseTreeBuilder<TokenType>::ParseTreeBuilder() :
  mTree(nullptr)
{
}

//***************************************************************************
template <typename TokenType>
ParseTreeBuilder<TokenType>::~ParseTreeBuilder()
{
  delete this->mTree;
}

//***************************************************************************
template <typename TokenType>
void ParseTreeBuilder<TokenType>::CreateRootNode()
{
  this->Restart();
  this->mTree = new ParseNode<TokenType>();
}

//***************************************************************************
template <typename TokenType>
void ParseTreeBuilder<TokenType>::Restart()
{
  // There should only ever be one here, but just be sure
  delete this->mTree;
  this->mTree = nullptr;
}

//***************************************************************************
template <typename TokenType>
void ParseTreeBuilder<TokenType>::StartRule(GrammarRule<TokenType>* rule)
{
  // We mark who our parent is, and then become the new root of the tree
  ParseNode<TokenType>* node = new ParseNode<TokenType>();
  node->mParent = this->mTree;
  this->mTree = node;
}

//***************************************************************************
template <typename TokenType>
void ParseTreeBuilder<TokenType>::EndRule(ParseNodeInfo<TokenType>* info)
{
  ParseNode<TokenType>* ourNode = this->mTree;
  ParseNode<TokenType>* parentNode = ourNode->mParent;

  // Copy all the parse info over
  *ourNode = *info;
  info->mCapture = nullptr;

  if (parentNode)
  {
    // Attach ourselves as a child of our parent
    parentNode->mChildren.PushBack(ourNode);

    // Iterate the tree back to its parent
    this->mTree = parentNode;
  }
}

//***************************************************************************
template <typename TokenType>
void ParseTreeBuilder<TokenType>::TokenParsed(ParseNodeInfo<TokenType>* info)
{
  ParseNode<TokenType>* node = new ParseNode<TokenType>(*info);
  node->mParent = this->mTree;
  this->mTree->mChildren.PushBack(node);
}

//***************************************************************************
template <typename TokenType>
void ParseTreeBuilder<TokenType>::StartParsing()
{
}

//***************************************************************************
template <typename TokenType>
void ParseTreeBuilder<TokenType>::EndParsing()
{
}

//***************************************************************************
template <typename TokenType, typename StreamType>
ParseError<TokenType, StreamType>::ParseError() :
  mStack(nullptr),
  mStream(nullptr),
  mFailedIndex(0)
{
}

//***************************************************************************
template <typename TokenType>
CaptureVariable<TokenType>::CaptureVariable() :
  mCapture(nullptr)
{
  static const String DefaultVariableName("value");
  this->mName = DefaultVariableName;
}

//***************************************************************************
template <typename TokenType, typename StreamType, typename ParseHandlerType>
RecursiveDescentParser<TokenType, StreamType, ParseHandlerType>::RecursiveDescentParser()
{
  this->mTokenMode = false;
  this->mDebug = false;
  this->mStartRule = nullptr;
  this->mStream = nullptr;
  this->mParseHandler = nullptr;

  this->mAcceptedRule = nullptr;
  this->mStartIndex = 0;
  this->mAcceptedIndex = 0;
  this->mEnd = false;

  this->mLastAttemptedAccept = nullptr;
  this->mIndex = 0;
  this->mDepth = 0;
  this->mReplacementDepth = 0;
  this->mBackout = false;
  this->mNonStartedRuleIndex = (size_t)-1;
}

//***************************************************************************
template <typename TokenType, typename StreamType, typename ParseHandlerType>
RecursiveDescentParser<TokenType, StreamType, ParseHandlerType>::~RecursiveDescentParser()
{
}

//***************************************************************************
template <typename TokenType, typename StreamType, typename ParseHandlerType>
RecursiveDescentParser<TokenType, StreamType, ParseHandlerType>::RecursiveDescentParser(const RecursiveDescentParser& rhs)
{
  this->mTokenMode = rhs.mTokenMode;
  this->mDebug = rhs.mDebug;
  this->mStartRule = rhs.mStartRule;
  this->mStream = rhs.mStream;
  this->mParseHandler = rhs.mParseHandler;

  this->mAcceptedRule = rhs.mAcceptedRule;
  this->mStartIndex = rhs.mStartIndex;
  this->mAcceptedIndex = rhs.mAcceptedIndex;
  this->mEnd = rhs.mEnd;
  this->mStatus = rhs.mStatus;
  this->mDebugOutput.Append(rhs.mDebugOutput.ToString());

  this->mLastAttemptedAccept = rhs.mLastAttemptedAccept;
  this->mIndex = rhs.mIndex;
  this->mDepth = 0;
  this->mReplacementDepth = 0;
  this->mBackout = rhs.mBackout;
  this->mNextToken = rhs.mNextToken;
  this->mNonStartedRuleIndex = rhs.mNonStartedRuleIndex;
}

//***************************************************************************
template <typename TokenType, typename StreamType, typename ParseHandlerType>
void RecursiveDescentParser<TokenType, StreamType, ParseHandlerType>::Parse()
{
  // Let the user know that we started parsing
  // This is useful for building parse trees
  if (this->mParseHandler)
    this->mParseHandler->StartParsing();

  this->mStatus.Reset();
  this->mAcceptedRule = nullptr;
  this->mNextToken = TokenType();
  this->mBackout = false;
  this->mLastAttemptedAccept = nullptr;

  this->mStartIndex = this->mIndex;
  this->mAcceptedIndex = this->mIndex;
  this->mNextToken = this->mStream->GetToken(this->mIndex);

  if (this->mNextToken.IsEnd())
  {
    this->mEnd = true;
    return;
  }

  this->EvaluateGrammar(this->mStartRule);

  if (this->mAcceptedIndex != this->mStartIndex)
  {
    this->mIndex = this->mAcceptedIndex;
  }
  else if (this->mTokenMode)
  {
    String nextTokenText = this->mNextToken.ToEscapedString();
    String message = String::Format("Got an unexpected token '%s' (nothing was parsed and the invalid token will be skipped)", nextTokenText.c_str());
    this->SetError(message);
    ++this->mIndex;
  }

  if (this->mDebug)
    this->mDebugOutput.Append("\n\n");

  // Let the user know that we ended parsing
  // This is useful for building parse trees
  if (this->mParseHandler)
    this->mParseHandler->EndParsing();
}

//***************************************************************************
template <typename TokenType, typename StreamType, typename ParseHandlerType>
void RecursiveDescentParser<TokenType, StreamType, ParseHandlerType>::ReplaceAndBackup(StringParam text, size_t startInclusive, size_t endExclusive)
{
  this->mStream->Replace(text, startInclusive, endExclusive);
  this->mIndex = startInclusive;

  // Reset whatever we might have accepted before
  this->mAcceptedIndex = startInclusive;
  this->mAcceptedRule = nullptr;

  this->mNextToken = this->mStream->GetToken(startInclusive);
}

//***************************************************************************
template <typename TokenType, typename StreamType, typename ParseHandlerType>
void RecursiveDescentParser<TokenType, StreamType, ParseHandlerType>::CheckRecursionDepth()
{
  if (this->mDepth > MaxRecursionDepth)
    this->SetErrorAndBackout("Exceeded maximum recursion depth");
}

//***************************************************************************
template <typename TokenType, typename StreamType, typename ParseHandlerType>
bool RecursiveDescentParser<TokenType, StreamType, ParseHandlerType>::EvaluateGrammar(GrammarNode<TokenType>* node)
{
  AutoIncrement depthIncrement(&this->mDepth);
  this->CheckRecursionDepth();

  if (this->mBackout)
    return false;

  if (this->mDebug)
  {
    static const String DebugBar("| ");

    this->mDebugOutput.Append("\n");
    size_t depth = this->mDepth;

    this->mDebugOutput.Repeat(depth, DebugBar);
    this->mDebugOutput.Append(GrammarNodeType::Names[node->mType]);
  }

  switch (node->mType)
  {
  case GrammarNodeType::Epsilon:
    return true;

  case GrammarNodeType::Rule:
  {
    GrammarRule<TokenType>* rule = static_cast<GrammarRule<TokenType>*>(node);
    AutoPush<GrammarRule<TokenType>*> rulePush(this->mRuleStack, rule);

    for (size_t i = 0; i < rule->mReplacements.Size(); ++i)
    {
      Pair<GrammarNode<TokenType>*, ReplacementNode*>& pair = rule->mReplacements[i];

      GrammarNode<TokenType>* find = pair.first;
      ReplacementNode* replaceWith = pair.second;

      Capture<TokenType>* captureRoot = new Capture<TokenType>();

      size_t startInclusive = this->mIndex;
      this->mCaptureStack.PushBack(captureRoot);
      ++this->mReplacementDepth;
      bool result = this->EvaluateGrammar(find);
      --this->mReplacementDepth;
      this->mCaptureStack.PopBack();

      if (this->mBackout)
        return false;

      if (result)
      {
        size_t endExclusive = this->mIndex;

        StringBuilder builder;
        this->mCaptureStack.PushBack(captureRoot);
        this->EvaluateReplacement(replaceWith, builder);
        this->mCaptureStack.PopBack();
        String replacementText = builder.ToString();

        if (replacementText.SizeInBytes() > MaxReplacementStringLength)
        {
          this->SetError("Exceeded maximum string replacement length");
          return false;
        }

        this->ReplaceAndBackup(replacementText, startInclusive, endExclusive);

        // Every time we backup, we restart walking through the replacements
        // This is because one replacement could have changed the text for another replacement
        i = 0;
      }
    }

    if (this->mDebug)
    {
      this->mDebugOutput.Append("(");
      this->mDebugOutput.Append(rule->mName);
      this->mDebugOutput.Append(")");
    }

    if (node->mOperand == nullptr)
      return true;

    if (this->ShouldInvokeParseHandler())
    {
      // If the index is invalid, then we set it to our current index, because this is the first
      if (this->mNonStartedRuleIndex >= this->mNonStartedRules.Size())
        this->mNonStartedRuleIndex = this->mNonStartedRules.Size();

      // We're not going to tell the user that a rule started until we've read at least one token
      this->mNonStartedRules.PushBack(rule);
    }

    // Capture nodes always create capture rules
    // However, rules also implicitly create a capture root (captures don't go beyond rules when parsing)
    // This is NOT the case when inside replacements
    bool createCaptureNode = (this->IsInReplacement() == false);
    if (createCaptureNode)
    {
      this->mCaptureStack.PushBack(nullptr);
    }

    size_t startInclusive = this->mIndex;
    bool result = this->EvaluateGrammar(node->mOperand);
    size_t endExclusive = this->mIndex;

    Capture<TokenType>* captureRoot = nullptr;
    if (createCaptureNode)
    {
      captureRoot = this->mCaptureStack.Back();
      this->mCaptureStack.PopBack();
    }

    bool accepted = false;

    // We only accept another rule if we parsed anything since the last accept
    // We also only do this if we're within token mode
    if (result && this->mAcceptedIndex != endExclusive && this->mTokenMode)
    {
      accepted = true;
      this->mAcceptedIndex = endExclusive;
      this->mAcceptedRule = static_cast<GrammarRule<TokenType>*>(node);
    }

    // We basically always want to create parse nodes, even if we failed (except when inside replacements)
    // The only exception is if nothing is parsed, then we pretty much don't want them
    if (this->ShouldInvokeParseHandler())
    {
      this->mNonStartedRules.PopBack();

      // Only let the user know about ending a rule if anything was parsed
      bool parsedAnything = (endExclusive != startInclusive);
      if (parsedAnything)
      {
        // Dispatch the event on the rule that was parsed, as well as on the entire parser itself
        // Note: The destructor of ParseNodeInfo automatically destroys the capture (if it isn't stolen by the user)
        ParseNodeInfo<TokenType> toSend;
        toSend.mStartInclusive = startInclusive;
        toSend.mEndExclusive = endExclusive;
        toSend.mRule = rule;
        toSend.mFailed = !result;
        toSend.mCapture = captureRoot;
        toSend.mAccepted = accepted;
        this->mParseHandler->EndRule(&toSend);

        // We do this because always delete the capture root at the bottom
        // The user should have set mCapture to null if they stole the data
        captureRoot = toSend.mCapture;
      }
    }
    else
    {
      // Note: In the above case, the destructor of ParseNodeInfo automatically destroys the capture (if it isn't stolen by the user)
      // Delete the capture node if we had one (if anyone consumed it or stole it, this will be set to null)
      // For example if the parse node steals it (above)
      delete captureRoot;
    }

    return result;
  }

  case GrammarNodeType::Or:
  {
    size_t startInclusive = this->mIndex;
    bool result = this->EvaluateGrammar(node->mLhs);
    size_t endExclusive = this->mIndex;

    bool lhsAdvancedTokenStream = (startInclusive != endExclusive);
    if (result && lhsAdvancedTokenStream)
      return true;

    if (this->mBackout)
      return false;

    return this->EvaluateGrammar(node->mRhs);
  }

  case GrammarNodeType::Concatenate:
  {
    // It is only an error if the left hand side does not advance the token stream
    size_t startInclusive = this->mIndex;
    if (this->EvaluateGrammar(node->mLhs) == false)
      return false;
    size_t endExclusive = this->mIndex;

    // The only case in which we can have an error is if this fails, AND we read something on the left side
    bool lhsAdvancedTokenStream = (startInclusive != endExclusive);

    // For error recovery, we keep reading until we succeed or reach the null terminator
    for (;;)
    {
      // This handles any backing out done by the lhs, as well as the rhs
      // error recovery (only in tokenization will backout get set)
      // In token mode we return false (we're done with this token)
      // In parse mode we return true (keep the parse tree)
      if (this->mBackout)
      return !this->mTokenMode;

    // If we parse the right hand side, then we succeeded
    // NOTE: This may have been after error recovery
    if (this->EvaluateGrammar(node->mRhs))
      return true;

    // Failing to parse the right side is ok, unless we parsed something on the left hand side
    // In that case, it's definitely an error
    if (lhsAdvancedTokenStream == false)
      return false;

    // If we're in tokenization mode, then this is only an error if we never accepted a token
    // (we still want to backout because we are finished, however)
    if (this->mTokenMode && this->mAcceptedRule != nullptr)
    {
      // All we do is backout quietly
      this->mBackout = true;
      return false;
    }
    else
    {
      // We're going to print an error message out here
      // If we're not in token mode, SetError will eat a single token (which may result in the null terminator token)
      // Because we're in a loop, we'll end up re-running 
      String nextUnit = this->mNextToken.ToEscapedString();

      if (this->mLastAttemptedAccept != nullptr)
      {
        String rangeSet = this->mLastAttemptedAccept->GetRangeSetString();
        String message = String::Format("Attempted to parse '%s', but instead we got '%s'", rangeSet.c_str(), nextUnit.c_str());
        this->SetError(message);
      }
      else
      {
        String message = String::Format("Attempted to parse a token, but instead we got '%s'", nextUnit.c_str());
        this->SetError(message);
      }

      ++this->mIndex;
      this->mNextToken = this->mStream->GetToken(this->mIndex);

      // If we reached the end of the stream, then we obviously failed to parse (exit out)
      // However, we return true because we want to keep any parse nodes that may have been created
      if (this->mNextToken.IsEnd())
        this->mBackout = true;
    }
    }
  }

  case GrammarNodeType::OneOrMore:
    if (this->EvaluateGrammar(node->mOperand) == false)
      return false;
    // Purposely fall through to ZeroOrMore case

  case GrammarNodeType::ZeroOrMore:
    for (;;)
    {
      size_t startInclusive = this->mIndex;
    bool result = this->EvaluateGrammar(node->mOperand);
    size_t endExclusive = this->mIndex;
    if (result == false || startInclusive == endExclusive)
      break;
    }
    return true;

  case GrammarNodeType::Optional:
    this->EvaluateGrammar(node->mOperand);
    return true;

  case GrammarNodeType::Terminate:
    this->mBackout = true;
    return true;

  case GrammarNodeType::RangeSet:
  case GrammarNodeType::NotRangeSet:
  {
    bool foundToken = false;
    forRange (TokenType& token, node->mSingleTokens.All())
    {
      if (this->mNextToken == token)
      {
        foundToken = true;
        break;
      }
    }

    if (foundToken == false)
    {
      forRange (GrammarRange<TokenType>& range, node->mRanges.All())
      {
        // Ranges are generally only going to work with characters
        if (this->mNextToken >= range.mStartInclusive && this->mNextToken <= range.mEndInclusive)
        {
          foundToken = true;
          break;
        }
      }
    }

    bool success = foundToken;
    if (node->mType == GrammarNodeType::NotRangeSet)
      success = !success;


    if (this->mDebug)
    {
      String nextToken = this->mNextToken.ToEscapedString();

      if (success)
        this->mDebugOutput.Append("(Accepted: ");
      else
        this->mDebugOutput.Append("(Rejected: ");

      this->mDebugOutput.Append(nextToken);
      this->mDebugOutput.Append(" from set ");
      node->GetRangeSetString(this->mDebugOutput);
      this->mDebugOutput.Append(")");
    }

    if (success)
    {
      if (this->ShouldInvokeParseHandler())
      {
        // Inform the user of all the rules that have now started
        // Note that the non-started rule index may be out of bounds, which indicates it is not valid and there
        // are no rules to be started
        for (size_t i = this->mNonStartedRuleIndex; i < this->mNonStartedRules.Size(); ++i)
        {
          GrammarRule<TokenType>* rule = this->mNonStartedRules[i];
          this->mParseHandler->StartRule(rule);
        }
        this->mNonStartedRuleIndex = this->mNonStartedRules.Size();

        // Let the user create their own parse nodes if they want to
        ParseNodeInfo<TokenType> toSend;
        toSend.mStartInclusive = this->mIndex;
        toSend.mEndExclusive = this->mIndex + 1;
        toSend.mToken = this->mNextToken;
        this->mParseHandler->TokenParsed(&toSend);
      }

      ++this->mIndex;
      this->mNextToken = this->mStream->GetToken(this->mIndex);
      return true;
    }
    this->mLastAttemptedAccept = node;
    return false;
  }

  case GrammarNodeType::Capture:
  {
    Capture<TokenType>* parent = this->mCaptureStack.Back();
    if (parent == nullptr)
    {
      parent = new Capture<TokenType>();
      this->mCaptureStack.Back() = parent;
    }

    Capture<TokenType>* capture = new Capture<TokenType>();
    this->mCaptureStack.PushBack(capture);

    size_t startInclusive = this->mIndex;
    bool result = this->EvaluateGrammar(node->mOperand);

    this->mCaptureStack.PopBack();

    if (result)
    {
      size_t endExclusive = this->mIndex;

      // Don't bother capturing empty strings
      if (startInclusive == endExclusive)
      {
        delete capture;
      }
      else
      {
        capture->mParent = parent;
        parent->mNestedCapturesByName[node->mName].PushBack(capture);
        parent->mNestedCaptures.PushBack(capture);
        capture->mStartInclusive = startInclusive;
        capture->mEndExclusive = endExclusive;
        capture->mName = node->mName;

        capture->mTokens.Reserve(endExclusive - startInclusive);
        for (size_t i = startInclusive; i < endExclusive; ++i)
        {
          TokenType token = this->mStream->GetToken(i);
          capture->mTokens.PushBack(token);
        }
      }
      return true;
    }
    else
    {
      delete capture;
    }
    return false;
  }

  default:
    Error("Hit a GrammarNode type we didn't expect");
    return false;
  }
}

//***************************************************************************
template <typename TokenType, typename StreamType, typename ParseHandlerType>
void RecursiveDescentParser<TokenType, StreamType, ParseHandlerType>::EvaluateReplacement(ReplacementNode* node, StringBuilder& builder)
{
  switch (node->mType)
  {
  case ReplacementNodeType::Text:
    builder.Append(node->mText);
    break;

  case ReplacementNodeType::JoinCapture:
  {
    Array<Capture<TokenType>*> captures;
    this->EvaluateCapture(node->mCaptureReference, captures);

    size_t lastIndex = captures.Size() - 1;
    for (size_t i = 0; i < captures.Size(); ++i)
    {
      Capture<TokenType>* capture = captures[i];
      if (capture != nullptr)
        builder.Append(this->mStream->GetText(capture->mStartInclusive, capture->mEndExclusive));

      if (node->mRhs != nullptr && i != lastIndex)
        this->EvaluateReplacement(node->mRhs, builder);
    }
    break;
  }

  case ReplacementNodeType::ForeachCapture:
  {
    Array<Capture<TokenType>*> captures;
    this->EvaluateCapture(node->mCaptureReference, captures);

    CaptureVariable<TokenType>& variable = this->mCaptureVariableStack.PushBack();

    for (size_t i = 0; i < captures.Size(); ++i)
    {
      Capture<TokenType>* capture = captures[i];
      variable.mCapture = capture;

      this->EvaluateReplacement(node->mRhs, builder);
    }

    this->mCaptureVariableStack.PopBack();
    break;
  }

  case ReplacementNodeType::Concatenate:
    this->EvaluateReplacement(node->mLhs, builder);
    this->EvaluateReplacement(node->mRhs, builder);
    break;

  default:
    Error("Hit a ReplacementNode type we didn't expect");
    return;
  }
}

//***************************************************************************
template <typename TokenType, typename StreamType, typename ParseHandlerType>
void RecursiveDescentParser<TokenType, StreamType, ParseHandlerType>::EvaluateCapture(CaptureExpressionNode* node, Array<Capture<TokenType>*>& capturesOut)
{
  switch (node->mType)
  {
  case CaptureExpressionNodeType::NamedCapture:
  {
    if (this->mCaptureStack.Empty())
      break;

    Capture<TokenType>* parent = this->mCaptureStack.Back();
    Array<Capture<TokenType>*>& captures = parent->mNestedCapturesByName[node->mName];
    if (captures.Empty() == false)
    {
      capturesOut.Append(captures.All());
    }
    else
    {
      for (int i = (int)(this->mCaptureVariableStack.Size() - 1); i >= 0; --i)
      {
        CaptureVariable<TokenType>& variable = this->mCaptureVariableStack[i];
        if (variable.mName == node->mName)
        {
          capturesOut.PushBack(variable.mCapture);
          break;
        }
      }
    }
    break;
  }

  case CaptureExpressionNodeType::NestedCapture:
  {
    Array<Capture<TokenType>*> captures;
    this->EvaluateCapture(node->mLhs, captures);

    forRange (Capture<TokenType>* capture, captures.All())
    {
      Array<Capture<TokenType>*>& nextedCaptures = capture->mNestedCapturesByName[node->mName];
      capturesOut.Append(nextedCaptures.All());
    }
    break;
  }

  case CaptureExpressionNodeType::IndexRange:
  {
    Array<Capture<TokenType>*> captures;
    this->EvaluateCapture(node->mLhs, captures);

    // Negative indices wrap around from the back
    int startIndexInclusive = node->mStartIndexInclusive;
    if (startIndexInclusive < 0)
      startIndexInclusive += captures.Size();
    int endIndexInclusive = node->mStartIndexInclusive;
    if (endIndexInclusive < 0)
      endIndexInclusive += captures.Size();

    // The index could still be out of bounds (more negative than the size, or greater)
    for (int i = startIndexInclusive; i <= endIndexInclusive; ++i)
    {
      if (i >= 0 && i < (int)capturesOut.Size())
        capturesOut.PushBack(captures[i]);
    }
    break;
  }

  case CaptureExpressionNodeType::Exists:
  {
    Array<Capture<TokenType>*> captures;
    this->EvaluateCapture(node->mRhs, captures);

    if (captures.Empty() == false)
      capturesOut.PushBack(nullptr);
    break;
  }

  case CaptureExpressionNodeType::Not:
  {
    Array<Capture<TokenType>*> captures;
    this->EvaluateCapture(node->mRhs, captures);

    if (captures.Empty())
      capturesOut.PushBack(nullptr);
    break;
  }

  case CaptureExpressionNodeType::Union:
  {
    this->EvaluateCapture(node->mLhs, capturesOut);
    this->EvaluateCapture(node->mRhs, capturesOut);
    break;
  }

  default:
    Error("Hit a CaptureExpressionNode type we didn't expect");
    return;
  }
}

//***************************************************************************
template <typename TokenType, typename StreamType, typename ParseHandlerType>
bool RecursiveDescentParser<TokenType, StreamType, ParseHandlerType>::ShouldInvokeParseHandler()
{
  return this->mParseHandler && this->IsInReplacement() == false;
}

//***************************************************************************
template <typename TokenType, typename StreamType, typename ParseHandlerType>
bool RecursiveDescentParser<TokenType, StreamType, ParseHandlerType>::IsInReplacement()
{
  return this->mReplacementDepth != 0;
}

//***************************************************************************
template <typename TokenType, typename StreamType, typename ParseHandlerType>
void RecursiveDescentParser<TokenType, StreamType, ParseHandlerType>::SetError(StringParam error)
{
  if (this->mStatus.Failed() == false)
  {
    ParseError<TokenType, StreamType>& parseError = this->mStatus.Context;
    parseError.mStream = this->mStream;
    parseError.mFailedIndex = this->mIndex;
    parseError.mStack = &this->mRuleStack;
    this->mStatus.SetFailed(error, parseError);
  }
}

//***************************************************************************
template <typename TokenType, typename StreamType, typename ParseHandlerType>
void RecursiveDescentParser<TokenType, StreamType, ParseHandlerType>::SetErrorAndBackout(StringParam error)
{
  this->SetError(error);
  this->mBackout = true;
}

//***************************************************************************
template <typename ParseHandlerType>
TokenRange<ParseHandlerType>::TokenRange() :
  mHasRunFirstIteration(false),
  mSet(nullptr)
{
  this->mSet = nullptr;
  this->mParser.mStream = &this->mStream;
  this->mParser.mTokenMode = true;
}

//***************************************************************************
template <typename ParseHandlerType>
TokenRange<ParseHandlerType>::TokenRange(GrammarSet<Character>& set, GrammarRule<Character>& rule, StringParam input, bool debug) :
  mHasRunFirstIteration(false),
  mSet(&set)
{
  this->mParser.mDebug = debug;
  this->mParser.mStream = &this->mStream;
  this->mParser.mTokenMode = true;
  this->mParser.mStartRule = &rule;
  this->mStream.mText = input;
}

//***************************************************************************
template <typename ParseHandlerType>
TokenRange<ParseHandlerType>::TokenRange(const TokenRange& rhs) :
  mParser(rhs.mParser),
  mStream(rhs.mStream),
  mHasRunFirstIteration(rhs.mHasRunFirstIteration),
  mSet(rhs.mSet)
{
  this->mParser.mStream = &this->mStream;
}

//***************************************************************************
template <typename ParseHandlerType>
TokenRange<ParseHandlerType>& TokenRange<ParseHandlerType>::operator=(const TokenRange& rhs)
{
  if (&rhs == this)
    return *this;

  this->~TokenRange();
  new (this) TokenRange(rhs);
  return *this;
}

//***************************************************************************
template <typename ParseHandlerType>
Token TokenRange<ParseHandlerType>::Front()
{
  Token token(this->mParser.mAcceptedRule);
  token.mStartInclusive = this->mParser.mStartIndex;
  token.mEndExclusive = this->mParser.mAcceptedIndex;
  token.mStream = &this->mStream;
  token.mString = token.mStream->GetText(token.mStartInclusive, token.mEndExclusive);

  GrammarRule<Character>* keywordRule = this->mSet->mKeywords.FindValue(token.mString, nullptr);
  if (keywordRule != nullptr)
    token.mRule = keywordRule;

  return token;
}

//***************************************************************************
template <typename ParseHandlerType>
bool TokenRange<ParseHandlerType>::Empty()
{
  if (this->mParser.mStream == nullptr)
    return true;

  if (this->mHasRunFirstIteration == false)
  {
    this->mHasRunFirstIteration = true;
    this->PopFront();
  }

  return this->mParser.mEnd;
}

//***************************************************************************
template <typename ParseHandlerType>
void TokenRange<ParseHandlerType>::PopFront()
{
  this->mParser.Parse();

  // While we still have a valid range, and we're finding ignorable tokens (or error tokens)
  // Then skip them by parsing again
  while (!this->Empty() && (this->mSet->mIgnore.Contains(this->mParser.mAcceptedRule) || this->mParser.mStatus.Failed()))
    this->mParser.Parse();
}

//***************************************************************************
template <typename ParseHandlerType>
Token TokenStream<ParseHandlerType>::GetToken(size_t index)
{
  while (index >= this->mTokens.Size())
  {
    if (this->mRange.Empty())
      return Token();

    this->mTokens.PushBack(this->mRange.Front());
    this->mRange.PopFront();
  }

  return this->mTokens[index];
}

//***************************************************************************
template <typename ParseHandlerType>
StringRange TokenStream<ParseHandlerType>::GetText(size_t startInclusive, size_t endExclusive)
{
  Token& start = this->mTokens[startInclusive];
  Token& end = this->mTokens[endExclusive - 1];

  String& text = this->mRange.mStream.mText;

  return text.SubStringFromByteIndices(start.mStartInclusive, end.mEndExclusive - start.mStartInclusive);
}

//***************************************************************************
template <typename ParseHandlerType>
void TokenStream<ParseHandlerType>::Replace(StringParam text, size_t startInclusive, size_t endExclusive)
{
  if (startInclusive == endExclusive)
    return;

  size_t startCharInclusive = this->mTokens[startInclusive].mStartInclusive;
  size_t endCharExclusive = this->mTokens[endExclusive - 1].mEndExclusive;

  this->mRange.mParser.mEnd = false;
  this->mRange.mParser.ReplaceAndBackup(text, startCharInclusive, endCharExclusive);
  this->mRange.PopFront();

  // Since we're iteratively building this token stream, just remove everything after the start
  // Getting a token will rebuild the token stream
  this->mTokens.Resize(startInclusive);
}
}
