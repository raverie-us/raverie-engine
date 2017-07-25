#pragma once
/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_DATA_DRIVEN_LEXER_HPP
#define ZILCH_DATA_DRIVEN_LEXER_HPP

namespace Zilch
{
  using Zero::GrammarSet;
  using Zero::GrammarNode;
  using Zero::Character;
  using Zero::Token;
  using Zero::GrammarRule;
  using Zero::TokenStream;
  using Zero::TokenRange;
  using Zero::RecursiveDescentParser;
  using Zero::ParseTreeBuilder;
  using Zero::ParseNodeInfo;
  using Zero::ReplacementNode;
  using Zero::CaptureExpressionNode;
  using Zero::T;
  using Zero::P;
  using Zero::R;
  using Zero::C;

  class DataDrivenLexerShared
  {
  public:
    DataDrivenLexerShared();
    static DataDrivenLexerShared& GetInstance();

    GrammarSet<Character> mTokenGrammar;
    GrammarSet<Token> mParserGrammar;

    // Special tokens that we may want to use later
    GrammarRule<Character>* mTokenStart;
    GrammarRule<Character>* mIdentifier;
    GrammarRule<Character>* mTokenLiteral;
    GrammarRule<Character>* mStringLiteral;
    GrammarRule<Character>* mIntegerLiteral;
    GrammarRule<Character>* mZeroOrMore;
    GrammarRule<Character>* mOneOrMore;
    GrammarRule<Character>* mOptional;
    GrammarRule<Character>* mEpsilon;
    GrammarRule<Character>* mTokenizer;
    GrammarRule<Character>* mParser;

    // Special parser rules that we may want to use later
    GrammarRule<Token>* mParserStart;
    GrammarRule<Token>* mIgnoreStatement;
    GrammarRule<Token>* mKeywordStatement;
    GrammarRule<Token>* mRuleStatement;
    GrammarRule<Token>* mGrammarExpressionCapture;
    GrammarRule<Token>* mGrammarExpressionCaptureRule;
    GrammarRule<Token>* mGrammarExpressionOr;
    GrammarRule<Token>* mGrammarExpressionConcatenate;
    GrammarRule<Token>* mGrammarExpressionUnary;
    GrammarRule<Token>* mGrammarExpressionValue;
    GrammarRule<Token>* mReplacementStatement;
    GrammarRule<Token>* mReplacementExpressionConcatenate;
    GrammarRule<Token>* mReplacementExpressionPost;
    GrammarRule<Token>* mReplacementExpressionText;
    GrammarRule<Token>* mReplacementExpressionJoin;
    GrammarRule<Token>* mReplacementExpressionForeach;
    GrammarRule<Token>* mCaptureExpression;
    GrammarRule<Token>* mCaptureExpressionName;
    GrammarRule<Token>* mCaptureExpressionNestedIndex;
  };

  DeclareEnum2(DataDrivenLexerMode, Tokenizer, Parser);

  class DataDrivenLexer
  {
  public:
    template <typename TokenType, typename StreamType, typename ParseHandlerType>
    friend class RecursiveDescentParser;

    void Parse(StringParam input, GrammarSet<Character>& tokenGrammar, GrammarSet<Token>& parserGrammar);

  private:
    
    DataDrivenLexerMode::Enum mMode;
    GrammarSet<Character>* mUserTokenGrammar;
    GrammarSet<Token>* mUserParserGrammar;

    Array<Array<GrammarNode<Character>*> > mTokenNodes;
    Array<Array<GrammarNode<Token>*> > mParserNodes;

    Array<Array<ReplacementNode*> > mReplacementNodes;

    Array<CaptureExpressionNode*> mCaptureNodes;

    // ParseHandler interface
    void StartRule(GrammarRule<Token>* rule);
    void EndRule(ParseNodeInfo<Token>* info);
    void TokenParsed(ParseNodeInfo<Token>* info);
    void StartParsing();
    void EndParsing();

    template <typename TokenType>
    void StartRule(GrammarRule<Token>* rule, GrammarSet<TokenType>& grammar, Array<Array<GrammarNode<TokenType>*> >& nodes);
    template <typename TokenType>
    void EndRule(ParseNodeInfo<Token>* info, GrammarSet<TokenType>& grammar, Array<Array<GrammarNode<TokenType>*> >& nodes);

    // We specialize these functions
    // For example, it is only legal to add a string literal in the tokenizer, and a token literal in the parser
    template <typename TokenType>
    void AddStringLiteralNode(StringParam string, Array<GrammarNode<TokenType>*>& nodes);
    template <typename TokenType>
    void AddTokenLiteralNode(StringParam string, Array<GrammarNode<TokenType>*>& nodes);
  };

  // Explicit specializations
  template <> void DataDrivenLexer::AddStringLiteralNode<Character>(StringParam string, Array<GrammarNode<Character>*>& nodes);
  template <> void DataDrivenLexer::AddStringLiteralNode<Token>    (StringParam string, Array<GrammarNode<Token>*>&     nodes);
  template <> void DataDrivenLexer::AddTokenLiteralNode <Character>(StringParam string, Array<GrammarNode<Character>*>& nodes);
  template <> void DataDrivenLexer::AddTokenLiteralNode <Token>    (StringParam string, Array<GrammarNode<Token>*>&     nodes);
}

#include "DataDrivenLexer.inl"

#endif
