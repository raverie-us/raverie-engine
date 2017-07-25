/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_STRING_CONSTANTS_HPP
#define ZILCH_STRING_CONSTANTS_HPP

namespace Zilch
{
  // Constants
  ZeroShared extern const String ThisKeyword;
  ZeroShared extern const String ValueKeyword;
  ZeroShared extern const String PreConstructorName;
  ZeroShared extern const String ConstructorName;
  ZeroShared extern const String DestructorName;
  ZeroShared extern const String FieldInitializerName;
  ZeroShared extern const String ExpressionLibrary;
  ZeroShared extern const String ExpressionProgram;
  ZeroShared extern const String ExpressionMain;
  ZeroShared extern const String PropertyDelegateName;
  ZeroShared extern const String StaticAttribute;
  ZeroShared extern const String OverrideAttribute;
  ZeroShared extern const String VirtualAttribute;
  ZeroShared extern const String HiddenAttribute;
  ZeroShared extern const String ExtensionAttribute;
  ZeroShared extern const String PropertyAttribute;
  ZeroShared extern const String ExportDocumentation;
  ZeroShared extern const String ImportDocumentation;
  ZeroShared extern const String CodeString;
  ZeroShared extern const String ExpressionInitializerLocal;
  ZeroShared extern const String OperatorInsert;
  ZeroShared extern const String OperatorGet;
  ZeroShared extern const String OperatorSet;
  ZeroShared extern const String UnknownOrigin;
  ZeroShared extern const String EmptyLowerIdentifier;
  ZeroShared extern const String EmptyUpperIdentifier;
  ZeroShared extern const String DefaultLibraryName;

  // Helper functions
  ZeroShared String BuildGetterName(StringParam name);
  ZeroShared String BuildSetterName(StringParam name);

  // Perform string escape replacements
  ZeroShared String ReplaceStringEscapes(StringRange input);

  // Strip outlining quotes (directly used for string literals)
  ZeroShared StringRange StripStringQuotes(StringRange input);

  // Adds quotes to the string
  ZeroShared StringRange AddStringQuotes(StringRange input);

  // Perform string escape replacements and outlining quotes (directly used for string literals)
  ZeroShared String ReplaceStringEscapesAndStripQuotes(StringRange input);

  // Replaces all ascii escapable characters with escapes and adds quotes to the string
  ZeroShared String EscapeStringAndAddQuotes(StringRange input);

  // Change an identifier between lower and upper camel cases (just modifies the first letter)
  ZeroShared String ToLowerCamelCase(StringRange input);
  ZeroShared String ToUpperCamelCase(StringRange input);
}

#endif
