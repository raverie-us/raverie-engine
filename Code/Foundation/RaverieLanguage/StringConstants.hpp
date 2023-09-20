// MIT Licensed (see LICENSE.md).

#pragma once

namespace Raverie
{
// Constants
extern const String ThisKeyword;
extern const String ValueKeyword;
extern const String PreConstructorName;
extern const String ConstructorName;
extern const String DestructorName;
extern const String FieldInitializerName;
extern const String ExpressionLibrary;
extern const String ExpressionProgram;
extern const String ExpressionMain;
extern const String PropertyDelegateName;
extern const String StaticAttribute;
extern const String OverrideAttribute;
extern const String VirtualAttribute;
extern const String HiddenAttribute;
extern const String ExtensionAttribute;
extern const String PropertyAttribute;
extern const String InternalAttribute;
extern const String DeprecatedAttribute;
extern const String ExportDocumentation;
extern const String ImportDocumentation;
extern const String CodeString;
extern const String ExpressionInitializerLocal;
extern const String OperatorInsert;
extern const String OperatorGet;
extern const String OperatorSet;
extern const String UnknownOrigin;
extern const String EmptyLowerIdentifier;
extern const String EmptyUpperIdentifier;
extern const String DefaultLibraryName;

// Helper functions
String BuildGetterName(StringParam name);
String BuildSetterName(StringParam name);

// Perform string escape replacements
String ReplaceStringEscapes(StringRange input);

// Strip outlining quotes (directly used for string literals)
StringRange StripStringQuotes(StringRange input);

// Adds quotes to the string
StringRange AddStringQuotes(StringRange input);

// Perform string escape replacements and outlining quotes (directly used for
// string literals)
String ReplaceStringEscapesAndStripQuotes(StringRange input);

// Replaces all ascii escapable characters with escapes and adds quotes to the
// string
String EscapeStringAndAddQuotes(StringRange input);

// Change an identifier between lower and upper camel cases (just modifies the
// first letter)
String ToLowerCamelCase(StringRange input);
String ToUpperCamelCase(StringRange input);
} // namespace Raverie
