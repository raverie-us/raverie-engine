/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_TOKEN_HPP
#define ZILCH_TOKEN_HPP

namespace Zilch
{
  // This struct is given back to the user when asking for tokens
  class ZeroShared UserToken
  {
  public:
    // Default constructor
    UserToken();
    
    // Construct a token for a keyword or symbol (NOT for variable length things such as identifiers)
    UserToken(Grammar::Enum tokenId, CodeLocation* location = nullptr);

    // Constructor for a special type of token
    UserToken(StringParam token, Grammar::Enum tokenId, CodeLocation* location = nullptr);

    // The location is optional (if null is given, this will do nothing)
    void SetLocationAndStartLength(CodeLocation* location);

    // Make it easier to get the c-string for the token
    cstr c_str() const;

    String Token;
    Grammar::Enum TokenId;
    CodeLocation Location;
    size_t Start;
    size_t Length;
  };

  // A classifcation of tokens (not the specific token, but rather a category)
  namespace TokenCategory
  {
    enum Enum
    {
      Keyword,
      Symbol,
      Unknown,
    };
  }
}

// Explicit specializations
namespace Zero
{
  // UserToken should be directly memory movable (so should CodeLocation)
  // String would technically just increment a reference and then decrement, so skip it!
  template <>
  struct ZeroShared MoveWithoutDestructionOperator<Zilch::UserToken>
  {
    static inline void MoveWithoutDestruction(Zilch::UserToken* dest, Zilch::UserToken* source)
    {
      memcpy(dest, source, sizeof(*source));
    }
  };
}

#endif