/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

namespace Zilch
{
  //***************************************************************************
  template <typename TokenType>
  void DataDrivenLexer::StartRule(GrammarRule<Token>* rule, GrammarSet<TokenType>& grammar, Array<Array<GrammarNode<TokenType>*> >& nodes)
  {
    DataDrivenLexerShared& shared = DataDrivenLexerShared::GetInstance();

    bool needsGrammarNodeStack =
      rule == shared.mRuleStatement                     ||
      rule == shared.mGrammarExpressionOr               ||
      rule == shared.mGrammarExpressionCapture          ||
      rule == shared.mGrammarExpressionConcatenate      ||
      rule == shared.mGrammarExpressionUnary            ||
      rule == shared.mReplacementStatement;
    if (needsGrammarNodeStack)
    {
      nodes.PushBack();
    }

    bool needsReplacementNodeStack =
      rule == shared.mReplacementStatement              ||
      rule == shared.mReplacementExpressionConcatenate  ||
      rule == shared.mReplacementExpressionJoin         ||
      rule == shared.mReplacementExpressionForeach;
    if (needsReplacementNodeStack)
    {
      this->mReplacementNodes.PushBack();
    }

    bool needsCaptureNodeStack =
      rule == shared.mReplacementExpressionPost         ||
      rule == shared.mReplacementExpressionJoin         ||
      rule == shared.mReplacementExpressionForeach;
    if (needsCaptureNodeStack)
    {
      this->mCaptureNodes.PushBack(nullptr);
    }
  }

  //***************************************************************************
  template <typename TokenType>
  void DataDrivenLexer::EndRule(ParseNodeInfo<Token>* info, GrammarSet<TokenType>& grammar, Array<Array<GrammarNode<TokenType>*> >& nodes)
  {
    GrammarRule<Token>* rule = info->mRule;
    DataDrivenLexerShared& shared = DataDrivenLexerShared::GetInstance();

    if (rule == shared.mIgnoreStatement)
    {
      String ruleName = info->GetFirstCapturedToken("RuleName").mString;
      GrammarRule<TokenType>& toBeIgnored = grammar[ruleName];
      grammar.mIgnore.Insert(&toBeIgnored);

      ErrorIf(this->mMode == DataDrivenLexerMode::Parser,
        "Cannot have an ignore on a parser grammar (it is made exclusively for token grammars)");
    }
    else if (rule == shared.mKeywordStatement)
    {
      String ruleName = info->GetFirstCapturedToken("RuleName").mString;
      String keywordStringLiteral = info->GetFirstCapturedToken("Keyword").mString;
      String keyword = ReplaceStringEscapesAndStripQuotes(keywordStringLiteral);

      GrammarRule<TokenType>& toBeKeyword = grammar[ruleName];
      grammar.mKeywords[keyword] = &toBeKeyword;

      ErrorIf(this->mMode == DataDrivenLexerMode::Parser,
        "Cannot have a keyword on a parser grammar (it is made exclusively for token grammars)");
    }
    else if (rule == shared.mRuleStatement)
    {
      Array<GrammarNode<TokenType>*>& operands = nodes.Back();
      if (operands.Size() == 1)
      {
        String ruleName = info->GetFirstCapturedToken("RuleName").mString;
        GrammarRule<TokenType>& modifiedRule = grammar[ruleName];

        // Needs to take into account rewrite
        modifiedRule |= *operands.Front();
        nodes.PopBack();
      }
      else
      {
        Error("We expected one operand");
      }
    }
    else if (rule == shared.mGrammarExpressionCapture)
    {
      Array<GrammarNode<TokenType>*>& operands = nodes.Back();
      if (operands.Size() == 1)
      {
        GrammarNode<TokenType>& node = *new GrammarNode<TokenType>();
        node.mType = Zero::GrammarNodeType::Capture;
        node.mName = info->GetFirstCapturedToken("CaptureName").mString;
        node.mOperand = operands.Front();

        nodes.PopBack();
        nodes.Back().PushBack(&node);
      }
      else
      {
        Error("We expected one operand");
      }
    }
    else if (rule == shared.mGrammarExpressionCaptureRule)
    {
      String captureAndRuleName = info->GetFirstCapturedToken("CaptureName").mString;
      GrammarNode<TokenType>& node = *new GrammarNode<TokenType>();
      node.mType = Zero::GrammarNodeType::Capture;
      node.mName = captureAndRuleName;
      node.mOperand = &grammar[captureAndRuleName];
      nodes.Back().PushBack(&node);
    }
    // If this is a binary operator (we should have pushed a node array in the StartRule)
    else if (rule == shared.mGrammarExpressionOr || rule == shared.mGrammarExpressionConcatenate)
    {
      Array<GrammarNode<TokenType>*>& operands = nodes.Back();
      if (operands.Size() >= 1)
      {
        GrammarNode<TokenType>* lhs = operands.Front();
        for (size_t i = 1; i < operands.Size(); ++i)
        {
          GrammarNode<TokenType>* rhs = operands[i];

          if (rule == shared.mGrammarExpressionOr)
            lhs = &(*lhs | *rhs);
          else
            lhs = &(*lhs << *rhs);
        }

        nodes.PopBack();
        nodes.Back().PushBack(lhs);
      }
      else
      {
        Error("We expected at least one operand (two or more for the binary operators, just 1 if its a pass through)");
      }
    }
    // If this is a unary operator (we should have pushed a node array in the StartRule)
    else if (rule == shared.mGrammarExpressionUnary)
    {
      Array<GrammarNode<TokenType>*>& operands = nodes.Back();
      if (operands.Size() == 1)
      {
        GrammarNode<Character>* op = info->GetFirstCapturedToken("UnaryOperator").mRule;

        GrammarNode<TokenType>* lhs = operands.Front();
        if (op == shared.mZeroOrMore)
          lhs = &*(*lhs);
        else if (op == shared.mOneOrMore)
          lhs = &+(*lhs);
        else if (op == shared.mOptional)
          lhs = &~(*lhs);

        nodes.PopBack();
        nodes.Back().PushBack(lhs);
      }
      else
      {
        Error("We expected one operand");
      }
    }
    else if (rule == shared.mGrammarExpressionValue)
    {
      // P(Identifier) | P(StringLiteral) | P(TokenLiteral) | P(Epsilon)
      Token value = info->GetFirstCapturedToken("Value");
      const String& valueName = value.mString;
      GrammarNode<Character>* valueRule = value.mRule;

      // If this is an identifier, then we're just referring to another rule
      // Note that the rule may not even exist yet (grammar[] creates the rule implicitly)
      if (valueRule == shared.mIdentifier)
      {
        GrammarRule<TokenType>& rule = grammar[valueName];
        nodes.Back().PushBack(&rule);
      }
      // A string literal is either a single character or a character range (range set)
      // This can only be used by the tokenizer
      else if (valueRule == shared.mStringLiteral)
      {
        this->AddStringLiteralNode(valueName, nodes.Back());
      }
      // A token literal must only be used by the parser and refers to a rule defined by the tokenizer
      else if (valueRule == shared.mTokenLiteral)
      {
        this->AddTokenLiteralNode(valueName, nodes.Back());
      }
      // Epsilon just returns true when we attempt to evaluate it... parse nothing!
      else if (valueRule == shared.mEpsilon)
      {
        nodes.Back().PushBack(new GrammarNode<TokenType>());
      }
    }
    else if (rule == shared.mReplacementStatement)
    {
      Array<GrammarNode<TokenType>*>& operands = nodes.Back();
      Array<ReplacementNode*>& replacements = this->mReplacementNodes.Back();
      if (operands.Size() == 1 && replacements.Size() == 1)
      {
        String ruleName = info->GetFirstCapturedToken("RuleName").mString;
        GrammarRule<TokenType>& modifiedRule = grammar[ruleName];
        
        GrammarNode<TokenType>* operand = operands.Front();
        ReplacementNode* replacement = replacements.Front();

        //typedef GrammarNode<TokenType>::ReplacementPair ReplacementPair;
        modifiedRule.mReplacements.PushBack(GrammarNode<TokenType>::ReplacementPair(operand, replacement));

        nodes.PopBack();
        this->mReplacementNodes.PopBack();
      }
      else
      {
        Error("We expected one operand");
      }
    }
    else if (rule == shared.mReplacementExpressionConcatenate)
    {
      Array<ReplacementNode*>& operands = this->mReplacementNodes.Back();
      if (operands.Size() >= 1)
      {
        ReplacementNode* lhs = operands.Front();
        for (size_t i = 1; i < operands.Size(); ++i)
        {
          ReplacementNode* rhs = operands[i];
          lhs = &(*lhs << *rhs);
        }

        this->mReplacementNodes.PopBack();
        this->mReplacementNodes.Back().PushBack(lhs);
      }
      else
      {
        Error("We expected at least one operand (two or more for the concatenation, just 1 if its a pass through)");
      }
    }
    else if (rule == shared.mReplacementExpressionText)
    {
      String textStringLiteral = info->GetFirstCapturedToken("ReplacementText").mString;
      String text = ReplaceStringEscapesAndStripQuotes(textStringLiteral);
      this->mReplacementNodes.Back().PushBack(&R(text));
    }
    else if (rule == shared.mReplacementExpressionPost)
    {
      // We could not get a capture node here, eg if it was wrapped in a
      // replacement, or we're just getting replacement text here
      CaptureExpressionNode* capture = this->mCaptureNodes.Back();
      this->mCaptureNodes.PopBack();
      if (capture != nullptr)
      {
        ReplacementNode* node = &R(*capture);
        this->mReplacementNodes.Back().PushBack(node);
      }
    }
    else if (rule == shared.mReplacementExpressionJoin || rule == shared.mReplacementExpressionForeach)
    {
      // We previously pushed a context when we started, so that other captures nested could occur
      this->mCaptureNodes.PopBack();

      // We must have one replacement and capture operand (this is the push from the Post expression)
      CaptureExpressionNode*& capture = this->mCaptureNodes.Back();
      Array<ReplacementNode*>& operands = this->mReplacementNodes.Back();
      if (operands.Size() == 1 && capture != nullptr)
      {
        // In a Join, this will be the delimiter
        // In a Foreach, this is what we loop over for each capture
        ReplacementNode* operand = operands.Front();
        
        ReplacementNode& node = *new ReplacementNode();
        if (rule == shared.mReplacementExpressionJoin)
          node.mType = Zero::ReplacementNodeType::JoinCapture;
        else
          node.mType = Zero::ReplacementNodeType::ForeachCapture;
        node.mCaptureReference = capture;
        capture = nullptr;
        node.mRhs = operand;

        this->mReplacementNodes.PopBack();
        this->mReplacementNodes.Back().PushBack(&node);
      }
      else
      {
        Error("We expected one operand");
      }
    }
    else if (rule == shared.mCaptureExpressionName)
    {
      Token captureName = info->GetFirstCapturedToken("CaptureName");
      this->mCaptureNodes.Back() = &C(captureName.mString);
    }
    else if (rule == shared.mCaptureExpressionNestedIndex)
    {
      CaptureExpressionNode* capture = this->mCaptureNodes.Back();
      if (capture != nullptr)
      {
        Token startIndex = info->GetFirstCapturedToken("StartIndex");
        Token endIndex = info->GetFirstCapturedToken("EndIndex");
        Token nestedCaptureName = info->GetFirstCapturedToken("NestedCaptureName");

        // If this is a single index (or a range)
        if (startIndex.IsValid())
        {
          int startIndexInclusive = atoi(startIndex.mString.c_str());

          // If this is a range
          if (endIndex.IsValid())
          {
            int endIndeInclusive = atoi(endIndex.mString.c_str());
            this->mCaptureNodes.Back() = &C(*capture, startIndexInclusive, endIndeInclusive);
          }
          // Otherwise, it's just a single index
          else
          {
            this->mCaptureNodes.Back() = &((*capture)[startIndexInclusive]);
          }
        }
        // Otherwise, its a named child capture
        else
        {
          ErrorIf(nestedCaptureName.IsValid() == false,
            "If we didn't get a start index, we must have gotten a nested capture name");
          this->mCaptureNodes.Back() = &((*capture)[nestedCaptureName.mString]);
        }
      }
      else
      {
        Error("Expected a capture node");
      }
    }
  }
}
