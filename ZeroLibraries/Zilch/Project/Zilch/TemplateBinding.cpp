/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  void ParameterNameTokenizing(ErrorEvent* e)
  {
    // Assert using the formatted error message
    Error("Error parsing parameter names: %s", e->ExactError.c_str());
  }

  //***************************************************************************
  void TemplateBinding::ParseParameterArrays(ParameterArray& parameters, StringRange commaDelimitedNames)
  {
    // We return immediately if the string is empty because this function
    // gets generically called even when no arguments are supplied
    if (commaDelimitedNames.Empty())
      return;

    // We're just going to use the tokenizer to parse the parameter names
    CompilationErrors errors;
    EventConnect(&errors, Events::CompilationError, ParameterNameTokenizing);
    Tokenizer tokenizer(errors);

    // The tokens and comments we'll parse (comments are entirely ignored)
    Array<UserToken> tokens;
    Array<UserToken> comments;

    // Parse the names into tokens (commas are allowed)
    CodeEntry namesEntry;
    namesEntry.Code = commaDelimitedNames;
    tokenizer.Parse(namesEntry, tokens, comments);

    // Store the index we're on in the parameters array
    size_t parameterIndex = 0;
    
    // Loop through the tokens in the order they were parsed
    for (size_t i = 0; i < tokens.Size(); ++i)
    {
      // If the parameter index goes outside the bounds...
      ErrorIf(parameterIndex >= parameters.Size(),
        "Too many parameter names for the number of parameter arguments provided");

      // If the current token is a name...
      UserToken& token = tokens[i];
      if (token.TokenId == Grammar::LowerIdentifier)
      {
        // Set the name of the current parameter
        parameters[parameterIndex].Name = token.Token;
        ++parameterIndex;
      }
      else if (token.TokenId != Grammar::ArgumentSeparator)
      {
        // Show an error to the user
        Error
        (
          "Unexpected token type when reading parameter names (must be lowercase identifiers): %s of token type %s",
          token.Token.c_str(),
          Grammar::GetName(token.TokenId).c_str()
        );
      }
    }

    // If we didn't parse as many tokens as the parameters, then we also throw an error
    ErrorIf(parameterIndex != parameters.Size(), "A different number of parameter names were given for the number of parameters");
  }
  
  //***************************************************************************
  BoundType* TemplateBinding::ValidateConstructorBinding(BoundType* type)
  {
    ErrorIf(type->Destructor != nullptr,
      "You must bind a destructor first before binding a constructor (use ZilchBindConstructor or ZilchFullBindConstructor)");
    return type;
  }
}