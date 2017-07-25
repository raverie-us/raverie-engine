/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  // To simplify our parsing, we work with generic nodes (and only specific nodes in specific cases)
  // Because of this, we need to ensure that the sizes always match
  ZilchStaticAssert(sizeof(GrammarNode<Character>) == sizeof(GrammarNode<Token>),
    "The sizes of the grammar nodes do not match",
    SizesOfGrammarNodesDoNotMatch);

  //***************************************************************************
  DataDrivenLexerShared::DataDrivenLexerShared()
  {
    GrammarRule<Character>& TokenStart        = this->mTokenGrammar["Start"];
    GrammarRule<Character>& Whitespace        = this->mTokenGrammar["Whitespace"];
    GrammarRule<Character>& SingleLineComment = this->mTokenGrammar["SingleLineComment"];
    GrammarRule<Character>& Identifier        = this->mTokenGrammar["Identifier"];
    GrammarRule<Character>& TokenLiteral      = this->mTokenGrammar["TokenLiteral"];
    GrammarRule<Character>& StringLiteral     = this->mTokenGrammar["StringLiteral"];
    GrammarRule<Character>& IntegerLiteral    = this->mTokenGrammar["IntegerLiteral"];
    GrammarRule<Character>& OpenBracket       = this->mTokenGrammar["OpenBracket"];
    GrammarRule<Character>& CloseBracket      = this->mTokenGrammar["CloseBracket"];
    GrammarRule<Character>& OpenCurley        = this->mTokenGrammar["OpenCurley"];
    GrammarRule<Character>& CloseCurley       = this->mTokenGrammar["CloseCurley"];
    GrammarRule<Character>& OpenParenthesis   = this->mTokenGrammar["OpenParenthesis"];
    GrammarRule<Character>& CloseParenthesis  = this->mTokenGrammar["CloseParenthesis"];
    GrammarRule<Character>& Comma             = this->mTokenGrammar["Comma"];
    GrammarRule<Character>& MemberAccess      = this->mTokenGrammar["MemberAccess"];
    GrammarRule<Character>& Colon             = this->mTokenGrammar["Colon"];
    GrammarRule<Character>& Semicolon         = this->mTokenGrammar["Semicolon"];
    GrammarRule<Character>& RewriteAssignment = this->mTokenGrammar["RewriteAssignment"];
    GrammarRule<Character>& OrAssignment      = this->mTokenGrammar["OrAssignment"];
    GrammarRule<Character>& ZeroOrMore        = this->mTokenGrammar["ZeroOrMore"];
    GrammarRule<Character>& OneOrMore         = this->mTokenGrammar["OneOrMore"];
    GrammarRule<Character>& Optional          = this->mTokenGrammar["Optional"];
    GrammarRule<Character>& Or                = this->mTokenGrammar["Or"];
    GrammarRule<Character>& Capture           = this->mTokenGrammar["Capture"];
    GrammarRule<Character>& CaptureRule       = this->mTokenGrammar["CaptureRule"];

    GrammarRule<Character>& Keyword           = this->mTokenGrammar["Keyword"];
    GrammarRule<Character>& Ignore            = this->mTokenGrammar["Ignore"];
    GrammarRule<Character>& Tokenizer         = this->mTokenGrammar["Tokenizer"];
    GrammarRule<Character>& Parser            = this->mTokenGrammar["Parser"];
    GrammarRule<Character>& Epsilon           = this->mTokenGrammar["Epsilon"];
    GrammarRule<Character>& Replace           = this->mTokenGrammar["Replace"];
    GrammarRule<Character>& In                = this->mTokenGrammar["In"];
    GrammarRule<Character>& With              = this->mTokenGrammar["With"];
    GrammarRule<Character>& Using             = this->mTokenGrammar["Using"];

    this->mTokenGrammar.mIgnore.Insert(&Whitespace);
    this->mTokenGrammar.mIgnore.Insert(&SingleLineComment);
    this->mTokenGrammar.mKeywords["keyword"  ] = &Keyword;
    this->mTokenGrammar.mKeywords["ignore"   ] = &Ignore;
    this->mTokenGrammar.mKeywords["tokenizer"] = &Tokenizer;
    this->mTokenGrammar.mKeywords["parser"   ] = &Parser;
    this->mTokenGrammar.mKeywords["e"        ] = &Epsilon;
    this->mTokenGrammar.mKeywords["replace"  ] = &Replace;
    this->mTokenGrammar.mKeywords["in"       ] = &In;
    this->mTokenGrammar.mKeywords["with"     ] = &With;
    this->mTokenGrammar.mKeywords["using"    ] = &Using;

    this->mTokenStart     = &TokenStart;
    this->mIdentifier     = &Identifier;
    this->mTokenLiteral   = &TokenLiteral;
    this->mStringLiteral  = &StringLiteral;
    this->mIntegerLiteral = &IntegerLiteral;
    this->mZeroOrMore     = &ZeroOrMore;
    this->mOneOrMore      = &OneOrMore;
    this->mOptional       = &Optional;
    this->mEpsilon        = &Epsilon;
    this->mTokenizer      = &Tokenizer;
    this->mParser         = &Parser;

    TokenStart        |= Whitespace | SingleLineComment | Identifier | TokenLiteral | StringLiteral | IntegerLiteral | OpenBracket | CloseBracket | OpenCurley | CloseCurley;
    TokenStart        |= OpenParenthesis | CloseParenthesis | Comma | MemberAccess | Semicolon | Colon | RewriteAssignment | OrAssignment | ZeroOrMore | OneOrMore | Optional | Or | Capture | CaptureRule;
    Whitespace        |= +T(" \t\r\n\v\f");
    SingleLineComment |= T("/") << T("/") << *T("^\r\n");
    Identifier        |= T("a-zA-Z_") << *T("a-zA-Z_0-9");
    TokenLiteral      |= T("<") << Identifier << T(">");
    StringLiteral     |= T("\"") << *(T("^\"\\") | T("\\") << T("^")) << T("\"") | T("\'") << *(T("^\'\\") | T("\\") << T("^")) << T("\'");
    IntegerLiteral    |= +T("0-9");
    OpenBracket       |= T("[");
    CloseBracket      |= T("]");
    OpenCurley        |= T("{");
    CloseCurley       |= T("}");
    OpenParenthesis   |= T("(");
    CloseParenthesis  |= T(")");
    Comma             |= T(",");
    MemberAccess      |= T(".");
    Colon             |= T(":");
    Semicolon         |= T(";");
    RewriteAssignment |= T("=");
    OrAssignment      |= Or << T("=");
    ZeroOrMore        |= T("*");
    OneOrMore         |= T("+");
    Optional          |= T("~");
    Or                |= T("|");
    Capture           |= T("$");
    CaptureRule       |= T("%");

    GrammarRule<Token>& ParserStart                       = this->mParserGrammar["Start"];
    GrammarRule<Token>& Scope                             = this->mParserGrammar["Scope"];
    GrammarRule<Token>& Statement                         = this->mParserGrammar["Statement"];
    GrammarRule<Token>& IgnoreStatement                   = this->mParserGrammar["IgnoreStatement"];
    GrammarRule<Token>& KeywordStatement                  = this->mParserGrammar["KeywordStatement"];
    GrammarRule<Token>& RuleStatement                     = this->mParserGrammar["RuleStatement"];
    GrammarRule<Token>& GrammarExpression                 = this->mParserGrammar["GrammarExpression"];
    GrammarRule<Token>& GrammarExpressionGrouped          = this->mParserGrammar["GrammarExpressionGrouped"];
    GrammarRule<Token>& GrammarExpressionCapture          = this->mParserGrammar["GrammarExpressionCapture"];
    GrammarRule<Token>& GrammarExpressionCaptureRule      = this->mParserGrammar["GrammarExpressionCaptureRule"];
    GrammarRule<Token>& GrammarExpressionOr               = this->mParserGrammar["GrammarExpressionOr"];
    GrammarRule<Token>& GrammarExpressionConcatenate      = this->mParserGrammar["GrammarExpressionConcatenate"];
    GrammarRule<Token>& GrammarExpressionUnary            = this->mParserGrammar["GrammarExpressionUnary"];
    GrammarRule<Token>& GrammarExpressionValue            = this->mParserGrammar["GrammarExpressionValue"];
    GrammarRule<Token>& ReplacementStatement              = this->mParserGrammar["ReplacementStatement"];
    GrammarRule<Token>& ReplacementWith                   = this->mParserGrammar["ReplacementWith"];
    GrammarRule<Token>& ReplacementUsing                  = this->mParserGrammar["ReplacementUsing"];
    GrammarRule<Token>& ReplacementExpression             = this->mParserGrammar["ReplacementExpression"];
    GrammarRule<Token>& ReplacementExpressionConcatenate  = this->mParserGrammar["ReplacementExpressionConcatenate"];
    GrammarRule<Token>& ReplacementExpressionPost         = this->mParserGrammar["ReplacementExpressionPost"];
    GrammarRule<Token>& ReplacementExpressionText         = this->mParserGrammar["ReplacementExpressionText"];
    GrammarRule<Token>& ReplacementExpressionJoin         = this->mParserGrammar["ReplacementExpressionJoin"];
    GrammarRule<Token>& ReplacementExpressionForeach      = this->mParserGrammar["ReplacementExpressionForeach"];
    GrammarRule<Token>& CaptureExpression                 = this->mParserGrammar["CaptureExpression"];
    GrammarRule<Token>& CaptureExpressionName             = this->mParserGrammar["CaptureExpressionName"];
    GrammarRule<Token>& CaptureExpressionNestedIndex      = this->mParserGrammar["CaptureExpressionNestedIndex"];

    //Text, CaptureJoin, CaptureIteration, Concatenate

    ParserStart                       |= *Scope;
    Scope                             |= (P(Tokenizer) | P(Parser)) << P(OpenCurley) << *Statement << P(CloseCurley);
    Statement                         |= IgnoreStatement | KeywordStatement | RuleStatement | ReplacementStatement;
    IgnoreStatement                   |= P(Ignore) << P("RuleName", P(Identifier)) << P(Semicolon);
    KeywordStatement                  |= P(Keyword) << P("RuleName", P(Identifier)) << P(Colon) << P("Keyword", P(StringLiteral)) << P(Semicolon);
    RuleStatement                     |= P("RuleName", P(Identifier)) << P("Assignment", P(OrAssignment) | P(RewriteAssignment)) << GrammarExpression << P(Semicolon);
    GrammarExpressionGrouped          |= P(OpenParenthesis) << GrammarExpression << P(CloseParenthesis);
    GrammarExpressionCapture          |= P(Capture) << P("CaptureName", P(Identifier)) << GrammarExpressionGrouped;
    GrammarExpressionCaptureRule      |= P(CaptureRule) << P("CaptureName", P(Identifier));
    GrammarExpression                 |= GrammarExpressionOr;
    GrammarExpressionOr               |= GrammarExpressionConcatenate << *(P(Or) << GrammarExpressionConcatenate);
    GrammarExpressionConcatenate      |= +GrammarExpressionUnary;
    GrammarExpressionUnary            |= P("UnaryOperator", P(ZeroOrMore) | P(OneOrMore) | P(Optional)) << GrammarExpressionUnary | GrammarExpressionValue;
    GrammarExpressionValue            |= P("Value", P(Identifier) | P(StringLiteral) | P(TokenLiteral) | P(Epsilon)) | GrammarExpressionGrouped | GrammarExpressionCapture | GrammarExpressionCaptureRule;
    ReplacementStatement              |= P(Replace) << GrammarExpression << P(In) << P("RuleName", P(Identifier)) << (ReplacementWith | ReplacementUsing);
    ReplacementWith                   |= P(With) << ReplacementExpression << P(Semicolon);
    ReplacementUsing                  |= P("Member", P(Identifier) << ~(P(MemberAccess) << P(Identifier)));
    ReplacementExpression             |= ReplacementExpressionConcatenate;
    ReplacementExpressionConcatenate  |= +ReplacementExpressionPost;
    ReplacementExpressionPost         |= ReplacementExpressionText | CaptureExpression << ~(ReplacementExpressionJoin | ReplacementExpressionForeach);
    ReplacementExpressionText         |= P("ReplacementText", P(StringLiteral));
    ReplacementExpressionJoin         |= P(OpenParenthesis) << ReplacementExpression << P(CloseParenthesis);
    ReplacementExpressionForeach      |= P(OpenCurley) << ReplacementExpression << P(CloseCurley);
    CaptureExpression                 |= CaptureExpressionName << *CaptureExpressionNestedIndex;
    CaptureExpressionName             |= P("CaptureName", P(Identifier));
    CaptureExpressionNestedIndex      |= P(OpenBracket) << (P("StartIndex", P(IntegerLiteral)) << ~(P(Comma) << P("EndIndex", P(IntegerLiteral))) | P("NestedCaptureName", P(Identifier)));

    this->mParserStart                      = &ParserStart;
    this->mIgnoreStatement                  = &IgnoreStatement;
    this->mKeywordStatement                 = &KeywordStatement;
    this->mRuleStatement                    = &RuleStatement;
    this->mGrammarExpressionCapture         = &GrammarExpressionCapture;
    this->mGrammarExpressionCaptureRule     = &GrammarExpressionCaptureRule;
    this->mGrammarExpressionOr              = &GrammarExpressionOr;
    this->mGrammarExpressionConcatenate     = &GrammarExpressionConcatenate;
    this->mGrammarExpressionUnary           = &GrammarExpressionUnary;
    this->mGrammarExpressionValue           = &GrammarExpressionValue;
    this->mReplacementStatement             = &ReplacementStatement;
    this->mReplacementExpressionConcatenate = &ReplacementExpressionConcatenate;
    this->mReplacementExpressionPost        = &ReplacementExpressionPost;
    this->mReplacementExpressionText        = &ReplacementExpressionText;
    this->mReplacementExpressionJoin        = &ReplacementExpressionJoin;
    this->mReplacementExpressionForeach     = &ReplacementExpressionForeach;
    this->mCaptureExpression                = &CaptureExpression;
    this->mCaptureExpressionName            = &CaptureExpressionName;
    this->mCaptureExpressionNestedIndex     = &CaptureExpressionNestedIndex;
  }

  //***************************************************************************
  DataDrivenLexerShared& DataDrivenLexerShared::GetInstance()
  {
    static DataDrivenLexerShared instance;
    return instance;
  }

  //***************************************************************************
  void DataDrivenLexer::Parse(StringParam input, GrammarSet<Character>& userTokenGrammar, GrammarSet<Token>& userParserGrammar)
  {
    this->mMode = DataDrivenLexerMode::Tokenizer;
    this->mUserTokenGrammar = &userTokenGrammar;
    this->mUserParserGrammar = &userParserGrammar;

    DataDrivenLexerShared& shared = DataDrivenLexerShared::GetInstance();
    TokenStream<> stream;
    stream.mRange = TokenRange<>(shared.mTokenGrammar, *shared.mTokenStart, input);

    //ParseTreeBuilder<Token> parserTreeBuilder;

    RecursiveDescentParser<Token, TokenStream<>, DataDrivenLexer> parser;
    parser.mParseHandler = this;
    parser.mStartRule = shared.mParserStart;
    parser.mStream = &stream;
    
    parser.Parse();

    //String diffParserTreeOutput = parserTreeBuilder.mTree->GetDebugRepresentation();
    //
    //String graph = parserTreeBuilder.mTree->GetGraphRepresentation();
    //Zero::WriteToFile("C:\\Sandbox\\Graph.gv", (const byte*)graph.c_str(), graph.size());
    //system("C:\\Progra~2\\Graphviz2.38\\bin\\dot.exe -Tpng \"C:\\Sandbox\\Graph.gv\" > \"C:\\Sandbox\\Graph.png\"");
    //system("C:\\Sandbox\\Graph.png");
  }

  //***************************************************************************
  void DataDrivenLexer::StartRule(GrammarRule<Token>* rule)
  {
    if (this->mMode == DataDrivenLexerMode::Tokenizer)
      this->StartRule<Character>(rule, *this->mUserTokenGrammar, this->mTokenNodes);
    else
      this->StartRule<Token>(rule, *this->mUserParserGrammar, this->mParserNodes);
  }

  //***************************************************************************
  void DataDrivenLexer::EndRule(ParseNodeInfo<Token>* info)
  {
    if (this->mMode == DataDrivenLexerMode::Tokenizer)
      this->EndRule<Character>(info, *this->mUserTokenGrammar, this->mTokenNodes);
    else
      this->EndRule<Token>(info, *this->mUserParserGrammar, this->mParserNodes);
  }

  //***************************************************************************
  void DataDrivenLexer::TokenParsed(ParseNodeInfo<Token>* info)
  {
    GrammarRule<Character>* rule = info->mToken.mRule;
    DataDrivenLexerShared& shared = DataDrivenLexerShared::GetInstance();

    if (rule == shared.mTokenizer)
      this->mMode = DataDrivenLexerMode::Tokenizer;
    else if (rule == shared.mParser)
      this->mMode = DataDrivenLexerMode::Parser;
  }

  //***************************************************************************
  void DataDrivenLexer::StartParsing()
  {
  }

  //***************************************************************************
  void DataDrivenLexer::EndParsing()
  {
  }

  //***************************************************************************
  template <> void DataDrivenLexer::AddStringLiteralNode<Character>(StringParam string, Array<GrammarNode<Character>*>& nodes)
  {
    String literal = ReplaceStringEscapesAndStripQuotes(string);
    GrammarNode<Character>& node = T(literal);
    nodes.PushBack(&node);
  }

  //***************************************************************************
  template <> void DataDrivenLexer::AddStringLiteralNode<Token>(StringParam string, Array<GrammarNode<Token>*>& nodes)
  {
    Error("Cannot add ranges/sets in a parser (you can only use token literals)");
    nodes.PushBack(&P());
  }

  //***************************************************************************
  template <> void DataDrivenLexer::AddTokenLiteralNode<Character>(StringParam string, Array<GrammarNode<Character>*>& nodes)
  {
    Error("Cannot add token literals in a tokenizer (the tokens must come from the tokenizer, and be used in the parser)");
    nodes.PushBack(&T());
  }

  //***************************************************************************
  template <> void DataDrivenLexer::AddTokenLiteralNode<Token>(StringParam string, Array<GrammarNode<Token>*>& nodes)
  {
    StringRange tokenLiteral = string;
    tokenLiteral.PopFront();
    tokenLiteral.PopBack();
    tokenLiteral.PopBack();
    GrammarRule<Character>& rule = (*this->mUserTokenGrammar)[tokenLiteral];
    nodes.PushBack(&P(rule));
  }
}
