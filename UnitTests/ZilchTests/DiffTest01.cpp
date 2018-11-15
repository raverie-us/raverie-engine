#include "Precompiled.hpp"

using namespace Zero;

void DiffTest01(Array<Diff>& diffs)
{
  String input = Zero::ReadFileIntoString("DiffInput01.txt");

  GrammarSet<Character> tokenGrammar;
  GrammarRule<Character>& TokenStart     = tokenGrammar["Start"];
  GrammarRule<Character>& Enum           = tokenGrammar["Enum"];
  GrammarRule<Character>& Identifier     = tokenGrammar["Identifier"];
  GrammarRule<Character>& Float          = tokenGrammar["Float"];
  GrammarRule<Character>& Integer        = tokenGrammar["Integer"];
  GrammarRule<Character>& Hex            = tokenGrammar["Hex"];
  GrammarRule<Character>& Whitespace     = tokenGrammar["Whitespace"];
  GrammarRule<Character>& StringLiteral  = tokenGrammar["StringLiteral"];
  GrammarRule<Character>& OpenBracket    = tokenGrammar["OpenBracket"];
  GrammarRule<Character>& CloseBracket   = tokenGrammar["CloseBracket"];
  GrammarRule<Character>& OpenCurley     = tokenGrammar["OpenCurley"];
  GrammarRule<Character>& CloseCurley    = tokenGrammar["CloseCurley"];
  GrammarRule<Character>& Assignment     = tokenGrammar["Assignment"];
  GrammarRule<Character>& Comma          = tokenGrammar["Comma"];
  GrammarRule<Character>& Colon          = tokenGrammar["Colon"];
  GrammarRule<Character>& True           = tokenGrammar["True"];
  GrammarRule<Character>& False          = tokenGrammar["False"];
  GrammarRule<Character>& Var            = tokenGrammar["Var"];
  GrammarRule<Character>& Test           = tokenGrammar["Test"];

  tokenGrammar.mIgnore.Insert(&Whitespace);
  tokenGrammar.mKeywords["true" ] = &True;
  tokenGrammar.mKeywords["false"] = &False;
  tokenGrammar.mKeywords["var"  ] = &Var;
  tokenGrammar.mKeywords["test" ] = &Test;

  TokenStart    |= Enum | Float | Whitespace | StringLiteral | OpenBracket | CloseBracket | OpenCurley | CloseCurley | Assignment | Comma | Colon;
  Enum          |= Identifier << T(".") << T("a-zA-Z_") << *T("a-zA-Z_0-9");
  Identifier    |= T("a-zA-Z_") << *T("a-zA-Z_0-9");
  Float         |= ~Integer << T(".") << +T("0-9") << ~T("f");
  Integer       |= ~T("-") << (T("0") << (Hex | *T("0-9")) | T("1-9") << *T("0-9"));
  Hex           |= T("xX") << +T("0-9a-fA-F") << T();
  Whitespace    |= +T(" \t\r\n\v\f");
  StringLiteral |= T("\"") << *(T("^\"\\") | T("\\") << T("^")) << T("\"");
  OpenBracket   |= T("[");
  CloseBracket  |= T("]");
  OpenCurley    |= T("{");
  CloseCurley   |= T("}");
  Assignment    |= T("=");
  Comma         |= T(",");
  Colon         |= T(":");

  GrammarSet<Token> parserGrammar;
  GrammarRule<Token>& ParserStart  = parserGrammar["Root"];
  GrammarRule<Token>& Object       = parserGrammar["Object"];
  GrammarRule<Token>& Attribute    = parserGrammar["Attribute"];
  GrammarRule<Token>& Property     = parserGrammar["Property"];
  GrammarRule<Token>& Value        = parserGrammar["Value"];

  ParserStart |= Attribute << Object;
  Object      |= P("Name", P(Identifier)) << *Attribute << P(OpenCurley) << *((Property | Value) << ~P(Comma)) << P(CloseCurley);
  Attribute   |= P(OpenBracket) << P(Identifier) << ~(P(Colon) << Value) << P(CloseBracket);
  Property    |= P(Var) << P("Name", P(Identifier)) << P(Assignment) << Value;
  Value       |= P(Integer) | P(Float) | P(Hex) | P(StringLiteral) | P(Enum) | P(True) | P(False) | Object;


  ParseTreeBuilder<Character> tokenTreeBuilder;
  tokenTreeBuilder.CreateRootNode();
  ParseTreeBuilder<Token> parserTreeBuilder;

  TokenStream<ParseTreeBuilder<Character> > stream;
  stream.mRange = TokenRange<ParseTreeBuilder<Character> >(tokenGrammar, TokenStart, input, true);
  stream.mRange.mParser.mParseHandler = &tokenTreeBuilder;

  RecursiveDescentParser<Token, TokenStream<ParseTreeBuilder<Character> >, ParseTreeBuilder<Token> > parser;
  parser.mParseHandler = &parserTreeBuilder;
  parser.mStartRule = &ParserStart;
  parser.mStream = &stream;
  parser.mDebug = true;

  parser.Parse();

  String diffTokenDebugSource = Zero::ReadFileIntoString("DiffTokenDebug01.txt");
  String diffTokenDebugOutput = stream.mRange.mParser.mDebugOutput.ToString();

  String diffTokenTreeSource = Zero::ReadFileIntoString("DiffTokenTree01.txt");
  String diffTokenTreeOutput = tokenTreeBuilder.mTree->GetDebugRepresentation();

  String diffParserDebugSource = Zero::ReadFileIntoString("DiffParserDebug01.txt");
  String diffParserDebugOutput = parser.mDebugOutput.ToString();

  String diffParserTreeSource = Zero::ReadFileIntoString("DiffParserTree01.txt");
  String diffParserTreeOutput = parserTreeBuilder.mTree->GetDebugRepresentation();

  //String graph = parserTreeBuilder.mTree->GetGraphRepresentation();
  //Zero::WriteToFile("C:\\Sandbox\\Graph.gv", (const byte*)graph.c_str(), graph.size());

  //system("C:\\Progra~2\\Graphviz2.38\\bin\\dot.exe -Tpng \"C:\\Sandbox\\Graph.gv\" > \"C:\\Sandbox\\Graph.png\"");
  //system("C:\\Sandbox\\Graph.png");

  //Zero::WriteToFile("DiffTokenDebug01.txt",  (const byte*)diffTokenDebugOutput.c_str(),  diffTokenDebugOutput.size());
  //Zero::WriteToFile("DiffTokenTree01.txt",   (const byte*)diffTokenTreeOutput.c_str(),   diffTokenTreeOutput.size());
  //Zero::WriteToFile("DiffParserDebug01.txt", (const byte*)diffParserDebugOutput.c_str(), diffParserDebugOutput.size());
  //Zero::WriteToFile("DiffParserTree01.txt",  (const byte*)diffParserTreeOutput.c_str(),  diffParserTreeOutput.size());

  diffs.PushBack(Diff(diffTokenDebugSource,  diffTokenDebugOutput));
  diffs.PushBack(Diff(diffTokenTreeSource,   diffTokenTreeOutput));
  diffs.PushBack(Diff(diffParserDebugSource, diffParserDebugOutput));
  diffs.PushBack(Diff(diffParserTreeSource,  diffParserTreeOutput));
}
