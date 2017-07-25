using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace RegexBuilder
{
	static class Program
	{
		static void Main()
		{
			//CharacterSet charSet1 = new CharacterSet();
			//CharacterSet charSet2 = new CharacterSet();
			//CharacterSet charSet3;

			//charSet1.AddCharacterRange('a', 'g');
			//charSet1.AddCharacterRange('i', 'p');
			//charSet1.AddCharacterRange('f', 'l');

			//charSet1.RemoveCharacterRange('m', 'z');
			//charSet1.RemoveCharacterRange('f', 'h');

			//charSet2.AddCharacterRange('k', 'x');

			//charSet3 = CharacterSet.Union(charSet1, charSet2);

			Tokens tokens = new Tokens();
			RegexBuilder builder = new RegexBuilder();

			tokens.AddToken(TokenType.Reserved, "abstract", "Abstract");
			tokens.AddToken(TokenType.Reserved, "alias", "Alias");
			tokens.AddToken(TokenType.Reserved, "alignof", "Alignof");
			tokens.AddToken(TokenType.Reserved, "assert", "Assert");
			tokens.AddToken(TokenType.Reserved, "auto", "Auto");
			tokens.AddToken(TokenType.Reserved, "case", "Case");
			tokens.AddToken(TokenType.Reserved, "catch", "Catch");
			tokens.AddToken(TokenType.Reserved, "checked", "Checked");
			tokens.AddToken(TokenType.Reserved, "const", "Const");
			tokens.AddToken(TokenType.Reserved, "default", "Default");
			tokens.AddToken(TokenType.Reserved, "dynamic", "Dynamic");
			tokens.AddToken(TokenType.Reserved, "explicit", "Explicit");
			tokens.AddToken(TokenType.Reserved, "export", "Export");
			tokens.AddToken(TokenType.Reserved, "extern", "Extern");
			tokens.AddToken(TokenType.Reserved, "finally", "Finally");
			tokens.AddToken(TokenType.Reserved, "fixed", "Fixed");
			tokens.AddToken(TokenType.Reserved, "friend", "Friend");
			tokens.AddToken(TokenType.Reserved, "global", "Global");
			tokens.AddToken(TokenType.Reserved, "goto", "Goto");
			tokens.AddToken(TokenType.Reserved, "immutable", "Immutable");
			tokens.AddToken(TokenType.Reserved, "implicit", "Implicit");
			tokens.AddToken(TokenType.Reserved, "import", "Import");
			tokens.AddToken(TokenType.Reserved, "in", "In");
			tokens.AddToken(TokenType.Reserved, "include", "Include");
			tokens.AddToken(TokenType.Reserved, "inline", "Inline");
			tokens.AddToken(TokenType.Reserved, "interface", "Interface");
			tokens.AddToken(TokenType.Reserved, "internal", "Internal");
			tokens.AddToken(TokenType.Reserved, "is", "Is");
			tokens.AddToken(TokenType.Reserved, "local", "Local");
			tokens.AddToken(TokenType.Reserved, "lock", "Lock");
			tokens.AddToken(TokenType.Reserved, "module", "Module");
			tokens.AddToken(TokenType.Reserved, "mutable", "Mutable");
			tokens.AddToken(TokenType.Reserved, "namespace", "Namespace");
			tokens.AddToken(TokenType.Reserved, "operator", "Operator");
			tokens.AddToken(TokenType.Reserved, "out", "Out");
			tokens.AddToken(TokenType.Reserved, "override", "Override");
			tokens.AddToken(TokenType.Reserved, "package", "Package");
			tokens.AddToken(TokenType.Reserved, "params", "Params");
			tokens.AddToken(TokenType.Reserved, "partial", "Partial");
			tokens.AddToken(TokenType.Reserved, "positional", "Positional");
			tokens.AddToken(TokenType.Reserved, "private", "Private");
			tokens.AddToken(TokenType.Reserved, "protected", "Protected");
			tokens.AddToken(TokenType.Reserved, "public", "Public");
			tokens.AddToken(TokenType.Reserved, "readonly", "Readonly");
			tokens.AddToken(TokenType.Reserved, "register", "Register");
			tokens.AddToken(TokenType.Reserved, "require", "Require");
			tokens.AddToken(TokenType.Reserved, "scope", "Scope");
			tokens.AddToken(TokenType.Reserved, "sealed", "Sealed");
			tokens.AddToken(TokenType.Reserved, "signed", "Signed");
			tokens.AddToken(TokenType.Reserved, "sizeof", "Sizeof");
			tokens.AddToken(TokenType.Reserved, "stackalloc", "Stackalloc");
			tokens.AddToken(TokenType.Reserved, "static", "Static");
			tokens.AddToken(TokenType.Reserved, "switch", "Switch");
			tokens.AddToken(TokenType.Reserved, "timeout", "Timeout");
			tokens.AddToken(TokenType.Reserved, "try", "Try");
			tokens.AddToken(TokenType.Reserved, "typedef", "Typedef");
			tokens.AddToken(TokenType.Reserved, "typename", "Typename");
			tokens.AddToken(TokenType.Reserved, "unchecked", "Unchecked");
			tokens.AddToken(TokenType.Reserved, "unsafe", "Unsafe");
			tokens.AddToken(TokenType.Reserved, "unsigned", "Unsigned");
			tokens.AddToken(TokenType.Reserved, "using", "Using");
			tokens.AddToken(TokenType.Reserved, "virtual", "Virtual");
			tokens.AddToken(TokenType.Reserved, "volatile", "Volatile");
			tokens.AddToken(TokenType.Reserved, "where", "Where");
			tokens.AddToken(TokenType.Reserved, "yield", "Yield");

			tokens.AddToken(TokenType.Keyword, "any", "Any");
			tokens.AddToken(TokenType.Keyword, "and", "And");
			tokens.AddToken(TokenType.Keyword, "as", "As");
			tokens.AddToken(TokenType.Keyword, "base", "Base");
			tokens.AddToken(TokenType.Keyword, "break", "Break");
			tokens.AddToken(TokenType.Keyword, "class", "Class");
			tokens.AddToken(TokenType.Keyword, "constructor", "Constructor");
            tokens.AddToken(TokenType.Keyword, "continue", "Continue");
            tokens.AddToken(TokenType.Keyword, "debug", "Debug");
			tokens.AddToken(TokenType.Keyword, "delegate", "Delegate");
			tokens.AddToken(TokenType.Keyword, "delete", "Delete");
			tokens.AddToken(TokenType.Keyword, "destructor", "Destructor");
			tokens.AddToken(TokenType.Keyword, "do", "Do");
			tokens.AddToken(TokenType.Keyword, "else", "Else");
			tokens.AddToken(TokenType.Keyword, "enum", "Enumeration");
			tokens.AddToken(TokenType.Keyword, "false", "False");
			tokens.AddToken(TokenType.Keyword, "flags", "Flags");
			tokens.AddToken(TokenType.Keyword, "for", "For");
			tokens.AddToken(TokenType.Keyword, "foreach", "ForEach");
			tokens.AddToken(TokenType.Keyword, "function", "Function");
			tokens.AddToken(TokenType.Keyword, "get", "Get");
			tokens.AddToken(TokenType.Keyword, "if", "If");
			tokens.AddToken(TokenType.Keyword, "loop", "Loop");
			tokens.AddToken(TokenType.Keyword, "memberid", "MemberId");
			tokens.AddToken(TokenType.Keyword, "new", "New");
			tokens.AddToken(TokenType.Keyword, "not", "Not");
			tokens.AddToken(TokenType.Keyword, "null", "Null");
			tokens.AddToken(TokenType.Keyword, "or", "Or");
			tokens.AddToken(TokenType.Keyword, "ref", "Ref");
			tokens.AddToken(TokenType.Keyword, "return", "Return");
			tokens.AddToken(TokenType.Keyword, "sends", "Sends");
			tokens.AddToken(TokenType.Keyword, "set", "Set");
			tokens.AddToken(TokenType.Keyword, "struct", "Struct");
			tokens.AddToken(TokenType.Keyword, "throw", "Throw");
			tokens.AddToken(TokenType.Keyword, "true", "True");
			tokens.AddToken(TokenType.Keyword, "typeid", "TypeId");
			tokens.AddToken(TokenType.Keyword, "typeof", "Typeof");
			tokens.AddToken(TokenType.Keyword, "var", "Variable");
			tokens.AddToken(TokenType.Keyword, "while", "While");

			tokens.AddToken(TokenType.Symbol, ".", "Access");
			tokens.AddToken(TokenType.Symbol, "->", "DynamicAccess");
			tokens.AddToken(TokenType.Symbol, "~>", "NonVirtualAccess");
			tokens.AddToken(TokenType.Symbol, ":", "TypeSpecifier", "NameSpecifier", "Inheritance", "InitializerList");
			tokens.AddToken(TokenType.Symbol, ",", "ArgumentSeparator");
			tokens.AddToken(TokenType.Symbol, "=>", "RefersTo");
			tokens.AddToken(TokenType.Symbol, "=", "Assignment");
			tokens.AddToken(TokenType.Symbol, "-=", "AssignmentSubtract");
			tokens.AddToken(TokenType.Symbol, "+=", "AssignmentAdd");
			tokens.AddToken(TokenType.Symbol, "/=", "AssignmentDivide");
			tokens.AddToken(TokenType.Symbol, "*=", "AssignmentMultiply");
			tokens.AddToken(TokenType.Symbol, "%=", "AssignmentModulo");
			tokens.AddToken(TokenType.Symbol, "^=", "AssignmentExponent");
			tokens.AddToken(TokenType.Symbol, "<<=", "AssignmentLeftShift");
			tokens.AddToken(TokenType.Symbol, ">>=", "AssignmentRightShift");
			tokens.AddToken(TokenType.Symbol, "$=", "AssignmentBitwiseXor");
			tokens.AddToken(TokenType.Symbol, "|=", "AssignmentBitwiseOr");
			tokens.AddToken(TokenType.Symbol, "&=", "AssignmentBitwiseAnd");
			tokens.AddToken(TokenType.Symbol, "==", "Equality");
			tokens.AddToken(TokenType.Symbol, "!=", "Inequality");
			tokens.AddToken(TokenType.Symbol, "<", "LessThan");
			tokens.AddToken(TokenType.Symbol, "<=", "LessThanOrEqualTo");
			tokens.AddToken(TokenType.Symbol, ">", "GreaterThan");
			tokens.AddToken(TokenType.Symbol, ">=", "GreaterThanOrEqualTo");
			tokens.AddToken(TokenType.Symbol, "-", "Negative", "Subtract");
			tokens.AddToken(TokenType.Symbol, "+", "Positive", "Add");
			tokens.AddToken(TokenType.Symbol, "/", "Divide");
			tokens.AddToken(TokenType.Symbol, "*", "Multiply", "Dereference");
			tokens.AddToken(TokenType.Symbol, "%", "Modulo");
			tokens.AddToken(TokenType.Symbol, "^", "Exponent");
			tokens.AddToken(TokenType.Symbol, "--", "Decrement");
			tokens.AddToken(TokenType.Symbol, "++", "Increment");
			tokens.AddToken(TokenType.Symbol, "<<", "BitshiftLeft");
			tokens.AddToken(TokenType.Symbol, ">>", "BitshiftRight");
			tokens.AddToken(TokenType.Symbol, "$", "BitwiseXor");
			tokens.AddToken(TokenType.Symbol, "|", "BitwiseOr");
			tokens.AddToken(TokenType.Symbol, "&", "BitwiseAnd", "AddressOf");
			tokens.AddToken(TokenType.Symbol, "~", "BitwiseNot");
			tokens.AddToken(TokenType.Symbol, "@", "PropertyDelegate");
			tokens.AddToken(TokenType.Symbol, "||", "LogicalOr");
			tokens.AddToken(TokenType.Symbol, "&&", "LogicalAnd");
			tokens.AddToken(TokenType.Symbol, "!", "LogicalNot");
			tokens.AddToken(TokenType.Symbol, ";", "StatementSeparator");
			tokens.AddToken(TokenType.Symbol, "[", "BeginIndex", "BeginTemplate", "BeginAttribute", "OldBeginInitializer");
			tokens.AddToken(TokenType.Symbol, "]", "EndIndex", "EndTemplate", "EndAttribute", "OldEndInitializer");
			tokens.AddToken(TokenType.Symbol, "(", "BeginFunctionCall", "BeginFunctionParameters", "BeginGroup");
			tokens.AddToken(TokenType.Symbol, ")", "EndFunctionCall", "EndFunctionParameters", "EndGroup");
			tokens.AddToken(TokenType.Symbol, "{", "BeginScope", "BeginInitializer");
			tokens.AddToken(TokenType.Symbol, "}", "EndScope", "EndInitializer");
			tokens.AddToken(TokenType.Symbol, "//", "CommentLine");
			tokens.AddToken(TokenType.Symbol, "/*", "CommentStart");
			tokens.AddToken(TokenType.Symbol, "*/", "CommentEnd");
			

			builder.DoRegex(tokens);

			//builder.SaveNFAGraph();
			builder.SaveDFAGraph();

			builder.SaveDFACode();

			tokens.OutputToEnumeration();
		}
	}
}
