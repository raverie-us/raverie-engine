///////////////////////////////////////////////////////////////////////////////
///
/// \file Tokenizer.hpp
/// Declaration of the Text tokenizer.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

struct TempToken
{
  enum TokenType
  {
    None = 0,
    Word,
    Number,
    String,
    Symbol,
    Comment
  };

  TokenType Type;
  StringRange Text;

  void Assign(TokenType type, const StringRange& range)
  {
    Type = type;
    Text = range;
  }

  bool operator==(cstr test) {return Text == test;}
  bool operator==(char test) {return Text == test;}
};

//A text parser that tokenizes a string buffer.
class Tokenizer
{
public:
  Tokenizer();

  void Load(StringRangeParam text);

  //Tokens
  bool ReadToken(TempToken& token);
  /// Move the parser's position back to the token start.
  bool PutBack(TempToken& token);
  Rune NextCharacter();
  //Processing

  //Skipping
  bool AtEnd();
  void SkipWhiteSpace();
  void SkipUntilOfEndWord();
  bool SkipPastString(StringRangeParam str);
  bool SkipUntilString(StringRangeParam str);

  bool ReadWord(TempToken& token);
  bool ReadNumber(TempToken& token);
  bool ReadStringConstant(TempToken& token);
  bool ReadSymbol(TempToken& token);
  bool ReadLetter(TempToken& token);

  /// Calculate the length of the entire current line
  size_t CaculateLine(TempToken& token);
  /// Calculate from the current position to the end of line
  size_t CurrentLine();

  void Validate();
  StringRange GetText() {return mPosition;}
  StringRange ReadUntil(char value);

  // Allow hex reading
  bool AllowHex;
  // Use for parsing old data files
  bool LegacyWordParse;

private:
  StringRange mPosition;
  StringRange mAll;
};

}//namespace Zero
