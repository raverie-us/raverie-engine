/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_PARSER_HPP
#define ZILCH_PARSER_HPP

namespace Zilch
{
  // This class implements a recursive descent parser that parses
  // through the token stream that we get from the tokenizer
  class ZeroShared Parser
  {
  public:

    // Constructor
    Parser(Project& project);

    // Parses all scripts in a project into an syntax tree (with classes, functions, members, etc)
    void ParseIntoTree(const Array<UserToken>& tokens, SyntaxTree& syntaxTree, EvaluationMode::Enum evaluation);

    // Parses a single expression in the context of a function (evaluation of local variables, etc)
    void ParseExpressionInFunctionAndClass(const Array<UserToken>& expression, const Array<UserToken>& function, const Array<UserToken>& classTokensWithoutFunction, SyntaxTree& syntaxTree);

    // From a token stream, just attempt to parse a single type
    SyntaxType* ParseType(const Array<UserToken>& type);

    // Checks if a token either has zero elements, or if the only element is the End/Eof element
    static bool IsTokenStreamEmpty(const Array<UserToken>& tokens);

    // Tests if a list of attributes Contains a particular attribute by name
    static AttributeNode* GetAttribute(NodeList<AttributeNode>& attributes, StringParam name);

  private:

    // Type-defines
    typedef ExpressionNode* (Parser::*ExpressionFn)();

    // Print out an error message corresponding to the current token
    void ErrorHere(ErrorCode::Enum errorCode, ...);

    // Print out an error message corresponding to the current token
    void ErrorHereArgs(ErrorCode::Enum errorCode, va_list argList);

    // Print out an error message corresponding to the current token (with extra context if needed)
    void ErrorHereArgs(ErrorCode::Enum errorCode, StringParam extra, va_list argList);

    // Set the starting line and character of a syntax node
    void SetNodeLocationStartHere(SyntaxNode* node);

    // Set the primary line and character of a syntax node
    void SetNodeLocationPrimaryHere(SyntaxNode* node);

    // Set the ending line and character of a syntax node
    void SetNodeLocationEndHere(SyntaxNode* node);

    // Set the starting line and character of a syntax node (at the last saved token)
    void SetNodeLocationStartToLastSave(SyntaxNode* node);

    // Set the starting line and character of a syntax node (at the last saved token)
    void SetNodeLocationPrimaryToLastSave(SyntaxNode* node);

    // Set the ending line and character of a syntax node (at the last saved token)
    void SetNodeLocationEndToLastSave(SyntaxNode* node);

    // Set the starting line and character of a syntax node to a given token's position
    static void SetNodeLocationStartToToken(SyntaxNode* node, const UserToken& token);

    // Set the starting line and character of a syntax node to a given token's position
    static void SetNodeLocationPrimaryToToken(SyntaxNode* node, const UserToken& token);

    // Set the ending line and character of a syntax node to a given token's position
    static void SetNodeLocationEndToToken(SyntaxNode* node, const UserToken& token);

    // Saves the tokens position onto the stack
    void SaveTokenPosition();

    // Recalls the token position by using the value on the top of the stack
    void RecallTokenPosition();

    // Accepts the current token position as the new token position, and removes the saved version from the stack
    void AcceptTokenPosition();

    // Attempts to accept any one of the given tokens, and outputs the one that matches (also moves the token index ahead by one)
    bool AcceptAnyArgs(size_t parameters, const UserToken** out_token, va_list vl);

    // Attempts to accept any one of the given tokens, and outputs the one that matches (also moves the token index ahead by one)
    bool AcceptAny(size_t parameters, const UserToken** out_token, ...);

    // Accepts the tokens in the order that they're given (moves the token index ahead by the number of tokens passed in, if they all match)
    bool Accept(size_t parameters, ...);

    // Accepts the tokens in the order that they're given and returns values out through parameters
    bool AcceptAndRetrieve(size_t parameters, ...);

    // Expects a given grammar constant, and otherwise produces an error (variadic for the error context)
    bool Expect(Grammar::Enum grammarConstant, ErrorCode::Enum errorCode, ...);

    // Expects a given grammar constant, and otherwise produces an error (variadic for the error context)
    bool ExpectAndRetrieve(Grammar::Enum grammarConstant, const UserToken*& outToken, ErrorCode::Enum errorCode, ...);

    // Expects a given grammar constant, and otherwise produces an error (variadic for the error context)
    bool ExpectAndRetrieveArgs(Grammar::Enum grammarConstant, const UserToken*& outToken, ErrorCode::Enum errorCode, va_list vl);

    // A helper function to read the contents of a delgate syntax type
    bool ReadDelegateTypeContents(DelegateSyntaxType* delegateSyntaxType);

    // Read a named type for the typing system
    BoundSyntaxType* ReadBoundTypeInfo();

    // Read a type for the typing system
    SyntaxType* ReadTypeInfo();

    // Typically only used in tolerant mode, this will skip tokens until we find the ending scope
    // It will also properly count scopes up and down as it looks for the end
    // Returns true if it find the end (and will advance the token), or false and the token will not move
    bool MoveToScopeEnd();

    // Find the nearest scope to a given code location
    // Generally this is only used for auto-complete
    static ScopeNode* FindNearestScope(SyntaxNode* root, const CodeLocation& location);

    // Parse an attribute definition
    void ParseAllOptionalAttributes();

    // Parse a single attribute (not a group)
    bool ParseOneOptionalAttribute();

    // Apply the last attribute to a node
    void AttachLastAttributeToNode(NodeList<AttributeNode>& attributes);

    // Accept a type specifier (return type for a function)
    // Returns true if it parses successfully, false otherwise (note that no type specifier will return true!)
    bool AcceptOptionalTypeSpecifier(SyntaxType*& outSyntaxType, ErrorCode::Enum notFound, ...);

    // Accept a type specifier (return type for a function)
    // Returns true if it parses successfully, false otherwise (note that no type specifier will return true!)
    bool AcceptOptionalTypeSpecifierArgs(SyntaxType*& outSyntaxType, ErrorCode::Enum notFound, va_list args);

    // Expect an argument list
    bool ExpectArgumentList(GenericFunctionNode* node, StringParam functionName, bool mustBeEmpty);

    // Parse a scope body
    bool ExpectScopeBody(GenericFunctionNode* node, StringParam functionName);

    // Parse a variable definition
    LocalVariableNode* LocalVariable(bool initialized = true);

    // Parse a variable definition
    MemberVariableNode* MemberVariable();

    // Parse a class definition
    ClassNode* Class();

    // Parse an enum definition
    EnumNode* Enum();

    // Parse an enum value (an integral constant with a name)
    EnumValueNode* EnumValue();

    // Parse a specialized function (code reuse)
    template <typename FunctionNodeType>
    FunctionNodeType* SpecializedFunction
    (
      Grammar::Enum type,
      String functionName,
      bool (Parser::*postArgs)(FunctionNodeType* node)
    );

    // Parse a function definition
    FunctionNode* Function();

    // Generate the standard function node for a get set
    FunctionNode* GenerateGetSetFunctionNode(MemberVariableNode* variable, bool isGet);

    // Parse a get/set function definition
    FunctionNode* GetSetFunctionBody(MemberVariableNode* variable, bool isGet);

    // Parse a constructor definition
    ConstructorNode* Constructor();
    bool ConstructorInitializerList(ConstructorNode* node);

    // Parse a destructor definition
    DestructorNode* Destructor();

    // Parse a sends statement
    SendsEventNode* SendsEvent();

    // Parse a function parameter
    ParameterNode* Parameter();

    // A binary operator helper for parsing expressions (with right to left associativity)
    ExpressionNode* BinaryOperatorRightToLeftAssociative(ExpressionFn currentPrecedence, ExpressionFn nextPrecedence, int parameters, ...);

    // A binary operator helper for parsing expressions (with left to right associativity)
    ExpressionNode* BinaryOperatorLeftToRightAssociative(ExpressionFn nextPrecedence, int parameters, ...);

    // Parse an expression (including all precedent levels of operators, lower number = lower precedence)
    ExpressionNode* Expression();
    ExpressionNode* Expression00();
    ExpressionNode* Expression01();
    ExpressionNode* Expression02();
    ExpressionNode* Expression03();
    ExpressionNode* Expression04();
    ExpressionNode* Expression05();
    ExpressionNode* Expression06();
    ExpressionNode* Expression07();
    ExpressionNode* Expression08();
    ExpressionNode* Expression09();
    ExpressionNode* Expression10();
    ExpressionNode* Expression11();
    ExpressionNode* Expression12();
    ExpressionNode* Expression13();
    ExpressionNode* Expression14();

    // The post-expression takes care of right hand side operataors whose operands arent exactly a "single" expression
    ExpressionNode* PostExpression(ExpressionNode* leftOperand);

    // Parse an indexer call
    IndexerCallNode* IndexerCall(ExpressionNode* leftOperand);

    // Parse a function call
    FunctionCallNode* FunctionCall(ExpressionNode* leftOperand);

    // Parse a member access
    MemberAccessNode* MemberAccess(ExpressionNode* leftOperand);

    // Parse the expression initializer { } which can initialize members and add to containers
    ExpressionNode* ExpressionInitializer(ExpressionNode* leftOperand);

    // Parse a delete statement
    StatementNode* Delete();

    // Parse a return statement
    StatementNode* Return();

    // Parse a break statement
    StatementNode* Break();

    // Parse a debug breakpoint statement
    StatementNode* DebugBreak();

    // Parse a continue statement
    StatementNode* Continue();

    // Parse a throw statement
    StatementNode* Throw();

    // Parse a statement
    StatementNode* Statement(bool optionalDelimiter = false);

    // Parse a delimited statement
    StatementNode* DelimitedStatement();

    // Parse a free statement
    StatementNode* FreeStatement();

    // Parse a scope statement
    StatementNode* Scope();

    // Parse a timeout statement
    StatementNode* Timeout();

    // Parse an if statement
    IfRootNode* If();

    // Parse an if statement's body (nulls out the condition if it attaches it)
    void IfBody(ExpressionNode*& condition, IfRootNode* root);

    // Parse an if statement's condition
    ExpressionNode* IfCondition();

    // Parse an else statment (with a possible condition)
    void Else(IfRootNode* root);

    // Parse a scope (with statements) or a single statement
    bool ExpectScopedStatements(NodeList<StatementNode>& statements, Grammar::Enum parentKeyword);

    // Parse a for statement
    StatementNode* For();

    // Parse a foreach statement
    StatementNode* ForEach();

    // Parse an while statement
    StatementNode* While();

    // Parse an do-while statement
    StatementNode* DoWhile();

    // Parse a loop statement
    StatementNode* Loop();

    // Read a creation call (like "new" or "local") or just a static type name
    StaticTypeNode* StaticTypeOrCreationCall();

    // Read a typeid expression (which returns type information)
    ExpressionNode* TypeId();

    // Read a memberid expression (which returns property information)
    ExpressionNode* MemberId();

    // Create a string literal value node (always sets the token directly to be a string literal)
    ValueNode* CreateStringLiteral(const UserToken* token);

    // Parse a string interpolant
    StringInterpolantNode* StringInterpolant();

    // Parse a value
    ExpressionNode* Value();

    // Read attributes for static, virtual, and overriding and apply them to a node
    // Expects that the node has the IsStatic and Virtualized members
    template <typename Node>
    void ApplyVirtualStaticExtensionAttributes(Node* node);

  private:

    // The last attribute we parsed, which will be attached to whatever node follows it
    NodeList<AttributeNode> LastAttributes;

    // Store a reference to the error handler
    CompilationErrors& Errors;

    // Store a pointer back to the project that we're created from
    Project* ParentProject;

    // The tokenizer we'll use that stores the input stream of tokens
    const Array<UserToken>* TokenStream;

    // This stack maintains any saved token positions (makes it easy to recall if necessary)
    PodArray<size_t> TokenPositions;

    // The index of the token we're currently parsing
    size_t TokenIndex;

    // Not copyable
    ZilchNoCopy(Parser);
  };
}

#endif
