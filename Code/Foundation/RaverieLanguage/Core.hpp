// MIT Licensed (see LICENSE.md).

#pragma once

namespace Raverie
{
// What scalar types are used for vectors
DeclareEnum3(VectorScalarTypes, Real, Integer, Boolean);

// The base type for all enumerations within Raverie
class Enum
{
public:
  RaverieDeclareType(Enum, TypeCopyMode::ValueType);
  Integer Value;
};

// The core of Raverie Contains all types that the compiler uses internally
// The core also acts as a global database of all immutable and sharable types
class Core : public StaticLibrary
{
public:
  // Friends
  friend class CodeGenerator;
  friend class Syntaxer;
  friend class CoreLibraries;
  friend class Type;

  // This is so that all binding macros work with this library
  typedef Core RaverieLibrary;

  // Grab the singleton instance of the type database
  static void InitializeInstance();
  static void Destroy();
  static Core& GetInstance();
  static Core* Instance;

  // Store pointers to the basic data-types
  BoundType* ByteType;
  BoundType* BooleanType;
  BoundType* Boolean2Type;
  BoundType* Boolean3Type;
  BoundType* Boolean4Type;
  BoundType* IntegerType;
  BoundType* Integer2Type;
  BoundType* Integer3Type;
  BoundType* Integer4Type;
  BoundType* RealType;
  BoundType* Real2Type;
  BoundType* Real3Type;
  BoundType* Real4Type;
  BoundType* QuaternionType;
  BoundType* Real2x2Type;
  BoundType* Real3x3Type;
  BoundType* Real4x4Type;
  BoundType* DoubleIntegerType;
  BoundType* DoubleRealType;
  BoundType* StringType;
  BoundType* StringRangeType;
  BoundType* ExceptionType;
  BoundType* MathType;

  // Void is a special type that can actually be declared, but is typically used
  // for specifying no return type
  BoundType* VoidType;

  // The null type is the type that the constant 'null' has before implicit
  // conversion
  BoundType* NullType;

  // This is used when types cannot be resolved (it should never show up in code
  // generation) Using this type helps to prevent crashes, which is useful for
  // more tolerant cases like code-completion
  BoundType* ErrorType;

  // The overloaded methods type is akin to the ErrorType and is only used when
  // we encounter method overloads Typically this type is replaced by another
  // type, however in the event that overloads are not resolved this will be the
  // final resulting type (an entirely useless type that should probably be an
  // error to store)
  BoundType* OverloadedMethodsType;

  // A special array of vector types
  // Useful for when we're using an index
  static const size_t MaxComponents = 4;
  BoundType* RealTypes[MaxComponents];
  BoundType* IntegerTypes[MaxComponents];
  BoundType* BooleanTypes[MaxComponents];
  // Note: AllRealTypes and AllIntegerTypes are currently expected to have a 1-1
  // mapping in the same order (so a function that takes a real2 can return an
  // int2)
  Array<BoundType*> AllRealTypes;
  Array<BoundType*> AllIntegerTypes;
  Array<BoundType*> AllBooleanTypes;

  BoundType* VectorScalarBoundTypes[VectorScalarTypes::Size];
  // To perform generic axis functions we need to know how to set a value for a
  // given type to 1
  typedef void (*ScalarTypeOneFunction)(byte* outData);
  ScalarTypeOneFunction ScalarTypeOneFunctions[VectorScalarTypes::Size];
  // An array of types to vectors (so VectorScalarTypes::Integer to an array of
  // IntegerTypes)
  BoundType** VectorTypes[VectorScalarTypes::Size];

  // Special array of what matrix types to make (real, int, etc...)
  static const size_t MinMatrixComponents = 2;
  static const size_t MaxMatrixComponents = 4;
  static const size_t MaxMatrixElementTypes = 3;
  BoundType* MatrixElementTypes[MaxMatrixElementTypes];
  // To perform certain generic matrix operations generically,
  // MultiplyAdd functions (a += b * c) are needed
  typedef void (*MatrixMultiplyAddFunction)(byte* outData, byte* inputA, byte* inputB);
  MatrixMultiplyAddFunction TypeMultiplyAddFunctions[MaxMatrixElementTypes];

  // A very special type that allows us to take a delegate of any type
  // This can currently not be declared in language, but can be used from C++
  DelegateType* AnyDelegateType;

  // A special delegate type that we use as a dummy for errors.
  DelegateType* ErrorDelegateType;

  // A very special type that allows us to take a handle of any type
  // This can currently not be declared in language, but can be used from C++
  BoundType* AnyHandleType;

  // A type that can be set to anything (delegates, value types, reference
  // types, etc)
  AnyType* AnythingType;

protected:
  // Setup the binding for the core library
  void SetupBinding(LibraryBuilder& builder) override;

private:
  // Binding for sub-sections of the core library
  void SetupBindingString(LibraryBuilder& builder);
  void SetupBindingMath(LibraryBuilder& builder);

  // Instantiates a hash-map template when requested
  static BoundType* InstantiateHashMap(LibraryBuilder& builder, StringParam baseName, StringParam fullyQualifiedName, const Array<Constant>& templateTypes, const void* userData);

  // Instantiates a hash-map range template when requested
  static BoundType* InstantiateHashMapRange(LibraryBuilder& builder, StringParam baseName, StringParam fullyQualifiedName, const Array<Constant>& templateTypes, const void* userData);

  // Instantiates a pair template when requested
  static BoundType* InstantiateKeyValue(LibraryBuilder& builder, StringParam baseName, StringParam fullyQualifiedName, const Array<Constant>& templateTypes, const void* userData);

  // Instantiates a property delegate template when requested
  static BoundType* InstantiatePropertyDelegate(LibraryBuilder& builder, StringParam baseName, StringParam fullyQualifiedName, const Array<Constant>& templateTypes, const void* userData);

  // Privatized constructor (it's a singleton)
  Core();

  // Destructor
  ~Core();

  // Not copyable
  RaverieNoCopy(Core);
};

// A single entry in a stack trace
class StackEntry
{
public:
  // Constructor
  StackEntry();

  // A file / line location of where we are in the stack
  CodeLocation Location;

  // The function we're currently on
  Function* ExecutingFunction;
};

// Contains all the locations that we're at in an execution of Raverie
class StackTrace
{
public:
  // Get the most recent non-native call (for debugging and visualization)
  // or return null if the entire stack is native or empty
  // Typically when an exception occurs (or any place a stack trace happens) you
  // will want to use this to point the user at the location where the error
  // occurred in their code
  StackEntry* GetMostRecentNonNativeStackEntry();

  // Get the relevant location that the exception occurred (where should show
  // the user) This will invoke the above 'GetMostRecentNonNativeStackEntry'
  // function, and if it returns null it will return an empty location
  CodeLocation GetMostRecentNonNativeLocation();

  // Get the standard formatting for stack traces (used by exception printing
  // too)
  String GetFormattedMessage(MessageFormat::Enum format);

  // The code locations of the entire stack and what functions were called
  // We have to use a special allocator here that supports being memset to zero
  // (because exceptions are just memset to 0, no constructor called)
  Array<StackEntry, Raverie::MemsetZeroDefaultAllocator> Stack;
};

// The base class of any exception that gets thrown
class Exception
{
public:
  RaverieDeclareType(Exception, TypeCopyMode::ReferenceType);

  // Default constructor
  Exception();

  // Construct an exception with a message
  Exception(StringParam message);

  // Get the standard formatting for error messages
  String GetFormattedMessage(MessageFormat::Enum format);

  // The error that occurred as a human readable string, including technical
  // details
  String Message;

  // The code locations of the entire stack and what functions were called
  StackTrace Trace;
};

// A property delegate Contains both the get and set delegates
// Technically we store the handle twice, which could be compacted because
// we know that the 'this' handle will always be the same among both
class PropertyDelegateTemplate
{
public:
  // The property getter and setters
  Delegate Get;
  Delegate Set;
  Handle ReferencedProperty;
};

// Functions for converting our primitives to strings
String BooleanToString(Boolean value);
String Boolean2ToString(Boolean2Param value);
String Boolean3ToString(Boolean3Param value);
String Boolean4ToString(Boolean4Param value);
String ByteToString(Byte value);
String IntegerToString(Integer value);
String Integer2ToString(Integer2Param value);
String Integer3ToString(Integer3Param value);
String Integer4ToString(Integer4Param value);
String RealToString(Real value);
String Real2ToString(Real2Param value);
String Real3ToString(Real3Param value);
String Real4ToString(Real4Param value);
String QuaternionToString(QuaternionParam value);
String DoubleIntegerToString(DoubleInteger value);
String DoubleRealToString(DoubleReal value);
} // namespace Raverie
