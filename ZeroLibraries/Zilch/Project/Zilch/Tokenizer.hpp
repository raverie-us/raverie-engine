/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_TOKENIZER_HPP
#define ZILCH_TOKENIZER_HPP

namespace Zilch
{
  // This class parses the input stream using a grammar
  class ZeroShared Tokenizer
  {
  public:

    // Constructor
    Tokenizer(CompilationErrors& errors);

    // Parse data from a null terminated memory pointer
    bool Parse(const CodeEntry& entry, Array<UserToken>& tokensOut, Array<UserToken>& commentsOut);

    // Finalizes a token stream
    void Finalize(Array<UserToken>& tokensOut);

    // Commonly used imposter tokens for generated code
    static const UserToken* GetBaseToken();
    static const UserToken* GetThisToken();
    static const UserToken* GetValueToken();
    static const UserToken* GetAccessToken();
    static const UserToken* GetAssignmentToken();

  private:

    // Initialize the internals
    void InitializeInternals();

    // Reads back a character out of the input stream
    inline char ReadCharacter();

    // Traverse through the rest of the input buffer and compare it to the given string
    bool DiffString(const char* string);

    // Attempts to read a keyword or a symbol (any non-varying token)
    inline bool ReadKeywordOrSymbol(UserToken* outToken, size_t& lastAcceptedPos, char& character, TokenCategory::Enum& tokenType);

    // Attempt to read an identifier
    bool ReadIdentifier(UserToken* outToken, bool startedFromKeyword, size_t& lastAcceptedPos, char& character);

    // Attempt to read a number (both real or integer)
    bool ReadNumber(UserToken* outToken, size_t& lastAcceptedPos, char& character);

    // Attempt to read a string
    bool ReadString(UserToken* outToken, size_t& lastAcceptedPos, char& character);

    // Attempts to read a token
    bool ReadToken(UserToken* outToken);

    // Update what line and character we're on
    void UpdateLineAndCharacterNumber(char character);

    // Skip to the end of the line via modifying the position
    String SkipToEndOfLine();

    // Parse the data
    bool ParseInternal(Array<UserToken>& tokensOut, Array<UserToken>& commentsOut);

  public:

    // When set, we will parse the special '`' characters in strings to mean string interpolation (default true)
    bool EnableStringInterpolation;

  private:

    // The token that means 'end of file'
    UserToken Eof;

    // Store a reference to the error handler
    CompilationErrors& Errors;

    // The script string that needs to be tokenized
    String Data;

    // If the last character we read was a carriage return
    // This is to support the CRLF style newlines (which only counts as one line, not two)
    bool WasCarriageReturn;

    // The position in the data stream
    size_t Position;

    // The most forward position we've reached
    size_t ForwardPosition;

    // The location that we're at in the tokenizer (only updated when we read full tokens)
    CodeLocation Location;

    // The location that we're at in the tokenizer that is updated with every character read
    size_t Character;
    size_t Line;

    // The depth of comment we're in (how many nested block comments inside of block comments)
    size_t CommentDepth;

    // Not copyable
    ZilchNoCopy(Tokenizer);
  };

  // Character utilities that we use for tokenizing
  class ZeroShared CharacterUtilities
  {
  public:
    // Detect if a character is a white-space character
    static bool IsWhiteSpace(Rune r);

    // Detect if a character is alpha
    static bool IsAlpha(Rune r);

    // Detect if a character is numeric
    static bool IsNumeric(Rune r);

    // Detect if a character is alpha-numeric
    static bool IsAlphaNumeric(Rune r);

    // Detect if a character is uppercase
    static bool IsUpper(Rune r);

    // Is this a valid escape character in a string literal
    static bool IsStringEscapee(Rune r);
  };
}

#endif