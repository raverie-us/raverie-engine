/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_FORMATTER_HPP
#define ZILCH_FORMATTER_HPP

namespace Zilch
{
  // Lets us define what lines tokens will go on, or if it relies upon a default setting
  // Used mostly for specifying styles of curley braces (eg. K&R style, Allman style, etc)
  namespace LineStyle
  {
    enum Enum
    {
      UseGlobalDefault,
      SameLine,
      NextLine,
      NextLineIndented
    };
  }
  
  // Lets us define whether we indent in certain places, or if it relies upon a default setting
  namespace IndentStyle
  {
    enum Enum
    {
      UseGlobalDefault,
      Indented,
      NotIndented
    };
  }
  
  // Lets us define if we want spaces before, after, or around both sides of a token,
  // or if it relies upon a default setting
  namespace SpaceStyle
  {
    enum Enum
    {
      UseGlobalDefault,
      None,
      Before,
      After,
      BeforeAndAfter
    };
  }

  // Allows us to specify almost every aspect of code formatting
  // The default values used here reflect the official coding standard of Zilch
  class ZeroShared CodeFormat
  {
  public:

    // Constructor
    CodeFormat();

    // If we use tabs for indents or spaces if false (default: false)
    bool IsTabs;

    // How many tabs or spaces we use when indenting (default: 2)
    size_t Identation;

    // Will remove whitespace from empty lines or fill whitespace if set to false (default: false)
    bool StripWhiteSpaceFromEmptyLines;

    // Whether we automatically indent after scopes, unless we override specific cases (default: Indented)
    IndentStyle::Enum IndentGlobalDefault;

    // Whether functions, properties, and fields get indented inside a class (default: UseGlobalDefault)
    IndentStyle::Enum IndentClassContents;

    // Whether values inside an enum are indented (default: UseGlobalDefault)
    IndentStyle::Enum IndentEnumContents;

    // Whether statements inside a function's scope get indented (default: UseGlobalDefault)
    IndentStyle::Enum IndentFunctionContents;

    // Whether statements inside a property's scope get indented (default: UseGlobalDefault)
    IndentStyle::Enum IndentPropertyContents;

    // Whether statements inside a property's get/set scope get indented (default: UseGlobalDefault)
    IndentStyle::Enum IndentGetSetContents;

    // Whether statements inside any other scope (such as an if statement) get indented (default: UseGlobalDefault)
    IndentStyle::Enum IndentScopeContents;

    // Where the braces go for any scope, unless we override specific cases (default: NextLine)
    LineStyle::Enum LineStyleGlobalDefaultScope;

    // Where the braces go for a class scope (default: UseGlobalDefault)
    LineStyle::Enum LineStyleClassScope;

    // Where the braces go for a enum/flags scope (default: UseGlobalDefault)
    LineStyle::Enum LineStyleEnumScope;

    // Where the braces go for a function scope (default: UseGlobalDefault)
    LineStyle::Enum LineStyleFunctionScope;

    // Where the braces go for a property scope (default: UseGlobalDefault)
    LineStyle::Enum LineStylePropertyScope;

    // Where the braces go for a get/set scope inside a property scope (default: UseGlobalDefault)
    LineStyle::Enum LineStyleGetSetScope;

    // Where the braces go for any other type of scope (for example, if statements) (default: UseGlobalDefault)
    LineStyle::Enum LineStyleBlockScope;

    // Where the initializer list colon is placed (default: SameLine)
    LineStyle::Enum LineStyleInitializerList;

    // Whether we put spaces around colons, unless we override specific cases (default: BeforeAndAfter)
    SpaceStyle::Enum SpaceStyleGlobalDefaultColon;

    // Whether we put spaces around the colon in an inheritance list (after the class keyword) (default: UseGlobalDefault)
    SpaceStyle::Enum SpaceStyleInheritanceColon;

    // Whether we put spaces around the colon in an initializer list (default: UseGlobalDefault)
    SpaceStyle::Enum SpaceStyleInitializerListColon;

    // Whether we put spaces around the colon that specifices a type (default: UseGlobalDefault)
    SpaceStyle::Enum SpaceStyleTypeColon;

    // Whether we put spaces around the colon that specifices a type (default: UseGlobalDefault)
    SpaceStyle::Enum SpaceStyleNamedArgumentColon;

    // Whether we put spaces around commas, unless we override specific cases (default: After)
    SpaceStyle::Enum SpaceStyleGlobalDefaultComma;

    // Whether we put spaces around the commas in an inheritance list (after the class keyword) (default: UseGlobalDefault)
    SpaceStyle::Enum SpaceStyleInheritanceComma;

    // Whether we put spaces around the commas in an initializer list (default: UseGlobalDefault)
    SpaceStyle::Enum SpaceStyleInitializerListComma;

    // Whether we put spaces around the commas in a parameter list for a function definition (default: UseGlobalDefault)
    SpaceStyle::Enum SpaceStyleFunctionDefinitionParameterComma;

    // Whether we put spaces around the commas in a parameter list for a function invokation (default: UseGlobalDefault)
    SpaceStyle::Enum SpaceStyleFunctionCallParameterComma;

    // Whether we put spaces around the commas in a parameter list for a template definition (default: UseGlobalDefault)
    SpaceStyle::Enum SpaceStyleTemplateDefinitionParameterComma;

    // Whether we put spaces around the commas in a parameter list for a template instantiation (default: UseGlobalDefault)
    SpaceStyle::Enum SpaceStyleTemplateInstantiationParameterComma;

    // Whether we put spaces around parenthesis, unless we override specific cases (default: None)
    SpaceStyle::Enum SpaceStyleGlobalDefaultParenthesis;
    
    // Whether we put spaces around the beginning parenthesis in a function definition (default: None)
    SpaceStyle::Enum SpaceStyleFunctionDefinitionBeginParenthesis;
    
    // Whether we put spaces around the ending parenthesis in a function definition (default: None)
    SpaceStyle::Enum SpaceStyleFunctionDefinitionEndParenthesis;

    // In certain scenarios this is used to wrap comments (not necessarily used in
    // general formatting, but in other places where we use a code builder)
    size_t CommentWordWrapLength;

    // Whether we put a space after the comment // or /* (default: true)
    bool SpaceAfterComment;
  };

  // The type of scope we emit
  namespace ScopeType
  {
    enum Enum
    {
      Class,
      Enumeration,
      Function,
      Property,
      GetSet,
      Block
    };
  }

  // This class can be used to directly emit Zilch code as text
  // It's mostly a convenience (handles scoping, formatting, spaces, keywords, etc)
  class ZeroShared ZilchCodeBuilder : public StringBuilderExtended
  {
  public:
    // Constructor
    ZilchCodeBuilder();
    
    // Writes out a keyword
    void WriteKeywordOrSymbol(Grammar::Enum token);

    // Writes out a keyword with spacing rules
    void WriteKeywordOrSymbolSpaceStyle(Grammar::Enum token, SpaceStyle::Enum specific, SpaceStyle::Enum globalDefault);

    // Add a scope to the builder, which typically will indent the contents and emit a 'begin scope' token
    void BeginScope(ScopeType::Enum scope);

    // Ends a scope, which typically will unindent and emit an 'end scope' token
    void EndScope();
    
    // Writes out a token
    using StringBuilderExtended::Write;
    void Write(const UserToken& token);

    // Creates a new line, and indents it to the current indentation level
    // Note that if this line is empty, it will be indented all the way, which means
    // we need to do a post-pass over the string in case 'StripWhiteSpaceFromEmptyLines' is set
    void WriteLineIndented();

    // A templated version that forwards writes to the Write function and then writes an indented line (see above)
    template <typename T>
    void WriteLineIndented(const T& value)
    {
      this->Write(value);
      this->WriteLineIndented();
    }

    // We only overwrite this so we can count lines
    // Note: This function is NOT virtual, which means that calling any
    // WriteLine overloads on the base will fail to count lines properly
    using StringBuilderExtended::WriteLine;
    void WriteLine();

    // Creates a new line based on the line style
    void WriteLineStyle(LineStyle::Enum specific, LineStyle::Enum globalDefault);

    // Write a single indent at the latest position
    void WriteIndent();

    // Write a single space
    void WriteSpace();

    // Write out a single line comment
    void WriteSingleLineComment(StringParam text);

    // Trims any ending whitespace or lines
    void TrimEnd();

    // Outputs the final string
    // This also removes trailing whitespace, and modifies space for empty lines
    // depending on the setting 'StripWhiteSpaceFromEmptyLines'
    String ToString();

    // Get the current line that we're on
    size_t GetLine();

  private:

    // Get the preferred line rule
    static LineStyle::Enum GetLineStyle(LineStyle::Enum specific, LineStyle::Enum globalDefault);

    // Get the preferred indent rule
    static IndentStyle::Enum GetIndentStyle(IndentStyle::Enum specific, IndentStyle::Enum globalDefault);

    // Get the preferred space rule
    static SpaceStyle::Enum GetSpaceStyle(SpaceStyle::Enum specific, SpaceStyle::Enum globalDefault);

  public:

    // The format we emit the code in
    CodeFormat Format;

    // The current indentation level we're at. Indentation usually increases as we increase scope
    size_t Indentation;

  private:

    // Styling information for a scope
    class ScopeStyle
    {
    public:
      // Constructor
      ScopeStyle();

      bool BracesIndented;
      bool InnardsIndented;
    };

    // This is the level of scope we're at (an empty array means root / flat level)
    Array<ScopeStyle> Scopes;

    // The current line that we're on
    size_t Line;

    // The current character that we're on
    size_t Character;
    
    // Upon ending a scope, we currently use Trim, which isn't a great
    StringBuilder builder;
  };

  class ZeroShared ScopeLastNode
  {
  public:
    // Constructor
    ScopeLastNode();

    SyntaxNode* LastNode;
    SyntaxNode* AssociatedScope;
  };

  // The context we use to generate code
  class ZeroShared CodeFormatterContext : public WalkerContext<CodeFormatter, CodeFormatterContext>
  {
  public:
    // Constructor
    CodeFormatterContext();

    // The current functions we're generating code for
    FunctionArray FunctionStack;

    // The current classes we're generating code for
    Array<BoundType*> ClassTypeStack;

    // Where we emit formatted code to
    ZilchCodeBuilder Builder;

    // At the top of this stack Contains the last statement we processed
    // for the current scope we're in. Every time we enter a scope, a 
    // statement is pushed on the stack (first null, then the last statement)
    Array<ScopeLastNode> FormatScopes;
  };

  // Responsible for converting a syntax tree back into code
  // This is incredibly useful for auto-formatting Zilch code, or translating into other languages
  class ZeroShared CodeFormatter
  {
  public:

    // Constructor
    CodeFormatter();

    // Formats a syntax tree back into code
    // The syntax tree can be 'unchecked', but must not be invalid (missing syntax types, etc)
    String FormatTree(SyntaxTree& syntaxTree, const CodeFormat& format);

  private:
    
    static bool IsDirectlyWithinScope(SyntaxNode* node);
    static size_t CountAttributes(SyntaxNode* node);

    void FormatCommentsAndLines(SyntaxNode*& node, CodeFormatterContext* context);

    // Handles delimiting of statements that require it
    void FormatStatement(StatementNode*& node, CodeFormatterContext* context);

    // Free (scoped) statements
    void FormatIf(IfNode*& node, CodeFormatterContext* context);
    void FormatLoop(LoopNode*& node, CodeFormatterContext* context);
    void FormatWhile(WhileNode*& node, CodeFormatterContext* context);
    void FormatDoWhile(DoWhileNode*& node, CodeFormatterContext* context);
    void FormatFor(ForNode*& node, CodeFormatterContext* context);
    void FormatForEach(ForEachNode*& node, CodeFormatterContext* context);

    // Delimited statements
    void FormatReturn(ReturnNode*& node, CodeFormatterContext* context);
    void FormatDelete(DeleteNode*& node, CodeFormatterContext* context);
    void FormatBreak(BreakNode*& node, CodeFormatterContext* context);
    void FormatDebugBreak(DebugBreakNode*& node, CodeFormatterContext* context);
    void FormatContinue(ContinueNode*& node, CodeFormatterContext* context);
    void FormatThrow(ThrowNode*& node, CodeFormatterContext* context);

    // Expressions (do not require emitting newline in front)
    void FormatBinaryOperator(BinaryOperatorNode*& node, CodeFormatterContext* context);
    void FormatUnaryOperator(UnaryOperatorNode*& node, CodeFormatterContext* context);
    void FormatTypeCast(TypeCastNode*& node, CodeFormatterContext* context);
    void FormatIndexerCall(IndexerCallNode*& node, CodeFormatterContext* context);
    void FormatFunctionCall(FunctionCallNode*& node, CodeFormatterContext* context);
    void FormatMemberAccess(MemberAccessNode*& node, CodeFormatterContext* context);
    void FormatLocalVariable(LocalVariableNode*& node, CodeFormatterContext* context);
    void FormatParameter(ParameterNode*& node, CodeFormatterContext* context);
    void FormatStaticTypeNode(StaticTypeNode*& node, CodeFormatterContext* context);
    void FormatValue(ValueNode*& node, CodeFormatterContext* context);
    void FormatStringInterpolant(StringInterpolantNode*& node, CodeFormatterContext* context);
    void FormatTypeId(TypeIdNode*& node, CodeFormatterContext* context);
    

    // Format a class back into Zilch format
    void FormatAttributes(NodeList<AttributeNode>& attributes, ZilchCodeBuilder& builder);
    void FormatEnum(EnumNode*& node, CodeFormatterContext* context);
    void FormatEnumValue(EnumValueNode*& node, CodeFormatterContext* context);
    void FormatClass(ClassNode*& node, CodeFormatterContext* context);
    void FormatSendsEvent(SendsEventNode*& node, CodeFormatterContext* context);
    void FormatMemberVariable(MemberVariableNode*& node, CodeFormatterContext* context);
    template <typename NodeType, typename FunctionType>
    void FormatGenericFunctionHelper(NodeType* node, CodeFormatterContext* context, FunctionType emitPostArgs);
    void FormatFunction(FunctionNode*& node, CodeFormatterContext* context);
    void FormatConstructor(ConstructorNode*& node, CodeFormatterContext* context);
    void FormatDestructor(DestructorNode*& node, CodeFormatterContext* context);
    
  private:

    // Store all the walkers
    BranchWalker<CodeFormatter, CodeFormatterContext> Walker;
  };
}

#endif
