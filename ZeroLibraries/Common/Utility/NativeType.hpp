///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//---------------------------------------------------------------------------------//
//                             Basic Native Type                                   //
//---------------------------------------------------------------------------------//

/// Basic native types are fundamental, standard engine types associated with additional compile-time type trait information
/// Provides a constant defined as true if T is a basic native type, else defined as false
/// (ex. bool, int, float, IntVector2, Vector3, String)
template <typename T>
struct IsBasicNativeType
{
  static const bool Value = false;
};

/// Arithmetic basic native types are integral or floating-point types made up of one or more arithmetic primitives
/// Provides a constant defined as true if T is an arithmetic basic native type, else defined as false
/// (ex. bool, int, float, IntVector2, Vector3)
template <typename T>
struct IsBasicNativeTypeArithmetic
{
  static const bool Value = false;
};

/// Integral basic native types are arithmetic types made up of one or more integral arithmetic primitives
/// Provides a constant defined as true if T is an integral arithmetic basic native type, else defined as false
/// (ex. bool, int, IntVector2)
template <typename T>
struct IsBasicNativeTypeIntegral
{
  static const bool Value = false;
};

/// Floating-point basic native types are arithmetic types made up of one or more floating-point arithmetic primitives
/// Provides a constant defined as true if T is an floating-point arithmetic basic native type, else defined as false
/// (ex. float, Vector3)
template <typename T>
struct IsBasicNativeTypeFloatingPoint
{
  static const bool Value = false;
};

/// Basic native type primitives are arithmetic basic native types that are also built-in C++ arithmetic types
/// Provides a constant defined as true if T is a basic native type primitive, else defined as false
/// (ex. bool, int, float)
template <typename T>
struct IsBasicNativeTypePrimitive
{
  static const bool Value = false;
};

/// Basic native type multi-primitives are arithmetic basic native types that are also user-defined types made up of multiple primitive member values
/// Provides a constant defined as true if T is a basic native type multi-primitive, else defined as false
/// (ex. IntVector2, Vector3)
template <typename T>
struct IsBasicNativeTypeMultiPrimitive
{
  static const bool Value = false;
};

/// Define basic native type macros
/// (Makes the type name available as a unique enum value in BasicNativeType::Enum)
#define ForwardDeclareBasicNativeType(Name)                                       ,Name
#define DeclareBasicNativeType(T, Name)                                           ,Name
#define DeclareBasicNativeTypePrimitive(T, Name)                                  ,Name
#define DeclareBasicNativeTypeMultiPrimitive(T, Name, PrimitiveT, PrimitiveCount) ,Name

/// Basic native types enumeration
/// (Provides all basic native types with a compile-time constant native type ID)
namespace BasicNativeType
{
typedef uint Type;
enum Enum
{
  // Not a basic native type
  Unknown

  // Include basic native type enum values
  #include "BasicNativeTypes.inl"
};
enum { Size = (BasicNativeType::String + 1) };
}

/// Undefine basic native type macros
#undef ForwardDeclareBasicNativeType
#undef DeclareBasicNativeType
#undef DeclareBasicNativeTypePrimitive
#undef DeclareBasicNativeTypeMultiPrimitive

/// Provides the corresponding BasicNativeType enum value of T, else BasicNativeType::Unknown
template <typename T>
struct BasicNativeTypeToEnum
{
  static const BasicNativeType::Enum Value = BasicNativeType::Unknown;
};

/// Provides the corresponding type of the BasicNativeType enum value, else type void
template <BasicNativeType::Enum Value>
struct BasicNativeTypeFromEnum
{
  typedef void Type;
};

/// Provides the underlying primitive member type and count of T, else type void and count 0
template <typename T>
struct BasicNativeTypePrimitiveMembers
{
  typedef void Type;
  static const size_t Count = 0;
};

/// Define basic native type macros
/// (Defines required information about the type via template specialization)
#define ForwardDeclareBasicNativeType(Name)

#define DeclareBasicNativeType(T, Name)                                           \
                                                                                  \
template <>                                                                       \
struct IsBasicNativeType<T>                                                       \
{                                                                                 \
  static const bool Value = true;                                                 \
};                                                                                \
                                                                                  \
template <>                                                                       \
struct BasicNativeTypeToEnum<T>                                                   \
{                                                                                 \
  static const BasicNativeType::Enum Value = BasicNativeType::##Name;             \
};                                                                                \
                                                                                  \
template <>                                                                       \
struct BasicNativeTypeFromEnum<BasicNativeType::##Name>                           \
{                                                                                 \
  typedef T Type;                                                                 \
};

#define DeclareBasicNativeTypePrimitive(T, Name)                                  \
                                                                                  \
DeclareBasicNativeType(T, Name)                                                   \
                                                                                  \
template <>                                                                       \
struct IsBasicNativeTypeArithmetic<T>                                             \
{                                                                                 \
  static const bool Value = true;                                                 \
};                                                                                \
                                                                                  \
template <>                                                                       \
struct IsBasicNativeTypeIntegral<T>                                               \
{                                                                                 \
  static const bool Value = is_integral<T>::value;                                \
};                                                                                \
                                                                                  \
template <>                                                                       \
struct IsBasicNativeTypeFloatingPoint<T>                                          \
{                                                                                 \
  static const bool Value = is_floating_point<T>::value;                          \
};                                                                                \
                                                                                  \
template <>                                                                       \
struct IsBasicNativeTypePrimitive<T>                                              \
{                                                                                 \
  static const bool Value = true;                                                 \
};                                                                                \
                                                                                  \
template <>                                                                       \
struct BasicNativeTypePrimitiveMembers<T>                                         \
{                                                                                 \
  typedef T Type;                                                                 \
  static const size_t Count = 1;                                                  \
};

#define DeclareBasicNativeTypeMultiPrimitive(T, Name, PrimitiveT, PrimitiveCount) \
                                                                                  \
DeclareBasicNativeType(T, Name)                                                   \
                                                                                  \
template <>                                                                       \
struct IsBasicNativeTypeArithmetic<T>                                             \
{                                                                                 \
  static const bool Value = true;                                                 \
};                                                                                \
                                                                                  \
template <>                                                                       \
struct IsBasicNativeTypeIntegral<T>                                               \
{                                                                                 \
  static const bool Value = is_integral<PrimitiveT>::value;                       \
};                                                                                \
                                                                                  \
template <>                                                                       \
struct IsBasicNativeTypeFloatingPoint<T>                                          \
{                                                                                 \
  static const bool Value = is_floating_point<PrimitiveT>::value;                 \
};                                                                                \
                                                                                  \
template <>                                                                       \
struct IsBasicNativeTypeMultiPrimitive<T>                                         \
{                                                                                 \
  static const bool Value = true;                                                 \
};                                                                                \
                                                                                  \
template <>                                                                       \
struct BasicNativeTypePrimitiveMembers<T>                                         \
{                                                                                 \
  typedef PrimitiveT Type;                                                        \
  static const size_t Count = PrimitiveCount;                                     \
};

// Include basic native type template specializations
#include "BasicNativeTypes.inl"

// (Intentionally left basic native type macros defined to be used later by "BasicNativeTypesMath.inl" in the Math project)

//---------------------------------------------------------------------------------//
//                               Native Type ID                                    //
//---------------------------------------------------------------------------------//

/// Unique C++ type identifier
typedef uint NativeTypeId;

/// Provides the constant native type ID of T, else fails to compile
/// (Defined only for basic native types)
template <typename T, typename Enable = void>
struct ConstantNativeTypeId;

template <typename T>
struct ConstantNativeTypeId<typename T, TC_ENABLE_IF(IsBasicNativeType<T>::Value)>
{
  static const NativeTypeId Value = NativeTypeId(BasicNativeTypeToEnum<T>::Value);
};

/// Invalid native type ID
/// (Used as a failure value or to represent a currently unknown native type)
static const NativeTypeId cInvalidNativeTypeId = NativeTypeId(BasicNativeType::Unknown);

/// Compile-time constant native type IDs
/// (Includes the invalid/unknown native type ID)
static const NativeTypeId cConstantNativeTypeIdMin   = NativeTypeId(BasicNativeType::Unknown);
static const NativeTypeId cConstantNativeTypeIdMax   = NativeTypeId(BasicNativeType::Size) - 1;
static const NativeTypeId cConstantNativeTypeIdCount = (cConstantNativeTypeIdMax - cConstantNativeTypeIdMin) + 1;

/// Runtime generated native type IDs
static const NativeTypeId cRuntimeNativeTypeIdMin = cConstantNativeTypeIdMax + 1;

/// Returns true if the specified native type ID is a runtime native type ID (as opposed to a compile-time constant ID), else false
inline bool IsRuntimeNativeTypeId(NativeTypeId nativeTypeId)
{
  return nativeTypeId >= cRuntimeNativeTypeIdMin;
}

/// Returns true if the specified native type ID is a constant native type ID (as opposed to a runtime generated ID), else false
/// (Includes the invalid/unknown native type ID)
inline bool IsConstantNativeTypeId(NativeTypeId nativeTypeId)
{
  return !IsRuntimeNativeTypeId(nativeTypeId);
}

/// Gets and increments the next available runtime native type ID
inline NativeTypeId AcquireNextRuntimeNativeTypeId()
{
  // Generate runtime native type ID counter (as lazy singleton)
  // (Atomic to ensure thread safety on post-increment)
  static Atomic<NativeTypeId> nextRuntimeNativeTypeId = cRuntimeNativeTypeIdMin;
  return nextRuntimeNativeTypeId++;
}

/// Returns the constant native type ID of the specified type T
/// (Defined only for basic native types)
template <typename T, TF_ENABLE_IF(IsBasicNativeType<T>::Value)>
NativeTypeId GetConstantNativeTypeId()
{
  return ConstantNativeTypeId<T>::Value;
}

/// Returns the runtime native type ID of the specified type T
/// (Defined only for non-basic native types)
template <typename T, TF_DISABLE_IF(IsBasicNativeType<T>::Value)>
NativeTypeId GetRuntimeNativeTypeId()
{
  // Generate runtime native type ID (as lazy singleton)
  static NativeTypeId runtimeNativeTypeId = AcquireNextRuntimeNativeTypeId();
  return runtimeNativeTypeId;
}

/// Returns the native type ID of the specified type T
/// (Definition for basic native types returns a constant native type ID)
template <typename T, TF_ENABLE_IF(IsBasicNativeType<T>::Value)>
NativeTypeId GetNativeTypeId()
{
  return GetConstantNativeTypeId<T>();
}
/// Returns the native type ID of the specified type T
/// (Definition for non-basic native types returns a runtime native type ID)
template <typename T, TF_DISABLE_IF(IsBasicNativeType<T>::Value)>
NativeTypeId GetNativeTypeId()
{
  return GetRuntimeNativeTypeId<T>();
}

//---------------------------------------------------------------------------------//
//                           Generic Object Methods                                //
//---------------------------------------------------------------------------------//

//
// Opaque Function Interface:
// Allows callers to operate on type-erased objects at run-time through this generic interface.
//

/// Destructs the object at source (does not delete or free, only calls the object's destructor)
typedef void (*DestructObjectFn) (void* source);

/// Default constructs a new object in place at destination
typedef void (*DefaultConstructObjectFn) (void* destination);

/// Copy constructs a new object in place at destination, copied from the object at source
typedef void (*CopyConstructObjectFn) (const void* source, void* destination);

/// Move constructs a new object in place at destination, moved from the object at source
typedef void (*MoveConstructObjectFn) (void* source, void* destination);

/// Returns true if the object at lhs is equal to the object at rhs, else false
typedef bool (*EqualToObjectFn) (const void* lhs, const void* rhs);

/// Returns the hash of the object at source
typedef size_t (*HashObjectFn) (const void* source);

/// Returns the string representation of the object at source (formatted according to shortFormat if applicable)
typedef String (*ObjectToStringFn) (const void* source, bool shortFormat);

/// Parses the source string as the erased type and assigns the result to the object at destination
typedef void (*StringToObjectFn) (StringRange source, void* destination);

//
// Transparent Function Definitions:
// Satisfies the opaque function interface for any given type.
// Hardcodes actual static type requirements and necessary type-remembrance conversions in the templated function definition itself.
//

/// Destructs the object at source (does not delete or free, only calls the object's destructor)
/// (Requires an accessible destructor on T)
template <typename T>
ZeroSharedTemplate void DestructObject(void* source)
{
  // Get source object
  T& sourceObject = *reinterpret_cast<T*>(source);

  // Invoke the destructor on the object
  // (We don't call delete or free because we're not managing this memory, only uninitializing it)
  sourceObject.~T();
}

/// Default constructs a new object in place at destination
/// (Requires an accessible default constructor on T)
template <typename T>
ZeroSharedTemplate void DefaultConstructObject(void* destination)
{
  // Use placement new to default construct in place at 'destination'
  new (destination) T();
}

/// Copy constructs a new object in place at destination, copied from the object at source
/// (Requires an accessible copy constructor on T)
template <typename T>
ZeroSharedTemplate void CopyConstructObject(const void* source, void* destination)
{
  // Get source object
  const T& sourceObject = *reinterpret_cast<const T*>(source);

  // Use placement new to copy construct in place at 'destination', copying the source object
  new (destination) T(sourceObject);
}

/// Move constructs a new object in place at destination, moved from the object at source
/// (Requires an accessible move constructor on T)
template <typename T>
ZeroSharedTemplate void MoveConstructObject(void* source, void* destination)
{
  // Get source object
  T& sourceObject = *reinterpret_cast<T*>(source);

  // Use placement new to move construct in place at 'destination', moving the source object
  new (destination) T(ZeroMove(sourceObject));
}

/// Returns true if the object at lhs is equal to the object at rhs, else false
/// (Requires a valid compare policy for T)
template <typename T>
ZeroSharedTemplate bool EqualToObject(const void* lhs, const void* rhs)
{
  // Generate compare policy (as lazy singleton)
  static ComparePolicy<T> comparePolicy;

  // Get lhs and rhs objects
  const T& lhsObject = *reinterpret_cast<const T*>(lhs);
  const T& rhsObject = *reinterpret_cast<const T*>(rhs);

  // Perform equality comparison
  return comparePolicy.Equal(lhsObject, rhsObject);
}

/// Returns the hash of the object at source
/// (Requires a valid hash policy for T)
template <typename T>
ZeroSharedTemplate size_t HashObject(const void* source)
{
  // Generate hash policy (as lazy singleton)
  static HashPolicy<T> hashPolicy;

  // Get source object
  const T& sourceObject = *reinterpret_cast<const T*>(source);

  // Perform hash operation
  return hashPolicy(sourceObject);
}

/// Returns the string representation of the object at source (formatted according to shortFormat if applicable)
/// (Requires a global to string function for T)
template <typename T>
ZeroSharedTemplate String ObjectToString(const void* source, bool shortFormat)
{
  // Get source object
  const T& sourceObject = *reinterpret_cast<const T*>(source);

  // Convert object to string
  return ToString(sourceObject, shortFormat);
}

/// Parses the source string as the erased type and assigns the result to the object at destination
/// (Requires a global to value function for T)
template <typename T>
ZeroSharedTemplate void StringToObject(StringRange source, void* destination)
{
  // Get destination object
  T& destinationObject = *reinterpret_cast<T*>(destination);

  // Convert string to object
  ToValue(source, destinationObject);
}

//
// Transparent Function Generation:
// Determines at compile-time the appropriate transparent function definition to return for any given type.
// Uses C++ type traits and SFINAE to determine each type's capabilities in order to choose the function with the most appropriate behavior.
// Returns nullptr to allow for optional type functionality, queryable at runtime to support dynamic programming.
//

/// Gets a destruct object function if the type has an accessible destructor, else returns nullptr
template <typename T, TF_ENABLE_IF(has_destructor<T>::value)>
ZeroSharedTemplate DestructObjectFn GetDestructObjectFn() { return DestructObject<T>; }
template <typename T, TF_DISABLE_IF(has_destructor<T>::value)>
ZeroSharedTemplate DestructObjectFn GetDestructObjectFn() { return nullptr; }

/// Gets a default construct object function if the type has an accessible default constructor, else returns nullptr
template <typename T, TF_ENABLE_IF(has_default_constructor<T>::value)>
ZeroSharedTemplate DefaultConstructObjectFn GetDefaultConstructObjectFn() { return DefaultConstructObject<T>; }
template <typename T, TF_DISABLE_IF(has_default_constructor<T>::value)>
ZeroSharedTemplate DefaultConstructObjectFn GetDefaultConstructObjectFn() { return nullptr; }

/// Gets a copy construct object function if the type has an accessible copy constructor, else returns nullptr
template <typename T, TF_ENABLE_IF(has_copy_constructor<T>::value)>
ZeroSharedTemplate CopyConstructObjectFn GetCopyConstructObjectFn() { return CopyConstructObject<T>; }
template <typename T, TF_DISABLE_IF(has_copy_constructor<T>::value)>
ZeroSharedTemplate CopyConstructObjectFn GetCopyConstructObjectFn() { return nullptr; }

/// Gets a move construct object function if the type has an accessible move constructor, else returns nullptr
template <typename T, TF_ENABLE_IF(has_move_constructor<T>::value)>
ZeroSharedTemplate MoveConstructObjectFn GetMoveConstructObjectFn() { return MoveConstructObject<T>; }
template <typename T, TF_DISABLE_IF(has_move_constructor<T>::value)>
ZeroSharedTemplate MoveConstructObjectFn GetMoveConstructObjectFn() { return nullptr; }

/// Gets an equal to object function if the type has a valid compare policy, else returns nullptr
template <typename T, TF_ENABLE_IF(has_valid_compare_policy<T>::value)>
ZeroSharedTemplate EqualToObjectFn GetEqualToObjectFn() { return EqualToObject<T>; }
template <typename T, TF_DISABLE_IF(has_valid_compare_policy<T>::value)>
ZeroSharedTemplate EqualToObjectFn GetEqualToObjectFn() { return nullptr; }

/// Gets a hash object function if the type has a valid hash policy, else returns nullptr
template <typename T, TF_ENABLE_IF(has_valid_hash_policy<T>::value)>
ZeroSharedTemplate HashObjectFn GetHashObjectFn() { return HashObject<T>; }
template <typename T, TF_DISABLE_IF(has_valid_hash_policy<T>::value)>
ZeroSharedTemplate HashObjectFn GetHashObjectFn() { return nullptr; }

/// Gets an object to string function if the type has a global to string function, else returns nullptr
template <typename T, TF_ENABLE_IF(has_global_to_string<T>::value)>
ZeroSharedTemplate ObjectToStringFn GetObjectToStringFn() { return ObjectToString<T>; }
template <typename T, TF_DISABLE_IF(has_global_to_string<T>::value)>
ZeroSharedTemplate ObjectToStringFn GetObjectToStringFn() { return nullptr; }

/// Gets a string to object function if the type has a global to value function, else returns nullptr
template <typename T, TF_ENABLE_IF(has_global_to_value<T>::value)>
ZeroSharedTemplate StringToObjectFn GetStringToObjectFn() { return StringToObject<T>; }
template <typename T, TF_DISABLE_IF(has_global_to_value<T>::value)>
ZeroSharedTemplate StringToObjectFn GetStringToObjectFn() { return nullptr; }

//---------------------------------------------------------------------------------//
//                                Native Type                                      //
//---------------------------------------------------------------------------------//

/// Gets the native type singleton uniquely representing type T (ignoring qualifications)
#define NativeTypeOf(T) NativeType::GetInstance<typename Decay<T>::Type>()

/// Gets the native type ID uniquely identifying type T (ignoring qualifications)
#define NativeTypeIdOf(T) GetNativeTypeId<typename Decay<T>::Type>()

/// Gets the constant native type ID uniquely identifying type T (ignoring qualifications), else fails to compile
#define ConstantNativeTypeIdOf(T) ConstantNativeTypeId<typename Decay<T>::Type>::Value

// TODO: Make NativeType singletons unique across EXE and DLLs

/// Native type contains type-erased run-time type information about a particular C++ type.
/// Type information is generated according to compile-time type traits for its given type.
/// Native type is a lazy singleton, uniquely representing its given type.
class ZeroShared NativeType
{
protected:
  /// Constructs a native type object representing type T
  template<typename T>
  NativeType(T*) // (C++ requires a dummy parameter in order to specify a templated default constructor)
  {
    // Initialize type info
    mDebugTypeName = typeid(T).name();
    mTypeId        = GetNativeTypeId<T>();
    mTypeSize      = sizeof(T);

    // Initialize type traits
    mIsBasicNativeType               = IsBasicNativeType<T>::Value;
    mIsBasicNativeTypeArithmetic     = IsBasicNativeTypeArithmetic<T>::Value;
    mIsBasicNativeTypeIntegral       = IsBasicNativeTypeIntegral<T>::Value;
    mIsBasicNativeTypeFloatingPoint  = IsBasicNativeTypeFloatingPoint<T>::Value;
    mIsBasicNativeTypePrimitive      = IsBasicNativeTypePrimitive<T>::Value;
    mIsBasicNativeTypeMultiPrimitive = IsBasicNativeTypeMultiPrimitive<T>::Value;
    mBasicNativeTypeEnum             = BasicNativeTypeToEnum<T>::Value;

    // Is a primitive?
    if(IsBasicNativeTypePrimitive<T>::Value)
    {
      // (NOTE: We must initialize using 'this' here instead of relying on NativeTypeOf() because we are still constructing this NativeType!
      // Trying to access a local static while initializing said local static from the same thread causes a thread deadlock!)
      mBasicNativeTypePrimitiveMembersType = this;
    }
    // Is not a primitive?
    else
    {
      // (This NativeType should not be the same C++ type as it's primitive members' NativeType,
      //  otherwise it should be initialized above using 'this' to prevent a thread deadlock.)
      Assert(typeid(T) != typeid(BasicNativeTypePrimitiveMembers<T>::Type));
      mBasicNativeTypePrimitiveMembersType = NativeTypeOf(BasicNativeTypePrimitiveMembers<T>::Type);
    }

    mBasicNativeTypePrimitiveMembersCount = BasicNativeTypePrimitiveMembers<T>::Count;

    // Initialize object methods
    mDestructObjectFn         = GetDestructObjectFn<T>();
    mDefaultConstructObjectFn = GetDefaultConstructObjectFn<T>();
    mCopyConstructObjectFn    = GetCopyConstructObjectFn<T>();
    mMoveConstructObjectFn    = GetMoveConstructObjectFn<T>();
    mEqualToObjectFn          = GetEqualToObjectFn<T>();
    mHashObjectFn             = GetHashObjectFn<T>();
    mObjectToStringFn         = GetObjectToStringFn<T>();
    mStringToObjectFn         = GetStringToObjectFn<T>();
  }

public:
  /// Gets the unique native type object representing type T
  /// Note: It's recommended to use the NativeTypeOf(T) macro instead of calling this directly,
  /// as the macro automatically handles decaying the type to it's most core identity (removes qualifications)
  template <typename T>
  static NativeType* GetInstance()
  {
    // Generate native type (as lazy singleton)
    static NativeType nativeType(static_cast<T*>(nullptr));
    return &nativeType;
  }
  template <>
  static NativeType* GetInstance<void>()
  {
    // Because void is an incomplete type we do not provide type info for it.
    // Instead, NativeTypeOf(void) is used as a sentinel value, returning nullptr.
    return nullptr;
  }

  //
  // Type Info
  //

  /// Implementation-defined type name, suitable for debugging purposes only
  /// Provided by std::type_info::name which only guarantees it will return a valid null-terminated string
  cstr mDebugTypeName;

  /// Unique C++ type identifier
  /// Constant for basic native types, generated on first access at runtime for all other types
  NativeTypeId mTypeId;

  /// Size of type in bytes
  /// Provided by sizeof(T)
  size_t mTypeSize;

  //
  // Type Traits
  //

  /// Is a basic native type?
  /// (Runtime version of IsBasicNativeType)
  bool mIsBasicNativeType;

  /// Is an arithmetic basic native type?
  /// (Runtime version of IsBasicNativeTypeArithmetic)
  bool mIsBasicNativeTypeArithmetic;

  /// Is an integral basic native type?
  /// (Runtime version of IsBasicNativeTypeIntegral)
  bool mIsBasicNativeTypeIntegral;

  /// Is a floating-point basic native type?
  /// (Runtime version of IsBasicNativeTypeFloatingPoint)
  bool mIsBasicNativeTypeFloatingPoint;

  /// Is a basic native type primitive?
  /// (Runtime version of IsBasicNativeTypePrimitive)
  bool mIsBasicNativeTypePrimitive;

  /// Is a basic native type multi-primitive?
  /// (Runtime version of IsBasicNativeTypeMultiPrimitive)
  bool mIsBasicNativeTypeMultiPrimitive;

  /// Corresponding BasicNativeType enum value of T, else BasicNativeType::Unknown
  /// (Runtime version of BasicNativeTypeToEnum)
  BasicNativeType::Enum mBasicNativeTypeEnum;

  /// Underlying arithmetic basic native type's primitive members type, else nullptr
  /// (Runtime version of BasicNativeTypePrimitiveMembers::Type)
  NativeType* mBasicNativeTypePrimitiveMembersType;

  /// Underlying arithmetic basic native type's primitive members count, else 0
  /// (Runtime version of BasicNativeTypePrimitiveMembers::Count)
  size_t mBasicNativeTypePrimitiveMembersCount;

  //
  // Object Methods
  //

  /// Destruct object function
  DestructObjectFn mDestructObjectFn;

  /// Default construct object function
  DefaultConstructObjectFn mDefaultConstructObjectFn;

  /// Copy construct object function
  CopyConstructObjectFn mCopyConstructObjectFn;

  /// Move construct object function
  MoveConstructObjectFn mMoveConstructObjectFn;

  /// Equal to object function
  EqualToObjectFn mEqualToObjectFn;

  /// Hash object function
  HashObjectFn mHashObjectFn;

  /// Object to string function
  ObjectToStringFn mObjectToStringFn;

  /// String to object function
  StringToObjectFn mStringToObjectFn;
};

} // namespace Zero
