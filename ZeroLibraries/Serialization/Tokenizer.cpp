///////////////////////////////////////////////////////////////////////////////
///
/// \file Tokenizer.cpp
/// Implementation of the Text Tokenizer.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

bool IsWord(Rune r)
{
  return IsAlpha(r) || IsDigit(r) || r.value == '_' || r.value == '.';
}

bool IsWordLegacy(Rune r)
{
  return IsAlpha(r) || IsDigit(r) || r.value == '_' || r.value == '.' || r.value == '[' || r.value == ']';
}

bool IsNumeric(Rune r)
{
  return IsDigit(r) || r.value == '.';
}

bool IsNumericHex(Rune r)
{
  return IsDigit(r) || r.value == '.' || r.value == '-' || r.value == '+' || (r.value >= 'a' && r.value <= 'f') || (r.value >= 'A' && r.value <= 'F');
}

bool IsNumericStart(Rune r)
{
  return IsDigit(r) || r.value == '-' || r.value == '.';
}

bool IsNewLine(Rune r)
{
  return r.value == '\n';
}

bool NotNewLine(Rune r)
{
  return !IsNewLine(r);
}

bool NotQuote(Rune r)
{
  return r.value != '"';
}

bool NotWhitespace(Rune r)
{
  return !IsSpace(r);
}

template<typename rangeType, typename Pred> 
rangeType while_is(rangeType range, Pred pred)
{
  while(!range.Empty() && pred(range.Front()))
    range.PopFront();
  return range;
}

template<typename rangeType, typename Pred> 
size_t Count(rangeType range, Pred pred)
{
  size_t n = 0;
  while(!range.Empty())
  {
    if(pred(range.Front()))
      ++n;
    range.PopFront();
  }
  return n;
}

StringRange MergeFront(StringRangeParam a, StringRangeParam b)
{
  return StringRange(a.Begin(), b.Begin());
}

void ExtractToken(TempToken& token, TempToken::TokenType type, StringRange& range)
{
  token.Text = range;
  token.Type = type;
}

/// Sets the token to an end state (no token to process)
bool EndToken(StringRange& cur, TempToken& token)
{
  token.Type = TempToken::None;
  token.Text = cur;
  return false;
}

Tokenizer::Tokenizer()
{
  AllowHex = true;
  LegacyWordParse = true;
}

void Tokenizer::Load(StringRangeParam text)
{
  mAll = text;
  mPosition = text;
}

bool Tokenizer::ReadToken(TempToken& token)
{
  Validate();

  //skip any leading whitespace
  SkipWhiteSpace();

  if(mPosition.Empty())
    return EndToken(mPosition, token);

  Rune curRune = mPosition.Front();

  //Need to loop because there might be multiple single line comments
  while(curRune == '/')
  {
    StringRange& next = mPosition;
    if(!next.Empty())
    {
      //make sure that this is a double slash comment
      next.PopFront();
      if(next.Front() == '/')
      {
        //eat the entire comment (newlines are considered whitespace)
        mPosition = while_is(mPosition, NotNewLine);
        SkipWhiteSpace();
        if(mPosition.Empty())
          return EndToken(mPosition, token);
        curRune = mPosition.Front();
      }
      else
      {
        return ReadSymbol(token);
      }
    }  
  }

  //determine what kind of token we are at
  if(IsAlpha(curRune))
    return ReadWord(token);

  if(IsNumericStart(curRune))
    return ReadNumber(token);

  if (curRune == '"')
    return ReadStringConstant(token);

  //if nothing above, just default to a symbol
  return ReadSymbol(token);
}

bool Tokenizer::PutBack(TempToken& token)
{
  //WARNING THIS MAY BE BROKEN NOW DO TO CHANGES TO STRING RANGE - Dane
  mPosition.mBegin = token.Text.Data();
  return true;
}

Rune Tokenizer::NextCharacter()
{
  StringRange text = while_is(mPosition, IsSpace);
  if(!text.Empty())
    return text.Front();
  else
    return StringRange::InvalidRune;
}

bool Tokenizer::AtEnd()
{
  return mPosition.Empty();
}

void Tokenizer::SkipWhiteSpace()
{
  mPosition = while_is(mPosition, IsSpace);
}

void Tokenizer::SkipUntilOfEndWord()
{
  if(LegacyWordParse)
    mPosition = while_is(mPosition, IsWordLegacy);
  else
    mPosition = while_is(mPosition, IsWord);
}

bool Tokenizer::SkipPastString(StringRangeParam str)
{
  //if the string exists, skip past it
  if(SkipUntilString(str))
  {
    //could be dangerous, no checks done here
    mPosition.mBegin += str.SizeInBytes();
    return true;
  }
  else
  {
    return false;
  }
}

bool Tokenizer::SkipUntilString(StringRangeParam str)
{
  StringRange pos = Search(mPosition, str);
  mPosition = pos;
  return !pos.Empty();
}

bool Tokenizer::ReadWord(TempToken& token)
{
  //read until the end of the word
  StringRange& text = mPosition;
  StringRange endOfWord = LegacyWordParse ? while_is(text, IsWordLegacy) : while_is(text, IsWord);
  //make the substring of the word and set it in the token
  token.Assign(TempToken::Word, MergeFront(text, endOfWord));
  mPosition = endOfWord;
  return true;
}

bool Tokenizer::ReadNumber(TempToken& token)
{
  StringRange& text = mPosition;
  //the first element of a number is always valid, but in the middle of a
  //number it might not be valid, so skip the first character to deal with
  //invalid middle characters (such as - or + when not in hex mode) 
  StringRange m = text;
  m.PopFront();

  //read the entire number, if hex is allowed then allow hex characters
  StringRange endOfWord = AllowHex ? while_is(m, IsNumericHex) : while_is(m, IsNumeric);

  token.Assign(TempToken::Number, MergeFront(text, endOfWord));
  mPosition = endOfWord;
  return true;
}

bool Tokenizer::ReadStringConstant(TempToken& token)
{
  SkipWhiteSpace();

  StringRange& text = mPosition;
  if(text == '"')
  {
    //remove the quote
    text.PopFront();

    // Create a temporary iterator starting at the beginning of the leftover text
    StringIterator it = text.Begin();

    // Loop until we hit the end of the text or until we reach an ending quote
    StringIterator textEnd = text.End();
    while (it != textEnd)
    {
      // If the current character is a quote (note: we should be after the first quote by now, so this would either be the end or an escape sequence)
      if (*it == '"')
      {
        StringIterator next = it + 1;
        // If there is another quote directly in front of us (and we're not heading off the end of the string)
        if (next != textEnd && *next == '"')
        {
          // Move all data from here backwards by one (basically, make the second quote go away)
          for (char* j = (char*)next.Data(); j != text.mEnd; ++j)
          {
            // Copy the data one to the left (we need to cast since it's a const char*)
            // Technically, we shouldn't be modifying this without knowing what it is!!!
            // Despite switching to UTF8 we know a quote is 1 byte so this is "Safe"
            *(j - 1) = *j;
          }

        }
        else
        {
          // Otherwise, it's not a double quote (escape sequence) so it must be the end
          break;
        }
      }
      // Increment the iterator in the string to the next character
      ++it;
    }

    // Create a string range that starts from the beginning of the text to the last quote 
    StringRange tokenString(text.Begin(), it);
    token.Assign(TempToken::String, tokenString);

    // Finally, the position is now past the quoted string
    text.mBegin = it.Data();
    text.IncrementByRune();
    mPosition = text;
    return true;
  }
  else
  {
    //not a quoted string, read until a whitespace
    StringRange endOfWord = while_is(text, NotWhitespace);
    token.Assign(TempToken::String, MergeFront(text, endOfWord));
    mPosition = endOfWord;
    return true;
  }
}

bool Tokenizer::ReadSymbol(TempToken& token)
{
  StringRange text = mPosition;
  //Eat the symbol
  //right now only support mono symbols
  text.PopFront();
  token.Assign(TempToken::Symbol, MergeFront(mPosition, text));
  mPosition = text;
  return true;
}

bool Tokenizer::ReadLetter(TempToken& token)
{
  StringRange text = mPosition;
  text.PopFront();
  token.Assign(TempToken::Word, MergeFront(mPosition, text));
  mPosition = text;
  return true;
}

size_t Tokenizer::CaculateLine(TempToken& token)
{
  StringRange tillToken = StringRange(mAll.Begin(), token.Text.Begin());
  return Count(tillToken, IsNewLine) + 1;
}

size_t Tokenizer::CurrentLine()
{
  StringRange readSoFar = StringRange(mAll.Begin(), mPosition.Begin());
  return Count(readSoFar, IsNewLine) + 1;
}

void Tokenizer::Validate()
{
  ErrorIf(!mPosition.IsValid(), "Tokenizer has read past the end somehow.");
}

StringRange Tokenizer::ReadUntil(char value)
{
  SkipWhiteSpace();

  StringRange text = mPosition;
  StringRange range = mPosition;

  int depth = 0;

  while(!range.Empty())
  {
    if(range.Front() == value)
      break;
    range.PopFront();
  }

  return MergeFront(text, range);
}

}//namespace Zero
