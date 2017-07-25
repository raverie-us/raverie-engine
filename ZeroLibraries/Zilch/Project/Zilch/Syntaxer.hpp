/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_SYNTAXER_HPP
#define ZILCH_SYNTAXER_HPP

namespace Zilch
{
  // A context we use to collect all class types
  class ZeroShared ClassContext : public WalkerContext<Syntaxer, ClassContext>
  {
  public:
    // Store all the classes we're compiling
    Array<BoundType*> AllClasses;
  };

  // A context used for walking everything (classes, functions, expressions, etc)
  class ZeroShared TypingContext : public WalkerContext<Syntaxer, TypingContext>
  {
  public:
    // Store the current function that we're building
    FunctionArray FunctionStack;

    // Store the current type that we're building
    Array<BoundType*> ClassTypeStack;

    // In general it should not be required to clear this context, however, because of
    // tolerant mode it is possible to end a tree walk and still have data in the context stacks
    // If we are not in tolerant mode, this will assert if anything is leftover
    void Clear(bool tolerantMode);
  };

  // When we walk dependencies, this is the state of a type used in dependencies
  namespace DependencyState
  {
    enum Enum
    {
      Undetermined,
      BeingDetermined,
      Completed
    };
  }

  // This class implements a recursive descent parser that parses
  // through the token stream that we get from the tokenizer
  class ZeroShared Syntaxer
  {
  public:

    // Friends
    friend class CodeGenerator;
    friend class Overload;

    // Constructor
    Syntaxer(CompilationErrors& errors);

    // Destructor
    ~Syntaxer();

    // Perform type collecting, assigning, and checking on the tree or an individual expression
    void ApplyToTree
    (
      SyntaxTree& syntaxTree,
      LibraryBuilder& builder,
      Project& project,
      const Module& dependencies
    );

    // Retrieves/resolves a type if it exists
    Type* RetrieveType(SyntaxType* syntaxType, const CodeLocation& location, const Module& dependencies);

  private:

    // Walk through all dependenices, collect all their types and store them in a map
    void PopulateDependencies();

    void FindDependencyCycles(BoundType* type, HashMap<BoundType*, DependencyState::Enum>& dependencies, const CodeLocation& location);

    // Print out an error message corresponding to a given node
    void ErrorAt(SyntaxNode* node, ErrorCode::Enum errorCode, ...);

    // Print out an error message corresponding to a given node
    void ErrorAtArgs(SyntaxNode* node, ErrorCode::Enum errorCode, va_list argList);

    // Replace a populated array of syntax types
    void ReplaceTypes(SyntaxTypes& types, Array<const UserToken*>& names, const BoundSyntaxType* instanceType, const CodeLocation& location);

    // Recursively perform a templated replacement of certain parameters
    void PerformTemplateReplacement(SyntaxType* type, Array<const UserToken*>& names, const BoundSyntaxType* instanceType);
    void PerformTemplateReplacement(SyntaxNode* node, Array<const UserToken*>& names, const BoundSyntaxType* instanceType);

    // Retrieves a type by name (for cases where you expect the type to exist
    // The 'classWeAreInsideOf' is only provided to give extra error information to the users for implicit members (not necessary)
    BoundType* RetrieveBoundType(BoundSyntaxType* type, const CodeLocation& location, BoundType* classWeAreInsideOf = nullptr);

    // Retrieves a type if it exists (or potentially creates a type if it's a qualified version of a type that exists)
    Type* RetrieveType(SyntaxType* syntaxType, const CodeLocation& location);

    // Make sure we don't have another class/struct/enum of the same name
    // Sets WasError if another type exists of the same name
    void PreventDuplicateTypeNames(StringParam name, const CodeLocation& location);

    // Make sure we don't have another member of the same name (an exception is made for functions due to overloading)
    // Sets WasError if another type exists of the same name
    void PreventDuplicateMemberNames(BoundType* type, StringParam memberName, const CodeLocation& location, bool isStatic, bool isFunction);

    // Make sure we don't have another member of the same name on a base class (an exception is made for functions due to overloading)
    // Sets WasError if another type exists of the same name
    void PreventNameHiddenBaseMembers(Member* member);

    // Collect a class type
    void CollectClass(ClassNode*& node, ClassContext* context);

    // Collect an enum type
    void CollectEnum(EnumNode*& node, ClassContext* context);

    // Collect all template instantiations
    void CollectTemplateInstantiations(SyntaxNode*& node, ClassContext* context);

    // Given a list of syntax types, attempt to instantiate any referenced templates from them (recursive)
    void InstantiateTemplatesFromSyntaxTypes(SyntaxTypes& types, ClassContext* context, const CodeLocation& location);
    
    // Setup a class instance for a given class node (called by CollectClass, and CollectTemplateInstantiations)
    void SetupClassInstance(ClassNode* node, ClassContext* context);

    // Setup the location for a function, as well as the this variable
    void SetupFunctionLocation(Function* function, const CodeLocation& location, const CodeLocation& nameLocation);

    // Convert a value to a constant
    Constant ValueNodeToConstant(ValueNode* node);

    // Read all the attributes from an attribute node list into an array of attributes
    void ReadAttributes(SyntaxNode* parentNode, NodeList<AttributeNode>& nodes, Array<Attribute>& attributesOut);

    // Setup the inheritance chain (including interfaces and base class)
    void CollectClassInheritance(ClassNode*& node, TypingContext* context);

    // Collect all instances of send events declarations
    void CollectSendsEvents(SendsEventNode*& node, TypingContext* context);

    // Setup the inheritance chain for enums (only one parent, no interfaces)
    void CollectEnumInheritance(EnumNode*& node, TypingContext* context);

    // Collect/setup all the functions (the owner can be passed in for extension functions)
    // Otherwise the owner is obtained from the typing context stack
    void SetupGenericFunction(GenericFunctionNode* node, TypingContext* context, const UserToken& name, FunctionOptions::Enum options, Type* returnType, BoundType* owner = nullptr);

    // Collect all the constructors
    void CollectConstructor(ConstructorNode*& node, TypingContext* context);

    // Collect the destructor (if it exists)
    void CollectDestructor(DestructorNode*& node, TypingContext* context);

    // Collect all the functions
    void CollectFunction(FunctionNode*& node, TypingContext* context);

    // Collect the member variables
    void CollectMemberVariableAndProperty(MemberVariableNode*& node, TypingContext* context);

    // Collect the member properties
    void CollectPropertyGetSet(MemberVariableNode*& node, TypingContext* context);

    // Store the class in the code context
    void PushClass(ClassNode*& node, TypingContext* context);

    // Process all statements in a scope
    void ProcessScopeStatements(ScopeNode* node, TypingContext* context);

    // Helper functions
    template <typename FunctionNodeType>
    void PushFunctionHelper
    (
      FunctionNodeType* node,
      TypingContext* context,
      void (Syntaxer::*postArgs)(FunctionNodeType* node)
    );

    // Store the function in the code context
    void PushFunction(GenericFunctionNode*& node, TypingContext* context);

    // Store the constructor function in the code context
    void CheckInitializerList(ConstructorNode* node);
    void PushConstructor(ConstructorNode*& node, TypingContext* context);

    // Assign a type to any child value node
    void DecorateValue(ValueNode*& node, TypingContext* context);

    // Let the string interpolant know it's of a string type
    void DecorateStringInterpolant(StringInterpolantNode*& node, TypingContext* context);
    
    // Assign a type to the initializer
    void DecorateInitializer(InitializerNode*& node, TypingContext* context);

    // Handle checking that the creation call is valid for its type (also infers new/local if not provided)
    void DecorateStaticTypeOrCreationCall(StaticTypeNode*& node, TypingContext* context);

    // Handle initializing a created object or adding to a container (technically this can come after any expression)
    void DecorateExpressionInitializer(ExpressionInitializerNode*& node, TypingContext* context);
    
    // A multi-expression is a single expression that runs multiple expressions and then yields the results of one of them
    // This forwards the type of the yielded expresion (and walks them all)
    void DecorateMultiExpression(MultiExpressionNode*& node, TypingContext* context);

    // Make sure typeid results in a type (of the most derived type)
    void DecorateTypeId(TypeIdNode*& node, TypingContext* context);

    // Make sure member results in a property (of the most derived type)
    void DecorateMemberId(MemberIdNode*& node, TypingContext* context);

    // Assign member variable types (if required), and check that the initialization type matches
    void CheckMemberVariable(MemberVariableNode*& node, TypingContext* context);

    // Assign local variable types (if required), and check that the initialization type matches
    void CheckLocalVariable(LocalVariableNode*& node, TypingContext* context);

    // Check the type of an delete statement's expression
    void CheckDelete(DeleteNode*& node, TypingContext* context);

    // Check that the type of a throw statement is an exception type (inherits from)
    void CheckThrow(ThrowNode*& node, TypingContext* context);

    // Check the condition and statements in a conditional loop
    void CheckConditionalLoop(ConditionalLoopNode* node, TypingContext* context);

    // Check the type of an while statement's condition
    void CheckWhile(WhileNode*& node, TypingContext* context);

    // Check the type of an do while statement's condition
    void CheckDoWhile(DoWhileNode*& node, TypingContext* context);

    // Check the type of an for statement's condition
    void CheckFor(ForNode*& node, TypingContext* context);

    // Checks the statements within a loop node
    void CheckLoop(LoopNode*& node, TypingContext* context);

    // Checks the statements within a scope node
    void CheckScope(ScopeNode*& node, TypingContext* context);

    // Checks the statements within a timeout node
    void CheckTimeout(TimeoutNode*& node, TypingContext* context);

    // Check the type of an if statement's condition
    void CheckIfRoot(IfRootNode*& node, TypingContext* context);

    // Check the type of an if statement's condition
    void CheckIf(IfNode*& node, TypingContext* context);

    // Find a loop scope node above our own node in the tree
    LoopScopeNode* FindLoopScope(size_t scopeCount, SyntaxNode* parent);

    // Check the type of an break statement's condition
    void CheckBreak(BreakNode*& node, TypingContext* context);

    // Check the type of an continue statement's condition
    void CheckContinue(ContinueNode*& node, TypingContext* context);

    // Resolve a local variable reference (get the variable its referencing)
    void ResolveLocalVariableReference(LocalVariableReferenceNode*& node, TypingContext* context);

    // Mark the parent scope node as being a complete path
    void MarkParentScopeAsAllPathsReturn(SyntaxNode* parent, bool isDebugReturn);

    // If the types are the same, no conversion is applied and the node is left alone
    // If the types are different and an implicit conversion exists, then it will reparent the expression to a TypeCastNode
    // Otherwise, it will return false since no conversion is available
    // Note: Always remember to pass the actual node pointer in instead of a stack local so we can modify it in place!
    static bool ImplicitConvertAfterWalkAndIo(ExpressionNode*& nodeToReparent, Type* toType);

    // Check the type of a return value
    void CheckReturn(ReturnNode*& node, TypingContext* context);

    // Assign a type to a type cast expression (and check the type)
    void DecorateCheckTypeCast(TypeCastNode*& node, TypingContext* context);

    // Assign a type to a function call (and check the type)
    void DecorateCheckFunctionCall(FunctionCallNode*& node, TypingContext* context);

    // Store the function in the code context and validate it as a member
    void CheckAndPushFunction(FunctionNode*& node, TypingContext* context);

    // Check that the binary operator is valid and that it's types are valid
    void DecorateCheckBinaryOperator(BinaryOperatorNode*& node, TypingContext* context);

    // Check that the unary operator is valid and that it's types are valid
    void DecorateCheckPropertyDelegateOperator(PropertyDelegateOperatorNode*& node, TypingContext* context);

    // Check that the unary operator is valid and that it's types are valid
    void DecorateCheckUnaryOperator(UnaryOperatorNode*& node, TypingContext* context);

    // Check all expressions and verify that their io modes are being used properly
    void CheckExpressionIoModes(ExpressionNode*& node, TypingContext* context);

    // Make sure all nodes know which library, class, and function they belong to
    void DecorateCodeLocations(SyntaxNode*& node, TypingContext* context);

    // Utility for replacing/clarifying a member access operator (works with both static and instance members)
    void ResolveMemberAccess(MemberAccessNode* node, const Resolver& resolver);

    // Resolve the node type of a member (after a member access operator...)
    void ResolveMember(MemberAccessNode*& node, TypingContext* context);

    // Because replacing the side-effect operator case for binary/unary operators is so similar, we functionalized it
    // The template type should be the operator node type
    template <typename NodeType>
    void BuildGetSetSideEffectIndexerNodes(NodeType*& node, IndexerCallNode* indexer, ExpressionNode* NodeType::* operandMemberThatWasIndexer, TypingContext* context);

    // When we hit a binary operator that has side effects and the left operand is an indexer
    // We need to transform the binary operator into being a child of the indexer, and promote the indexer to parent
    // If the binary operator is a straight assignment, then it only invokes Set, otherwise its a Get/Set
    void IndexerBinaryOperator(BinaryOperatorNode*& node, TypingContext* context);

    // When we hit a unary operator that has side effects and the left operand is an indexer
    // We need to transform the binary operator into being a child of the indexer, and promote the indexer to parent
    // If the binary operator is a straight assignment, then it only invokes Set, otherwise its a Get/Set
    void IndexerUnaryOperator(UnaryOperatorNode*& node, TypingContext* context);

    // If we visit an indexer by itself (not as a side effect left-binary/unary operator) then we simply
    // replace it with a call to .Get(index0, index1...)
    void IndexerIndexerCall(IndexerCallNode*& node, TypingContext* context);

  private:

    // Store a pointer to the current syntax tree
    SyntaxTree* Tree;

    // Store a pointer back to the project that we're created from
    Project* ParentProject;

    // Store a reference to the error handler
    CompilationErrors& Errors;

    // The library builder we use to generate the library
    LibraryBuilder* Builder;

    // All the other libraries we depend upon
    const Module* Dependencies;

    // All the dependency libraries, and our own (useful for generic searching)
    LibraryArray AllLibraries;

    // Store a map of all the named external types
    BoundTypeMap ExternalBoundTypes;

    // All named tempalte types
    HashMap<String, ClassNode*> InternalBoundTemplates;

    // A map that allows us to cut down on the number of qualified types we allocate
    TypeToIndirect IndirectTypes;

    // All the branch walkers
    BranchWalker<Syntaxer, ClassContext>  ClassWalker;
    BranchWalker<Syntaxer, ClassContext>  TemplateWalker;
    BranchWalker<Syntaxer, TypingContext> MemberWalker;
    BranchWalker<Syntaxer, TypingContext> IndexerWalker;
    BranchWalker<Syntaxer, TypingContext> FunctionWalker;
    BranchWalker<Syntaxer, TypingContext> LocationWalker;
    BranchWalker<Syntaxer, TypingContext> TypingWalker;
    BranchWalker<Syntaxer, TypingContext> ExpressionWalker;

    // Not copyable
    ZilchNoCopy(Syntaxer);
  };
}

#endif
