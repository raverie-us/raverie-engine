// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"

namespace Raverie
{
// The symbols or keywords we use, if available for given grammar symbols
static String KeywordsOrSymbols[] = {
    "Invalid",
    "End",
    "Error",
    "Whitespace",
    "UpperIdentifier",
    "LowerIdentifier",
    "IntegerLiteral",
    "DoubleIntegerLiteral",
    "RealLiteral",
    "DoubleRealLiteral",
    "CharacterLiteral",
    "StringLiteral",
    "BeginStringInterpolate",
    "EndStringInterpolate",
    "EndBeginStringInterpolate",
    "abstract",
    "alias",
    "alignof",
    "assert",
    "auto",
    "case",
    "catch",
    "checked",
    "const",
    "default",
    "dynamic",
    "explicit",
    "export",
    "extern",
    "finally",
    "fixed",
    "friend",
    "global",
    "goto",
    "immutable",
    "implicit",
    "import",
    "in",
    "include",
    "inline",
    "interface",
    "internal",
    "is",
    "local",
    "lock",
    "module",
    "mutable",
    "namespace",
    "operator",
    "out",
    "override",
    "package",
    "params",
    "partial",
    "positional",
    "private",
    "protected",
    "public",
    "readonly",
    "register",
    "require",
    "scope",
    "sealed",
    "signed",
    "sizeof",
    "stackalloc",
    "static",
    "switch",
    "timeout",
    "try",
    "typedef",
    "typename",
    "unchecked",
    "unsafe",
    "unsigned",
    "using",
    "virtual",
    "volatile",
    "where",
    "yield",
    "any",
    "and",
    "as",
    "base",
    "break",
    "class",
    "constructor",
    "continue",
    "debug",
    "delegate",
    "delete",
    "destructor",
    "do",
    "else",
    "enum",
    "false",
    "flags",
    "for",
    "foreach",
    "function",
    "get",
    "if",
    "loop",
    "memberid",
    "new",
    "not",
    "null",
    "or",
    "ref",
    "return",
    "sends",
    "set",
    "struct",
    "throw",
    "true",
    "typeid",
    "typeof",
    "var",
    "while",
    ".",
    "->",
    "~>",
    ":",
    ",",
    "=>",
    "=",
    "-=",
    "+=",
    "/=",
    "*=",
    "%=",
    "^=",
    "<<=",
    ">>=",
    "$=",
    "|=",
    "&=",
    "==",
    "!=",
    "<",
    "<=",
    ">",
    ">=",
    "-",
    "+",
    "/",
    "*",
    "%",
    "^",
    "--",
    "++",
    "<<",
    ">>",
    "$",
    "|",
    "&",
    "~",
    "@",
    "||",
    "&&",
    "!",
    ";",
    "[",
    "]",
    "(",
    ")",
    "{",
    "}",
    "//",
    "/*",
    "*/",
};

// The human readable names we give to every symbol
static String Names[] = {
    "Invalid",
    "End",
    "Error",
    "Whitespace",
    "UpperIdentifier",
    "LowerIdentifier",
    "IntegerLiteral",
    "DoubleIntegerLiteral",
    "RealLiteral",
    "DoubleRealLiteral",
    "CharacterLiteral",
    "StringLiteral",
    "BeginStringInterpolate",
    "EndStringInterpolate",
    "EndBeginStringInterpolate",
    "Abstract",
    "Alias",
    "Alignof",
    "Assert",
    "Auto",
    "Case",
    "Catch",
    "Checked",
    "Const",
    "Default",
    "Dynamic",
    "Explicit",
    "Export",
    "Extern",
    "Finally",
    "Fixed",
    "Friend",
    "Global",
    "Goto",
    "Immutable",
    "Implicit",
    "Import",
    "In",
    "Include",
    "Inline",
    "Interface",
    "Internal",
    "Is",
    "Local",
    "Lock",
    "Module",
    "Mutable",
    "Namespace",
    "Operator",
    "Out",
    "Override",
    "Package",
    "Params",
    "Partial",
    "Positional",
    "Private",
    "Protected",
    "Public",
    "Readonly",
    "Register",
    "Require",
    "Scope",
    "Sealed",
    "Signed",
    "Sizeof",
    "Stackalloc",
    "Static",
    "Switch",
    "Timeout",
    "Try",
    "Typedef",
    "Typename",
    "Unchecked",
    "Unsafe",
    "Unsigned",
    "Using",
    "Virtual",
    "Volatile",
    "Where",
    "Yield",
    "Any",
    "And",
    "As",
    "Base",
    "Break",
    "Class",
    "Constructor",
    "Continue",
    "Debug",
    "Delegate",
    "Delete",
    "Destructor",
    "Do",
    "Else",
    "Enumeration",
    "False",
    "Flags",
    "For",
    "ForEach",
    "Function",
    "Get",
    "If",
    "Loop",
    "MemberId",
    "New",
    "Not",
    "Null",
    "Or",
    "Ref",
    "Return",
    "Sends",
    "Set",
    "Struct",
    "Throw",
    "True",
    "TypeId",
    "Typeof",
    "Variable",
    "While",
    "Access",
    "DynamicAccess",
    "NonVirtualAccess",
    "TypeSpecifier / NameSpecifier / Inheritance / InitializerList",
    "ArgumentSeparator",
    "RefersTo",
    "Assignment",
    "AssignmentSubtract",
    "AssignmentAdd",
    "AssignmentDivide",
    "AssignmentMultiply",
    "AssignmentModulo",
    "AssignmentExponent",
    "AssignmentLeftShift",
    "AssignmentRightShift",
    "AssignmentBitwiseXor",
    "AssignmentBitwiseOr",
    "AssignmentBitwiseAnd",
    "Equality",
    "Inequality",
    "LessThan",
    "LessThanOrEqualTo",
    "GreaterThan",
    "GreaterThanOrEqualTo",
    "Negative / Subtract",
    "Positive / Add",
    "Divide",
    "Multiply / Dereference",
    "Modulo",
    "Exponent",
    "Decrement",
    "Increment",
    "BitshiftLeft",
    "BitshiftRight",
    "BitwiseXor",
    "BitwiseOr",
    "BitwiseAnd / AddressOf",
    "BitwiseNot",
    "PropertyDelegate",
    "LogicalOr",
    "LogicalAnd",
    "LogicalNot",
    "StatementSeparator",
    "BeginIndex / BeginTemplate / BeginAttribute / OldBeginInitializer",
    "EndIndex / EndTemplate / EndAttribute / OldEndInitializer",
    "BeginFunctionCall / BeginFunctionParameters / BeginGroup",
    "EndFunctionCall / EndFunctionParameters / EndGroup",
    "BeginScope / BeginInitializer",
    "EndScope / EndInitializer",
    "CommentLine",
    "CommentStart",
    "CommentEnd",
};

const String& Grammar::GetName(Grammar::Enum value)
{
  // We add one because of the 'Invalid' index
  return Names[value + 1];
}

const String& Grammar::GetKeywordOrSymbol(Grammar::Enum value)
{
  // We add one because of the 'Invalid' index
  return KeywordsOrSymbols[value + 1];
}

const Array<String>& Grammar::GetUsedKeywords()
{
  static Array<String> results;
  if (results.Empty())
  {
    results.PushBack("any");
    results.PushBack("and");
    results.PushBack("as");
    results.PushBack("base");
    results.PushBack("break");
    results.PushBack("class");
    results.PushBack("constructor");
    results.PushBack("continue");
    results.PushBack("debug");
    results.PushBack("delegate");
    results.PushBack("delete");
    results.PushBack("destructor");
    results.PushBack("do");
    results.PushBack("else");
    results.PushBack("enum");
    results.PushBack("false");
    results.PushBack("flags");
    results.PushBack("for");
    results.PushBack("foreach");
    results.PushBack("function");
    results.PushBack("get");
    results.PushBack("if");
    results.PushBack("loop");
    results.PushBack("memberid");
    results.PushBack("new");
    results.PushBack("not");
    results.PushBack("null");
    results.PushBack("or");
    results.PushBack("ref");
    results.PushBack("return");
    results.PushBack("sends");
    results.PushBack("set");
    results.PushBack("struct");
    results.PushBack("throw");
    results.PushBack("true");
    results.PushBack("typeid");
    results.PushBack("typeof");
    results.PushBack("var");
    results.PushBack("while");
  }
  return results;
}

const Array<String>& Grammar::GetSpecialKeywords()
{
  static Array<String> results;
  if (results.Empty())
  {
    results.PushBack("this");
    results.PushBack("value");
    results.PushBack("event");
  }
  return results;
}

const Array<String>& Grammar::GetReservedKeywords()
{
  static Array<String> results;
  if (results.Empty())
  {
    results.PushBack("abstract");
    results.PushBack("alias");
    results.PushBack("alignof");
    results.PushBack("assert");
    results.PushBack("auto");
    results.PushBack("case");
    results.PushBack("catch");
    results.PushBack("checked");
    results.PushBack("const");
    results.PushBack("default");
    results.PushBack("dynamic");
    results.PushBack("explicit");
    results.PushBack("export");
    results.PushBack("extern");
    results.PushBack("finally");
    results.PushBack("fixed");
    results.PushBack("friend");
    results.PushBack("global");
    results.PushBack("goto");
    results.PushBack("immutable");
    results.PushBack("implicit");
    results.PushBack("import");
    results.PushBack("in");
    results.PushBack("include");
    results.PushBack("inline");
    results.PushBack("interface");
    results.PushBack("internal");
    results.PushBack("is");
    results.PushBack("local");
    results.PushBack("lock");
    results.PushBack("module");
    results.PushBack("mutable");
    results.PushBack("namespace");
    results.PushBack("operator");
    results.PushBack("out");
    results.PushBack("override");
    results.PushBack("package");
    results.PushBack("params");
    results.PushBack("partial");
    results.PushBack("positional");
    results.PushBack("private");
    results.PushBack("protected");
    results.PushBack("public");
    results.PushBack("readonly");
    results.PushBack("register");
    results.PushBack("require");
    results.PushBack("scope");
    results.PushBack("sealed");
    results.PushBack("signed");
    results.PushBack("sizeof");
    results.PushBack("stackalloc");
    results.PushBack("static");
    results.PushBack("switch");
    results.PushBack("timeout");
    results.PushBack("try");
    results.PushBack("typedef");
    results.PushBack("typename");
    results.PushBack("unchecked");
    results.PushBack("unsafe");
    results.PushBack("unsigned");
    results.PushBack("using");
    results.PushBack("virtual");
    results.PushBack("volatile");
    results.PushBack("where");
    results.PushBack("yield");
  }
  return results;
}
} // namespace Raverie
