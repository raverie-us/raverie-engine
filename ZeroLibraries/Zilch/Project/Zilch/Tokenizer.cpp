/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  UserToken::UserToken() :
    TokenId(Grammar::Invalid),
    Start(0),
    Length(0)
  {
  }
  
  //***************************************************************************
  UserToken::UserToken(Grammar::Enum tokenId, CodeLocation* location) :
    Token(Grammar::GetKeywordOrSymbol(tokenId)),
    TokenId(tokenId),
    Start(0),
    Length(0)
  {
    this->SetLocationAndStartLength(location);
  }

  //***************************************************************************
  UserToken::UserToken(StringParam token, Grammar::Enum tokenId, CodeLocation* location) :
    Token(token),
    TokenId(tokenId),
    Start(0),
    Length(0)
  {
    this->SetLocationAndStartLength(location);
  }
  
  //***************************************************************************
  void UserToken::SetLocationAndStartLength(CodeLocation* location)
  {
    // If the user provided a location, then use that to fill token locations
    if (location != nullptr)
    {
      this->Location = *location;
      this->Start = location->StartPosition;
      this->Length = location->EndPosition - location->StartPosition;
    }
  }
  
  //***************************************************************************
  cstr UserToken::c_str() const
  {
    return this->Token.c_str();
  }

  //***************************************************************************
  Tokenizer::Tokenizer(CompilationErrors& errors) :
    WasCarriageReturn(false),
    Errors(errors),
    EnableStringInterpolation(true)
  {
    ZilchErrorIfNotStarted(Tokenizer);

    // Start the position off at the beginning
    this->Position = 0;
    this->ForwardPosition = 0;

    // Start off with a comment depth of zero (we're not in a comment yet!)
    this->CommentDepth = 0;

    // Setup the 'Eof' token
    this->Eof.TokenId = Grammar::End;
    this->Eof.Start  = 0;
    this->Eof.Length = 0;
  }

  //***************************************************************************
  void Tokenizer::Finalize(Array<UserToken>& tokensOut)
  {
    UserToken eof = this->Eof;
    eof.Location = this->Location;
    tokensOut.PushBack(eof);
  }

  //***************************************************************************
  const UserToken* Tokenizer::GetBaseToken()
  {
    static UserToken token(Grammar::GetKeywordOrSymbol(Grammar::Base), Grammar::Base);
    return &token;
  }

  //***************************************************************************
  const UserToken* Tokenizer::GetThisToken()
  {
    static UserToken token(ThisKeyword, Grammar::LowerIdentifier);
    return &token;
  }

  //***************************************************************************
  const UserToken* Tokenizer::GetValueToken()
  {
    static UserToken token(ValueKeyword, Grammar::LowerIdentifier);
    return &token;
  }

  //***************************************************************************
  const UserToken* Tokenizer::GetAccessToken()
  {
    static UserToken token(Grammar::GetKeywordOrSymbol(Grammar::Access), Grammar::Access);
    return &token;
  }

  //***************************************************************************
  const UserToken* Tokenizer::GetAssignmentToken()
  {
    static UserToken token(Grammar::GetKeywordOrSymbol(Grammar::Assignment), Grammar::Assignment);
    return &token;
  }

  //***************************************************************************
  bool Tokenizer::Parse(const CodeEntry& entry, Array<UserToken>& tokensOut, Array<UserToken>& commentsOut)
  {
    // Clear out our location for proper line and character counting
    this->Location = CodeLocation();
    this->Location.Origin = entry.Origin;
    this->Location.CodeUserData = entry.CodeUserData;
    this->Location.Code = entry.Code;
    this->Location.IsNative = false;
    this->Line = 1;
    this->Character = 1;

    // Store the data pointer
    this->Data = entry.Code;

    // The average number of characters per token (including whitespace)
    const int AverageCharsPerToken = 5;

    // Reserve some memory for parsed tokens
    tokensOut.Reserve(this->Data.SizeInBytes() / AverageCharsPerToken);
    
    // Start the position off at the beginning
    this->Position = 0;
    this->ForwardPosition = 0;

    // Now attempt to parse the data
    return ParseInternal(tokensOut, commentsOut);
  }

  //***************************************************************************
  bool Tokenizer::ParseInternal(Array<UserToken>& tokensOut, Array<UserToken>& commentsOut)
  {
    // The next token that will be parsed as we move along
    UserToken nextToken;

    // Loop and read tokens until we get an error or reach the end
    while (this->ReadToken(&nextToken))
    {
      // If we got here, then we know the symbol is valid because the ReadToken returned true
      // Do something based on the symbol's type
      switch (nextToken.TokenId)
      {
        // If we get a comment start symbol, continue on into block comment testing
        case Grammar::CommentStart:
        {
          // Since we just hit a comment start, increment the comment depth
          ++this->CommentDepth;
        }
        break;

        // If we get a comment end symbol, back one comment scope out
        case Grammar::CommentEnd:
        {
          // Make sure our comment depth is greater than zero
          if (this->CommentDepth > 0)
          {
            // Since we just hit a comment end, decrement the comment depth
            --this->CommentDepth;
          }
          else
          {
            // Add a parsing error
            this->Errors.Raise(this->Location, ErrorCode::BlockCommentNotFound);
          }
        }
        break;

        // If we begin a line comment, skip to the end of the line
        case Grammar::CommentLine:
        {
          // Check and make sure we're not in a comment
          if (this->CommentDepth == 0)
          {
            // Skip to the end of the line via modifying the position
            nextToken.Token = this->SkipToEndOfLine();
            commentsOut.PushBack(nextToken);
          }
        }
        break;

        // Whitespace we do absolutely nothing (just ignore it)
        case Grammar::Whitespace:
        {
        }
        break;

        // This was not a special symbol, just report it to the user
        default:
        {
          // Check and make sure we're not in a comment
          if (this->CommentDepth == 0)
          {
            // Push the token into the list
            tokensOut.PushBack(nextToken);
          }
        }
        break;
      }
    }

    //printf("Was Error: %d\n", this->Errors.WasError);
    //printf("------ Begin Tokens ------\n");
    //for (size_t i = 0; i < tokensOut.Size(); ++i)
    //{
    //  printf("Token: %d : %d '%s'\n", i, tokensOut[i].TokenId, tokensOut[i].Token.c_str());
    //}
    //printf("------ End Tokens ------\n");

    // As long as there were no errors...
    if (this->Errors.WasError == false)
    {
      // Check and make sure we're not in a comment
      if (this->CommentDepth == 0)
      {
        // We reached the end, no problems!
        return true;
      }
      // If we were in the middle of a comment, this is an error!
      else
      {
        // Add a parsing error and return out
        this->Errors.Raise(this->Location, ErrorCode::BlockCommentNotComplete);
        return false;
      }
    }
    else
    {
      // There was an error, so we failed
      return false;
    }
  }

  //***************************************************************************
  char Tokenizer::ReadCharacter()
  {
    // If the current position is outside the data bounds...
    if (this->Position >= this->Data.SizeInBytes())
    {
      // Increment the position by a byte even though we're past the end
      // We rely on this position being pushed out to properly get the length of tokens
      ++this->Position;

      // Return the null character. Every place that calls ReadCharacter
      // should expect and handle the case of reaching the end
      return '\0';
    }
     
    // Stores the result of the read
    char result = this->Data.Data()[this->Position];

    // Increment the position by a byte
    ++this->Position;

    // If we've actually moved forward
    if (this->ForwardPosition < this->Position)
    {
      // Update the line and character number
      UpdateLineAndCharacterNumber(result);

      // Move the forward position forward
      this->ForwardPosition = this->Position;
    }

    // Return the stored temporary value
    return result;
  }

  //***************************************************************************
  bool Tokenizer::DiffString(const char* string)
  {
    // Store the local string position
    int localStringPos = 0;

    // Have we matched the string to the input stream yet?
    bool match = false;

    // Loop until we reach the end of the given string, or we hit an invalid character
    ZilchLoop
    {
      // If we hit the end of our given string (and we've matched up to this point) then it's a match!
      if (string[localStringPos] == '\0')
      {
        // Mark it as being a match and jump out
        match = true;
        break;
      }
      // Otherwise, if the characters were not the same, we must break out
      // Eof is handled because the string we compare with should never have eof in it (see the check above)
      else if (ReadCharacter() != string[localStringPos])
      {
        // Backup the position by once since we just read a character that wasn't in this string
        --this->Position;
        break;
      }

      // Increment the local string position
      ++localStringPos;
    }

    // Return whether we matched or not
    return match;
  }

  //***************************************************************************
  bool Tokenizer::ReadKeywordOrSymbol(UserToken* outToken, size_t& lastAcceptedPos, char& character, TokenCategory::Enum& tokenType)
  {
    // Was a token ever accepted?
    bool acceptedToken = false;

    // Store the type of token we're parsing (we should know immediately from the first branch)
    tokenType = TokenCategory::Unknown;

    // In the event that we've gotten here, 
    #include "TokenReader.inl"

    // If we accepted a token...
    if (acceptedToken)
    {
      // If the token type we started reading was a keyword...
      if (tokenType == TokenCategory::Keyword)
      {
        // As long as the last character read was not an alpha-numeric character...
        if (CharacterUtilities::IsAlphaNumeric(character) == false && character != '_')
        {
          // Backup the last read character
          --this->Position;

          // Return true since it must be a keyword
          return true;
        }

        // It's not at the end of a token, don't accept it... (but keep the position where it is)
        return false;
      }
      else
      {
        // Backup to the last accepted position
        this->Position = lastAcceptedPos;

        // Accept the token
        return true;
      }
    }

    // Since we got here, we must not have accepted the token
    return false;
  }

  //***************************************************************************
  bool Tokenizer::ReadIdentifier(UserToken* outToken, bool startedFromKeyword, size_t& lastAcceptedPos, char& character)
  {
    // Whether or not we've accepted the token
    bool acceptedToken = startedFromKeyword;

    // By default, assume we parsed a lower-case identifier
    // (starting from keywords will always lead to this case)
    Grammar::Enum assumedTokenType = Grammar::LowerIdentifier;

    // Loop until we hit a non-alpha numeric character
    ZilchLoop
    {
      // If we've accepted the token already
      if (acceptedToken)
      {
        // If it's not an alpha-numeric value or underscore, then we have to stop
        if (CharacterUtilities::IsAlphaNumeric(character) == false && character != '_')
          break;
      }
      // Otherwise, we haven't accepted anything yet (so we're on the first character)
      else
      {
        // If the first character is not just an alpha character, then we must stop
        // We cannot begin an identifier with an underscore
        if (CharacterUtilities::IsAlpha(character) == false)
          break;

        // If the first character is upper case, then this is an upper case identifier
        if (CharacterUtilities::IsUpper(character))
          assumedTokenType = Grammar::UpperIdentifier;
      }

      // Store away the position
      lastAcceptedPos = this->Position;

      // We read something! So we must accept the identifier
      acceptedToken = true;

      // Read the next character. We handle the eof in the next loop;
      // we will terminate since the character is not alphanumeric
      character = ReadCharacter();
    }

    // If we accepted the identifier...
    if (acceptedToken)
    {
      // Backup to the last accepted position
      this->Position = lastAcceptedPos;

      // It must be an identifier
      outToken->TokenId = assumedTokenType;
    }

    // Return the result...
    return acceptedToken;
  }

  //***************************************************************************
  bool Tokenizer::ReadNumber(UserToken* outToken, size_t& lastAcceptedPos, char& character)
  {
    // Whether or not we've accepted the token
    bool acceptedToken = false;

    // Is the number a real? This will be -1 if it's not
    size_t isRealPos = (size_t) -1;

    // If we're parsing an exponential number, then we can expect an exponent sign
    bool nextCanBeExplonentialSign = false;

    // If we hit an exponent 'e' or not (only may occur once)
    bool isScientificNotation = false;

    // If the number is double size (or precision)
    bool isDoubleSize = false;

    // While what we're reading is a digit or a decimal place
    ZilchLoop
    {
      // Clear the flag so we know that the next character cannot be a sign (store the flag for this loop)
      bool canBeExplonentialSign = nextCanBeExplonentialSign;
      nextCanBeExplonentialSign = false;

      // Note: Normally if ReadNumber was standalone, we'd need to be careful that we never
      // read something that starts with a '.', 'e', or 'd' as a token, however
      // because it only ever gets called after ReadIdentifier and ReadKeywordOrSymbol it cannot happen

      // If the character is a digit...
      if (CharacterUtilities::IsNumeric(character))
      {
        // Store away the position
        lastAcceptedPos = this->Position;

        // We read something! So we must accept the token
        acceptedToken = true;
      }
      // We only bother to read any other kind of symbol if we read a number already
      else if (acceptedToken)
      {
        // If the character is a decimal place...
        if (character == '.')
        {
          // If we haven't yet accepted a number, then it can't just start with '.'
          // (Though technically this should have been accepted as a symbol in the first place...)
          // Also break if we already determined it was a real, and then we broke out
          if (isRealPos != (size_t) -1 || acceptedToken == false)
            break;

          // Since we found a decimal, it must be a real number
          isRealPos = this->Position;
        }
        // If we hit an exponent 'e' and we're not already an exponent
        else if (character == 'e' && isScientificNotation == false)
        {
          // Since we're an exponent, the next character is allowed to be a '-' or '+'
          nextCanBeExplonentialSign = true;
        }
        // Check to see if this character is a sign...
        else if (canBeExplonentialSign && (character == '-' || character == '+'))
        {
          // We don't do anything here, including not accepting this token position
          // We need a number to follow in order to accept this!
        }
        // If we hit the 'double size' marker...
        else if (character == 'd')
        {
          // If the number ends in a 'd', it means our number is double sized (64 bit typically)
          isDoubleSize = true;

          // Accept the number and break out, since 'd' only ever occurs at the end
          lastAcceptedPos = this->Position;
          acceptedToken = true;
          break;
        }
        // If the character is unknown, we must break out
        else
        {
          break;
        }
      }
      // We attempted to read something that wasn't a number...
      else
      {
        break;
      }
      
      // Read the next character. We handle the eof in the next loop;
      // we will terminate since the character is not numeric or '.'
      character = ReadCharacter();
    }

    // If we accepted the identifier...
    if (acceptedToken)
    {
      // Backup to the last accepted position
      this->Position = lastAcceptedPos;

      // If the numeric literal is double sized...
      if (isDoubleSize)
      {
        // If we found a real instead of an integer...
        if (isRealPos < lastAcceptedPos)
          outToken->TokenId = Grammar::DoubleRealLiteral;
        else
          outToken->TokenId = Grammar::DoubleIntegerLiteral;
      }
      else
      {
        // If we found a real instead of an integer...
        if (isRealPos < lastAcceptedPos)
          outToken->TokenId = Grammar::RealLiteral;
        else
          outToken->TokenId = Grammar::IntegerLiteral;
      }
    }

    // Return the result...
    return acceptedToken;
  }

  //***************************************************************************
  bool Tokenizer::ReadString(UserToken* outToken, size_t& lastAcceptedPos, char& character)
  {
    // We only consider this string to be an interpolant end if it starts with a grave accent
    bool isInterpolantEnd = (this->EnableStringInterpolation && character == '`');

    // If the character is not a quote, then return failure immediately
    if (character != '"' && isInterpolantEnd == false)
      return false;

    // This is only used for tolerant mode when we don't parse a string because it wasn't closed
    size_t startPosition = this->Position;

    // Was an escape character hit?
    bool escaped = false;

    // Loop until we hit a non-alpha numeric character
    ZilchLoop
    {
      // Read the next character. We handle the eof specifically below
      character = ReadCharacter();

      // If the character is an escape character...
      if (character == '\\')
      {
        // Toggle the escaped flag
        // Note that we toggle it, instead of just turning it on
        // This is because \\ is actually escaping an escape
        escaped = !escaped;
      }
      else
      {
        // If we are currently escaped...
        if (escaped)
        {
          // Determine all the valid characters that we could escape on
          if (CharacterUtilities::IsStringEscapee(character) == false)
          {
            // If we're in a comment, ignore invalid escape sequences
            if (this->CommentDepth == 0)
            {
              // We hit an invalid escape
              this->Errors.Raise(this->Location, ErrorCode::InvalidEscapeInStringLiteral, character);
              return false;
            }
          }

          // We are no longer escaped
          escaped = false;
        }
        // Otherwise, if it's a non escaped quotation..
        else if (character == '"' || (this->EnableStringInterpolation && character == '`'))
        {
          // Set that the last accepted position was this position
          lastAcceptedPos = this->Position;
          
          // If we're ending this string with a grave, then it means we're starting an interpolant
          bool isInterpolantStart = (this->EnableStringInterpolation && character == '`');

          // If the character is an interpolant starting character...
          if (isInterpolantStart)
          {
            if (isInterpolantEnd)
            {
              // We are both ending an intpolant and starting another...
              outToken->TokenId = Grammar::EndBeginStringInterpolate;
            }
            else
            {
              // We are just starting a new interpolant
              outToken->TokenId = Grammar::BeginStringInterpolate;
            }
          }
          else
          {
            if (isInterpolantEnd)
            {
              // We are just ending an interpolant
              outToken->TokenId = Grammar::EndStringInterpolate;
            }
            else
            {
              // It's just a straight up string literal
              outToken->TokenId = Grammar::StringLiteral;
            }
          }

          // We parsed the string or interpolant, call it good!
          return true;
        }
        // Is it the eof character or the end of a line?
        else if (character == '\0' || character == '\r' || character == '\n')
        {
          // If we're in tolerant mode, most likely the user was typing a string
          // or string interpolant and it wasn't yet closed
          // What we're going to do here is just parse this as a string token and cut it off
          if (this->Errors.TolerantMode)
          {
            // Set that the last accepted position was this position
            lastAcceptedPos = this->Position;

            // Just return this as if the entire line is a string
            outToken->TokenId = Grammar::StringLiteral;
            return true;
          }
          else
          {
            // We hit the eof before closing the string
            this->Errors.Raise(this->Location, ErrorCode::StringLiteralNotComplete);
            return false;
          }
        }
      }
    }
  }

  //***************************************************************************
  bool Tokenizer::ReadToken(UserToken* outToken)
  {
    // Store the last position at which we accepted a token (or the beginning of the token)
    size_t lastAcceptedPos = this->Position;

    // Set the starting position that the token is reading from
    outToken->Start = this->Position;

    // Store the character that we read
    char character = '\0';

    // Store the type of token we're parsing (we should know immediately from the first branch)
    TokenCategory::Enum tokenType;

    // Loop until we accept a token
    ZilchLoop
    {
      // Attempt to read a keyword or symbol
      // Internally we call 'ReadCharacter', which can return eof, so we handle it below
      if (ReadKeywordOrSymbol(outToken, lastAcceptedPos, character, tokenType) == true)
      {
        // Break out since we've found the full token
        break;
      }
      else
      {
        // If we started by reading a keyword...
        if (tokenType == TokenCategory::Keyword)
        {
          // Store away the position
          lastAcceptedPos = this->Position - 1;

          // Read the identifier (we don't actually need to check the return since we always know its 'true')
          bool result = ReadIdentifier(outToken, true, lastAcceptedPos, character);
          ErrorIf(result != true, "Reading the identifier must always succeed");
          break;
        }
        // If we didn't read anything yet...
        else if (tokenType == TokenCategory::Unknown)
        {
          // If the character is some sort of space character...
          if (CharacterUtilities::IsWhiteSpace(character))
          {
            // Set the starting position that the token is reading from
            outToken->Start = this->Position;

            // Since we hit whitespace, we want to update the location
            // so that the next token doesn't think it started in whitespace
            this->Location.StartLine = this->Line;
            this->Location.StartCharacter = this->Character;
            this->Location.StartPosition = this->Position;
            this->Location.PrimaryLine = this->Line;
            this->Location.PrimaryCharacter = this->Character;
            this->Location.PrimaryPosition = this->Position;

            // Continue to the next loop iteration
            continue;
          }
          // Attempt to read it as an identifier
          else if (ReadIdentifier(outToken, false, lastAcceptedPos, character)  ||
                   ReadNumber(outToken, lastAcceptedPos, character)             ||
                   ReadString(outToken, lastAcceptedPos, character))
          {
            // Break out since we've found the full token
            break;
          }
          // If we reach the end of the file...
          else if (character == '\0')
          {
            // We hit the end
            return false;
          }
          // Otherwise, if there was an error...
          else if (this->Errors.WasError)
          {
            // An error occurred, return that we failed
            return false;
          }
        }

        // As long as we're not in the middle of a comment...
        if (this->CommentDepth == 0)
        {
          // Get the bad character
          char badCharacter = '?';

          // Ensure that the bad character was within the data bounds (for safety)
          if (outToken->Start < this->Data.SizeInBytes())
          {
            // Get the bad character
            badCharacter = this->Data.Data()[outToken->Start];
          }

          // If we got here, some sort of unknown data must have been input
          // Add a parsing error that informs the user that a token could not be read
          this->Errors.Raise(this->Location, ErrorCode::UnidentifiedSymbol, badCharacter);
          return false;
        }
      }
    }
    
    // Set the token's line and character number
    outToken->Location = this->Location;

    // Set the length of the token as the last recorded end minus the start
    outToken->Length = lastAcceptedPos - outToken->Start;

    // Read in the token
    outToken->Token = String(this->Data.c_str() + outToken->Start, outToken->Length);

    // Move the location's start forward
    this->Location.StartLine = this->Location.EndLine;
    this->Location.StartCharacter = this->Location.EndCharacter;
    this->Location.StartPosition = this->Location.EndPosition;
    this->Location.PrimaryLine = this->Location.EndLine;
    this->Location.PrimaryCharacter = this->Location.EndCharacter;
    this->Location.PrimaryPosition = this->Location.EndPosition;

    // Return if the symbol was not null
    return true;
  }


  //***************************************************************************
  void Tokenizer::UpdateLineAndCharacterNumber(char character)
  {
    // The end of the location is always one character behind
    this->Location.EndLine = this->Line;
    this->Location.EndCharacter = this->Character;
    this->Location.EndPosition = this->Position - 1;

    // Skip the null character
    if (character != '\0')
    {
      // Increment the character count for the current line
      ++this->Character;

      // Check if the character matches that of the newline or carriage return
      if (character == '\n' || character == '\r')
      {
        // As long as the last character wasn't a carriage return and this character isn't a newline (CRLF)...
        // Note: If this was a full CRLF, we already incremented the line on the first CR, no need to do it again!
        if ((this->WasCarriageReturn && character == '\n') == false)
        {
          // Increment the line count since we hit a line
          ++this->Line;
        }

        // If the character is a carriage return then set it
        this->WasCarriageReturn = (character == '\r');

        // For this new line, we start out on character one
        this->Character = 1;
      }
    }
  }

  //***************************************************************************
  String Tokenizer::SkipToEndOfLine()
  {
    // Get the range of text
    StringRange range;
    range = this->Data.c_str() + this->Position;
    int startingPos = this->Position;
    // Loop until we hit the end of a line or the end of the file
    ZilchLoop
    {
      // Read the next character. We handle eof specifically below
      char character = ReadCharacter();

      // Check if we hit either a newline or a carriage return
      if (character == '\n' || character == '\r' || character == '\0')
      {
        // Break out of the loop
        break;
      }
    }
     
    // Backup the input stream by one character
    --this->Position;
    
    // Set the end of the string range
    range.mEnd = range.mBegin + (this->Position - startingPos);

    // Return the parsed string
    return range;
  }

  //***************************************************************************
  bool CharacterUtilities::IsWhiteSpace(Rune r)
  {
    // Depending on the character...
    switch (r.mValue.value)
    {
      // Look for any of the standard white-space characters
      case ' ':
      case '\t':
      case '\n':
      case '\v':
      case '\f':
      case '\r':
        return true;
    }

    // Otherwise, we got here so it must not be white-space!
    return false;
  }

  //***************************************************************************
  bool CharacterUtilities::IsAlpha(Rune r)
  {
    return (r.mValue >= 'a' && r.mValue <= 'z') || (r.mValue >= 'A' && r.mValue <= 'Z');
  }

  //***************************************************************************
  bool CharacterUtilities::IsNumeric(Rune r)
  {
    return (r.mValue >= '0'&& r.mValue <= '9');
  }

  //***************************************************************************
  bool CharacterUtilities::IsAlphaNumeric(Rune r)
  {
    return (r.mValue >= 'a' && r.mValue <= 'z') || (r.mValue >= 'A' && r.mValue <= 'Z') || (r.mValue >= '0'&& r.mValue <= '9');
  }

  //***************************************************************************
  bool CharacterUtilities::IsUpper(Rune r)
  {
    return (r.mValue.value & 0x20) == 0;
  }

  //***************************************************************************
  bool CharacterUtilities::IsStringEscapee(Rune r)
  {
    // Determine all the valid characters that we could escape on
    switch (r.mValue.value)
    {
      // All of these are valid characters to be escaped (in a string literal)
      case '"':
      case '`':
      case '\\':
      case '0':
      case 'a':
      case 'b':
      case 'f':
      case 'n':
      case 'r':
      case 't':
      case 'v':
        return true;

      // We hit an invalid character...
      default:
        return false;
    }
  }
}
