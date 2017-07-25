/**************************************************************\
* Author: Trevor Sundberg
* Copyright 2016, DigiPen Institute of Technology
\**************************************************************/

// Note: This header is typically used as a precompiled header

#pragma once
#ifndef ZILCH_PRECOMPILED_HPP
#define ZILCH_PRECOMPILED_HPP

// This needs to be included before anything else
// since it disables warnings and defines macros we use
#include "LocalBuild.hpp"
#include "General.hpp"

// Zero includes
#include "Common/CommonStandard.hpp"
#include "Math/MathStandard.hpp"
#include "Platform/PlatformStandard.hpp"

// Bring Zero primitives into the Zilch namespace
namespace Zilch
{
  // Zero types
  using Zero::AutoHandle;
  using Zero::Array;
  using Zero::BaseInList;
  using Zero::BuildString;
  using Zero::EqualTo;
  using Zero::ErrorSignaler;
  using Zero::ExternalLibrary;
  using Zero::File;
  using Zero::FixedArray;
  using Zero::FixedString;
  using Zero::FindFirstIndex;
  using Zero::HashMap;
  using Zero::HashPolicy;
  using Zero::HashSet;
  using Zero::HashString;
  using Zero::InList;
  using Zero::JoinStrings;
  using Zero::Link;
  using Zero::Memory::Shutdown;
  using Zero::NullPointerType;
  using Zero::OsEvent;
  using Zero::OsInt;
  using Zero::OwnedArray;
  using Zero::PodArray;
  using Zero::Pair;
  using Zero::PodBlockArray;
  using Zero::ProcessStartInfo;
  using Zero::Semaphore;
  using Zero::Socket;
  using Zero::SocketAddress;
  using Zero::Sort;
  using Zero::StandardTraits;
  using Zero::Status;
  using Zero::String;
  using Zero::StringRange;
  using Zero::StringIterator;
  using Zero::StringBuilder;
  using Zero::Thread;
  using Zero::ThreadLock;
  using Zero::ToValue;
  using Zero::UniquePointer;
  using Zero::ZeroMove;

  // Math types
  typedef Math::BoolVector2 Boolean2;
  typedef Math::BoolVector3 Boolean3;
  typedef Math::BoolVector4 Boolean4;
  typedef Math::IntVector2 Integer2;
  typedef Math::IntVector3 Integer3;
  typedef Math::IntVector4 Integer4;
  typedef Math::Vector2 Real2;
  typedef Math::Vector3 Real3;
  typedef Math::Vector4 Real4;
  typedef Math::Quaternion Quaternion;
  typedef Math::Matrix2 Real2x2;
  typedef Math::Matrix3 Real3x3;
  typedef Math::Matrix4 Real4x4;

  // The type we'll use for operands
  typedef signed int          OperandIndex;
  typedef signed int          OperandLocal;
  typedef signed int          ByteCodeOffset;
  typedef unsigned long long  GuidType;

  // Type-defines for the primitive language types
  typedef void                Void;
  typedef unsigned char       Byte;
  typedef int                 Integer;
  typedef long long int       DoubleInteger;
  typedef float               Real;
  typedef double              DoubleReal;
  typedef bool                Boolean;

  // For efficiency
  typedef const Boolean2& Boolean2Param;
  typedef const Boolean3& Boolean3Param;
  typedef const Boolean4& Boolean4Param;
  typedef const Integer2& Integer2Param;
  typedef const Integer3& Integer3Param;
  typedef const Integer4& Integer4Param;
  typedef const Real2& Real2Param;
  typedef const Real3& Real3Param;
  typedef const Real4& Real4Param;
  typedef const Real2x2& Real2x2Param;
  typedef const Real3x3& Real3x3Param;
  typedef const Real4x4& Real4x4Param;
  using Zero::StringParam;
  typedef const Quaternion& QuaternionParam;

  // Math functions
  using Math::EulerAngles;
  using Math::ToAxisAngle;
  using Math::ToEulerAngles;
  using Math::ToQuaternion;

  // Standard types / functions
  using std::numeric_limits;
  using std::clock;
  using std::clock_t;
}

// Special macros
#define ZilchForEach(VariableName, Range)                 forRange(VariableName, (Range).All())
#define ZilchRefForEach(VariableName, Range)              forRangeRef(VariableName, (Range).All())
#define ZilchForRange(VariableName, RangeName, Range)     ZeroForRangeVar(VariableName, RangeName, (Range).All())
#define ZilchRefForRange(VariableName, RangeName, Range)  ZeroForRangeRefVar(VariableName, RangeName, (Range).All())

// Zilch Includes
#include "ForwardDeclarations.hpp"
#include "SharedReference.hpp"

// Standard Zilch Type-defines
namespace Zilch
{
  typedef Array<Function*>                                FunctionArray;
  typedef FunctionArray::range                            FunctionArrayRange;
  typedef HashMap<String, Variable*>                      VariableMap;
  typedef VariableMap::range                              VariableRange;
  typedef VariableMap::valuerange                         VariableValueRange;
  typedef HashMap<String, FunctionArray>                  FunctionMultiMap;
  typedef FunctionMultiMap::range                         FunctionMultiRange;
  typedef FunctionMultiMap::valuerange                    FunctionMultiValueRange;
  typedef HashMap<String, Field*>                         FieldMap;
  typedef FieldMap::range                                 FieldMapRange;
  typedef FieldMap::valuerange                            FieldMapValueRange;
  typedef HashMap<String, GetterSetter*>                  GetterSetterMap;
  typedef GetterSetterMap::range                          GetterSetterMapRange;
  typedef GetterSetterMap::valuerange                     PropertyMapValueRange;
  typedef Array<Property*>                                PropertyArray;
  typedef PropertyArray::range                            PropertyArrayRange;
  typedef Array<Member*>                                  MemberArray;
  typedef MemberArray::range                              MemberArrayRange;
  typedef Array<SendsEvent*>                              SendsEventArray;
  typedef SendsEventArray::range                          SendsEventRange;
  typedef HashMap<GuidType, Function*>                    FunctionGuidMap;
  typedef FunctionGuidMap::range                          FunctionGuidRange;
  typedef HashMap<String, BoundType*>                     BoundTypeMap;
  typedef BoundTypeMap::pair                              BoundTypePair;
  typedef BoundTypeMap::range                             BoundTypeRange;
  typedef BoundTypeMap::valuerange                        BoundTypeValueRange;
  typedef Array<Type*>                                    TypeArray;
  typedef Array<BoundType*>                               BoundTypeArray;
  typedef HashMap<GuidType, Type*>                        TypeGuidMap;
  typedef TypeGuidMap::range                              TypeGuidRange;
  typedef Array<String>                                   StringArray;
  typedef StringArray::range                              StringArrayRange;
  typedef HashMap<GuidType, FunctionMultiMap>             FunctionExtensionMap;
  typedef FunctionExtensionMap::range                     FunctionExtensionRange;
  typedef HashMap<GuidType, GetterSetterMap>              GetterSetterExtensionMap;
  typedef GetterSetterExtensionMap::range                 GetterSetterExtensionRange;
  typedef HashMap<BoundType*, IndirectionType*>           TypeToIndirect;
  typedef TypeToIndirect::range                           TypeToIndirectRange;
  typedef Ref<Library>                                    LibraryRef;
  typedef const Ref<Library>&                             LibraryParam;
  typedef HashMap<GuidType, LibraryRef>                   LibraryGuidMap;
  typedef LibraryGuidMap::range                           LibraryGuidRange;
  typedef Array<LibraryRef>                               LibraryArray;
  typedef LibraryArray::range                             LibraryRange;

  typedef Function*                   (BoundType::*GetFunctionFn)            (StringParam name) const;
  typedef const FunctionArray*        (BoundType::*GetOverloadedFunctionsFn) (StringParam name) const;
  typedef Field*                      (BoundType::*GetFieldFn)               (StringParam name) const;
  typedef GetterSetter*               (BoundType::*GetGetterSetterFn)        (StringParam name) const;

  typedef void (*PostDestructorFn)(BoundType* boundType, byte* objectData);
  typedef void (*ThreadLockFn)(bool lock);

  // The C++ function that's bound to the script function
  typedef void (*BoundFn)(Call& call, ExceptionReport& report);
  
  // Every time we created a handle manager, we expect an index back of this type
  typedef size_t HandleManagerId;

  // How a type gets copied
  namespace TypeCopyMode
  {
    enum Enum
    {
      // Value types are considered memory copyable,
      // and can therefore not contain any sort of handles
      ValueType,

      // Reference types are never copied (which means that we only ever copy
      // the handle to the reference, and not the object itself).
      ReferenceType,

      // All enums bound to Zilch must be of Integer size, so force it on all platforms
      ForceIntegerSize = 0x7FFFFFFF
    };
  }

  // The user can define 'ZilchDisableDocumentation' globally to disable all processing of documentation strings in Zilch (freeing up memory)
  // If the user wants to make a single build of an executable, but optionally choose whether to enable documentation or not, they can pass in
  // These functions will return an empty string in the case that runtime documentation is disabled (which is implied by compile time disabling)
  String GetDocumentationStringOrEmpty(StringParam string);
  String GetDocumentationCStringOrEmpty(cstr string);

  // If the user disabled documentation at compile-time...
  #if defined(ZilchDisableDocumentation)
    // Defines a documentation string and allows users to easily disable documentation via globally defining 'ZilchDisableDocumentation'
    #define ZilchDocumentString(DocumentationString) Zilch::String()

    // Defines a static member function that allows us to query the documentation for a particular member (including properties, functions, and fields)
    #define ZilchDocument(Member, DocumentationString) static Zilch::String Member##Documentation() { return Zilch::String(); }
  #else
    // Defines a documentation string and allows users to easily disable documentation via globally defining 'ZilchDisableDocumentation'
    #define ZilchDocumentString(DocumentationString) Zilch::GetDocumentationStringOrEmpty(DocumentationString)

    // Defines a global function that allows us to query the documentation for a particular type including classes, enums, primitives, etc)
    #define ZilchDocument(Name, DocumentationString)                                              \
      static Zilch::String Name##Documentation()                                                  \
      {                                                                                           \
        static Zilch::String Documentation = GetDocumentationCStringOrEmpty(DocumentationString); \
        return Documentation;                                                                     \
      }
  #endif
}

// To speed up compilation, we also put all of our includes here rather than being inside any of the headers
// NOTE: These headers must be sorted in order of dependencies (e.g. Delegate has a Handle, so Handle must come first)
#include "Handle.hpp"
#include "Delegate.hpp"
#include "Traits.hpp"
#include "Binding.hpp"
#include "CodeLocation.hpp"
#include "Range.hpp"
#include "UntypedBlockArray.hpp"
#include "DestructibleBuffer.hpp"
#include "Composition.hpp"
#include "Members.hpp"
#include "Opcode.hpp"
#include "StringConstants.hpp"
#include "Function.hpp"
#include "Type.hpp"
#include "MultiPrimitive.hpp"
#include "StringBuilderClass.hpp"
#include "Documentation.hpp"
#include "Library.hpp"
#include "StaticLibrary.hpp"
#include "Core.hpp"
#include "SyntaxTreeHelpers.hpp"
#include "GrammarConstants.hpp"
#include "Shared.hpp"
#include "Token.hpp"
#include "SyntaxTree.hpp"
#include "Events.hpp"
#include "ArrayClass.hpp"
#include "CodeGenerator.hpp"
#include "ErrorDatabase.hpp"
#include "CompilationErrors.hpp"
#include "ConsoleClass.hpp"
#include "WebSocket.hpp"
#include "Debugging.hpp"
#include "HandleManager.hpp"
#include "Timer.hpp"
#include "ExecutableState.hpp"
#include "Any.hpp"
#include "FilePathClass.hpp"
#include "StreamInterface.hpp"
#include "FileStreamClass.hpp"
#include "Formatter.hpp"
#include "HashContainer.hpp"
#include "Json.hpp"
#include "Matrix.hpp"
#include "OverloadResolver.hpp"
#include "Parser.hpp"
#include "Plugin.hpp"
#include "Project.hpp"
#include "RandomClass.hpp"
#include "Setup.hpp"
#include "Sha1.hpp"
#include "StubCode.hpp"
#include "Syntaxer.hpp"
#include "TemplateBinding.hpp"
#include "RangeBinding.hpp"
#include "Tokenizer.hpp"
#include "VirtualMachine.hpp"
#include "Base64.hpp"
#include "DataDrivenLexer.hpp"
#include "Wrapper.hpp"
#include "ProcessClass.hpp"

#endif
