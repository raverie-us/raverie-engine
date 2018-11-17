////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2016, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

DeclareEnum18(TokenState,
              Start,
              Identifier, EnumerationStart, Enumeration,
              StringLiteralStart, StringLiteralEscape, StringLiteral,
              Integer, ZeroInteger, NegativeStart,
              FloatStart, Float, ExplicitFloat,
              ScientificNotationStart, ScientificNotationSign, ScientificNotationFloat,
              HexStart, Hex);

//**************************************************************************************************
inline DataTokenType::Enum GetTokenFromState(TokenState::Enum currentState)
{
  switch(currentState)
  {
  case TokenState::Identifier:
    return DataTokenType::Identifier;
  case TokenState::Enumeration:
    return DataTokenType::Enumeration;
  case TokenState::StringLiteral:
    return DataTokenType::StringLiteral;
  case TokenState::Integer:
  case TokenState::ZeroInteger:
    return DataTokenType::Integer;
  case TokenState::Hex:
    return DataTokenType::Hex;
  case TokenState::Float:
  case TokenState::ExplicitFloat:
  case TokenState::ScientificNotationFloat:
    return DataTokenType::Float;
  }

  return DataTokenType::None;
}

//------------------------------------------------------------------------------ Data Tree Tokenizer
//**************************************************************************************************
DataTreeTokenizer::DataTreeTokenizer(StringParam text) : 
  mLineNumber(0)
{
  mText = text;
  mRange = mText.All();
}

//**************************************************************************************************
bool DataTreeTokenizer::ReadToken(DataToken& token, Status& status)
{
  EatWhitespace();

  // Reset token data
  token.mType = DataTokenType::None;
  token.mText = String();
  token.mLineNumber = mLineNumber;

  // Store where we started so we can get the full text of the token
  StringIterator tokenStart = mRange.Begin();

  // Start in the starting state
  TokenState::Enum currentState = TokenState::Start;

  while(!mRange.Empty())
  {
    Rune rune = mRange.Front();
    bool tokenAccepted = false;

    switch(currentState)
    {
      //---------------------------------------------------------------------------- Start
    case TokenState::Start:
    {
      if(IsAlpha(rune) || rune == '_')
        currentState = TokenState::Identifier;
      else if(rune == '\"')
        currentState = TokenState::StringLiteralStart;
      else if(rune == '-')
        currentState = TokenState::NegativeStart;
      else if(rune == '0')
        currentState = TokenState::ZeroInteger;
      else if(IsNumber(rune))
        currentState = TokenState::Integer;
      else if(IsSymbol(rune))
      {
        if(rune == '=')
          token.mType = DataTokenType::Assignment;
        else if(rune == '{')
          token.mType = DataTokenType::OpenCurley;
        else if(rune == '}')
          token.mType = DataTokenType::CloseCurley;
        else if(rune == '[')
          token.mType = DataTokenType::OpenBracket;
        else if(rune == ']')
          token.mType = DataTokenType::CloseBracket;
        else if(rune == ',')
          token.mType = DataTokenType::Comma;
        else if(rune == ':')
          token.mType = DataTokenType::Colon;
      
        mRange.PopFront();
        return (token.mType != DataTokenType::None);
      }
      else
      {
        status.SetFailed(String::Format("Invalid starting character"));
        return false;
      }
      break;
    }
    //------------------------------------------------------------- String Literal Start
    case TokenState::StringLiteralStart:
    {
      if(rune == '\\')
        currentState = TokenState::StringLiteralEscape;
      else if(rune == '\"')
        currentState = TokenState::StringLiteral;

      // Otherwise, we accept any character so we don't need to do anything
      break;
    }
    //------------------------------------------------------------ String Literal Escape
    case TokenState::StringLiteralEscape:
    {
      currentState = TokenState::StringLiteralStart;

      // Otherwise, we accept any character so we don't need to do anything
      break;
    }
    //------------------------------------------------------------------- String Literal
    case TokenState::StringLiteral:
    {
      tokenAccepted = true;

      // Otherwise, we accept any character so we don't need to do anything
      break;
    }
    //----------------------------------------------------------------------- Identifier
    case TokenState::Identifier:
    {
      // Transition to enum if it's a period
      if(rune == '.')
        currentState = TokenState::EnumerationStart;
      // Only accept alpha and underscores
      else if(!IsAlpha(rune) && rune != '_' && !IsNumber(rune))
        tokenAccepted = true;

      break;
    }
    //---------------------------------------------------------------- Enumeration Start
    case TokenState::EnumerationStart:
    {
      if(IsAlpha(rune) || rune == '_')
        currentState = TokenState::Enumeration;
      else
        status.SetFailed("Expected literal after enumeration start");

      break;
    }
    //---------------------------------------------------------------------- Enumeration
    case TokenState::Enumeration:
    {
      // Only accept alpha and underscores
      if(!IsAlpha(rune) && rune != '_' && !IsNumber(rune))
        tokenAccepted = true;
      break;
    }
    //-------------------------------------------------------------------------- Integer
    case TokenState::Integer:
    {
      if(IsNumber(rune))
        break;
      else if(rune == '.')
        currentState = TokenState::FloatStart;
      else if(rune == 'f')
        currentState = TokenState::ExplicitFloat;
      else if(rune == 'e')
        currentState = TokenState::ScientificNotationStart;
      else
        tokenAccepted = true;
      break;
    }
    //------------------------------------------------------------------- Negative Start
    case TokenState::NegativeStart:
    {
      if(rune == '0')
        currentState = TokenState::ZeroInteger;
      else if(IsNumber(rune))
        currentState = TokenState::Integer;
      else
        status.SetFailed("Negative symbol must be followed by a number");
      break;
    }
    //--------------------------------------------------------------------- Zero Integer
    case TokenState::ZeroInteger:
    {
      if(IsNumber(rune))
        currentState = TokenState::Integer;
      else if(rune == 'x' || rune == 'X')
        currentState = TokenState::HexStart;
      else if(rune == '.')
        currentState = TokenState::FloatStart;
      else
        tokenAccepted = true;
      break;
    }
    //------------------------------------------------------------------------ Hex Start
    case TokenState::HexStart:
    {
      if(IsHex(rune))
        currentState = TokenState::Hex;
      else
        status.SetFailed("Incomplete hex value");
      break;
    }
    //------------------------------------------------------------------------------ Hex
    case TokenState::Hex:
    {
      if(IsHex(rune))
        currentState = TokenState::Hex;
      else
        tokenAccepted = true;
      break;
    }
    //---------------------------------------------------------------------- Float Start
    case TokenState::FloatStart:
    {
      if(IsNumber(rune))
        currentState = TokenState::Float;
      else
        status.SetFailed("Incomplete float. A number must follow the '.'");
      break;
    }
    //---------------------------------------------------------------------------- Float
    case TokenState::Float:
    {
      if(rune == 'e')
        currentState = TokenState::ScientificNotationStart;
      else if(!IsNumber(rune))
        tokenAccepted = true;
      break;
    }
    //------------------------------------------------------------------- Explicit Float
    case TokenState::ExplicitFloat:
    {
      tokenAccepted = true;
      break;
    }
    //--------------------------------------------------------- Scientific Notation Start
    case TokenState::ScientificNotationStart:
    {
      if(rune == '+' || rune == '-')
        currentState = TokenState::ScientificNotationSign;
      else if(IsNumber(rune))
        currentState = TokenState::ScientificNotationFloat;
      else
        status.SetFailed("Incomplete scientific notation. A number or +- should follow 'e'");
      break;
    }
    //--------------------------------------------------------- Scientific Notation Sign
    case TokenState::ScientificNotationSign:
    {
      if(IsNumber(rune))
        currentState = TokenState::ScientificNotationFloat;
      else
        status.SetFailed("Incomplete scientific notation. A number should follow the + or -");
      break;
    }
    //-------------------------------------------------------- Scientific Notation Float
    case TokenState::ScientificNotationFloat:
    {
      if(rune == 'f')
        currentState = TokenState::ExplicitFloat;
      else if(!IsNumber(rune))
        tokenAccepted = true;
      break;
    }
    }

    // Return if parsing failed
    if(status.Failed())
    {
      Error(BuildString("Tokenizer error: ", status.Message).c_str());
      return false;
    }

    if(tokenAccepted)
      break;
    else
      mRange.PopFront();
  }

  // Check to see if the state is an accepting state
  token.mType = GetTokenFromState(currentState);

  // If we found a valid token, assign the text and return success
  // Strip the quotes for string literals
  if(token.mType == DataTokenType::StringLiteral)
    token.mText = mText.SubString(tokenStart + 1, mRange.Begin() - 1);
  else if(token.mType != DataTokenType::None)
    token.mText = mText.SubString(tokenStart, mRange.Begin());

  // Lookup keywords
  if(token.mType == DataTokenType::Identifier)
  {
    if(token.mText == "var")
      token.mType = DataTokenType::Var;
    else if(token.mText == "true")
      token.mType = DataTokenType::True;
    else if(token.mText == "false")
      token.mType = DataTokenType::False;
  }

  return (token.mType != DataTokenType::None);
}

//**************************************************************************************************
void DataTreeTokenizer::EatWhitespace()
{
  bool lastWasCarriageReturn = false;

  while(!mRange.Empty())
  {
    Rune rune = mRange.Front();

    // Increase line number if it's a newline
    if(rune == '\r')
    {
      ++mLineNumber;
    }
    else if(rune == '\n')
    {
      if(!lastWasCarriageReturn)
        ++mLineNumber;
    }
    else if(!IsSpace(rune))
    {
      break;
    }

    lastWasCarriageReturn = (rune == '\r');

    mRange.PopFront();
  }
}

}//namespace Zero
