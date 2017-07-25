/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#include "Zilch.hpp"

namespace Zilch
{
  //***************************************************************************
  CodeGenerator::CodeGenerator() :
    Builder(nullptr)
  {
    ZilchErrorIfNotStarted(CodeGenerator);

    // Walk all any type of expression (often, expressions are nested within each other)
    this->GeneratorWalker.Register(&CodeGenerator::ClassAndPreconstructorContext);
    this->GeneratorWalker.RegisterDerived<FunctionNode>(&CodeGenerator::FunctionContext);
    this->GeneratorWalker.RegisterDerived<ConstructorNode>(&CodeGenerator::FunctionContext);
    this->GeneratorWalker.RegisterDerived<DestructorNode>(&CodeGenerator::FunctionContext);
    this->GeneratorWalker.Register(&CodeGenerator::GenerateInitializer);
    this->GeneratorWalker.Register(&CodeGenerator::GenerateParameter);
    this->GeneratorWalker.Register(&CodeGenerator::GenerateLocalVariable);
    this->GeneratorWalker.Register(&CodeGenerator::GenerateDebugBreak);
    this->GeneratorWalker.Register(&CodeGenerator::GenerateMemberVariable);
    this->GeneratorWalker.Register(&CodeGenerator::GenerateTimeout);
    this->GeneratorWalker.Register(&CodeGenerator::GenerateIfRoot);
    this->GeneratorWalker.Register(&CodeGenerator::GenerateWhile);
    this->GeneratorWalker.Register(&CodeGenerator::GenerateDoWhile);
    this->GeneratorWalker.Register(&CodeGenerator::GenerateFor);
    this->GeneratorWalker.RegisterDerived<ForEachNode>(&CodeGenerator::GenerateFor);
    this->GeneratorWalker.Register(&CodeGenerator::GenerateLoop);
    this->GeneratorWalker.Register(&CodeGenerator::GenerateScope);
    this->GeneratorWalker.Register(&CodeGenerator::GenerateBinaryOperation);
    this->GeneratorWalker.Register(&CodeGenerator::GenerateUnaryOperation);
    this->GeneratorWalker.Register(&CodeGenerator::GeneratePropertyDelegateOperation);
    this->GeneratorWalker.Register(&CodeGenerator::GenerateMemberAccess);
    this->GeneratorWalker.Register(&CodeGenerator::GenerateTypeCast);
    this->GeneratorWalker.Register(&CodeGenerator::GenerateTypeId);
    this->GeneratorWalker.Register(&CodeGenerator::GenerateMemberId);
    this->GeneratorWalker.Register(&CodeGenerator::GenerateLocalVariableReference);
    this->GeneratorWalker.Register(&CodeGenerator::GenerateReturnValue);
    this->GeneratorWalker.Register(&CodeGenerator::GenerateFunctionCall);
    this->GeneratorWalker.Register(&CodeGenerator::CollectValue);
    this->GeneratorWalker.Register(&CodeGenerator::GenerateStringInterpolants);
    this->GeneratorWalker.Register(&CodeGenerator::GenerateDelete);
    this->GeneratorWalker.Register(&CodeGenerator::GenerateThrow);
    this->GeneratorWalker.Register(&CodeGenerator::GenerateStaticTypeOrCreationCall);
    this->GeneratorWalker.Register(&CodeGenerator::GenerateExpressionInitializer);
    this->GeneratorWalker.Register(&CodeGenerator::GenerateMultiExpression);
    this->GeneratorWalker.Register(&CodeGenerator::GenerateBreak);
    this->GeneratorWalker.Register(&CodeGenerator::GenerateContinue);
    
    this->PropertySetWalker.Register(&CodeGenerator::GeneratePropertySetMemberAccess);
  }

  //***************************************************************************
  void CodeGenerator::ComputeSize(BoundType* type, const CodeLocation& location)
  {
    // If we already computed a size for this type then don't bother recomputing
    if (type->Size != UndeterminedSize)
      return;

    // Start the size off at zero
    type->Size = 0;

    // If we have a base class type...
    if (type->BaseType != nullptr)
    {
      // Compute the size if we haven't already
      this->ComputeSize(type->BaseType, type->BaseType->Location);
  
      // Add the base type's size to the size of our object
      type->Size += AlignToBusWidth(type->BaseType->GetAllocatedSize());
    }

    // Loop through all the members
    FieldMapRange allFields = type->InstanceFields.All();
    while (allFields.Empty() == false)
    {
      // Get the member
      Field& field = *allFields.Front().second;
      allFields.PopFront();
  
      // Set the position of the member as the current size
      field.Offset = type->Size;
  
      // We only care about computing the size of the type if it's a value type
      if (Type::IsValueType(field.PropertyType))
      {
        // If the class type exists without our hash set
        BoundType* propertyType = Type::DynamicCast<BoundType*>(field.PropertyType);

        // Compute the size if we haven't already (it may not be one of our types,
        // but compute will early out if it has its size already computed)
        if (propertyType != nullptr)
          this->ComputeSize(propertyType, field.Location);
      }
      // If this type is a handle type...
      else if (Type::IsHandleType(field.PropertyType))
      {
        type->Handles.PushBack(type->Size);
      }
      // Otherwise, if it's a delegate type
      else if (Type::DynamicCast<DelegateType*>(field.PropertyType) != nullptr)
      {
        type->Delegates.PushBack(type->Size);
      }
  
      // Add to the size of the object
      type->Size += AlignToBusWidth(field.PropertyType->GetCopyableSize());
    }
  }

  //***************************************************************************
  LibraryRef CodeGenerator::Generate(SyntaxTree& syntaxTree, LibraryBuilder& builder)
  {
    // Create the context
    GeneratorContext generatorContext;
    
    // Store the builder
    this->Builder = &builder;

    // Before we do anything else, we want to compute the sizes of any class whose size has yet to be determined
    BoundTypeValueRange boundTypes = builder.BoundTypes.Values();

    // We've collected all the classes we're compiling, as well as the members
    // Loop through all the classes and compute their sizes
    while (boundTypes.Empty() == false)
    {
      // Grab the current type and iterate to the next
      BoundType* type = boundTypes.Front();
      boundTypes.PopFront();

      // Compute the size for the class type
      this->ComputeSize(type, type->Location);
    }

    // Make sure all delegates know their sizes (may be computed more than once due to code-gen needing the sizes)
    builder.ComputeDelegateAndFunctionSizesOnce();

    // Now generate all the code
    this->GeneratorWalker.Walk(this, syntaxTree.Root, &generatorContext);

    // Create the library
    return this->Builder->CreateLibrary();
  }

  //***************************************************************************
  void CodeGenerator::ClassContext(ClassNode*& node, GeneratorContext* context)
  {
    // Push the class onto the stack so that children can access it
    // (the top of the stack will be the most relevant class to them)
    context->ClassTypeStack.PushBack(node->Type);
    
    // Walk all children of the class generically
    context->Walker->GenericWalkChildren(this, node, context);

    // We are exiting this class, so pop it off
    context->ClassTypeStack.PopBack();
  }

  //***************************************************************************
  void CodeGenerator::ClassAndPreconstructorContext(ClassNode*& node, GeneratorContext* context)
  {
    // Store the pre-constructor for convenience
    Function* preCtor = node->PreConstructor;

    // We're about to generate code for the pre-constructor, push it onto the stack
    context->FunctionStack.PushBack(preCtor);

    // NOTE:
    // This was here, but if you look above in 'GeneratePreConstructorAndPushClassContext'
    // it is already doing this (doing it again would not break anything, just use extra memory)
    // Allocate space for the implicit 'this' pointer
    //preCtor->AllocateRegister(preCtor->This->ResultType->GetCopyableSize());

    // Invoke the class context, which walks the rest of the tree and pushes the class type
    this->ClassContext(node, context);

    // Generate the return opcode, which simply just stops execution of a function
    preCtor->AllocateArgumentFreeOpcode(Instruction::Return, DebugOrigin::ReturnValue, node->Location);

    // Pop the pre constructor
    context->FunctionStack.PopBack();
  }

  // Store the function in the code context
  void CodeGenerator::FunctionContext(GenericFunctionNode*& node, GeneratorContext* context)
  {
    // Push the function onto the stack so that children can access it
    // (the top of the stack will be the most relevant function to them)
    Function* function = node->DefinedFunction;
    context->FunctionStack.PushBack(function);

    // Loop through all the parameters
    // The parameters are supposed to occur before checking if the function is static
    // That way, the 'this' pointer always ends up at the end of the stack
    // Yet the instruction ends up at the beginning
    // See CodeGenerator.cpp, approximately line 695, in the function GenerateFunctionCall there is an explanation
    for (size_t i = 0; i < node->Parameters.Size(); ++i)
    {
      // Walk the statements
      context->Walker->Walk(this, node->Parameters[i], context);
    }

    // Loop through all the statements
    for (size_t i = 0; i < node->Statements.Size(); ++i)
    {
      // Walk the statements
      context->Walker->Walk(this, node->Statements[i], context);
    }

    // Get a reference to the core library
    Core& core = Core::GetInstance();

    // If the function has no return type and not all code paths return...
    if (Type::IsSame(function->FunctionType->Return, core.VoidType) && node->AllPathsReturn == false)
    {
      // Generate an implicit return opcode at the end of the function
      function->AllocateArgumentFreeOpcode(Instruction::Return, DebugOrigin::FunctionContext, node->Location.GetEndOnlyLocation());
    }

    // We are exiting this function, so pop it off
    context->FunctionStack.PopBack();
  }

  //***************************************************************************
  void CodeGenerator::GenerateOutOfScope(ScopeNode*& node, GeneratorContext* context)
  {
    VariableValueRange variables = node->ScopedVariables.Values();
    ZilchTodo("Todo: Generate out of scope destructors")

    //// Loop through all the variables in this scope
    //while (variables.Empty() == false)
    //{
    //  // Get the current variable and iterate to the next
    //  Variable* variable = variables.Front();
    //  variables.PopFront();
    //  // 
    //  //variable->ResultType
    //}
  }

  //***************************************************************************
  void CodeGenerator::GenerateParameter(ParameterNode*& node, GeneratorContext* context)
  {
    // Get a reference to the current function that we're building
    Function* function = context->FunctionStack.Back();
    ZilchTodo("Default parameter values (expressions)");

    // We already allocated space for this parameter inside the delegate's parameter computing
    // Just set our variable's local position to the delegate's same parameter stack position
    node->CreatedVariable->Local = function->FunctionType->Parameters[node->ParameterIndex].StackOffset;
  }

  //***************************************************************************
  void CodeGenerator::GenerateLocalVariable(LocalVariableNode*& node, GeneratorContext* context)
  {
    // Get a reference to the current function that we're building
    Function* function = context->FunctionStack.Back();

    // Make sure we generate code for the initial value of the variable
    context->Walker->Walk(this, node->InitialValue, context);

    // If the flag is set, this local variable will forward access directly
    // to another stack local (without allocating storage of its own)
    if (node->ForwardLocalAccessIfPossible && node->InitialValue->Access.Type == OperandType::Local)
    {
      // Copy of the access to the initial value, and set the local variable definition to point at the same local
      node->Access = node->InitialValue->Access;
      node->CreatedVariable->Local = node->Access.HandleConstantLocal;
    }
    else
    {
      // This expression's result will be stored in the last created register
      this->CreateLocal(function, node->ResultType->GetCopyableSize(), node->Access);

      // The variable is always on the stack (hence local) so we only need to look at the local part of the access
      node->CreatedVariable->Local = node->Access.HandleConstantLocal;

      // Generate a copy to copy the initial value to the local register
      // The register is not yet initialized, so this must be an init copy
      this->GenerateCopyInitialize
      (
        function,
        node->CreatedVariable->ResultType,
        node->InitialValue->Access,
        Operand(node->CreatedVariable->Local),
        DebugOrigin::LocalVariable,
        node->Location
      );
    }
  }
  
  //***************************************************************************
  void CodeGenerator::GenerateDebugBreak(DebugBreakNode*& node, GeneratorContext* context)
  {
    // Get a reference to the current function that we're building
    Function* function = context->FunctionStack.Back();

    // Generate the simple debug breakpoint opcode
    function->AllocateOpcode<Opcode>(Instruction::InternalDebugBreakpoint, DebugOrigin::DebugBreak, node->Location);
  }

  //***************************************************************************
  void CodeGenerator::GenerateMemberVariable(MemberVariableNode*& node, GeneratorContext* context)
  {
    // Get a reference to the current function that we're building
    Function* function = context->FunctionStack.Back();

    // The function should always be the pre-constructor
    // Note: If we're generating initialization code for a static field, then we end up pushing another function onto the stack below
    ErrorIf(function != context->ClassTypeStack.Back()->PreConstructor,
      "The function on the top of the stack should be the pre-constructor");

    // Error checking
    ErrorIf(node->IsGetterSetter && node->InitialValue != nullptr,
      "Properties should not have initial values");

    // If this is a data member with an initial value...
    if (node->InitialValue != nullptr)
    {
      // Get a reference directly to the field
      Field* field = node->CreatedField;
      
      // Function options for the field initializer we generate below
      FunctionOptions::Enum functionOptions = FunctionOptions::None;

      // If the member is static, then the getter and setter are static too
      if (field->IsStatic)
      {
        functionOptions = FunctionOptions::Static;
      }
      // Generate pre-constructor initialization of a member variable (only for instance fields)
      else
      {
        // Make sure we generate code for the initial value of the member variable
        // This code will be generated in the pre-constructor function
        context->Walker->Walk(this, node->InitialValue, context);

        // Get the 'this' parameter (which will be the only parameter, since this is the Pre-Constructor)
        Variable* thisVariable = function->This;

        // Establish where we're going to be writing to
        Operand destination(thisVariable->Local, field->Offset, OperandType::Field);

        // Generate a copy to copy the initial value to the member
        // Since this is copying over un-initialized memory, then this is a init copy
        this->GenerateCopyInitialize
        (
          function,
          node->ResultType,
          node->InitialValue->Access,
          destination,
          DebugOrigin::MemberVariable,
          node->Location
        );
      }

      // For both static and instance fields, we want to generate an initializer (a function that can be ran on its own)
      // Technically the CreateRawFunction should be generated during the Syntaxer, just in case we ever expose the ability to access it
      // However, initializers are a pretty internal detail (only used by patching and static variable initialization)
      Function* initializer = this->Builder->CreateRawFunction
      (
        field->Owner,
        FieldInitializerName,
        VirtualMachine::ExecuteNext,
        ParameterArray(),
        ZilchTypeId(void),
        functionOptions
      );

      // Push the function onto the stack so that children can access it
      // (the top of the stack will be the most relevant function to them)
      field->Initializer = initializer;
      context->FunctionStack.PushBack(initializer);
      
      // Generate code to compute the initial value
      context->Walker->Walk(this, node->InitialValue, context);

      // Establish where we're going to be writing to (a member of an instance of an object, eg field, or a static location)
      Operand destination;

      // If the field is a static field
      if (field->IsStatic)
      {
        // Copy the value computed by walking the initial value into a static variable location
        destination.StaticField = field;
        destination.FieldOffset = 0;
        destination.Type = OperandType::StaticField;
      }
      else
      {
        // Get the 'this' parameter since this is an instance version of a field initializer
        Variable* thisVariable = function->This;

        // We'll write to a field on the this handle using the given offset
        destination.HandleConstantLocal = thisVariable->Local;
        destination.FieldOffset = field->Offset;
        destination.Type = OperandType::Field;
      }

      // Generate a copy to copy the initial value to the member
      // Since this is copying over un-initialized memory, then this is a init copy
      this->GenerateCopyInitialize
      (
        initializer,
        node->ResultType,
        node->InitialValue->Access,
        destination,
        DebugOrigin::MemberVariable,
        node->Location
      );

      // Generate the return opcode, which simply just stops execution of a function
      initializer->AllocateArgumentFreeOpcode(Instruction::Return, DebugOrigin::MemberVariable, node->Location);

      // Pop the function from the stack
      context->FunctionStack.PopBack();
    }

    // Generate the get function if we have one
    if (node->Get != nullptr)
    {
      context->Walker->Walk(this, node->Get, context);
    }

    // Generate the set function if we have one
    if (node->Set != nullptr)
    {
      context->Walker->Walk(this, node->Set, context);
    }
  }

  //***************************************************************************
  // All the if statements (except for the last one) need to jump to the end of the entire if statement
  struct IfEndJump
  {
    RelativeJumpOpcode* Opcode;
    size_t JumpInstructionIndex;
  };
  
  //***************************************************************************
  void CodeGenerator::GenerateIfRoot(IfRootNode*& node, GeneratorContext* context)
  {
    // At the end of each if statement (except for the last one) we need a jump
    // that jumps to the very end of the entire if, store pointers to all those opcode here
    Array<IfEndJump> jumpsAfterEveryElse;

    // Get a pointer to the current function that we're building
    Function* function = context->FunctionStack.Back();

    // Walk through all the parts of the if statement
    for (size_t i = 0; i < node->IfParts.Size(); ++i)
    {
      // Grab the current if part node
      IfNode* ifPart = node->IfParts[i];

      // If we have no condition... (that means we're an else statement)
      if (ifPart->Condition == nullptr)
      {
        // Loop through all the statements and generate opcode for each
        this->GenerateStatements(context, ifPart);
        break;
      }

      // Generate code for the condition of the if statement
      context->Walker->Walk(this, ifPart->Condition, context);

      // Store the index that we generated the opcode
      size_t ifFalseInstructionIndex = function->GetCurrentOpcodeIndex();

      // Generate an opcode that will jump if the given conditional-expression is false
      // For now, we'll leave out the "jump to" address as we don't yet know where to go
      // The jump-to address will be filled in upon the post pass
      IfOpcode& ifOpcode = function->AllocateOpcode<IfOpcode>(Instruction::IfFalseRelativeGoTo, DebugOrigin::If, ifPart->Condition->Location);
      ifOpcode.Condition = ifPart->Condition->Access;

      // Loop through all the statements and generate opcode for each
      this->GenerateStatements(context, ifPart);

      // If this if part has another else statement to follow (literally whether we're the last node)
      bool hasElseStatement = (i != node->IfParts.Size() - 1);

      // The index of the jump instruction
      RelativeJumpOpcode* jumpOpcode = nullptr;
      size_t jumpInstructionIndex = 0;

      // If we have an else statement, we need one more opcode
      // This has to be here (and not below) because the if-false needs to skip this instruction
      if (hasElseStatement)
      {
        // Create a jump opcode and store it so we can fill in the parameters after we've 
        IfEndJump& endJump = jumpsAfterEveryElse.PushBack();
        endJump.JumpInstructionIndex = function->GetCurrentOpcodeIndex();
        endJump.Opcode = &function->AllocateOpcode<RelativeJumpOpcode>(Instruction::RelativeGoTo, DebugOrigin::If, ifPart->Location);
      }

      // Jump to wherever the statements inside the if-statement end
      size_t opcodeIndexAfterStatements = function->GetCurrentOpcodeIndex();
      ifOpcode.JumpOffset = (ByteCodeOffset)(opcodeIndexAfterStatements - ifFalseInstructionIndex);
    }

    // Get the opcode index after all the else statements have been processed
    size_t opcodeIndexAfterAllElses = function->GetCurrentOpcodeIndex();

    // Loop through all the jumps
    for (size_t i = 0; i < jumpsAfterEveryElse.Size(); ++i)
    {
      // Grab the current jump
      IfEndJump& endJump = jumpsAfterEveryElse[i];

      // Retreive that jump opcode we made earlier, and set its jump offset
      endJump.Opcode->JumpOffset = (ByteCodeOffset)(opcodeIndexAfterAllElses - endJump.JumpInstructionIndex);
    }
  }

  //***************************************************************************
  void CodeGenerator::GenerateStatements(GeneratorContext* context, ScopeNode* node)
  {
    // Get a pointer to the current function that we're building
    Function* function = context->FunctionStack.Back();

    // Loop through all the statements
    for (size_t i = 0; i < node->Statements.Size(); ++i)
    {
      // Walk the statements and generate all their code
      context->Walker->Walk(this, node->Statements[i], context);
    }
  }

  //***************************************************************************
  void CodeGenerator::GenerateLoopStatementsAndContinues(GeneratorContext* context, LoopScopeNode* node)
  {
    // Get a pointer to the current function that we're building
    Function* function = context->FunctionStack.Back();

    // Generate all statements for the loop node
    this->GenerateStatements(context, node);

    // By here, we should have collected any continue statements
    // (for break to work properly, this has to come after that backwards jump opcode)
    for (size_t i = 0; i < node->Continues.Size(); ++i)
    {
      // Get the current continue node
      ContinueNode* continueNode = node->Continues[i];

      // Get the instruction index for the continue's jump
      size_t continueInstructionIndex = continueNode->InstructionIndex;

      // Get the continue opcode
      RelativeJumpOpcode& continueOpcode = *continueNode->JumpOpcode;

      // We want to jump forward
      continueOpcode.JumpOffset = (ByteCodeOffset)(function->GetCurrentOpcodeIndex() - continueInstructionIndex);
    }
  }

  //***************************************************************************
  void CodeGenerator::GenerateBackwardsLoopJump(GeneratorContext* context, size_t backwardsJumpInstructionIndex, const CodeLocation& debugLocation)
  {
    // Get a pointer to the current function that we're building
    Function* function = context->FunctionStack.Back();

    // Determine the jump offset that we'd like to jump
    ByteCodeOffset jumpOffset = (ByteCodeOffset)(backwardsJumpInstructionIndex - function->GetCurrentOpcodeIndex());

    // Generate one more opcode that will jump back up the beginning (before the condition is checked)
    RelativeJumpOpcode& jumpOpcode = function->AllocateOpcode<RelativeJumpOpcode>(Instruction::RelativeGoTo, DebugOrigin::While, debugLocation);
    jumpOpcode.JumpOffset = jumpOffset;
  }

  //***************************************************************************
  void CodeGenerator::GenerateLoopBreaks(GeneratorContext* context, LoopScopeNode* node)
  {
    // Get a pointer to the current function that we're building
    Function* function = context->FunctionStack.Back();

    // By here, we should have collected any break statements
    // and finished the opcode that returns to the beginning of the loop
    // (for break to work properly, this has to come after that backwards jump opcode)
    for (size_t i = 0; i < node->Breaks.Size(); ++i)
    {
      // Get the current break node
      BreakNode* breakNode = node->Breaks[i];

      // Get the instruction index for the break's jump
      size_t breakInstructionIndex = breakNode->InstructionIndex;

      // Get the break opcode
      RelativeJumpOpcode& breakOpcode = *breakNode->JumpOpcode;

      // We want to jump forward
      breakOpcode.JumpOffset = (ByteCodeOffset)(function->GetCurrentOpcodeIndex() - breakInstructionIndex);
    }
  }

  //***************************************************************************
  void CodeGenerator::GenerateTimeout(TimeoutNode*& node, GeneratorContext* context)
  {
    // Get a pointer to the current function that we're building
    Function* function = context->FunctionStack.Back();

    // Generate the beginning of the scope
    function->AllocateOpcode<Opcode>(Instruction::BeginScope, DebugOrigin::Timeout, node->Location);

    // Generate the beginning of the timeout with seconds
    TimeoutOpcode& timeout = function->AllocateOpcode<TimeoutOpcode>(Instruction::BeginTimeout, DebugOrigin::Timeout, node->Location);
    timeout.LengthSeconds = node->Seconds;

    // Generate all the statements code and continues
    this->GenerateStatements(context, node);

    // Generate the ending of the timeout scope
    function->AllocateOpcode<Opcode>(Instruction::EndTimeout, DebugOrigin::Timeout, node->Location);

    // Generate the final ending of the scope
    function->AllocateOpcode<Opcode>(Instruction::EndScope, DebugOrigin::Timeout, node->Location);
  }

  //***************************************************************************
  void CodeGenerator::GenerateWhile(WhileNode*& node, GeneratorContext* context)
  {
    // Get a pointer to the current function that we're building
    Function* function = context->FunctionStack.Back();

    // Store the index that we'd like to jump back to at the end of the loop
    size_t backwardsJumpInstructionIndex = function->GetCurrentOpcodeIndex();
    
    // Generate the beginning of the scope
    function->AllocateOpcode<Opcode>(Instruction::BeginScope, DebugOrigin::While, node->Location);

    // Generate code for the condition of the while statement
    context->Walker->Walk(this, node->Condition, context);

    // Store the index that we generated the opcode
    size_t ifFalseInstructionIndex = function->GetCurrentOpcodeIndex();

    // Generate an opcode that will jump if the given conditional-expression is false
    // For now, we'll leave out the "jump to" address as we don't yet know where to go
    // The jump-to address will be filled in upon the post pass
    IfOpcode& ifOpcode = function->AllocateOpcode<IfOpcode>(Instruction::IfFalseRelativeGoTo, DebugOrigin::While, node->Condition->Location);
    ifOpcode.Condition = node->Condition->Access;

    // Generate all the statements code and continues
    GenerateLoopStatementsAndContinues(context, node);

    // Generate the ending of the scope
    function->AllocateOpcode<Opcode>(Instruction::EndScope, DebugOrigin::While, node->Location);

    // Generate the backwards jump, and the breaks
    GenerateBackwardsLoopJump(context, backwardsJumpInstructionIndex, node->Location);
    GenerateLoopBreaks(context, node);

    // Jump to wherever the statements inside the if-statement end
    ifOpcode.JumpOffset = (ByteCodeOffset)(function->GetCurrentOpcodeIndex() - ifFalseInstructionIndex);

    // Generate the final ending of the scope, called when we break or fail the loop conditional
    function->AllocateOpcode<Opcode>(Instruction::EndScope, DebugOrigin::While, node->Location);
  }

  //***************************************************************************
  void CodeGenerator::GenerateDoWhile(DoWhileNode*& node, GeneratorContext* context)
  {
    // Get a pointer to the current function that we're building
    Function* function = context->FunctionStack.Back();

    // Store the index that we'd like to jump back to at the end of the loop
    size_t backwardsJumpInstructionIndex = function->GetCurrentOpcodeIndex();

    // Generate the beginning of the scope
    function->AllocateOpcode<Opcode>(Instruction::BeginScope, DebugOrigin::DoWhile, node->Location);

    // Generate all the statements code and continues
    // (we don't actually have a typical backwards jump, that's on the condition)
    GenerateLoopStatementsAndContinues(context, node);

    // Generate code for the condition of the while statement
    context->Walker->Walk(this, node->Condition, context);

    // Generate the ending of the scope
    function->AllocateOpcode<Opcode>(Instruction::EndScope, DebugOrigin::DoWhile, node->Location);

    // We have to compute the jump offset before creating the if-opcode, since we don't typically include the
    // size of any opcode when jumping backwards
    ByteCodeOffset jumpOffset = (ByteCodeOffset)(backwardsJumpInstructionIndex - function->GetCurrentOpcodeIndex());

    // Generate an opcode that will jump if the given conditional-expression is false
    // For now, we'll leave out the "jump to" address as we don't yet know where to go
    // The jump-to address will be filled in upon the post pass
    IfOpcode& ifOpcode = function->AllocateOpcode<IfOpcode>(Instruction::IfTrueRelativeGoTo, DebugOrigin::DoWhile, node->Condition->Location);
    ifOpcode.Condition = node->Condition->Access;
    ifOpcode.JumpOffset = jumpOffset;

    // Store the index that we'd like to jump back to at the end of the loop
    size_t jumpInstructionIndex = function->GetCurrentOpcodeIndex();

    // Typically in other loops we end up having one 'EndScope' at the end because the last iteration always terminates inside a BeginScope (while, for, etc)
    // Since the if-condition in a do-while comes outside the BeginScope/EndScope, then we need to take special care because we do NOT need another EndScope opcode
    // The EndScope NEEDS to be run in the case of a break statement, however, because breaks are within the BeginScope/EndScope
    RelativeJumpOpcode& jumpPastEndScopeWhenConditionMet = function->AllocateOpcode<RelativeJumpOpcode>(Instruction::RelativeGoTo, DebugOrigin::DoWhile, node->Condition->Location);
    
    // Generate the break (note that we have to have an EndScope after this, but not after the condition fails)
    GenerateLoopBreaks(context, node);
    
    // Generate the final ending of the scope, called when we break or fail the loop conditional
    function->AllocateOpcode<Opcode>(Instruction::EndScope, DebugOrigin::While, node->Location);
    
    // Get the opcode index after the 'EndScope' instruction
    size_t opcodeIndexAfterEndScope = function->GetCurrentOpcodeIndex();

    // Retrieve that jump opcode we made earlier, and set its jump offset
    jumpPastEndScopeWhenConditionMet.JumpOffset = (ByteCodeOffset)(opcodeIndexAfterEndScope - jumpInstructionIndex);
  }

  //***************************************************************************
  void CodeGenerator::GenerateFor(ForNode*& node, GeneratorContext* context)
  {
    // Get a pointer to the current function that we're building
    Function* function = context->FunctionStack.Back();

    // If we have a range variable, generate it first
    if (node->RangeVariable != nullptr)
    {
      // Generate code for the variable
      context->Walker->Walk(this, node->RangeVariable, context);
    }

    // If we have a variable
    if (node->ValueVariable != nullptr)
    {
      // Generate code for the variable
      context->Walker->Walk(this, node->ValueVariable, context);
    }
    else if (node->Initialization != nullptr)
    {
      // Generate code for the initialization expression
      context->Walker->Walk(this, node->Initialization, context);
    }

    // Store the index that we'd like to jump back to at the end of the loop
    size_t backwardsJumpInstructionIndex = function->GetCurrentOpcodeIndex();

    // Generate the beginning of the scope
    function->AllocateOpcode<Opcode>(Instruction::BeginScope, DebugOrigin::For, node->Location);

    // Generate code for the condition of the while statement
    context->Walker->Walk(this, node->Condition, context);

    // Store the index that we generated the opcode
    size_t ifFalseInstructionIndex = function->GetCurrentOpcodeIndex();

    // Generate an opcode that will jump if the given conditional-expression is false
    // For now, we'll leave out the "jump to" address as we don't yet know where to go
    // The jump-to address will be filled in upon the post pass
    IfOpcode& ifOpcode = function->AllocateOpcode<IfOpcode>(Instruction::IfFalseRelativeGoTo, DebugOrigin::For, node->Condition->Location);
    ifOpcode.Condition = node->Condition->Access;

    // Generate all the statements code, the continues
    GenerateLoopStatementsAndContinues(context, node);

    // Walk the iterator since that will still come after a continue
    context->Walker->Walk(this, node->Iterator, context);

    // Generate the ending of the scope
    // All continues and proper loops will hit here, however, the loop conditional failing
    // or a break statement being hit will cause this opcode to not be reached, hence the one below
    // The break / loop conditional should be the only ways to exit the loop
    function->AllocateOpcode<Opcode>(Instruction::EndScope, DebugOrigin::For, node->Location);

    // After the iterator, do the backwards jump, and then any breaks
    GenerateBackwardsLoopJump(context, backwardsJumpInstructionIndex, node->Location);
    GenerateLoopBreaks(context, node);

    // Jump to wherever the statements inside the if-statement end
    ifOpcode.JumpOffset = (ByteCodeOffset)(function->GetCurrentOpcodeIndex() - ifFalseInstructionIndex);

    // Generate the final ending of the scope, called when we break or fail the loop conditional
    function->AllocateOpcode<Opcode>(Instruction::EndScope, DebugOrigin::For, node->Location);
  }

  //***************************************************************************
  void CodeGenerator::GenerateScope(ScopeNode*& node, GeneratorContext* context)
  {
    // Get a pointer to the current function that we're building
    Function* function = context->FunctionStack.Back();

    // Generate the beginning of the scope
    function->AllocateOpcode<Opcode>(Instruction::BeginScope, DebugOrigin::Scope, node->Location);

    // Loop through all the statements and generate opcode for each
    this->GenerateStatements(context, node);

    // Generate the ending of the scope
    function->AllocateOpcode<Opcode>(Instruction::EndScope, DebugOrigin::Scope, node->Location);
  }

  //***************************************************************************
  void CodeGenerator::GenerateLoop(LoopNode*& node, GeneratorContext* context)
  {
    // Get a pointer to the current function that we're building
    Function* function = context->FunctionStack.Back();

    // Store the index that we'd like to jump back to at the end of the loop
    size_t backwardsJumpInstructionIndex = function->GetCurrentOpcodeIndex();

    // Generate the beginning of the scope
    function->AllocateOpcode<Opcode>(Instruction::BeginScope, DebugOrigin::Loop, node->Location);

    // Generate all the statements code, the continues, the backwards jump, and the breaks
    GenerateLoopStatementsAndContinues(context, node);

    // Generate the ending of the scope
    function->AllocateOpcode<Opcode>(Instruction::EndScope, DebugOrigin::Loop, node->Location);

    // Do the backwards jump, and then any breaks
    GenerateBackwardsLoopJump(context, backwardsJumpInstructionIndex, node->Location);
    GenerateLoopBreaks(context, node);

    // Generate the final ending of the scope, called when we break on the last iteration
    function->AllocateOpcode<Opcode>(Instruction::EndScope, DebugOrigin::Loop, node->Location);
  }

  //***************************************************************************
  void CodeGenerator::GenerateBreak(BreakNode*& node, GeneratorContext* context)
  {
    // Get a pointer to the current function that we're building
    Function* function = context->FunctionStack.Back();

    // Make an opcode that will jump to the end of the loop
    node->InstructionIndex = function->GetCurrentOpcodeIndex();
    node->JumpOpcode = &function->AllocateOpcode<RelativeJumpOpcode>(Instruction::RelativeGoTo, DebugOrigin::Break, node->Location);
  }

  //***************************************************************************
  void CodeGenerator::GenerateContinue(ContinueNode*& node, GeneratorContext* context)
  {
    // Get a pointer to the current function that we're building
    Function* function = context->FunctionStack.Back();

    // Make an opcode that will jump to the end of the loop
    node->InstructionIndex = function->GetCurrentOpcodeIndex();
    node->JumpOpcode = &function->AllocateOpcode<RelativeJumpOpcode>(Instruction::RelativeGoTo, DebugOrigin::Continue, node->Location);
  }

  //***************************************************************************
  void CodeGenerator::GenerateBinaryOperation(BinaryOperatorNode*& node, GeneratorContext* context)
  {
    // Get a reference to the current function that we're building
    Function* function = context->FunctionStack.Back();

    // Create the opcode so that we can fill it in
    size_t opcodeStart = function->GetCurrentOpcodeIndex();

    // Grab the token we used for this operator
    Grammar::Enum opToken = node->Operator->TokenId;

    // Get the instance of the type database
    Core& core = Core::GetInstance();

    // Store the binary operator info for convenience (shared definition)
    BinaryOperator& info = node->OperatorInfo;

    // If we need to flip the arguments (only matters for the opcode, but in these cases the result should be the same
    if (info.FlipArguments)
    {
      // Swap the left and right operands
      ExpressionNode* oldLeft = node->LeftOperand;
      node->LeftOperand = node->RightOperand;
      node->RightOperand = oldLeft;
    }

    // For debugging...
    DebugOrigin::Enum debug = DebugOrigin::BinaryOperation;

    // We always handle assignment specially since it's actually just the same as a copy opcode
    if (opToken == Grammar::Assignment)
    {
      // Generate code for the left and right operands
      context->Walker->Walk(this, node->LeftOperand, context);
      context->Walker->Walk(this, node->RightOperand, context);

      // Pull out things we need
      Type* type = node->RightOperand->ResultType;
      Operand& source = node->RightOperand->Access;
      Operand& destination = node->LeftOperand->Access;

      // If this is a strict property set...
      if (node->LeftOperand->IoUsage & IoMode::StrictPropertySet)
      {
        // We actually need to perform initialization, despite it being an assignment
        // because a strict property set just generates a temporary space to initialize
        CreateCopyOpcode(function, CopyMode::Initialize, type, source, destination, debug, node->Location);
      }
      else
      {
        // This is just a straight assignment copy
        CreateCopyOpcode(function, CopyMode::Assignment, type, source, destination, debug, node->Location);
      }
    }
    // If the operator results in an l-value...
    else if (info.Io & IoMode::WriteLValue)
    {
      // Generate code for the left and right operands
      context->Walker->Walk(this, node->LeftOperand, context);
      context->Walker->Walk(this, node->RightOperand, context);

      // Create the opcode
      BinaryLValueOpcode& opcode = function->AllocateOpcode<BinaryLValueOpcode>(info.Instruction, debug, node->Location);

      // All l-value binary operations result in a chained left hand value,
      // therefore we use the same primary and secondary index as the left operand
      // (no register allocation should ever be needed)
      node->Access = node->LeftOperand->Access;

      // We output to the left operand, and our right operand is the right expression
      opcode.Output = node->LeftOperand->Access;
      opcode.Right = node->RightOperand->Access;
    }
    // Otherwise, the operator results in an r-value...
    else
    {
      // All r-value binary operations result in a value (temporary) on the stack
      // Therefore we need to allocate a register to store our result in
      CreateLocal(function, node->ResultType->GetCopyableSize(), node->Access);

      // Handle the short-circuit operators specially
      if (opToken == Grammar::LogicalAnd || opToken == Grammar::LogicalOr)
      {
        // The first thing we do is always walk the left, because we could end up not running the right's opcode
        context->Walker->Walk(this, node->LeftOperand, context);

        // In both short circuit cases, we generate an if-jump that jumps after the right opcode
        Instruction::Enum ifInstruction = Instruction::InvalidInstruction;

        // For a logical 'and', we need to evaluate the first argument and if it returns false we early out
        if (opToken == Grammar::LogicalAnd)
        {
          ifInstruction = Instruction::IfFalseRelativeGoTo;
        }
        // For a logical 'or', we need to evaluate the first argument and if it returns true we early out
        else
        {
          ifInstruction = Instruction::IfTrueRelativeGoTo;
        }

        // OPTIMIZATION:
        // This could be removed if we just had the left operand directly output to our own local, instead of its own
        // Copy the resulting value from the left to the output, even though it may not be the final result
        GenerateCopyInitialize
        (
          function,
          node->ResultType,
          node->LeftOperand->Access,
          node->Access,
          DebugOrigin::BinaryOperation,
          node->Location
        );
        
        // Store the index that we generated the if opcode (so we know how far to relative jump)
        size_t ifFalseInstructionIndex = function->GetCurrentOpcodeIndex();

        // The condition (regardless of whether testing false or true) is always based on the left argument
        IfOpcode& ifOpcode = function->AllocateOpcode<IfOpcode>(ifInstruction, DebugOrigin::BinaryOperation, node->Location);
        ifOpcode.Condition = node->LeftOperand->Access;
        
        // Now generate opcode to evaluate the right argument
        context->Walker->Walk(this, node->RightOperand, context);

        // OPTIMIZATION: (see above)
        // This could be removed if we just had the right operand directly output to our own local, instead of its own
        // Copy the resulting value from the right to the output, if this occurs it will always be the final result (may be skipped by above if)
        // Not that it really matters for value types, but this copy is an 'Assignment', because technically the local will always be initialized above
        CreateCopyOpcode
        (
          function,
          CopyMode::Assignment,
          node->ResultType,
          node->RightOperand->Access,
          node->Access,
          DebugOrigin::BinaryOperation,
          node->Location
        );

        // The jump we generated before will skip directly to here, after the right argument gets evaluated
        size_t opcodeIndexAfterStatements = function->GetCurrentOpcodeIndex();
        ifOpcode.JumpOffset = (ByteCodeOffset)(opcodeIndexAfterStatements - ifFalseInstructionIndex);
      }
      else
      {
        // Note: The walkers MUST come before allocating the opcode (makes sense) otherwise the parent
        // opcode will run before the children's opcode get evaluated
        // Generate code for the left and right operands
        context->Walker->Walk(this, node->LeftOperand, context);
        context->Walker->Walk(this, node->RightOperand, context);

        // Create the opcode
        BinaryRValueOpcode& opcode = function->AllocateOpcode<BinaryRValueOpcode>(info.Instruction, debug, node->Location);

        // We always output to the stack
        opcode.Output = node->Access.HandleConstantLocal;

        // Initialize both operands
        opcode.Left = node->LeftOperand->Access;
        opcode.Right = node->RightOperand->Access;

        // The size is needed for some operations, such as value comparison
        opcode.Size = node->LeftOperand->ResultType->GetCopyableSize();
      }
    }

    // Error checking
    ErrorIf(opcodeStart == function->GetCurrentOpcodeIndex(), "No instructions were written!");

    // We have to generate set functions for any properties that need it
    GeneratorContext propContext;
    propContext.FunctionStack.PushBack(function);
    this->PropertySetWalker.Walk(this, node->LeftOperand, &propContext);
    propContext.FunctionStack.PopBack();
  }

  //***************************************************************************
  void CodeGenerator::GeneratePropertyDelegateOperation(PropertyDelegateOperatorNode*& node, GeneratorContext* context)
  {
    // Get a reference to the current function that we're building
    Function* function = context->FunctionStack.Back();

    // Attempt to get the operand as a member access node...
    // This operator only works on properties and members
    MemberAccessNode* member = Type::DynamicCast<MemberAccessNode*>(node->Operand);

    // We already verified that this was a member access in 
    ErrorIf(member == nullptr,
      "Somehow our operand was not a MemberAccessNode, even though we verified that in the Syntaxer");

    // Normally we would walk the operand, but we need to only
    // walk the operand's left since we're ignoring the property
    context->Walker->Walk(this, member->LeftOperand, context);

    // We only have a 'this' handle for the opcode if the property is not a static
    OperandLocal thisHandle = 0;
    if (!node->AccessedProperty->IsStatic)
    {
      // The 'this' of the property is going to be the left operand (who we're accessing the property on)
      Type* thisType = member->LeftOperand->ResultType;
      Operand& thisSource = member->LeftOperand->Access;

      // Create a location on the stack to store the 'this' handle
      thisHandle = function->AllocateRegister(sizeof(Handle));

      // Make sure that we make a handle out of the left argument (will be used as our this handle)
      // We'll save this new handle on the stack in the location that we allocated above
      GenerateHandleInitialize
      (
        function,
        thisType,
        thisSource,
        Operand(thisHandle),
        DebugOrigin::FunctionMemberAccess,
        node->Location
      );
    }

    // Make an opcode that will jump to the end of the loop
    CreatePropertyDelegateOpcode& opcode = function->AllocateOpcode<CreatePropertyDelegateOpcode>(Instruction::PropertyDelegate, DebugOrigin::PropertyDelegate, node->Location);

    // Tell the opcode the type it will be creating (this cast should always be safe, because property delegates are always bound types)
    opcode.CreatedType = (BoundType*)node->ResultType;
    
    // Setup the 'this' handle from where we get the properties
    opcode.ThisHandleLocal = thisHandle;

    // Setup the save handle where the property delegate goes
    // This expression's result will be stored in the last created register
    CreateLocal(function, sizeof(Handle), node->Access);
    opcode.SaveHandleLocal = node->Access.HandleConstantLocal;

    // Tell the opcode the getter and setter functions it will be using from the accessed property
    opcode.ReferencedProperty = node->AccessedProperty;
  }

  //***************************************************************************
  void CodeGenerator::GenerateUnaryOperation(UnaryOperatorNode*& node, GeneratorContext* context)
  {
    // Get a reference to the current function that we're building
    Function* function = context->FunctionStack.Back();

    // Generate code for the only operand
    context->Walker->Walk(this, node->Operand, context);

    // Generate the binary operator instructions, and also make sure to set our register/secondary index and access modes
    GenerateUnaryOp(function, *node, DebugOrigin::UnaryOperation);

    // We have to generate set functions for any properties that need it
    GeneratorContext propContext;
    propContext.FunctionStack.PushBack(function);
    this->PropertySetWalker.Walk(this, node->Operand, &propContext);
    propContext.FunctionStack.PopBack();
  }

  //***************************************************************************
  void CodeGenerator::GenerateMemberAccess(MemberAccessNode*& node, GeneratorContext* context)
  {
    if (node->MemberType == MemberAccessType::Field)
    {
      if (node->AccessedField->IsStatic)
        GenerateStaticFieldAccess(node, context);
      else
        GenerateFieldAccess(node, context);
    }
    else if (node->MemberType == MemberAccessType::Function)
    {
      GenerateFunctionDelegateMemberAccess(node, context);
    }
    else if (node->MemberType == MemberAccessType::Property)
    {
      GeneratePropertyGetMemberAccess(node, context);
    }
    else if (node->MemberType == MemberAccessType::Dynamic)
    {
      //GenerateDynamicGetMemberAccess(node, context);
    }
    else
    {
      Error("A member access type was used that we didn't know about, or memory got corrupted");
    }
  }
  
  //***************************************************************************
  void CodeGenerator::GenerateStaticFieldAccess(MemberAccessNode*& node, GeneratorContext* context)
  {
    // Get a reference to the current function that we're building
    Function* function = context->FunctionStack.Back();
    
    // Normally an Operand can actually point at a handle through another handle (class A containing a reference to another class B)
    // and this will work properly with GetOperand<Handle> where OperandType is Field
    // In this case, we only need to copy the handle for A to the stack, but not B (because again, Operand access solves this)
    // We do not need to copy any handles to the stack because the first one will be resolved by an OperandType of Static
    // Any subsequent member accesses will not be just a regular Field/Property accesses
    // which will automatically copy the handle to the stack when needed
    
    // Our current implementation is a bit silly, but we just shove a direct pointer to the Field*
    // into the operand's Field size_t value, (it will always fit, because size_t should be as big as a pointer)
    node->Access.StaticField = node->AccessedField;
    node->Access.FieldOffset = 0;

    // Just treat this as if it's just any other local on the stack
    node->Access.Type = OperandType::StaticField;
  }

  //***************************************************************************
  void CodeGenerator::GenerateFieldAccess(MemberAccessNode*& node, GeneratorContext* context)
  {
    // Get a reference to the current function that we're building
    Function* function = context->FunctionStack.Back();
    
    // For the awkward reason that type references are post expressions, we need to check
    if (node->LeftOperand != nullptr)
    {
      // Generate the code for the left operand
      context->Walker->Walk(this, node->LeftOperand, context);
    }
    
    // Get the type that we're performing the access on (not the resulting type, but basically leftType.SomeMember)
    Type* leftType = node->LeftOperand->ResultType;

    // Delegates will need a special
    ErrorIf(Type::DynamicCast<DelegateType*>(leftType) != nullptr,
      "I haven't properly handled accessing members on delegates yet, see below");

    // If the left-hand type is a handle (indirection type or a reference type)...
    if (Type::IsHandleType(leftType))
    {
      // Set the handle index to be the left's primary index
      OperandIndex handleIndex = node->LeftOperand->Access.HandleConstantLocal;

      // If the left type is accessed as a data member... (we need to copy it onto the stack!)
      // Even when we access a static field (and then we access a member on that field, eg TypeMemberAccess -> MemberAccess)
      // we still need to copy the handle to a stack local before we use it further
      if (node->LeftOperand->Access.Type == OperandType::Field || node->LeftOperand->Access.Type == OperandType::StaticField)
      {
        // Allocate a register to store the handle
        handleIndex = function->AllocateRegister(leftType->GetCopyableSize());

        // Generate a copy to bring the handle to the local register
        // Since we are not assigning (we're copying this to the stack) then
        // this is a copy over uninitialized memory
        GenerateCopyInitialize
        (
          function,
          leftType,
          node->LeftOperand->Access,
          Operand(handleIndex),
          DebugOrigin::DataMemberAccess,
          node->Location
        );
      }

      // Set our primary index to be the handle index
      node->Access.HandleConstantLocal = handleIndex;

      // Use the member index into the class
      node->Access.FieldOffset = node->AccessedField->Offset;

      // We are accessed as a field...
      node->Access.Type = OperandType::Field;
    }
    // If the left-hand type is a data-type / value...
    else
    {
      // If the left hand side is entirely on the stack
      if (node->LeftOperand->Access.Type == OperandType::Local)
      {
        // Simply just offset the primary index so that it points at the member on the stack
        node->Access.HandleConstantLocal = (OperandIndex)(node->LeftOperand->Access.HandleConstantLocal + node->AccessedField->Offset);

        // Our secondary index is zero since we don't use it
        node->Access.FieldOffset = 0;

        // Just treat this as if it's just any other local on the stack
        node->Access.Type = OperandType::Local;
      }
      // If the left hand side is still being accessed as a field
      else if (node->LeftOperand->Access.Type == OperandType::Field)
      {
        // Our access is just the same access as the field itself (this logic is recursive for as many struct accesses beyond this)
        node->Access = node->LeftOperand->Access;

        // Our offset is just the previous structs offset plus the offset to that newly accessed field
        node->Access.FieldOffset += node->AccessedField->Offset;
      }
      // If the left hand side is accessed as a static field
      // Note: We never need to worry about the left hand side being a type, and us being a static
      // because that would make our node a TypeMemberAccess, which is handled in GenerateStaticFieldAccess
      // This is the case where we are accessing a member, and our left operand should be a TypeMemberAccess (not us)
      else if (node->LeftOperand->Access.Type == OperandType::StaticField)
      {
        // Our access is just the same access as the field itself (this logic is recursive for as many struct accesses beyond this)
        node->Access = node->LeftOperand->Access;
        
        // Our offset is just the previous structs offset plus the offset to that newly accessed field
        node->Access.FieldOffset += node->AccessedField->Offset;
      }
      else
      {
        // We should never be able to get here
        Error("Unhandled case");
      }
    }
  }

  //***************************************************************************
  void CodeGenerator::GenerateFunctionDelegateMemberAccess(MemberAccessNode*& node, GeneratorContext* context)
  {
    // Get a reference to the current function that we're building
    Function* function = context->FunctionStack.Back();

    // For the awkward reason that type references are post expressions, we need to check
    if (node->LeftOperand != nullptr)
    {
      // Generate the code for the left operand
      context->Walker->Walk(this, node->LeftOperand, context);
    }

    // A temporary barrier whilst we don't have a way to resolve overloads with anything other than function calls
    if (node->AccessedFunction == nullptr)
      return;
    
    // Note: In the case where we're accessing a static from a type reference the
    // 'node->LeftOperand' will always be null, but note we do not need it (no this handle)!
    
    // If the function we're calling is a member function (not a static function)
    if (node->AccessedFunction->This != nullptr)
    {
      this->CreateInstanceDelegateAndThisHandle
      (
        function,
        node->AccessedFunction,
        node->LeftOperand->ResultType, // Note that the left operand in a member access is the object ('this')
        node->LeftOperand->Access,
        node->Access,
        (node->Operator != Grammar::NonVirtualAccess),
        node->Location,
        DebugOrigin::FunctionMemberAccess
      );
    }
    else
    {
      this->CreateStaticDelegate
      (
        function,
        node->AccessedFunction,
        node->Access,
        node->Location,
        DebugOrigin::FunctionMemberAccess
      );
    }
  }

  //***************************************************************************
  void CodeGenerator::GeneratePropertyGetMemberAccess(MemberAccessNode*& node, GeneratorContext* context)
  {
    // Get a reference to the current function that we're building
    Function* function = context->FunctionStack.Back();
    
    // For the awkward reason that type references are post expressions, we need to check
    if (node->LeftOperand != nullptr)
    {
      // Generate the code for the left operand
      context->Walker->Walk(this, node->LeftOperand, context);
    }

    // Note: In the case where we're accessing a static from a type reference the
    // 'node->LeftOperand' will always be null, but note we do not need it (no this handle)!
    
    // Get the property for ease of use
    GetterSetter* property = node->AccessedGetterSetter;
    
    // Check if we're even being read
    if ((node->IoUsage & IoMode::ReadRValue) != 0)
    {
      // Get the 'get' for ease of use
      Function* get = property->Get;

      // We generate a delegate before calling the function
      Operand delegateLocal;

      // If the get function we're calling is a member function (not a static function)
      if (get->This != nullptr)
      {
        this->CreateInstanceDelegateAndThisHandle
        (
          function,
          get,
          node->LeftOperand->ResultType, // Note that the left operand in a member access is the object ('this')
          node->LeftOperand->Access,
          delegateLocal,
          (node->Operator != Grammar::NonVirtualAccess),
          node->Location,
          DebugOrigin::PropertyGetMemberAccess
        );
      }
      else
      {
        this->CreateStaticDelegate
        (
          function,
          get,
          delegateLocal,
          node->Location,
          DebugOrigin::PropertyGetMemberAccess
        );
      }

      // Generate opcode for calling the function (we still need to copy arguments ourselves)
      GenerateCallOpcodePreArgs(function, get->FunctionType, delegateLocal, node->Location, DebugOrigin::PropertyGetMemberAccess);

      // Generate opcode for finishing up the call to the function
      GenerateCallOpcodePostArgs(function, get->FunctionType, &node->Access, node->Location, DebugOrigin::PropertyGetMemberAccess);
    }
    // Check if we're ONLY being written to
    else if ((node->IoUsage & IoMode::WriteLValue) != 0)
    {
      // We need to let any assignments know that this is NOT an initialized
      // value, and therefore the copy must also perform initialization!
      // Note: Assignment should be the only thing possible with a set-only property
      node->IoUsage = (IoMode::Enum)(node->IoUsage | IoMode::StrictPropertySet);

      // Make space that any operator will write to
      CreateLocal(function, property->PropertyType->GetCopyableSize(), node->Access);
    }
    else
    {
      // Otherwise, we're not being used by anything
      ErrorIf(node->IoUsage != IoMode::Ignore, "Unexpected case");
    }
  }


  //***************************************************************************
  void CodeGenerator::GeneratePropertySetMemberAccess(MemberAccessNode*& node, GeneratorContext* context)
  {
    // If this node is a property, we don't care about it
    if (node->MemberType != MemberAccessType::Property)
      return;

    // Get a reference to the current function that we're building
    Function* function = context->FunctionStack.Back();
    
    // Note: We never need to walk the left operand (it was already done)

    // Check if we're even being written to
    if ((node->IoUsage & IoMode::WriteLValue) != 0)
    {
      // Get the property for ease of use
      GetterSetter* property = node->AccessedGetterSetter;
      Function* set = property->Set;

      // We generate a delegate before calling the function
      Operand delegateLocal;

      // If the set function we're calling is a member function (not a static function)
      if (set->This != nullptr)
      {
        this->CreateInstanceDelegateAndThisHandle
        (
          function,
          set,
          node->LeftOperand->ResultType, // Note that the left operand in a member access is the object ('this')
          node->LeftOperand->Access,
          delegateLocal,
          (node->Operator != Grammar::NonVirtualAccess),
          node->Location,
          DebugOrigin::PropertySetMemberAccess
        );
      }
      else
      {
        this->CreateStaticDelegate
        (
          function,
          set,
          delegateLocal,
          node->Location,
          DebugOrigin::PropertySetMemberAccess
        );
      }

      // Generate opcode for calling the function (we still need to copy arguments ourselves)
      GenerateCallOpcodePreArgs(function, set->FunctionType, delegateLocal, node->Location, DebugOrigin::PropertySetMemberAccess);

      // Check if we were read, then we assume that 'get' was
      // already called, and our modified value is on the stack
      if ((node->IoUsage & IoMode::ReadRValue) != 0)
      {
        // Note: In the case where 'get'  was called, we know our ResultType is the property type
        ErrorIf(node->ResultType != property->PropertyType,
          "The resulting type when a property has 'get' called on it should be the property type");

        // Generate the opcode for copying a parameter in a function call
        GenerateCopyToParameter
        (
          function,
          node->ResultType,
          node->Access,
          set->FunctionType->Parameters.Front().StackOffset,
          DebugOrigin::PropertySetMemberAccess,
          node->Location
        );
      }
      else
      {
        // Generate the opcode for copying a parameter in a function call
        GenerateCopyToParameter
        (
          function,
          property->PropertyType,
          node->Access,
          set->FunctionType->Parameters.Front().StackOffset,
          DebugOrigin::PropertySetMemberAccess,
          node->Location
        );
      }

      // Generate opcode for finishing up the call to the function
      GenerateCallOpcodePostArgs(function, set->FunctionType, nullptr, node->Location, DebugOrigin::PropertySetMemberAccess);
    }
  }


  //***************************************************************************
  void CodeGenerator::GenerateInitializer(InitializerNode*& node, GeneratorContext* context)
  {
    // Get a reference to the current function that we're building
    Function* function = context->FunctionStack.Back();

    // If the function we're calling is a member function (not a static function)
    if (node->InitializerFunction->This != nullptr)
    {
      this->CreateInstanceDelegateAndThisHandle
      (
        function,
        node->InitializerFunction,
        function->This->ResultType,     // The this type is the type we're compiling
        Operand(function->This->Local), // Get the 'this' handle from the local variable
        node->Access,
        false,
        node->Location,
        DebugOrigin::FunctionMemberAccess
      );
    }
    else
    {
      this->CreateStaticDelegate
      (
        function,
        node->InitializerFunction,
        node->Access,
        node->Location,
        DebugOrigin::FunctionMemberAccess
      );
    }
  }

  //***************************************************************************
  template <typename T>
  T& CodeGenerator::DelegateOpcode
  (
    Function*           caller,
    Function*           toCall,
    OperandIndex        delegateDest,
    const CodeLocation& location,
    Instruction::Enum   instruction,
    DebugOrigin::Enum   debug
  )
  {
    // Create an opcode that will create a delegate
    T& opcode = caller->AllocateOpcode<T>(instruction, debug, location);
    opcode.BoundFunction = toCall;

    // We'll save the delegate at the location indicated by this node
    opcode.SaveLocal = delegateDest;

    // Return the allocated opcode
    return opcode;
  }

  //***************************************************************************
  void CodeGenerator::CreateInstanceDelegateAndThisHandle
  (
    Function*           caller,
    Function*           toCall,
    Type*               thisType,
    const Operand&      thisSource,
    Operand&            delegateDestOut,
    bool                canBeVirtual,
    const CodeLocation& location,
    DebugOrigin::Enum   debug
  )
  {
    // Note: 'thisSource' is NOT necessarily a handle
    // as it is possible to invoke a function on value types (we need to generate a handle for those cases)
    // By default we just assume the source is a handle
    Operand thisHandle = thisSource;

    // If the type is not already a handle, we need to make a handle for it on the stack
    if (Type::IsHandleType(thisType) == false)
    {
      // Create a location on the stack to store the 'this' handle
      OperandLocal thisHandleLocal = caller->AllocateRegister(sizeof(Handle));
      thisHandle = Operand(thisHandleLocal);

      // Make sure that we make a handle out of the left argument (will be used as our this handle)
      // We'll save this new handle on the stack in the location that we allocated above
      GenerateHandleInitialize
      (
        caller,
        thisType,
        thisSource,
        Operand(thisHandleLocal),
        DebugOrigin::FunctionMemberAccess,
        location
      );
    }

    // Now create the delegate (the 'this' handle could be copied, or not)
    this->CreateInstanceDelegateWithThisHandle
    (
      caller,
      toCall,
      thisHandle,
      delegateDestOut,
      canBeVirtual,
      location,
      debug
    );
  }

  //***************************************************************************
  void CodeGenerator::CreateInstanceDelegateWithThisHandle
  (
    Function*           caller,
    Function*           toCall,
    const Operand&      thisHandle,
    Operand&            delegateDestOut,
    bool                canBeVirtual,
    const CodeLocation& location,
    DebugOrigin::Enum   debug
  )
  {
    // This expression's result will be stored in the last created register
    CreateLocal(caller, toCall->FunctionType->GetCopyableSize(), delegateDestOut);

    // Create an instance delegate opcode
    CreateInstanceDelegateOpcode& delegateOpcode = DelegateOpcode<CreateInstanceDelegateOpcode>
    (
      caller,
      toCall,
      delegateDestOut.HandleConstantLocal,
      location,
      Instruction::CreateInstanceDelegate,
      debug
    );

    // Mark whether or not this member can be virtual
    delegateOpcode.CanBeVirtual = canBeVirtual;

    // Let the delegate know where to get it's 'this' handle from
    delegateOpcode.ThisHandle = thisHandle;
  }

  //***************************************************************************
  void CodeGenerator::CreateStaticDelegate
  (
    Function*           caller,
    Function*           toCall,
    Operand&            delegateDest,
    const CodeLocation& location,
    DebugOrigin::Enum   debug
  )
  {
    // This expression's result will be stored in the last created register
    CreateLocal(caller, toCall->FunctionType->GetCopyableSize(), delegateDest);

    // Create a static delegate opcode
    DelegateOpcode<CreateStaticDelegateOpcode>
    (
      caller,
      toCall,
      delegateDest.HandleConstantLocal,
      location,
      Instruction::CreateStaticDelegate,
      debug
    );
  }

  //***************************************************************************
  void CodeGenerator::GenerateTypeCast(TypeCastNode*& node, GeneratorContext* context)
  {
    // Get a reference to the current function that we're building
    Function* function = context->FunctionStack.Back();

    // Generate the code for the operand of the type cast
    context->Walker->Walk(this, node->Operand, context);

    // Generate a copy opcode for the return type to the return value
    // (also puts the result in a register and sets this node's register index)
    GenerateConversion(function, *node, DebugOrigin::TypeCast);
  }
  
  //***************************************************************************
  void CodeGenerator::GenerateTypeId(TypeIdNode*& node, GeneratorContext* context)
  {
    // Get a reference to the current function that we're building
    Function* function = context->FunctionStack.Back();

    // If we have a value...
    if (node->Value != nullptr)
    {
      // Technically the expression inside a type-id does not need
      // to be run, but just for the principle of least surprise
      context->Walker->Walk(this, node->Value, context);
    }

    // If this is a compile time type that was given, then this is always just a constant
    if (node->CompileTimeSyntaxType != nullptr)
    {
      // We treat the type as a constant
      node->Access.Type = OperandType::Constant;
      node->Access.FieldOffset = 0;

      // Create a handle in constant space for the type pointer
      Handle& handle = function->AllocateConstant<Handle>(node->ResultType->GetCopyableSize(), node->Access.HandleConstantLocal);

      // Set the handle manager and type of the handle
      // We know explicitly that the string handle manager is a shared manager
      handle.Manager = HandleManagers::GetInstance().GetManager(ZilchManagerId(PointerManager));
      handle.StoredType = Type::DynamicCast<BoundType*>(node->ResultType);

      // Store the pointer to the compile time type into the handle
      handle.Manager->ObjectToHandle((byte*)node->CompileTimeType, handle.StoredType, handle);
    }
    else
    {
      // This expression's result will be stored in the last created register
      this->CreateLocal(function, sizeof(Handle), node->Access);

      // Set the register indices for the operands, and set the location that the result should be stored into
      TypeIdOpcode& opcode = function->AllocateOpcode<TypeIdOpcode>(Instruction::TypeId, DebugOrigin::TypeId, node->Location);
      opcode.Expression = node->Value->Access;
      opcode.SaveTypeHandleLocal = node->Access.HandleConstantLocal;
      opcode.CompileTimeType = node->CompileTimeType;
    }
  }

  //***************************************************************************
  void CodeGenerator::GenerateMemberId(MemberIdNode*& node, GeneratorContext* context)
  {
    // Get a reference to the current function that we're building
    Function* function = context->FunctionStack.Back();
    
    // Note: MemberId never walks the actual member

    // We treat the type as a constant
    node->Access.Type = OperandType::Constant;
    node->Access.FieldOffset = 0;

    // Create a handle in constant space for the type pointer
    Handle& handle = function->AllocateConstant<Handle>(node->ResultType->GetCopyableSize(), node->Access.HandleConstantLocal);

    // Set the handle manager and type of the handle
    // We know explicitly that the string handle manager is a shared manager
    handle.Manager = HandleManagers::GetInstance().GetManager(ZilchManagerId(PointerManager));
    handle.StoredType = Type::DynamicCast<BoundType*>(node->ResultType);

    // Store the pointer to the accessed type into the handle
    handle.Manager->ObjectToHandle((byte*)node->Member->AccessedMember, handle.StoredType, handle);
  }

  //***************************************************************************
  void CodeGenerator::GenerateLocalVariableReference(LocalVariableReferenceNode*& node, GeneratorContext* /*context*/)
  {
    // Point to the exact same register as the local variable
    node->Access.HandleConstantLocal = node->AccessedVariable->Local;

    // We have no secondary index
    node->Access.FieldOffset = 0;

    // We are accessing a local variable on the stack
    node->Access.Type = OperandType::Local;
  }

  //***************************************************************************
  void CodeGenerator::GenerateReturnValue(ReturnNode*& node, GeneratorContext* context)
  {
    // Get a reference to the current function that we're building
    Function* function = context->FunctionStack.Back();

    // Loop through all the return values
    if (node->ReturnValue != nullptr)
    {
      // Store the return value for convenience
      ExpressionNode* returnValue = node->ReturnValue;

      // Generate the code for the current return expression
      context->Walker->Walk(this, returnValue, context);

      // Generate a copy opcode for the return type to the return value
      // The return value should be uninitialized memory before this point
      // so this must be an initialized copy
      GenerateCopyToReturn
      (
        function,
        returnValue->ResultType,
        returnValue->Access,
        Operand(0),
        DebugOrigin::ReturnValue,
        returnValue->Location
      );
    }

    // Generate the return opcode, which simply just stops execution of a function
    function->AllocateArgumentFreeOpcode(Instruction::Return, DebugOrigin::ReturnValue, node->Location);
  }

  //***************************************************************************
  void CodeGenerator::GenerateFunctionCall(FunctionCallNode*& node, GeneratorContext* context)
  {
    // If we have no left operand, don't bother walking it (such is the case with attribute calls)
    if (node->LeftOperand == nullptr)
      return;

    // Get a reference to the current function that we're building
    Function* function = context->FunctionStack.Back();

    // Generate the code for the left operand first
    context->Walker->Walk(this, node->LeftOperand, context);

    // Note: We generate code for the passed in arguments below, but we needed to do the left
    //       expression right here since we immediately grab its delegate type and start using it
    Operand delegateLocal;

    // Store the delegate type that we'll be calling
    DelegateType* delegateType = nullptr;

    // For debugging purposes
    DebugOrigin::Enum debugOrigin = DebugOrigin::FunctionCall;

    // We need to generate access for a return value (except in some cases, like creation calls!)
    Operand* returnValueAccess = &node->Access;

    // If the left hand node is a creation call node (basically is this a constructor?)...
    StaticTypeNode* creationNode = node->FindCreationCall();
    if (creationNode != nullptr)
    {
      // A creation call does not return a handle to the created value, primarily because
      // we do not know whether it is being allocated as a handle or as a local!
      // Therefore, it always returns void (however, we definitely do not want to generate anything for returns)
      returnValueAccess = nullptr;

      // Update the debug origin to be a little more clear (we're in a constructor)
      debugOrigin = DebugOrigin::FunctionCallConstructor;

      // Technically this constructor call comes above the actual new/local in the syntax tree
      // which means that whoever is referencing the new/local will actually be getting our FunctionCallNode result
      // Because this is specifically a constructor, we need to return exactly what the left operand returns
      // Our result type should have already been set in the syntaxer phase
      node->Access = creationNode->Access;

      // If we have no constructor function (eg the default constructor, or just pre-constructor)
      if (creationNode->ConstructorFunction == nullptr)
      {
        // Return early, since there is no function call to be made
        return;
      }

      // Create an instance delegate for the constructor
      this->CreateInstanceDelegateWithThisHandle
      (
        function,
        creationNode->ConstructorFunction,
        Operand(creationNode->ThisHandleLocal),
        delegateLocal,
        false,
        node->Location,
        debugOrigin
      );

      // The delegate type should be grabbed from the constructor
      delegateType = creationNode->ConstructorFunction->FunctionType;
    }
    else
    {
      // The delegate local is the left expression
      delegateLocal = node->LeftOperand->Access;

      // The delegate type should be grabbed from the left operand
      delegateType = Type::DynamicCast<DelegateType*>(node->LeftOperand->ResultType);
    }

    // Generate opcode for calling the function (we still need to copy arguments ourselves)
    GenerateCallOpcodePreArgs(function, delegateType, delegateLocal, node->Location, debugOrigin);

    ZilchTodo("Parameters are currently not re-ordered");

    // Loop through all the function's parameters
    for (size_t i = 0; i < node->Arguments.Size(); ++i)
    {
      // Get the expression node that will be placed into the parameter
      ExpressionNode* currentArgument = node->Arguments[i];

      // Generate the code for the current expression
      context->Walker->Walk(this, currentArgument, context);

      // Generate the opcode for copying a parameter in a function call
      GenerateCopyToParameter
      (
        function,
        currentArgument->ResultType,
        currentArgument->Access,
        delegateType->Parameters[i].StackOffset,
        DebugOrigin::FunctionCall,
        currentArgument->Location
      );
    }

    // Generate opcode for finishing up the call to the function
    GenerateCallOpcodePostArgs(function, delegateType, returnValueAccess, node->Location, debugOrigin);
  }


  //***************************************************************************
  void CodeGenerator::GenerateCallOpcodePreArgs
  (
    Function* caller,
    DelegateType* delegateTypeToCall,
    const Operand& delegateOperand,
    const CodeLocation& location,
    DebugOrigin::Enum debugOrigin
  )
  {
    // Record the current offset in instructions (this is not where we will jump to)
    size_t opcodePosBeforeThisCopy = caller->GetCurrentOpcodeIndex();
    
    // Allocate the prep-for-function instruction, and give it the index that the 
    // function lives as well as the register that we'd like to store the return value
    PrepForFunctionCallOpcode& prepOpcode = caller->AllocateOpcode<PrepForFunctionCallOpcode>(Instruction::PrepForFunctionCall, debugOrigin, location);
    prepOpcode.Delegate = delegateOperand;

    // We basically always generate the copy opcode for copying a 'this' parameter, even when its a static function
    // That being said, this opcode will be skipped if the PrepForFunctionCall opcode determines that the calling function is not static
    // Note that the calling convention is currently that the 'this' object goes last, but that it's the first opcode to be run
    // Being last in memory allows it to be easily omitted for static function calls
    // Being the first opcode to run allows the opcode to easily be skipped after the PrepForFunctionCall
    // See CodeGenerator.cpp, approximately line 292, in the function FunctionContext is the counterpart to this explanation
    {
      // Determine the position of the handle (it resides inside the delegate)
      Operand handleOperand = delegateOperand;

      if (handleOperand.Type == OperandType::Field)
      {
        handleOperand.FieldOffset += offsetof(Delegate, ThisHandle);
      }
      else
      {
        handleOperand.HandleConstantLocal += offsetof(Delegate, ThisHandle);
      }

      // Get the instance of the type database
      Core& core = Core::GetInstance();

      // Copy the handle from the delegate local into the first argument
      GenerateCopyToParameter
      (
        caller,
        core.NullType,
        handleOperand,
        delegateTypeToCall->ThisHandleStackOffset,
        debugOrigin,
        location
      );
    }

    // We want to jump to the next opcode if the function is static
    prepOpcode.JumpOffsetIfStatic = (OperandIndex)(caller->GetCurrentOpcodeIndex() - opcodePosBeforeThisCopy);
  }


  //***************************************************************************
  void CodeGenerator::GenerateCallOpcodePostArgs
  (
    Function* caller,
    DelegateType* delegateTypeToCall,
    Operand* returnAccessOut,
    const CodeLocation& location,
    DebugOrigin::Enum debugOrigin
  )
  {
    // Generate an opcode for a function call (the last parameter is where the return value will be stored)
    caller->AllocateArgumentFreeOpcode(Instruction::FunctionCall, debugOrigin, location);

    // Grab the core
    Core& core = Core::GetInstance();

    // In certain cases it is possible that we're not even able to get the return value (even if it is void!)
    // For example, the case of a setter (technically returns void, which is storable, but cannot every be accessed)
    // In these cases, the 'returnAccessOut' will be null since there's nothing to return
    if (returnAccessOut != nullptr)
    {
      // Note: We used to have an exception here for Void types (we wouldn't generate a return because they could not be stored)
      // Now void types can actually be stored, so therefore we must create a local for the void type
      // Having said that, a Void type has a size of 0 (creating a 0 sized local does nothing)
      // Moreover, we should also not copy the void type to the return, as there is nothing to copy (optimization and sanity!)

      // Note: There was a previous bug where we relied upon the node->ResultType as our return, which is true
      // for all function calls, however this is not the case in terms of constructor calls
      // A constructor call's delegateType will always be null (and for a function, it should be the same as node->ResultType)

      // This function call's result will be stored in the last created register
      CreateLocal(caller, delegateTypeToCall->Return->GetCopyableSize(), *returnAccessOut);

      // Generate the opcode for copying a return in a function call
      GenerateCopyFromReturn
      (
        caller,
        delegateTypeToCall->Return,
        0,
        returnAccessOut->HandleConstantLocal,
        debugOrigin,
        location
      );
    }
  }

  //***************************************************************************
  void CodeGenerator::GenerateStringInterpolants(StringInterpolantNode*& node, GeneratorContext* context)
  {
    // Get a reference to the current function that we're building
    Function* function = context->FunctionStack.Back();

    // We need to create a temporary string builder that we use to efficiently concatenate strings together
    function->AllocateOpcode<BeginStringBuilderOpcode>(Instruction::BeginStringBuilder, DebugOrigin::StringInterpolant, node->Location);

    // Walk through all the children we want to stringify
    for (size_t i = 0; i < node->Elements.Size(); ++i)
    {
      // Get the current element (it may be a string itself...)
      ExpressionNode* elementNode = node->Elements[i];

      // Make sure we generate code for the element expression
      context->Walker->Walk(this, elementNode, context);

      // Set the register indices for the operands, and set the location that the result should be stored into
      AddToStringBuilderOpcode& opcode = function->AllocateOpcode<AddToStringBuilderOpcode>(Instruction::AddToStringBuilder, DebugOrigin::StringInterpolant, node->Location);
      opcode.Value = elementNode->Access;
      opcode.TypeToConvert = elementNode->ResultType;
    }

    // Make space for the string local (the string should be the result type)
    CreateLocal(function, node->ResultType->GetCopyableSize(), node->Access);

    // Finish off the string builder (with all our additions to it) and store the resulting string on the stack
    EndStringBuilderOpcode& endOpcode = function->AllocateOpcode<EndStringBuilderOpcode>(Instruction::EndStringBuilder, DebugOrigin::StringInterpolant, node->Location);
    endOpcode.SaveStringHandleLocal = node->Access.HandleConstantLocal;
  }

  //***************************************************************************
  void CodeGenerator::CollectValue(ValueNode*& node, GeneratorContext* context)
  {
    // Get a reference to the current function that we're building
    Function* function = context->FunctionStack.Back();

    // A value is always a constant
    node->Access.Type = OperandType::Constant;

    // We have no secondary index
    node->Access.FieldOffset = 0;

    // Based off the type of token...
    switch (node->Value.TokenId)
    {
      // The value is an integer
      case Grammar::IntegerLiteral:
      {
        // Read the value as an Integer (and allocate a constant for it)
        function->AllocateConstant<Integer>(node->ResultType->GetCopyableSize(), node->Access.HandleConstantLocal) = atoi(node->Value.Token.c_str());
        break;
      }

      // The value is an double integer
      case Grammar::DoubleIntegerLiteral:
      {
        // Read the value as an Integer (and allocate a constant for it)
        function->AllocateConstant<DoubleInteger>(node->ResultType->GetCopyableSize(), node->Access.HandleConstantLocal) = ZilchStringToDoubleInteger(node->Value.Token.c_str(), 10);
        break;
      }
      
      // The value is a real
      case Grammar::RealLiteral:
      {
        // Read the value as a Real (and allocate a constant for it)
        function->AllocateConstant<Real>(node->ResultType->GetCopyableSize(), node->Access.HandleConstantLocal) = (Real)atof(node->Value.Token.c_str());
        break;
      }
      
      // The value is a double real
      case Grammar::DoubleRealLiteral:
      {
        // Read the value as a DoubleReal (and allocate a constant for it)
        function->AllocateConstant<DoubleReal>(node->ResultType->GetCopyableSize(), node->Access.HandleConstantLocal) = (DoubleReal)atof(node->Value.Token.c_str());
        break;
      }
      
      // The value is a string
      case Grammar::StringLiteral:
      {
        // Get a reference to the core library
        Core& core = Core::GetInstance();

        // Create a new handle that will point at a string node
        Handle& handle = function->AllocateConstant<Handle>(core.StringType->GetCopyableSize(), node->Access.HandleConstantLocal);

        // Set the handle manager and type of the handle
        // We know explicitly that the string handle manager is a shared manager
        handle.Manager = HandleManagers::GetInstance().GetManager(ZilchManagerId(StringManager));
        handle.StoredType = core.StringType;

        // The token is always stored in it's original vanilla form
        // We need to get rid of any quotes and perform escape replacements
        String& token = node->Value.Token;
        String unescapedString = ReplaceStringEscapesAndStripQuotes(token);
        
        // Create the string literal in the library builder
        const String& stringLiteral = this->Builder->AddStringLiteral(unescapedString);

        // Copy a string into the handle
        // This method will actually increase the reference count
        // which means we would need to store an array of destructors for constant memory (which we do)
        handle.Manager->ObjectToHandle((byte*)&stringLiteral, handle.StoredType, handle);
        break;
      }
      
      // The value is a bool
      case Grammar::True:
      {
        // Read the value as a "real" (and allocate a constant for it)
        function->AllocateConstant<Boolean>(node->ResultType->GetCopyableSize(), node->Access.HandleConstantLocal) = true;
        break;
      }
      
      // The value is a bool
      case Grammar::False:
      {
        // Read the value as a "real" (and allocate a constant for it)
        function->AllocateConstant<Boolean>(node->ResultType->GetCopyableSize(), node->Access.HandleConstantLocal) = false;
        break;
      }
      
      // The value is a null
      case Grammar::Null:
      {
        ZilchTodo("We probably want to eventually share null constants, also examine the behavior of using sizeof(Delegate)!");

        // At the moment, since we use null for handles, delegates, etc, we just allocate wiped space (with 0s) that
        // is big enough to support all nullable things (all primitives support being set to all 0)
        size_t largeIndex;
        byte* data = function->Constants.Allocate(sizeof(Delegate), nullptr, nullptr, &largeIndex);
        memset(data, 0, sizeof(Delegate));
        node->Access.HandleConstantLocal = (OperandIndex)largeIndex;
        break;
      }

      default:
      {
        // We don't know what type it is???
        // This especially should not be an identifier, since identifiers are caught as VariableReferences
        Error("The node was marked as being a value node, but had a token that was not recognizable as a value type");
      }
    }
  }

  //***************************************************************************
  void CodeGenerator::GenerateDelete(DeleteNode*& node, GeneratorContext* context)
  {
    // Get a reference to the current function that we're building
    Function* function = context->FunctionStack.Back();

    // Make sure we generate code for the deleted object expression (its most likely a variable reference)
    context->Walker->Walk(this, node->DeletedObject, context);

    // Set the register indices for the operands, and set the location that the result should be stored into
    DeleteObjectOpcode& opcode = function->AllocateOpcode<DeleteObjectOpcode>(Instruction::DeleteObject, DebugOrigin::DeleteObject, node->Location);
    opcode.Object = node->DeletedObject->Access;
  }
  
  //***************************************************************************
  void CodeGenerator::GenerateThrow(ThrowNode*& node, GeneratorContext* context)
  {
    // Get a reference to the current function that we're building
    Function* function = context->FunctionStack.Back();

    // Make sure we generate code for the exception expression
    context->Walker->Walk(this, node->Exception, context);

    // Set the register indices for the operands, and set the location that the result should be stored into
    ThrowExceptionOpcode& opcode = function->AllocateOpcode<ThrowExceptionOpcode>(Instruction::ThrowException, DebugOrigin::ThrowException, node->Location);
    opcode.Exception = node->Exception->Access;
  }

  //***************************************************************************
  void CodeGenerator::GenerateExpressionInitializer(ExpressionInitializerNode*& node, GeneratorContext* context)
  {
    // Walk the left operand first (allocate and pre-construct an object, then invoke a constructor on it)
    context->Walker->Walk(this, node->LeftOperand, context);

    // Now walk the rest of the statements to invoke 'Add's and initialize members
    context->Walker->Walk(this, node->InitializerStatements, context);

    // Our output is just the output of the construction call
    node->Access = node->LeftOperand->Access;
  }
  
  //***************************************************************************
  void CodeGenerator::GenerateMultiExpression(MultiExpressionNode*& node, GeneratorContext* context)
  {
    // Generate code for all the expressions
    context->Walker->Walk(this, node->Expressions, context);

    // Forward the access from the yielded expression to ourself
    ExpressionNode* yieldedExpression = node->Expressions[node->YieldChildExpressionIndex];
    node->Access = yieldedExpression->Access;
  }

  //***************************************************************************
  void CodeGenerator::GenerateStaticTypeOrCreationCall(StaticTypeNode*& node, GeneratorContext* context)
  {
    // If this is being used as a creation...
    if (node->Mode == CreationMode::Invalid)
      return;

    // Note: The NewObject/LocalObject opcodes only invoke the pre-constructor (via InvokePreConstructorOrRelease)
    // This pre-constructs all classes up to the base class
    // The function call that has the CreationCallNode as its Operand is what is responsible for invoking the real constructor
    // Constructors walk up and invoke their base constructors, via the InitializerNode

    // Get a reference to the current function that we're building
    Function* function = context->FunctionStack.Back();

    // This expression's result will be stored in the last created register
    this->CreateLocal(function, node->ResultType->GetCopyableSize(), node->Access);

    // If we're creating a heap object with 'new'
    if (node->Mode == CreationMode::New)
    {
      // Set the register indices for the operands, and set the location that the result should be stored into
      CreateTypeOpcode& opcode = function->AllocateOpcode<CreateTypeOpcode>(Instruction::NewObject, DebugOrigin::NewObject, node->Location);
      opcode.SaveHandleLocal = node->Access.HandleConstantLocal;
      opcode.CreatedType = node->ReferencedType;

      // We want an object handle for other calls (such as the constructor)
      node->ThisHandleLocal = node->Access.HandleConstantLocal;
    }
    // Otherwise we're creating a local or member object with 'local'
    else
    {
      // Set the register indices for the operands, and set the location that the result should be stored into
      CreateLocalTypeOpcode& opcode = function->AllocateOpcode<CreateLocalTypeOpcode>(Instruction::LocalObject, DebugOrigin::LocalObject, node->Location);
      opcode.CreatedType = node->ReferencedType;

      // Error checking
      ErrorIf(node->ReferencedType != node->ResultType,
        "The created type should always be the expression type");

      // Allocate space for a handle created to the local object
      // The local creation opcode will generate a handle for us at the specified location
      // This handle gets used in calling the preconstructor (inside the 'LocalObject' instruction)
      // and in calling the actual constructor (inside of 'FunctionCall')
      OperandIndex handleIndex = function->AllocateRegister(this->Builder->ReferenceOf(node->ReferencedType)->GetCopyableSize());
      opcode.SaveHandleLocal = handleIndex;
    
      // The object should be allocated here on the stack
      opcode.StackLocal = node->Access.HandleConstantLocal;

      // We want an object handle for other calls (such as the constructor)
      node->ThisHandleLocal = handleIndex;
    }
  }

  //***************************************************************************
  void CodeGenerator::GenerateHandleInitialize(Function* function, Type* type, const Operand& source, const Operand& destination, DebugOrigin::Enum debugOrigin, const CodeLocation& location)
  {
    // If the type is a handle...
    if (Type::IsHandleType(type))
    {
      // Since the type of the expression is already a handle, just copy it to the location that the user wants it at
      // This handle is always initializing uninitialized memory on the stack
      GenerateCopyInitialize(function, type, source, destination, debugOrigin, location);
    }
    // If the type is a named-type / value...
    else if (BoundType* boundType = Type::DynamicCast<BoundType*>(type))
    {
      // Allocate an opcode for creating a handle from a local
      ToHandleOpcode& opcode = function->AllocateOpcode<ToHandleOpcode>(Instruction::ToHandle, debugOrigin, location);

      // Get the handle type
      Type* handleType = this->Builder->ReferenceOf(boundType);
      
      // If we're attempting to generate a handle to a constant, then we want to copy it locally to the stack first
      if (source.Type == OperandType::Constant)
      {
        // Generate a stack local to store the constant
        Operand stackLocal(function->AllocateRegister(type->GetCopyableSize()));

        // Copy the constant to the stack
        this->GenerateCopyInitialize(function, type, source, stackLocal, debugOrigin, location);

        // Create a handle to the stack local
        opcode.ToHandle = stackLocal;
      }
      else
      {
        // Set the value that we want to make a handle of
        opcode.ToHandle = source;
      }

      // Save the handle in a newly allocated register
      opcode.SaveLocal = function->AllocateRegister(handleType->GetCopyableSize());

      // We also need to know the type of this handle
      opcode.Type = boundType;

      // Since the type of the expression is already a handle, just copy it to the location that the user wants it at
      // This handle is always initializing uninitialized memory on the stack
      GenerateCopyInitialize(function, handleType, Operand(opcode.SaveLocal), destination, debugOrigin, location);
    }
  }

  //***************************************************************************
  void CodeGenerator::CreateLocal(Function* function, size_t size, Operand& accessOut)
  {
    // All r-value binary operations result in a value on the stack
    // Therefore we need to allocate a register to store our result in
    accessOut.HandleConstantLocal = function->AllocateRegister(size);
    
    // We don't use the secondary index (we aren't writing to a member or anything) so just ignore it
    accessOut.FieldOffset = 0;

    // Since we are going to be stored on the stack, our access type is as a local
    accessOut.Type = OperandType::Local;
  }

  //***************************************************************************
  void CodeGenerator::CreateRValueUnaryOpcode(Function* function, UnaryOperatorNode& node, Instruction::Enum instruction, DebugOrigin::Enum debugOrigin)
  {
    // Create the opcode
    UnaryRValueOpcode& opcode = function->AllocateOpcode<UnaryRValueOpcode>(instruction, debugOrigin, node.Location);

    // All r-value binary operations result in a value on the stack
    // Therefore we need to allocate a register to store our result in
    CreateLocal(function, node.ResultType->GetCopyableSize(), node.Access);

    // We always output to the stack
    opcode.Output = node.Access.HandleConstantLocal;

    // Initialize the only operand
    opcode.SingleOperand = node.Operand->Access;
  }

  //***************************************************************************
  void CodeGenerator::CreateLValueUnaryOpcode(Function* function, UnaryOperatorNode& node, Instruction::Enum instruction, DebugOrigin::Enum debugOrigin)
  {
    // Create the opcode
    UnaryLValueOpcode& opcode = function->AllocateOpcode<UnaryLValueOpcode>(instruction, debugOrigin, node.Location);

    // All l-value binary operations result in a chained left hand value,
    // therefore we use the same primary and secondary index as the left operand
    // (no register allocation should ever be needed)
    node.Access = node.Operand->Access;

    // Initialize the only operand
    opcode.SingleOperand = node.Operand->Access;
  }

  //***************************************************************************
  void CodeGenerator::CreateConversionOpcode(Function* function, TypeCastNode& node, Instruction::Enum instruction, DebugOrigin::Enum debugOrigin)
  {
    // Create the opcode
    ConversionOpcode& opcode = function->AllocateOpcode<ConversionOpcode>(instruction, debugOrigin, node.Location);

    // This expression's result will be stored in the last created register
    CreateLocal(function, node.ResultType->GetCopyableSize(), node.Access);

    // We always output to the stack (it's a conversion, not a storage operator)
    opcode.Output = node.Access.HandleConstantLocal;
    
    // We pull the value out of our node's operand
    // (the operand is the left side of the 'as', eg 5 as Real, 5 would be the Operand)
    opcode.ToConvert = node.Operand->Access;
  }

  //***************************************************************************
  void CodeGenerator::GenerateUnaryOp(Function* function, UnaryOperatorNode& node, DebugOrigin::Enum debugOrigin)
  {
    // Create the opcode so that we can fill it in
    size_t opcodeStart = function->GetCurrentOpcodeIndex();

    // Get the instance of the type database
    Core& core = Core::GetInstance();

    // If this is creating a property delegate object
    ErrorIf(node.Operator->TokenId == Grammar::PropertyDelegate,
      "The property delegate operator is a unary operator, but should be handled by a separate handler");

    // Store the unary operator info for convenience (shared definition)
    UnaryOperator& info = node.OperatorInfo;

    // Unary plus always does nothing
    if (node.Operator->TokenId == Grammar::Positive)
    {
      // We need to make sure that the node knows its output is just its only operand (early out)
      node.Access = node.Operand->Access;
      return;
    }

    // If the operator results in an l-value...
    if (info.Io & IoMode::WriteLValue)
    {
      CreateLValueUnaryOpcode(function, node, info.Instruction, debugOrigin);
    }
    // Otherwise, the operator results in an r-value...
    else
    {
      CreateRValueUnaryOpcode(function, node, info.Instruction, debugOrigin);
    }

    // Error checking
    ErrorIf(opcodeStart == function->GetCurrentOpcodeIndex(), "No instructions were written!");
  }

  //***************************************************************************
  void CodeGenerator::GenerateConversion(Function* function, TypeCastNode& node, DebugOrigin::Enum debugOrigin)
  {
    // Create the opcode so that we can fill it in
    size_t opcodeStart = function->GetCurrentOpcodeIndex();
    
    // Get the instance of the type database
    Core& core = Core::GetInstance();

    // Store the cast operation for convenience
    CastOperation::Enum castOperation = node.OperatorInfo.Operation;

    // If the type cast is a primitive cast...
    if (castOperation == CastOperation::Primitive)
    {
      // Use the primitive instruction on the conversion operator
      CreateConversionOpcode(function, node, node.OperatorInfo.PrimitiveInstruction, debugOrigin);

      // Error checking
      ErrorIf(opcodeStart == function->GetCurrentOpcodeIndex(), "No instructions were written!");
    }
    // If it's an up cast, we pretty much don't do anything (just forward the access, type should be changed already)
    // Note: Dynamic down is currently NOT SAFE at all
    // It can also be a 'Same' cast, ex: Integer to Integer, in which we do absolutely nothing
    else if (castOperation == CastOperation::Raw)
    {
      // Forward access (there's no actual opcode we need to write!)
      node.Access = node.Operand->Access;
    }
    // Stores the value generically into an 'any' type, which does proper reference counting / handle / delegate / value copies
    // This also constructs an Any on the stack and initializes it
    // Note: May not go through the Any constructor as an optimization!
    else if (castOperation == CastOperation::ToAny)
    {
      // Create the opcode
      AnyConversionOpcode& opcode = function->AllocateOpcode<AnyConversionOpcode>(Instruction::ConvertToAny, debugOrigin, node.Location);
      
      // This expression's result will be stored in the last created register
      size_t anySize = node.ResultType->GetCopyableSize();
      ErrorIf(anySize != sizeof(Any), "The only way we should hit the 'ToAny' case is if we're casting to an Any type");
      CreateLocal(function, anySize, node.Access);

      // We always output to the stack (it's a conversion, not a storage operator)
      opcode.Output = node.Access.HandleConstantLocal;

      // We pull the value out of our node's operand
      // (the operand is the left side of the 'as', eg 5 as Real, 5 would be the Operand)
      opcode.ToConvert = node.Operand->Access;

      // The conversion opcode needs to know about the operands type so it knows how to store it in the Any
      // (see the comment above for what 'Operand' is)
      opcode.RelatedType = node.Operand->ResultType;
    }
    // Copies the value back out of the any onto the stack
    else if (castOperation == CastOperation::FromAny)
    {
      // Create the opcode
      AnyConversionOpcode& opcode = function->AllocateOpcode<AnyConversionOpcode>(Instruction::ConvertFromAny, debugOrigin, node.Location);
      
      // This expression's result will be stored in the last created register
      CreateLocal(function, node.ResultType->GetCopyableSize(), node.Access);

      // We always output to the stack (it's a conversion, not a storage operator)
      opcode.Output = node.Access.HandleConstantLocal;

      // We pull the value out of our node's operand
      // (the operand is the left side of the 'as', eg 5 as Real, 5 would be the Operand)
      ErrorIf(node.Operand->ResultType != core.AnythingType, "When converting from an Any, the Operand should always be an Any");
      opcode.ToConvert = node.Operand->Access;

      // The conversion opcode needs to know about the type its converting to
      opcode.RelatedType = node.ResultType;
    }
    // When we cast from a base class down into a more derived class (handles only at the moment)
    else if (castOperation == CastOperation::DynamicDown)
    {
      // Create the opcode
      DowncastConversionOpcode& opcode = function->AllocateOpcode<DowncastConversionOpcode>(Instruction::ConvertDowncast, debugOrigin, node.Location);
      
      // This expression's result will be stored in the last created register
      ErrorIf(node.ResultType->GetCopyableSize() != sizeof(Handle),
        "We only support downcasting of handles at the moment");
      CreateLocal(function, node.ResultType->GetCopyableSize(), node.Access);

      // We always output to the stack (it's a conversion, not a storage operator)
      opcode.Output = node.Access.HandleConstantLocal;

      // We pull the value out of our node's operand
      // (the operand is the left side of the 'as', eg 5 as Real, 5 would be the Operand)
      opcode.ToConvert = node.Operand->Access;

      // The conversion opcode needs to know about the type its converting to
      opcode.ToType = node.ResultType;
    }
    else if (castOperation == CastOperation::NullToDelegate)
    {
      // The 'Null' object is allocated as a constant, and the constant is guaranteed to be as big as a delegate or handle
      // Just redirect access to the same constant
      node.Access = node.Operand->Access;
    }
    else
    {
      Error("Unknown cast type!");
    }
  }

  //***************************************************************************
  void CodeGenerator::CreateCopyOpcode(Function* function, CopyMode::Enum mode, Type* type, const Operand& source, const Operand& destination, DebugOrigin::Enum debugOrigin, const CodeLocation& location)
  {
    // The instruction that we will end up using
    Instruction::Enum instruction = Instruction::InvalidInstruction;

    // Get the instance of the type database
    Core& core = Core::GetInstance();

    if (Type::IsSame(type, core.IntegerType))
    {
      instruction = Instruction::CopyInteger;
    }
    else if (Type::IsSame(type, core.RealType))
    {
      instruction = Instruction::CopyReal;
    }
    else if (Type::IsSame(type, core.BooleanType))
    {
      instruction = Instruction::CopyBoolean;
    }
    else if (Type::IsHandleType(type))
    {
      instruction = Instruction::CopyHandle;
    }
    else if (Type::IsDelegateType(type))
    {
      instruction = Instruction::CopyDelegate;
    }
    else if (Type::IsValueType(type))
    {
      instruction = Instruction::CopyValue;
    }
    else if (Type::IsAnyType(type))
    {
      instruction = Instruction::CopyAny;
    }
    else
    {
      Error("Unhandled case, should have been caught in syntaxer (what type could this be?)");
    }

    // Create the opcode
    CopyOpcode& opcode = function->AllocateOpcode<CopyOpcode>(instruction, debugOrigin, location);

    // Initialize the source and destination of the opcode
    opcode.Source = source;
    opcode.Destination = destination;
    opcode.Mode = mode;

    // The object opcode requires a size
    // Currently, unless we introduce the concept back
    // of general memory copy, this concept is not used
    opcode.Size = type->GetCopyableSize();
  }

  //***************************************************************************
  void CodeGenerator::GenerateCopyToReturn(Function* function, Type* type, const Operand& source, const Operand& destination, DebugOrigin::Enum debugOrigin, const CodeLocation& location)
  {
    return CreateCopyOpcode(function, CopyMode::ToReturn, type, source, destination, debugOrigin, location);
  }

  //***************************************************************************
  void CodeGenerator::GenerateCopyInitialize(Function* function, Type* type, const Operand& source, const Operand& destination, DebugOrigin::Enum debugOrigin, const CodeLocation& location)
  {
    return CreateCopyOpcode(function, CopyMode::Initialize, type, source, destination, debugOrigin, location);
  }

  //***************************************************************************
  void CodeGenerator::GenerateCopyToParameter(Function* function, Type* type, const Operand& source, OperandIndex destRegister, DebugOrigin::Enum debugOrigin, const CodeLocation& location)
  {
    return CreateCopyOpcode(function, CopyMode::ToParameter, type, source, Operand(destRegister), debugOrigin, location);
  }

  //***************************************************************************
  void CodeGenerator::GenerateCopyFromReturn(Function* function, Type* type, OperandIndex sourceRegister, OperandIndex destRegister, DebugOrigin::Enum debugOrigin, const CodeLocation& location)
  {
    // Side note, copying returns always copies to an uninitialized
    // place in memory so it must always be an initializing copy
    return CreateCopyOpcode(function, CopyMode::FromReturn, type, Operand(sourceRegister), Operand(destRegister), debugOrigin, location);
  }
}
