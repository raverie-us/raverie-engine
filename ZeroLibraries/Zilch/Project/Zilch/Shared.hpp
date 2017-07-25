/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

#pragma once
#ifndef ZILCH_SHARED_HPP
#define ZILCH_SHARED_HPP

// We need to make a specialization because the hash maps do not work with enums
// On other compilers, the 'Enum' type is actually an int, which would produce a
// duplicate definition compiler error since a specialization of int already exists
#ifdef _MSC_VER
template<>
struct ZeroShared Zero::HashPolicy<Zilch::Grammar::Enum> : public Zero::ComparePolicy<size_t>
{
public:
  inline size_t operator()(const Zilch::Grammar::Enum& value) const
  {
    return HashUint(*(unsigned int*)&value);
  }
};
#endif

namespace Zilch
{
  // What type of IO an expression allows
  namespace IoMode
  {
    enum Enum
    {
      // Some expressions we ignore their io-usage (but never the io of the expression itself)
      // Examples being expressions used as a standalone statement (nobody reads/writes)
      Ignore = 0,
      // A variable is readable (constants, temporaries, property get)
      ReadRValue = 1,
      // A variable is writable (variables, property set)
      WriteLValue = 2,
      // We're strictly doing a property 'set', which means 'WriteLValue' should be set
      // This is used to let handle and delegate properties know that they are initializing
      // a value rather than assigning a value (when assignment '=' is used)
      StrictPropertySet = 4,
      // If the access type was not set, we either haven't resolved it or it's a bug
      NotSet = (uint)-1
    };
  }

  // This struct is given back to the user when asking for tokens
  class ZeroShared BinaryOperator
  {
  public:
    // Constructor
    BinaryOperator();

    // The hash function which allows us to put it in a hash container
    size_t Hash() const;

    // The hash function which allows us to put it in a hash container
    bool operator==(const BinaryOperator& rhs) const;

    // Whether this operator was valid or not (a non-existant operator is invalid)
    bool IsValid;

    // In a binary operator, these include the left and right hand types
    Type* Lhs;
    Type* Rhs;

    // If the left or right hand argument needs to be casted for this operation to work
    // This only appears when the direct operator does not exist, but an implicit cast of one side, or the other, or both exists
    Type* CastLhsTo;
    Type* CastRhsTo;

    // The resulting type of the operation
    Type* Result;

    // The operator used in grammar to represent
    Grammar::Enum Operator;

    // The resulting instruction
    Instruction::Enum Instruction;

    // If the operation is communative but it's between two different types, this tells
    // us if the opcode requires the arguments to be flipped (to reduce opcodes)
    bool FlipArguments;

    // Whether or not this results in an l-value or r-value
    IoMode::Enum Io;
  };

  // This struct is given back to the user when asking for tokens
  class ZeroShared UnaryOperator
  {
  public:
    // Constructor
    UnaryOperator();

    // The hash function which allows us to put it in a hash container
    size_t Hash() const;

    // The hash function which allows us to put it in a hash container
    bool operator==(const UnaryOperator& rhs) const;

    // Whether this operator was valid or not (a non-existant operator is invalid)
    bool IsValid;

    // In a unary operator, this is the operand
    Type* Operand;

    // The resulting type of the operation
    Type* Result;

    // The operator used in grammar to represent
    Grammar::Enum Operator;

    // The resulting instruction
    Instruction::Enum Instruction;

    // Whether or not this results in an l-value or r-value
    IoMode::Enum Io;
  };

  // Tells us which way an operator evaluates it's arguments
  namespace OperatorAssociativity
  {
    enum Enum
    {
      RightToLeft,
      LeftToRight
    };
  }

  // Lets us query information about the validity of a cast, as well as what kind it will be
  class ZeroShared CastOperator
  {
  public:
    // Constructor
    CastOperator();

    // The hash function which allows us to put it in a hash container
    size_t Hash() const;

    // The hash function which allows us to put it in a hash container
    bool operator==(const CastOperator& rhs) const;

    // Whether this operator was valid or not (a non-existant operator is invalid)
    bool IsValid;

    // Only used for efficient query / lookup
    // These may be null in cases other than primitive casts
    Type* From;
    Type* To;

    // The operator used in grammar to represent
    CastOperation::Enum Operation;

    // The resulting instruction if a single one exists (eg ConvertRealToInteger)
    // Only valid when the cast operation is Primitive
    // Otherwise if no direct instruction exists, the instruction will be set to 'InvalidInstruction'
    Instruction::Enum PrimitiveInstruction;

    // If we allow this cast to be implicit (default false)
    // If true, then the syntaxer will automatically allow it in cases
    // such as return, passing parameters, resolving overloads, etc
    bool CanBeImplicit;

    // Some casts require actual instructions to run
    // Ex: Real to Integer must perform a floating point conversion, Integer to the special Any type, etc
    // Other casts can be directly raw convertable with no execution
    // Ex: An enum value to an Integer, or a derived class to a base class (Cat to Animal)
    // This will be set if the cast type requires any sort of code generation / execution
    bool RequiresCodeGeneration;
  };

  // Tells us which way an operator evaluates it's arguments
  namespace OperatorArity
  {
    enum Enum
    {
      Unary,
      Binary
    };
  }

  // Encompasses everything we need to know about operator precedence
  class ZeroShared UntypedOperator
  {
  public:
    // Constructor
    UntypedOperator();

    // Whether this operator was valid or not (a non-existant operator is invalid)
    bool IsValid;

    Grammar::Enum Operator;
    size_t Precedence;
    OperatorAssociativity::Enum Associativity;
    OperatorArity::Enum Arity;
  };

  // Contains information that is shared between the syntaxer and the code generator
  class ZeroShared Shared
  {
  public:

    // Construct the shared object
    Shared();

    // Get the instance of the singleton
    static Shared& GetInstance();

    // Lookup a binary operator between two types
    // Both entries for communative operators will exist, eg, scalar * vector and vector * scalar
    BinaryOperator GetBinaryOperator(Type* lhs, Type* rhs, Grammar::Enum oper, bool allowRecursiveLookup = true);

    // Lookup a unary operator
    UnaryOperator GetUnaryOperator(Type* type, Grammar::Enum oper);

    // Lookup any cast operators from this type to any other type
    Array<CastOperator> GetPrimitiveCastOperatorsFrom(Type* from);

    // Lookup a cast operator (can be explicit or implicit)
    CastOperator GetCastOperator(Type* from, Type* to);

    // Get a structure that represents the precedence and associativity of an operator, regardless of types
    UntypedOperator GetOperatorPrecedence(Grammar::Enum oper, OperatorArity::Enum arity);

    // Gets all the operators stored in an array thats indexed by precedence
    // Note: Precedence starts at 0 and ends at Size() - 1
    const Array<Array<UntypedOperator> >& GetPrecedences();

  private:

    // Adds a binary operator
    void AddBinary(Type* lhs, Type* rhs, Type* result, Grammar::Enum oper, Instruction::Enum instruction, IoMode::Enum io, bool flip);

    // Adds a binary communative operator (which adds the reversed operator too)
    void AddBinaryCommunative(Type* type1, Type* type2, Type* result, Grammar::Enum oper, Instruction::Enum instruction, IoMode::Enum io);

    // Adds a binary non-communative operator (the reverse will not be added)
    void AddBinaryNonCommunative(Type* lhs, Type* rhs, Type* result, Grammar::Enum oper, Instruction::Enum instruction, IoMode::Enum io);
    
    // Adds a binary operator where the operands are the same type
    // Note: If the operator is the same type, we don't care if it's communative or not because we always perform
    // the operation in the correct order, and we only need one opcode to represent it
    void AddBinary(Type* sameType, Type* result, Grammar::Enum oper, Instruction::Enum instruction, IoMode::Enum io);

    // Adds a unary operator
    void AddUnary(Type* operand, Type* result, Grammar::Enum oper, Instruction::Enum instruction, IoMode::Enum io);

    // Adds a primitive cast operator (must have an instruction
    void AddPrimitiveCast(Type* fromType, Type* toType, Instruction::Enum instruction, bool canBeImplicit);

    // Adds an operator to the precedence chart (maps it both ways)
    void AddPrecedence(size_t precedence, OperatorAssociativity::Enum associativity, OperatorArity::Enum arity, Grammar::Enum oper);

  private:

    // Special binary operators
    BinaryOperator HandleAssignment;
    BinaryOperator HandleEquality;
    BinaryOperator HandleInequality;
    BinaryOperator ValueAssignment;
    BinaryOperator ValueEquality;
    BinaryOperator ValueInequality;
    BinaryOperator DelegateAssignment;
    BinaryOperator DelegateEquality;
    BinaryOperator DelegateInequality;
    BinaryOperator AnyAssignment;
    BinaryOperator AnyEquality;
    BinaryOperator AnyInequality;

    // Our hash set of binary operators that get registered once (hence the singleton)
    HashSet<BinaryOperator> BinaryOperators;

    // Our hash set of unary operators that get registered once (hence the singleton)
    HashSet<UnaryOperator> UnaryOperators;
    
    // Special cast operators
    // RawImplicitCast includes same-cast, up-cast, null-cast, any-delegate-cast
    CastOperator RawImplicitCast;
    CastOperator DynamicDownCast;
    CastOperator ToAnyCast;
    CastOperator FromAnyCast;
    CastOperator FromAnyHandleCast;
    CastOperator EnumIntegerCast;
    CastOperator IntegerEnumCast;
    CastOperator NullToDelegate;

    // Our hash set of casting operators that get registered once (hence the singleton)
    HashSet<CastOperator> CastOperators;

    // Associate all the cast operators from this type to any other type
    ZilchTodo("This should actually use some kind of policy because we're not hashing types correctly (works because we only care about BoundType* right now, Real/Integer, etc )");
    HashMap<Type*, Array<CastOperator> > PrimitiveCastOperatorsFrom;

    // We use this as a key into a hash map
    class ZeroShared OperatorWithArity
    {
    public:
      // Define these so we can be used as a key
      bool operator==(const OperatorWithArity& rhs) const;
      size_t Hash() const;

      Grammar::Enum Operator;
      OperatorArity::Enum Arity;
    };

    // We map operators to their precedence and back (useful for code formatters and documentation)
    HashMap<OperatorWithArity, UntypedOperator> OperatorToPrecedence;
    Array<Array<UntypedOperator> > PrecedenceToOperators;
  };
}

#endif