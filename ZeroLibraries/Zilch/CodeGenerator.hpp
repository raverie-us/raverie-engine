/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_CODE_GENERATOR_HPP
#define ZILCH_CODE_GENERATOR_HPP

namespace Zilch
{
  // The context we use to generate code
  class ZeroShared GeneratorContext : public WalkerContext<CodeGenerator, GeneratorContext>
  {
  public:
    // Store the current function that we're building
    FunctionArray FunctionStack;

    // Store the current type that we're building
    Array<BoundType*> ClassTypeStack;
  };

  // This class uses the syntax tree (after type checking) to generate a byte-code known as the "three-address"
  class ZeroShared CodeGenerator
  {
  public:

    // Constructor
    CodeGenerator();

    // Generates a buffer of op-codes from the given syntax-tree
    LibraryRef Generate(SyntaxTree& syntaxTree, LibraryBuilder& builder);

  private:

    // Walks through the members of a type and determines the total size of those members
    // If a member is left uncomputed, then it will be walked and computed also
    void ComputeSize(BoundType* type, const CodeLocation& location);

    // Store the class in the code context
    void ClassContext(ClassNode*& node, GeneratorContext* context);

    // Store the class in the code context
    void ClassAndPreconstructorContext(ClassNode*& node, GeneratorContext* context);

    // Store the function in the code context
    void FunctionContext(GenericFunctionNode*& node, GeneratorContext* context);

    // Generate out of scope destructors
    void GenerateOutOfScope(ScopeNode*& node, GeneratorContext* context);

    // Generate the storage for parameters
    void GenerateParameter(ParameterNode*& node, GeneratorContext* context);

    // Generate the storage for variables
    void GenerateLocalVariable(LocalVariableNode*& node, GeneratorContext* context);

    // Generate debug breakpoints
    void GenerateDebugBreak(DebugBreakNode*& node, GeneratorContext* context);

    // Generate the initialization of member variables (static and fields) and also generate properties
    void GenerateMemberVariable(MemberVariableNode*& node, GeneratorContext* context);

    // Generate opcode for timeout statements
    void GenerateTimeout(TimeoutNode*& node, GeneratorContext* context);

    // Generate opcode for if statements
    void GenerateIfRoot(IfRootNode*& node, GeneratorContext* context);

    // Generate all the statements and the continue jumps inside
    void GenerateLoopStatementsAndContinues(GeneratorContext* context, LoopScopeNode* node);

    // Generate all the statements inside a node
    void GenerateStatements(GeneratorContext* context, ScopeNode* node);

    // Generate the backwards jump that most loops use to go back to the beginning
    void GenerateBackwardsLoopJump(GeneratorContext* context, size_t backwardsJumpInstructionIndex, const CodeLocation& debugLocation);

    // Generate the code for break statements in the loop
    void GenerateLoopBreaks(GeneratorContext* context, LoopScopeNode* node);

    // Generate opcode for while statements
    void GenerateWhile(WhileNode*& node, GeneratorContext* context);

    // Generate opcode for do while statements
    void GenerateDoWhile(DoWhileNode*& node, GeneratorContext* context);

    // Generate opcode for for statements
    void GenerateFor(ForNode*& node, GeneratorContext* context);

    // Generate opcode for loop statements
    void GenerateLoop(LoopNode*& node, GeneratorContext* context);

    // Generate opcode for scope statements
    void GenerateScope(ScopeNode*& node, GeneratorContext* context);

    // Generate opcode for break statements
    void GenerateBreak(BreakNode*& node, GeneratorContext* context);

    // Generate opcode for continue statements
    void GenerateContinue(ContinueNode*& node, GeneratorContext* context);

    // Generate opcode for binary operations
    void GenerateBinaryOperation(BinaryOperatorNode*& node, GeneratorContext* context);

    // Generate opcode for unary operations
    void GenerateUnaryOperation(UnaryOperatorNode*& node, GeneratorContext* context);

    // Generate opcode for the unary property delegate operator
    void GeneratePropertyDelegateOperation(PropertyDelegateOperatorNode*& node, GeneratorContext* context);

    // Generate opcode for member accesses (function, data, etc)
    void GenerateMemberAccess(MemberAccessNode*& node, GeneratorContext* context);

    // Generate opcode for static data-member accesses
    void GenerateStaticFieldAccess(MemberAccessNode*& node, GeneratorContext* context);

    // Generate opcode for data-member accesses
    void GenerateFieldAccess(MemberAccessNode*& node, GeneratorContext* context);

    // Generate opcode for function-member accesses
    void GenerateFunctionDelegateMemberAccess(MemberAccessNode*& node, GeneratorContext* context);

    // Generate opcode for property-member 'get'
    void GeneratePropertyGetMemberAccess(MemberAccessNode*& node, GeneratorContext* context);

    // Generate opcode for property-member 'set'
    void GeneratePropertySetMemberAccess(MemberAccessNode*& node, GeneratorContext* context);

    // Generate opcode for the initializers in the initializer list (base, this, etc)
    void GenerateInitializer(InitializerNode*& node, GeneratorContext* context);

    // Allocate a delegate opcode of type T
    template <typename T>
    T& DelegateOpcode
    (
      Function*           caller,
      Function*           toCall,
      OperandIndex        delegateDest,
      const CodeLocation& location,
      Instruction::Enum   instruction,
      DebugOrigin::Enum   debug
    );

    // Create an instance delegate for the given type or source (the this handle will be created)
    void CreateInstanceDelegateAndThisHandle
    (
      Function*           caller,
      Function*           toCall,
      Type*               thisType,
      const Operand&      thisSource,
      Operand&            delegateDestOut,
      bool                canBeVirtual,
      const CodeLocation& location,
      DebugOrigin::Enum   debug
    );

    // Create an instance delegate for the given type or source (we provide the this handle)
    void CreateInstanceDelegateWithThisHandle
    (
      Function*           caller,
      Function*           toCall,
      const Operand&      thisHandle,
      Operand&            delegateDestOut,
      bool                canBeVirtual,
      const CodeLocation& location,
      DebugOrigin::Enum   debug
    );

    // Create a static delegate for the given type or source
    void CreateStaticDelegate
    (
      Function*           caller,
      Function*           toCall,
      Operand&            delegateDest,
      const CodeLocation& location,
      DebugOrigin::Enum   debug
    );

    // Generate opcode for type-casts
    void GenerateTypeCast(TypeCastNode*& node, GeneratorContext* context);
    
    // Generate the retrieval of a type
    void GenerateTypeId(TypeIdNode*& node, GeneratorContext* context);
    
    // Generate the retrieval of a member
    void GenerateMemberId(MemberIdNode*& node, GeneratorContext* context);

    // Generate opcode for referencing local and parameter variables
    void GenerateLocalVariableReference(LocalVariableReferenceNode*& node, GeneratorContext* context);

    // Generate opcode for return values
    void GenerateReturnValue(ReturnNode*& node, GeneratorContext* context);

    // Generate opcode for function calls
    void GenerateFunctionCall(FunctionCallNode*& node, GeneratorContext* context);

    // Generate the opcode for a function call (*before* opcode for argument copying)
    void GenerateCallOpcodePreArgs(Function* caller, DelegateType* delegateTypeToCall, const Operand& delegateLocal, const CodeLocation& location, DebugOrigin::Enum debugOrigin);

    // Generate the opcode for a function call (*after* opcode for argument copying)
    void GenerateCallOpcodePostArgs(Function* caller, DelegateType* delegateTypeToCall, Operand* returnAccessOut, const CodeLocation& location, DebugOrigin::Enum debugOrigin);

    // Collect all the values used in expressions
    void CollectValue(ValueNode*& node, GeneratorContext* context);

    // Collect all the string interpolant expressions
    void GenerateStringInterpolants(StringInterpolantNode*& node, GeneratorContext* context);

    // Generate opcode for deleting objects in memory
    void GenerateDelete(DeleteNode*& node, GeneratorContext* context);

    // Generate opcode for throwing an exception
    void GenerateThrow(ThrowNode*& node, GeneratorContext* context);

    // Finds all the new calls and generates opcode to create objects
    void GenerateStaticTypeOrCreationCall(StaticTypeNode*& node, GeneratorContext* context);

    // Invokes 'Add' and initializes members
    void GenerateExpressionInitializer(ExpressionInitializerNode*& node, GeneratorContext* context);

    // Perform all the expressions of the multi-expression, and then yield the access to one of them
    void GenerateMultiExpression(MultiExpressionNode*& node, GeneratorContext* context);

    // Allocate a local on a function (and setup an access to point at it)
    void CreateLocal(Function* function, size_t size, Operand& accessOut);

    // Create a r-value unary operator opcode
    void CreateRValueUnaryOpcode(Function* function, UnaryOperatorNode& node, Instruction::Enum instruction, DebugOrigin::Enum debugOrigin);

    // Create a l-value unary operator opcode
    void CreateLValueUnaryOpcode(Function* function, UnaryOperatorNode& node, Instruction::Enum instruction, DebugOrigin::Enum debugOrigin);

    // Create a conversion opcode
    void CreateConversionOpcode(Function* function, TypeCastNode& node, Instruction::Enum instruction, DebugOrigin::Enum debugOrigin);

    // Determine the proper opcode for unary operations
    void GenerateUnaryOp(Function* function, UnaryOperatorNode& node, DebugOrigin::Enum debugOrigin);

    // Determine the proper opcode for conversion operations
    void GenerateConversion(Function* function, TypeCastNode& node, DebugOrigin::Enum debugOrigin);

    // Create a copy opcode
    void CreateCopyOpcode(Function* function, CopyMode::Enum mode, Type* type, const Operand& source, const Operand& destination, DebugOrigin::Enum debugOrigin, const CodeLocation& location);

    // Determine the proper opcode for copy operations (we're initializing the return value)
    void GenerateCopyToReturn(Function* function, Type* type, const Operand& source, const Operand& destination, DebugOrigin::Enum debugOrigin, const CodeLocation& location);

    // Determine the proper opcode for copy operations (we're initializing memory)
    void GenerateCopyInitialize(Function* function, Type* type, const Operand& source, const Operand& destination, DebugOrigin::Enum debugOrigin, const CodeLocation& location);

    // Determine the proper opcode for copy parameter operations
    void GenerateCopyToParameter(Function* function, Type* type, const Operand& source, OperandIndex destRegister, DebugOrigin::Enum debugOrigin, const CodeLocation& location);

    // Determine the proper opcode for copy return operations
    void GenerateCopyFromReturn(Function* function, Type* type, OperandIndex sourceRegister, OperandIndex destRegister, DebugOrigin::Enum debugOrigin, const CodeLocation& location);

    // Determine the proper opcode for creating a handle
    void GenerateHandleInitialize(Function* function, Type* type, const Operand& source, const Operand& destination, DebugOrigin::Enum debugOrigin, const CodeLocation& location);

  private:

    // Store all the walkers
    BranchWalker<CodeGenerator, GeneratorContext> GeneratorWalker;
    BranchWalker<CodeGenerator, GeneratorContext> PropertySetWalker;

    // The library that we're currently building
    LibraryBuilder* Builder;
  };
}

#endif
