/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  const String ThisKeyword                ("this");
  const String ValueKeyword               ("value");
  const String PreConstructorName         ("[PreConstructor]");
  const String ConstructorName            ("[Constructor]");
  const String DestructorName             ("[Destructor]");
  const String FieldInitializerName       ("[FieldInitializer]");
  const String ExpressionLibrary          ("Expression");
  const String ExpressionProgram          ("Program");
  const String ExpressionMain             ("Main");
  const String PropertyDelegateName       ("Property");
  const String StaticAttribute            ("Static");
  const String OverrideAttribute          ("Override");
  const String VirtualAttribute           ("Virtual");
  const String HiddenAttribute            ("Hidden");
  const String ExtensionAttribute         ("Extension");
  const String PropertyAttribute          ("Property");
  const String ExportDocumentation        ("ExportDocumentation");
  const String ImportDocumentation        ("ImportDocumentation");
  const String CodeString                 ("CodeString");
  const String ExpressionInitializerLocal ("ExpressionInitializer");
  const String OperatorInsert             ("Add");
  const String OperatorGet                ("Get");
  const String OperatorSet                ("Set");
  const String UnknownOrigin              ("<Unknown>");
  const String EmptyLowerIdentifier       ("value");
  const String EmptyUpperIdentifier       ("Value");
  const String DefaultLibraryName         ("Library");
  

  //***************************************************************************
  String BuildGetterName(StringParam name)
  {
    return BuildString("[Get", name, "]");
  }

  //***************************************************************************
  String BuildSetterName(StringParam name)
  {
    return BuildString("[Set", name, "]");
  }

  //***************************************************************************
  String ReplaceStringEscapes(StringRange input)
  {
    StringBuilder builder;

    // Whether or not we hit the escape character '\'
    bool isEscape = false;
    
    // Walk through all the input characters
    while (!input.Empty())
    {
      // Grab the current character
      Zero::Rune r = input.Front();

      // If we hit the escape character...
      if (isEscape)
      {
        // Based on the escaped character type...
        switch (r.value)
        {
          // These characters have special meanings or ascii values
          case '0':
            builder.Append('\0');
            break;
          case 'a':
            builder.Append('\a');
            break;
          case 'b':
            builder.Append('\b');
            break;
          case 'f':
            builder.Append('\f');
            break;
          case 'n':
            builder.Append('\n');
            break;
          case 'r':
            builder.Append('\r');
            break;
          case 't':
            builder.Append('\t');
            break;
          case 'v':
            builder.Append('\v');
            break;

          // All other characters just get directly inserted as themselves
          default:
            builder.Append(r);
            break;
        }

        // No matter what, we've already handled the escape
        isEscape = false;
      }
      else if (r == '\\')
      {
        isEscape = true;
      }
      else
      {
        // Otherwise, we just hit a normal character
        builder.Append(r);
      }

      input.PopFront();
    }

    // Return the resulting string
    return builder.ToString();
  }
  
  //***************************************************************************
  bool IsZilchQuoteCharacter(Zero::Rune r)
  {
    return r == '"' || r == '`' || r == '\'';
  }

  //***************************************************************************
  StringRange StripStringQuotes(StringRange input)
  {
    // Error checking
    ErrorIf(input.SizeInBytes() < 2, "A string cannot contain quotes on both ends if it has a size less than 2");
    ErrorIf(IsZilchQuoteCharacter(input.Front()) == false, "The starting character was not a Zilch quotation");
    ErrorIf(IsZilchQuoteCharacter(input.Back()) == false, "The ending character was not a Zilch quotation");

    // Move the beginning inward and the end backwards by 1 to strip the quotes
    input.PopFront();
    input.PopBack();

    return input;
  }
  
  //***************************************************************************
  StringRange AddStringQuotes(StringRange input)
  {
    return BuildString("\"", input, "\"");
  }

  //***************************************************************************
  String ReplaceStringEscapesAndStripQuotes(StringRange input)
  {
    // Strip the quotes
    input = StripStringQuotes(input);

    // Now perform the string replacements
    return ReplaceStringEscapes(input);
  }
  
  //***************************************************************************
  String EscapeStringAndAddQuotes(StringRange input)
  {
    return AddStringQuotes(EscapeString(input));
  }

  //***************************************************************************
  String ToLowerCamelCase(StringRange input)
  {
    if (input.Empty())
      return String();

    Zero::Rune firstCharacter = input.Front();
    firstCharacter = Zero::UTF8::ToLower(firstCharacter);
    input.PopFront();
    return BuildString(String(firstCharacter), input);
  }
  
  //***************************************************************************
  String ToUpperCamelCase(StringRange input)
  {
    if (input.Empty())
      return String();

    Zero::Rune firstCharacter = input.Front();
    firstCharacter = Zero::UTF8::ToUpper(firstCharacter);
    input.PopFront();
    return BuildString(String(firstCharacter), input);
  }
}
