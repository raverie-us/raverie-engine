////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2016, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

DeclareEnum17(DataTokenType, None, Enumeration, Identifier, Float, Integer, Hex,
                             StringLiteral, OpenBracket, CloseBracket,
                             OpenCurley, CloseCurley, Assignment, Comma, Colon, True, False, Var);

//--------------------------------------------------------------------------------------- Data Token
struct DataToken
{
  DataToken() : mType(DataTokenType::None) {}
  DataTokenType::Enum mType;
  StringRange mText;
  uint mLineNumber;
};

//------------------------------------------------------------------------------ Data Tree Tokenizer
class DataTreeTokenizer
{
public:
  DataTreeTokenizer(StringParam text);

  bool ReadToken(DataToken& token, Status& status);

private:
  void EatWhitespace();
  uint mLineNumber;
  String mText;
  StringRange mRange;
};

}//namespace Zero
