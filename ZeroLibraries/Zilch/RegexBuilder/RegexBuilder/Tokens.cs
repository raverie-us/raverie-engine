using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace RegexBuilder
{
	class Token
	{
		public String mRegex;
		public String[] mNames;
		public int mID;
		public TokenType mType;
	}

	enum TokenType
	{
		Variant,
		Symbol,
		Keyword,
		Reserved,
	}

	class Tokens
	{
		public List<Token> mTokens = new List<Token>();

		const String DirectoryBase = @"..\..\..\..\Project\Zilch\";

		public void AddToken(TokenType type, String regex, params String[] names)
		{
			mTokens.Add(new Token() { mNames = names, mRegex = regex, mID = mTokens.Count, mType = type });
		}

		private void OutputHpp()
		{
			StringBuilder output = new StringBuilder();

			output.AppendLine(@"/**************************************************************\");
			output.AppendLine(@"* Author: Trevor Sundberg");
			output.AppendLine(@"\**************************************************************/");
			output.AppendLine(@"");
			output.AppendLine(@"// Include protection");
			output.AppendLine(@"#pragma once");
			output.AppendLine(@"#ifndef ZILCH_GRAMMAR_CONSTANTS_HPP");
			output.AppendLine(@"#define ZILCH_GRAMMAR_CONSTANTS_HPP");
			output.AppendLine(@"");
			output.AppendLine(@"namespace Zilch");
			output.AppendLine(@"{");
			output.AppendLine(@"  // All the symbol constants in the language");
			output.AppendLine(@"  namespace Grammar");
			output.AppendLine(@"  {");
			output.AppendLine(@"    enum Enum");
			output.AppendLine(@"    {");

			int enumCounter = -1;

			foreach (Token token in mTokens)
			{
				foreach (String name in token.mNames)
				{
					output.AppendLine(@"      " + name + " = " + enumCounter.ToString() + ",");
				}

				++enumCounter;
			}

			output.AppendLine(@"      SymbolCount");
			output.AppendLine(@"    };");
			output.AppendLine(@"");
			output.AppendLine(@"    // Gets the name of a given grammar constant");
			output.AppendLine(@"    const String& GetName(Grammar::Enum value);");
			output.AppendLine(@"");
			output.AppendLine(@"    // Gets the keyword or symbol associated with a grammar constant, or returns the string 'Invalid'");
			output.AppendLine(@"    const String& GetKeywordOrSymbol(Grammar::Enum value);");
			output.AppendLine(@"");
			output.AppendLine(@"    // Get a list of keywords used by Zilch (typically provided for syntax highlighting)");
			output.AppendLine(@"    // If you need a list of words separated by spaces, you can use Zilch::JoinStrings");
			output.AppendLine(@"    const Array<String>& GetUsedKeywords();");
			output.AppendLine(@"");
			output.AppendLine(@"    // Special keywords that only exist in certain contexts, (eg this, value...)");
			output.AppendLine(@"    // If you need a list of words separated by spaces, you can use Zilch::JoinStrings");
			output.AppendLine(@"    const Array<String>& GetSpecialKeywords();");
			output.AppendLine(@"");
			output.AppendLine(@"    // Get a list of keywords reserved by Zilch (these may not be used, but do nothing)");
			output.AppendLine(@"    const Array<String>& GetReservedKeywords();");
			output.AppendLine(@"  }");
			output.AppendLine(@"}");
			output.AppendLine(@"");
			output.AppendLine(@"// End header protection");
			output.AppendLine(@"#endif");

			String code = output.ToString();

			File.WriteAllText(DirectoryBase + @"GrammarConstants.hpp", code);
		}

		private void OutputCpp()
		{
			StringBuilder output = new StringBuilder();

			output.AppendLine(@"/**************************************************************\");
			output.AppendLine(@"* Author: Trevor Sundberg");
			output.AppendLine(@"\**************************************************************/");
			output.AppendLine(@"");
			output.AppendLine(@"// Includes");
			output.AppendLine(@"#include ""Common.hpp""");
			output.AppendLine(@"#include ""GrammarConstants.hpp""");
			output.AppendLine(@"");
			output.AppendLine(@"namespace Zilch");
			output.AppendLine(@"{");

			output.AppendLine(@"  // The symbols or keywords we use, if available for given grammar symbols");
			output.AppendLine(@"  static String KeywordsOrSymbols[] = ");
			output.AppendLine(@"  {");

			foreach (Token token in mTokens)
			{
				if (token.mType != TokenType.Variant)
				{
					output.AppendLine("    \"" + token .mRegex + "\",");
				}
				else
				{
					output.AppendLine("    \"" + token.mNames[0] + "\",");
				}
			}
			output.AppendLine(@"  };");
			output.AppendLine(@"");

			output.AppendLine(@"  // The human readable names we give to every symbol");
			output.AppendLine(@"  static String Names[] = ");
			output.AppendLine(@"  {");

			foreach (Token token in mTokens)
			{
				StringBuilder result = new StringBuilder();

				foreach (String name in token.mNames)
				{
					result.Append(name);

					if (name != token.mNames[token.mNames.Length - 1])
					{
						result.Append(" / ");
					}
				}

				output.AppendLine("    \"" + result.ToString() + "\",");
			}
			output.AppendLine(@"  };");
			output.AppendLine(@"");
			output.AppendLine(@"  //***************************************************************************");
			output.AppendLine(@"  const String& Grammar::GetName(Grammar::Enum value)");
			output.AppendLine(@"  {");
			output.AppendLine(@"    // We add one because of the 'Invalid' index");
			output.AppendLine(@"    return Names[value + 1];");
			output.AppendLine(@"  }");
			output.AppendLine(@"");
			output.AppendLine(@"  //***************************************************************************");
			output.AppendLine(@"  const String& Grammar::GetKeywordOrSymbol(Grammar::Enum value)");
			output.AppendLine(@"  {");
			output.AppendLine(@"    // We add one because of the 'Invalid' index");
			output.AppendLine(@"    return KeywordsOrSymbols[value + 1];");
			output.AppendLine(@"  }");
			output.AppendLine(@"");
			output.AppendLine(@"  //***************************************************************************");
			output.AppendLine(@"  const Array<String>& Grammar::GetUsedKeywords()");
			output.AppendLine(@"  {");
			output.AppendLine(@"    static Array<String> results;");
			output.AppendLine(@"    if (results.empty())");
			output.AppendLine(@"    {");

			foreach (Token token in mTokens)
			{
				if (token.mType == TokenType.Keyword)
				{
					output.AppendLine(@"      results.push_back(""" + token.mRegex + @""");");
				}
			}

			output.AppendLine(@"    }");
			output.AppendLine(@"    return results;");
			output.AppendLine(@"  }");
			output.AppendLine(@"");
			output.AppendLine(@"  //***************************************************************************");
			output.AppendLine(@"  const Array<String>& Grammar::GetSpecialKeywords()");
			output.AppendLine(@"  {");
			output.AppendLine(@"    static Array<String> results;");
			output.AppendLine(@"    if (results.empty())");
			output.AppendLine(@"    {");
			output.AppendLine(@"      results.push_back(""this"");");
			output.AppendLine(@"      results.push_back(""value"");");
			output.AppendLine(@"      results.push_back(""event"");");
			output.AppendLine(@"    }");
			output.AppendLine(@"    return results;");
			output.AppendLine(@"  }");
			output.AppendLine(@"");
			output.AppendLine(@"  //***************************************************************************");
			output.AppendLine(@"  const Array<String>& Grammar::GetReservedKeywords()");
			output.AppendLine(@"  {");
			output.AppendLine(@"    static Array<String> results;");
			output.AppendLine(@"    if (results.empty())");
			output.AppendLine(@"    {");


			foreach (Token token in mTokens)
			{
				if (token.mType == TokenType.Reserved)
				{
					output.AppendLine(@"      results.push_back(""" + token.mRegex + @""");");
				}
			}

			output.AppendLine(@"    }");
			output.AppendLine(@"    return results;");
			output.AppendLine(@"  }");
			output.AppendLine(@"}");

			String code = output.ToString();
			File.WriteAllText(DirectoryBase + @"GrammarConstants.cpp", code);
		}

		public void OutputToEnumeration()
		{
			Token[] specialTokens = new Token[]
			{
				new Token() { mNames = new string[] { @"Invalid" } },
				new Token() { mNames = new string[] { @"End" } },
				new Token() { mNames = new string[] { @"Error" } },
				new Token() { mNames = new string[] { @"Whitespace" } },
				new Token() { mNames = new string[] { @"UpperIdentifier" } },
				new Token() { mNames = new string[] { @"LowerIdentifier" } },
				new Token() { mNames = new string[] { @"IntegerLiteral" } },
				new Token() { mNames = new string[] { @"DoubleIntegerLiteral" } },
				new Token() { mNames = new string[] { @"RealLiteral" } },
				new Token() { mNames = new string[] { @"DoubleRealLiteral" } },
				new Token() { mNames = new string[] { @"CharacterLiteral" } },
				new Token() { mNames = new string[] { @"StringLiteral" } },
				new Token() { mNames = new string[] { @"BeginStringInterpolate" } },
				new Token() { mNames = new string[] { @"EndStringInterpolate" } },
				new Token() { mNames = new string[] { @"EndBeginStringInterpolate" } },
			};

			mTokens.InsertRange(0, specialTokens);

			this.OutputCpp();
			this.OutputHpp();
		}
	}
}
