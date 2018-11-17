////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2016, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "OldDataTreeParser.hpp"
#include "DataTreeNode.hpp"
#include "DataTree.hpp"

namespace Zero
{

//------------------------------------------------------------ Data Tree Grammar
//******************************************************************************
DataTreeGrammar& DataTreeGrammar::GetInstance()
{
  static DataTreeGrammar sInstance;
  return sInstance;
}

//******************************************************************************
DataTreeGrammar::DataTreeGrammar()
{
  // Tokenizer
  GrammarRule<Character>& TokenStart    = mTokenGrammar["Start"];
  GrammarRule<Character>& Enum          = mTokenGrammar["Enum"];
  GrammarRule<Character>& Identifier    = mTokenGrammar["Identifier"];
  GrammarRule<Character>& Float         = mTokenGrammar["Float"];
  GrammarRule<Character>& Integer       = mTokenGrammar["Integer"];
  GrammarRule<Character>& Hex           = mTokenGrammar["Hex"];
  GrammarRule<Character>& Whitespace    = mTokenGrammar["Whitespace"];
  GrammarRule<Character>& StringLiteral = mTokenGrammar["StringLiteral"];
  GrammarRule<Character>& OpenBracket   = mTokenGrammar["OpenBracket"];
  GrammarRule<Character>& CloseBracket  = mTokenGrammar["CloseBracket"];
  GrammarRule<Character>& OpenCurley    = mTokenGrammar["OpenCurley"];
  GrammarRule<Character>& CloseCurley   = mTokenGrammar["CloseCurley"];
  GrammarRule<Character>& Assignment    = mTokenGrammar["Assignment"];
  GrammarRule<Character>& Comma         = mTokenGrammar["Comma"];
  GrammarRule<Character>& Colon         = mTokenGrammar["Colon"];
  GrammarRule<Character>& True          = mTokenGrammar["True"];
  GrammarRule<Character>& False         = mTokenGrammar["False"];
  GrammarRule<Character>& Var           = mTokenGrammar["Var"];
  
  TokenStart    |=  Enum | Float | Whitespace | StringLiteral | OpenBracket | CloseBracket |
                    OpenCurley | CloseCurley | Assignment | Comma | Colon;
  Enum          |= Identifier << T(".") << T("a-zA-Z_") << *T("a-zA-Z_0-9");
  Identifier    |=  T("a-zA-Z_") << *T("a-zA-Z_0-9");
  Float         |= ~Integer << (T(".") << +T("0-9") << ~(T("e") << ~T("-+") << +T("0-9")) | T("e") << ~T("-+") << +T("0-9")) << ~T("f");
  Integer       |= ~T("-") << ((T("0") << (Hex | *T("0-9"))) | T("1-9") << *T("0-9"));
  Hex           |=  T("xX") << +T("0-9a-fA-F") << T();
  Whitespace    |= +T(" \t\r\n\v\f");
  StringLiteral |=  T("\"") << *(T("^\"\\") | T("\\") << T("^")) << T("\"");
  OpenBracket   |=  T("[");
  CloseBracket  |=  T("]");
  OpenCurley    |=  T("{");
  CloseCurley   |=  T("}");
  Assignment    |=  T("=");
  Comma         |=  T(",");
  Colon         |=  T(":");

  mTokenGrammar.AddIgnore(Whitespace);
  mTokenGrammar.AddKeyword("true", True);
  mTokenGrammar.AddKeyword("false", False);
  mTokenGrammar.AddKeyword("var", Var);
  
  // Parser
  GrammarRule<Token>& ParserStart = mParserGrammar["Root"];
  GrammarRule<Token>& Object      = mParserGrammar["Object"];
  GrammarRule<Token>& Attribute   = mParserGrammar["Attribute"];
  GrammarRule<Token>& Property    = mParserGrammar["Property"];
  GrammarRule<Token>& Value       = mParserGrammar["Value"];
  
  // We're allowing any amount of file attributes at the start
  ParserStart |= *Attribute << +Object;
  Object      |= P("Identifier", P(Identifier)) << *Attribute << P(OpenCurley) << *((Property | Value | Object) << ~P(Comma)) << P(CloseCurley);
  Attribute   |= P(OpenBracket) << P("Name", P(Identifier)) << ~(P(Colon) << (P("Value", Value) | Object)) << P(CloseBracket);
  Property    |= P(Var) << P("Name", P(Identifier)) << P(Assignment) << (P("Value", Value) | Object);
  Value       |= P("Value", P(Integer) | P(Float) | P(Hex) | P(StringLiteral) | P(Enum) | P(True) | P(False));
  
  // Store tokens so they can be used later
  mTokenStart    = &TokenStart;
  mEnum          = &Enum;
  mIdentifier    = &Identifier;
  mFloat         = &Float;
  mInteger       = &Integer;
  mHex           = &Hex;
  mWhitespace    = &Whitespace;
  mStringLiteral = &StringLiteral;
  mTrue          = &True;
  mFalse         = &False;
  mVar           = &Var;
  
  // Same for rules
  mParserStart = &ParserStart;
  mObject      = &Object;
  mAttribute   = &Attribute;
  mProperty    = &Property;
  mValue       = &Value;
}

//------------------------------------------------------------- Data Tree Parser
//******************************************************************************
DataNode* OldDataTreeParser::BuildTree(DataTreeContext& context, StringRange data)
{
  DataTreeGrammar& grammar = DataTreeGrammar::GetInstance();

  // Create a token range from the data
  TokenStream<> tokenStream;
  tokenStream.mRange = TokenRange<>(grammar.mTokenGrammar, *grammar.mTokenStart, data);

  // This will build the data tree for us
  OldDataTreeParser dataTreeParser;
  dataTreeParser.mContext = &context;

  // Parse the token stream
  RecursiveDescentParser<Token, TokenStream<>, OldDataTreeParser> parser;
  parser.mParseHandler = &dataTreeParser;
  parser.mStartRule = grammar.mParserStart;
  parser.mStream = &tokenStream;
  parser.Parse();

  // Return the root node
  DataNode* root = dataTreeParser.mLastEndedNode;
  dataTreeParser.PostProcessAttributes(root);
  return root;
}

//***************************************************************************
//StringRange MyStripStringQuotes(StringRange input)
//{
//  // Move the beginning inward and the end backwards by 1 to strip the quotes
//  ++input.mBegin;
//  --input.mEnd;
//
//  return input;
//}

//------------------------------------------------------------- Data Tree Parser
OldDataTreeParser::OldDataTreeParser() :
  mAttributeStarted(false)
{

}

//******************************************************************************
void OldDataTreeParser::StartRule(GrammarRule<Token>* rule)
{
  String ruleName = rule->mName;

  DataNode* parent = nullptr;
  if(!mNodeStack.Empty())
    parent = mNodeStack.Back();
  
  if(ruleName == "Attribute")
  {
    mAttributeStarted = true;
  }
  else if(ruleName == "Object")
  {
    // For properties, we had two choices for building the tree:
    // 1. Make a 'Property' node with the property name, then the child node would be 
    //    either a 'Value' or 'Object' node with the type name / value text.
    // 2. Only have the 'Value' or 'Object' node with the property name and type name / value text.
    //
    // The old parser built the tree as described in option 2. For now, we're going to continue
    // with option 2 as it doesn't require any other changes.

    // If our parent is a property, we're going to add our data to it instead of making
    // a new node (this will reflect option 2 of building the tree)
    if(parent && parent->mNodeType == DataNodeType::Value)
    {
      // It's now an Object node
      parent->mNodeType = DataNodeType::Object;
    }
    else
    {
      DataNode* objectNode = new DataNode(DataNodeType::Object, parent);
      mNodeStack.PushBack(objectNode);
    }
  }
  else if(ruleName == "Property")
  {
    // We're creating it as a value node, but the node type can be changed to an
    // object node depending on what follows
    DataNode* valueNode = new DataNode(DataNodeType::Value, parent);
    valueNode->mFlags.SetFlag(DataNodeFlags::Property);
    mNodeStack.PushBack(valueNode);
  }
}

//******************************************************************************
void AssignValue(DataNode* node, ParseNodeInfo<Token>* info)
{
  DataTreeGrammar& grammar = DataTreeGrammar::GetInstance();

  const String cValueText = "Value";
  Token valueToken = info->GetFirstCapturedToken(cValueText);

  // Default behavior
  node->mTextValue = valueToken.mString;

  // Integer
  if(valueToken.mRule == grammar.mInteger)
  {
    node->mTypeName = Serialization::Trait<int>::TypeName();
  }
  // Hex
  else if(valueToken.mRule == grammar.mHex)
  {
    node->mTypeName = Serialization::Trait<u64>::TypeName();
  }
  // Float
  else if(valueToken.mRule == grammar.mFloat)
  {
    node->mTypeName = Serialization::Trait<float>::TypeName();
  }
  // Boolean
  else if(valueToken.mRule == grammar.mTrue || valueToken.mRule == grammar.mFalse)
  {
    node->mTypeName = Serialization::Trait<bool>::TypeName();
  }
  else if(valueToken.mRule == grammar.mStringLiteral)
  {
    node->mTypeName = ZilchTypeId(String)->Name;
    StringRange processedValue = valueToken.mString.All();

    // 1 and -2 to strip the quotes
    node->mTextValue = processedValue.SubStringFromByteIndices(1, processedValue.SizeInBytes() - 1);
  }
  // Enum
  else if(valueToken.mRule == grammar.mEnum)
  {
    // The enum comes in as 'Type.Value' (ie. 'LightType.PointLight'), so we need
    // to separate the type name from the value
    StringTokenRange r(valueToken.mString, '.');
    node->mTypeName = r.Front();
    r.PopFront();
    node->mTextValue = r.Front();
    node->mFlags.SetFlag(DataNodeFlags::Enumeration);
  }
}

//******************************************************************************
void ReadObjectAttributes(Token& nameToken, Token& valueToken, DataNode* objectNode)
{
  objectNode->AddAttribute(nameToken.mString, valueToken.mString);
}

//******************************************************************************
void OldDataTreeParser::EndRule(ParseNodeInfo<Token>* info)
{
  DataTreeGrammar& grammar = DataTreeGrammar::GetInstance();

  GrammarRule<Token>* rule = info->mRule;

  String ruleName = info->mRule->mName;
  DataNode* currentNode = nullptr;
  if(!mNodeStack.Empty())
    currentNode = mNodeStack.Back();

  // Object Node
  if(rule == grammar.mObject)
  {
    DataNode* objectNode = mNodeStack.Back();

    // We combined the Property and Object rules into one node. The Property
    // rule still needs to add information to this node, and because it's
    // going to be the next rule to end, we don't want to pop the node off
    // of the stack
    if(!objectNode->mFlags.IsSet(DataNodeFlags::Property))
      mNodeStack.PopBack();

    // Grab the identifier
    objectNode->mTypeName = info->GetFirstCapturedToken("Identifier").mString;

    mLastEndedNode = objectNode;
  }
  // Value node
  else if(rule == grammar.mValue && !info->mFailed)
  {
    // We captured the value text in the attribute rule, so we don't need to do
    // anything here
    if(mAttributeStarted)
      return;

    // If our parent is a property, we can just add our text value
    if(currentNode->mNodeType == DataNodeType::Value)
    {
      AssignValue(currentNode, info);
    }
    // If our parent is an object, we're likely in an array or vector type (Vec3, etc..)
    // In this case, we need to create a new node for this value
    else
    {
      DataNode* dataValue = new DataNode(DataNodeType::Value, currentNode);
      AssignValue(dataValue, info);

      // We now know that our parent is an array
      currentNode->mFlags.SetFlag(DataNodeFlags::Array);
    }
  }
  // Property node
  else if(rule == grammar.mProperty)
  {
    DataNode* objectNode = mNodeStack.Back();
    mNodeStack.PopBack();

    objectNode->mPropertyName = info->GetFirstCapturedToken("Name").mString;
  }
  // Attribute node
  else if(rule == grammar.mAttribute)
  {
    mAttributeStarted = false;

    if(info->mFailed)
      return;

    Token nameToken = info->GetFirstCapturedToken("Name");
    Token valueToken = info->GetFirstCapturedToken("Value");

    // Add the attribute to the most recent opened node
    if(currentNode)
    {
      ReadObjectAttributes(nameToken, valueToken, currentNode);
    }
    // If no nodes have been created yet, add the attribute to the root
    else
    {
      DataAttribute attribute(nameToken.mString, valueToken.mString);
      mContext->Loader->mRootAttributes.PushBack(attribute);
    }
  }
}

//******************************************************************************
void OldDataTreeParser::PostProcessAttributes(DataNode* node)
{
  // Walk each attribute and update relevant flags
  forRange(DataAttribute& attribute, node->mAttributes.All())
  {
    if(attribute.mName == SerializationAttributes::Id)
    {
      ToValue(attribute.mValue, node->mUniqueNodeId);
    }
    else if(attribute.mName == SerializationAttributes::InheritId)
    {
      node->mInheritedFromId = attribute.mValue;
      mContext->PatchRequired = true;
    }
    else if(attribute.mName == SerializationAttributes::LocallyAdded)
    {
      node->mFlags.SetFlag(DataNodeFlags::LocallyAdded);
      mContext->PatchRequired = true;
    }
    else if(attribute.mName == SerializationAttributes::LocallyRemoved)
    {
      node->mPatchState = PatchState::ShouldRemove;
      mContext->PatchRequired = true;
    }
    else if(attribute.mName == SerializationAttributes::ChildOrderOverride)
    {
      node->mFlags.SetFlag(DataNodeFlags::ChildOrderOverride);
      mContext->PatchRequired = true;
    }
  }

  // Update children
  forRange(DataNode& child, node->mChildren.All())
    PostProcessAttributes(&child);
}

}//namespace Zero
