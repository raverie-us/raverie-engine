////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2016, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class DataNode;
struct DataTreeContext;

//------------------------------------------------------------ Data Tree Grammar
class DataTreeGrammar
{
public:
  static DataTreeGrammar& GetInstance();

  DataTreeGrammar();

  // Tokenizer
  GrammarSet<Character> mTokenGrammar;
  GrammarRule<Character>* mTokenStart;
  GrammarRule<Character>* mEnum;
  GrammarRule<Character>* mIdentifier;
  GrammarRule<Character>* mFloat;
  GrammarRule<Character>* mInteger;
  GrammarRule<Character>* mHex;
  GrammarRule<Character>* mWhitespace;
  GrammarRule<Character>* mStringLiteral;
  GrammarRule<Character>* mTrue;
  GrammarRule<Character>* mFalse;
  GrammarRule<Character>* mVar;
  
  // Parser
  GrammarSet<Token> mParserGrammar;
  GrammarRule<Token>* mParserStart;
  GrammarRule<Token>* mObject;
  GrammarRule<Token>* mAttribute;
  GrammarRule<Token>* mProperty;
  GrammarRule<Token>* mValue;
};

//------------------------------------------------------------- Data Tree Parser
class OldDataTreeParser
{
public:
  static DataNode* BuildTree(DataTreeContext& context, StringRange data);

  OldDataTreeParser();

  /// Parser template interface.
  void StartRule(GrammarRule<Token>* rule);
  void EndRule(ParseNodeInfo<Token>* info);
  void TokenParsed(ParseNodeInfo<Token>* info) {}
  void StartParsing() {}
  void EndParsing() {}

  /// Set all flags on nodes based on the read in attributes.
  void PostProcessAttributes(DataNode* node);

  bool mOpenedProperty;
  bool mAttributeStarted;
  Array<DataNode*> mNodeStack;
  bool mPropertyObject;
  DataTreeContext* mContext;
  DataNode* mLastEndedNode;
};

}//namespace Zero
