///////////////////////////////////////////////////////////////////////////////
///
/// \file TypeTraits.hpp
/// Type Traits for containers.
///
/// Authors: Chris Peters, Andrew Colean
/// Copyright 2010-2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <utility>
#include <typeinfo>
#include "Utility/Typedefs.hpp"

namespace Zero
{

//
// Integral Constant
//

/// Provides a compile-time integral constant as a templated type
template<typename IntegerType, IntegerType Value>
struct integral_constant
{
  static const IntegerType value = Value;
  typedef IntegerType value_type;
  typedef integral_constant<IntegerType, value> type;
};
/// Static Data Member Instantiation
template<typename IntegerType, IntegerType Value>
const IntegerType integral_constant<IntegerType, Value>::value;

/// Integral constant true type
typedef integral_constant<bool, true>  true_type;
/// Integral constant false type
typedef integral_constant<bool, false> false_type;

//
// Is Same
//

/// is_same helper class, does not use integral_constant
template <typename TypeA, typename TypeB>
struct is_same_helper
{
  static const bool value = false;
};
template <typename TypeA>
struct is_same_helper<TypeA, TypeA>
{
  static const bool value = true;
};

/// Provides a constant defined as true if TypeA and TypeB are the same exact type, else defined as false
template <typename TypeA, typename TypeB>
struct is_same : public integral_constant<bool, (is_same_helper< TypeA, TypeB >::value) > {};

//
// Is Base Of
//

template <typename T> static true_type  is_base_of_helper(T*);
template <typename T> static false_type is_base_of_helper(...);

/// Provides a constant defined as true if DerivedType derives from BaseType
template <typename BaseType, typename DerivedType>
struct is_base_of : public TypeOf(is_base_of_helper<BaseType>((DerivedType*)nullptr)) {};

//
// Make Type
//

/// Provides a typedef defined as T (seems pointless but it's actual necessary to use this in some cases)
template <typename T> struct make_type { typedef T type; };

//
// Make Void
//

/// Provides a typedef defined as void (seems pointless but it's actual necessary to use this in some cases)
template <typename> struct make_void { typedef void type; };

//
// Is Class (or Struct)
//

/// is_class helper class, does not use integral_constant
template <typename T, typename = void>
struct is_class_helper
{
  static const bool value = false;
};
template <typename T>
struct is_class_helper<T, typename make_void<int T::*>::type>
{
  static const bool value = true;
};

/// Provides a constant defined as true if T is a class or struct type, else defined as false
template <typename T>
struct is_class : public integral_constant<bool, (is_class_helper< T >::value) > {};

//
// Remove Qualifiers (1)
//

/// Removes the top-most pointer from a given type
template<typename T> struct remove_pointer     { typedef T type; };
template<typename T> struct remove_pointer<T*> { typedef T type; };

/// Removes the top-most const qualifier from a given type
template<typename T> struct remove_const            { typedef T type; };
template<typename T> struct remove_const< const T > { typedef T type; };

/// Removes the top-most volatile qualifier from a given type
template<typename T> struct remove_volatile               { typedef T type; };
template<typename T> struct remove_volatile< volatile T > { typedef T type; };

/// Removes the top-most const and volatile qualifiers from a given type
template<typename T> struct remove_const_and_volatile { typedef typename remove_volatile<typename remove_const< T >::type>::type type; };

//
// Type Traits
//

/// Provides a constant defined as true if T is a pointer type, else defined as false
template<typename T> struct is_pointer     : public false_type {};
template<typename T> struct is_pointer<T*> : public true_type  {};

/// Provides a constant defined as true if T is a reference type, else defined as false
template<typename T> struct is_reference     : public false_type {};
template<typename T> struct is_reference<T&> : public true_type  {};

/// Provides a constant defined as true if T is a floating-point type, else defined as false
template<typename T> struct is_floating_point : public false_type {};
template<> struct is_floating_point<float>    : public true_type  {};
template<> struct is_floating_point<double>   : public true_type  {};

/// Provides a constant defined as true if T is a signed arithmetic type, else defined as false
template<typename T> struct is_signed  : public false_type {};
template<> struct is_signed<slonglong> : public true_type  {};
template<> struct is_signed<slong>     : public true_type  {};
template<> struct is_signed<sint>      : public true_type  {};
template<> struct is_signed<sshort>    : public true_type  {};
template<> struct is_signed<schar>     : public true_type  {};
template<> struct is_signed<float>     : public true_type  {};
template<> struct is_signed<double>    : public true_type  {};

/// Provides a constant defined as true if T is an unsigned arithmetic type, else defined as false
template<typename T> struct is_unsigned  : public false_type {};
template<> struct is_unsigned<ulonglong> : public true_type  {};
template<> struct is_unsigned<ulong>     : public true_type  {};
template<> struct is_unsigned<uint>      : public true_type  {};
template<> struct is_unsigned<ushort>    : public true_type  {};
template<> struct is_unsigned<uchar>     : public true_type  {};

/// Provides a constant defined as true if T is an integral type, else defined as false
template<typename T> struct is_integral  : public false_type {};
template<> struct is_integral<slonglong> : public true_type  {};
template<> struct is_integral<slong>     : public true_type  {};
template<> struct is_integral<sint>      : public true_type  {};
template<> struct is_integral<sshort>    : public true_type  {};
template<> struct is_integral<schar>     : public true_type  {};
template<> struct is_integral<ulonglong> : public true_type  {};
template<> struct is_integral<ulong>     : public true_type  {};
template<> struct is_integral<uint>      : public true_type  {};
template<> struct is_integral<ushort>    : public true_type  {};
template<> struct is_integral<uchar>     : public true_type  {};
template<> struct is_integral<char>      : public true_type  {};
template<> struct is_integral<bool>      : public true_type  {};

/// Provides a constant defined as true if T is a bool type, else defined as false
template<typename T> struct is_bool : public false_type {};
template<> struct is_bool<bool>     : public true_type  {};

/// Provides a constant defined as true if T is a void type, else defined as false
template<typename T> struct is_void : public false_type {};
template<> struct is_void<void>     : public true_type  {};

/// Provides a constant defined as true if T is a nullptr_t type, else defined as false
template<typename T> struct is_null_pointer        : public false_type {};
// template<> struct is_null_pointer<NullPointerType> : public true_type  {}; // TODO

/// Provides a constant defined as true if T is a array type, else defined as false
template<typename T> struct is_array                 : public false_type {};
template<typename T> struct is_array<T[]>            : public true_type  {};
template<typename T, size_t N> struct is_array<T[N]> : public true_type  {};

/// is_member_pointer helper class
template<typename T> struct is_member_pointer_helper                     : public false_type {};
template<typename T, typename U> struct is_member_pointer_helper<T U::*> : public true_type {};

/// Provides a constant defined as true if T is a member pointer type, else defined as false
template<typename T>
struct is_member_pointer : public integral_constant<bool, (is_member_pointer_helper< typename remove_const_and_volatile< T >::type >::value) > {};

// These classes are guaranteed to be sizeof() 1 and 2, for use in compile time conditionals
struct yes { char bytes[1]; };
struct no { char bytes[2]; };

/// is_union_or_class_helper helper class
template<typename T>
struct is_union_or_class_helper
{
  template<typename T2>
  static yes Test(int T2::*);
  template<typename T2>
  static no Test(...);

  static const bool value = sizeof((Test<T>(0))) == sizeof(yes);
};

/// Provides a constant defined as true if T is a union or class type, else defined as false
template<typename T>
struct is_union_or_class : public integral_constant<bool, (is_union_or_class_helper<T>::value) > {};

/// TODO: Update type traits to use compiler intrinsics per compiler
/// TODO: Implement is_function, and exclude function types
/// Provides a constant defined as true if T is an enum type, else defined as false
template<typename T>
struct is_enum : public integral_constant<bool, (!is_void< T >::value
                                              && !is_integral< T >::value
                                              && !is_floating_point< T >::value
                                              && !is_array< T >::value
                                              && !is_pointer< T >::value
                                              && !is_reference< T >::value
                                              && !is_member_pointer< T >::value
                                              && !is_union_or_class< T >::value
                                              /*&& !is_function< T >::value*/) > {};

/// Use SFINAE to detect if we have a member
/// This must be a macro because the name of the member cannot be provided as a template argument
#define ZeroDeclareHasMemberTrait(TypeTraitName, MemberName)                                        \
  template <typename T> static ::Zero::true_type  check_##TypeTraitName(TypeOf(&T::MemberName)*);   \
  template <typename T> static ::Zero::false_type check_##TypeTraitName(...);                       \
  template <typename T> struct TypeTraitName : public TypeOf(check_##TypeTraitName<T>(nullptr)) {};

/// Provides a constant defined as true if T is an enum or integral type, else defined as false
template<typename T>
struct is_enum_or_integral : public integral_constant<bool, (is_enum< T >::value || is_integral< T >::value) > {};

/// Provides a constant defined as true if T is an arithmetic (integral or floating point) type, else defined as false
template<typename T>
struct is_arithmetic : public integral_constant<bool, (is_integral< T >::value || is_floating_point< T >::value) > {};

/// Provides a constant defined as true if T is a fundamental (arithmetic, void, or nullptr_t) type, else defined as false
template<typename T>
struct is_fundamental : public integral_constant<bool, (is_arithmetic< T >::value || is_void< T >::value || is_null_pointer< T >::value) > {};

/// Provides a constant defined as true if T is a scalar (arithmetic, enum, pointer, or nullptr_t) type, else defined as false
template<typename T>
struct is_scalar : public integral_constant<bool, (is_arithmetic< T >::value || is_enum< T >::value || is_pointer< T >::value || is_null_pointer< T >::value) > {};

/// Provides a constant defined as true if T is a POD type, else defined as false
template<typename T>
struct is_pod : public integral_constant<bool, (is_scalar< T >::value) > {};

/// Provides a constant defined as true if T has a trivial constructor, else defined as false
template<typename T> struct has_trivial_constructor : public integral_constant<bool, is_pod< T >::value> {};
/// Provides a constant defined as true if T has a trivial copy constructor, else defined as false
template<typename T> struct has_trivial_copy        : public integral_constant<bool, is_pod< T >::value> {};
/// Provides a constant defined as true if T has a trivial copy assignment operator, else defined as false
template<typename T> struct has_trivial_assign      : public integral_constant<bool, is_pod< T >::value> {};
/// Provides a constant defined as true if T has a trivial destructor, else defined as false
template<typename T> struct has_trivial_destructor  : public integral_constant<bool, is_pod< T >::value> {};

/// Provides standard type traits, provided for convenience
template<typename T>
struct StandardTraits
{
  typedef is_pod< T >                  is_pod_;
  typedef has_trivial_constructor< T > has_trivial_constructor_;
  typedef has_trivial_copy< T >        has_trivial_copy_;
  typedef has_trivial_assign< T >      has_trivial_assign_;
  typedef has_trivial_destructor< T >  has_trivial_destructor_;
};

/// Provides type trait overrides to treat a type as POD, provided for convenience
struct PodOverride
{
  typedef true_type is_pod_;
  typedef true_type has_trivial_copy_;
  typedef true_type has_trivial_assign_;
  typedef true_type has_trivial_constructor_;
  typedef true_type has_trivial_destructor_;
};

//
// Conditional
//

/// Provides a typedef defined as TypeA if Condition is true, else defined as TypeB
template <bool Condition, typename TypeA, typename TypeB>
struct conditional
{
  typedef TypeA type;
};
template <typename TypeA, typename TypeB>
struct conditional<false, TypeA, TypeB>
{
  typedef TypeB type;
};

//
// Enable/Disable If
//

/// Provides a typedef defined as Type (void by default) if Condition is true, else contains nothing
/// Used to selectively "enable" templated class and function declarations if the specific condition is met
template <bool Condition, typename Type = void>
struct enable_if
{
};
template <typename Type>
struct enable_if<true, Type>
{
  typedef Type type;
};

/// Provides a typedef defined as Type (void by default) if Condition is false, else contains nothing
/// This enables the exact inverse of enable_if templated class and function declarations if the specific condition is met
template <bool Condition, typename Type = void>
struct disable_if
{
};
template <typename Type>
struct disable_if<false, Type>
{
  typedef Type type;
};

/// Enable If via Class Template Parameter
#define TC_ENABLE_IF(Condition)             typename Zero::enable_if<(Condition)>::type
#define TC_ENABLE_IF_IS_SAME(TypeA, TypeB)  typename Zero::enable_if<(Zero::is_same<TypeA, TypeB>::value)>::type
#define TC_DISABLE_IF(Condition)            typename Zero::disable_if<(Condition)>::type
#define TC_DISABLE_IF_IS_SAME(TypeA, TypeB) typename Zero::disable_if<(Zero::is_same<TypeA, TypeB>::value)>::type

/// Enable If via Function Template Parameter (Declaration)
#define TF_ENABLE_IF(Condition)             typename Zero::enable_if<(Condition)>::type* = nullptr
#define TF_ENABLE_IF_IS_SAME(TypeA, TypeB)  typename Zero::enable_if<(Zero::is_same<TypeA, TypeB>::value)>::type* = nullptr
#define TF_DISABLE_IF(Condition)            typename Zero::disable_if<(Condition)>::type* = nullptr
#define TF_DISABLE_IF_IS_SAME(TypeA, TypeB) typename Zero::disable_if<(Zero::is_same<TypeA, TypeB>::value)>::type* = nullptr

/// Enable If via Function Template Parameter (Definition)
#define TF_ENABLE_IF_DEF(Condition)             typename Zero::enable_if<(Condition)>::type*
#define TF_ENABLE_IF_IS_SAME_DEF(TypeA, TypeB)  typename Zero::enable_if<(Zero::is_same<TypeA, TypeB>::value)>::type*
#define TF_DISABLE_IF_DEF(Condition)            typename Zero::disable_if<(Condition)>::type*
#define TF_DISABLE_IF_IS_SAME_DEF(TypeA, TypeB) typename Zero::disable_if<(Zero::is_same<TypeA, TypeB>::value)>::type*

/// Enable If via Function Return Type
#define R_ENABLE_IF(Condition, ReturnType)  typename Zero::enable_if<(Condition), ReturnType>::type
#define R_DISABLE_IF(Condition, ReturnType) typename Zero::disable_if<(Condition), ReturnType>::type

/// Enable If via Function Parameter (Declaration)
#define P_ENABLE_IF(Condition)             typename Zero::enable_if<(Condition)>::type* = nullptr
#define P_ENABLE_IF_IS_SAME(TypeA, TypeB)  typename Zero::enable_if<(Zero::is_same<TypeA, TypeB>::value)>::type* = nullptr
#define P_DISABLE_IF(Condition)            typename Zero::disable_if<(Condition)>::type* = nullptr
#define P_DISABLE_IF_IS_SAME(TypeA, TypeB) typename Zero::disable_if<(Zero::is_same<TypeA, TypeB>::value)>::type* = nullptr

/// Enable If via Function Parameter (Definition)
#define P_ENABLE_IF_DEF(Condition)             typename Zero::enable_if<(Condition)>::type*
#define P_ENABLE_IF_IS_SAME_DEF(TypeA, TypeB)  typename Zero::enable_if<(Zero::is_same<TypeA, TypeB>::value)>::type*
#define P_DISABLE_IF_DEF(Condition)            typename Zero::disable_if<(Condition)>::type*
#define P_DISABLE_IF_IS_SAME_DEF(TypeA, TypeB) typename Zero::disable_if<(Zero::is_same<TypeA, TypeB>::value)>::type*

//
// Make Signed/Unsigned
//

/// Provides a typedef defined as the signed equivalent of integral Type
template <typename Type> struct make_signed {};
template <> struct make_signed<ulonglong>   { typedef slonglong type; };
template <> struct make_signed<ulong>       { typedef slong     type; };
template <> struct make_signed<uint>        { typedef sint      type; };
template <> struct make_signed<ushort>      { typedef sshort    type; };
template <> struct make_signed<uchar>       { typedef schar     type; };
template <> struct make_signed<slonglong>   { typedef slonglong type; };
template <> struct make_signed<slong>       { typedef slong     type; };
template <> struct make_signed<sint>        { typedef sint      type; };
template <> struct make_signed<sshort>      { typedef sshort    type; };
template <> struct make_signed<schar>       { typedef schar     type; };
template <> struct make_signed<char>        { typedef schar     type; };

/// Provides a typedef defined as the unsigned equivalent of integral Type
template <typename Type> struct make_unsigned {};
template <> struct make_unsigned<slonglong>   { typedef ulonglong type; };
template <> struct make_unsigned<slong>       { typedef ulong     type; };
template <> struct make_unsigned<sint>        { typedef uint      type; };
template <> struct make_unsigned<sshort>      { typedef ushort    type; };
template <> struct make_unsigned<schar>       { typedef uchar     type; };
template <> struct make_unsigned<ulonglong>   { typedef ulonglong type; };
template <> struct make_unsigned<ulong>       { typedef ulong     type; };
template <> struct make_unsigned<uint>        { typedef uint      type; };
template <> struct make_unsigned<ushort>      { typedef ushort    type; };
template <> struct make_unsigned<uchar>       { typedef uchar     type; };
template <> struct make_unsigned<char>        { typedef uchar     type; };

//
// Move Semantics (1)
//

/// Move reference type
template<typename T>
struct MoveReference
{
  /// Stored value type
  typedef T type;

  /// Constructors
  explicit MoveReference(T& reference)
    : mReference(reference)
  {
  }
  MoveReference(const MoveReference& rhs)
    : mReference(rhs.mReference)
  {
  }

  /// Arrow Operator
  /// Provides direct member access
  T* operator ->() const
  {
    return &mReference;
  }

  /// Indirection Operator
  /// Provides reference access
  T& operator *() const
  {
    return mReference;
  }

  /// Address Of Operator
  /// Provides address access
  T* operator &() const
  {
    return &mReference;
  }

// #ifdef SupportsMoveSemantics
//   /// Conversion Operator
//   operator T&&()
//   {
//     return std::move(mReference);
//   }
// #endif

  /// Data reference
  T& mReference;

private:
  /// No Copy Assignment Operator
  MoveReference& operator=(const MoveReference&);
};

//
// Add Qualifiers
//

/// Adds a pointer to the given type
template<typename T> struct add_pointer { typedef T* type; };

/// Adds a reference to the given type
template<typename T> struct add_reference { typedef T& type; };

//
// Remove Qualifiers (2)
//

/// Removes the top-most reference from a given type
template<typename T> struct remove_reference        { typedef T type; };
template<typename T> struct remove_reference< T& >  { typedef T type; };
#ifdef SupportsMoveSemantics
template<typename T> struct remove_reference< T&& > { typedef T type; };
#endif
template<typename T> struct remove_reference< MoveReference< T > >   { typedef T type; };
template<typename T> struct remove_reference< MoveReference< T >& >  { typedef T type; };
#ifdef SupportsMoveSemantics
template<typename T> struct remove_reference< MoveReference< T >&& > { typedef T type; };
#endif

/// Removes the top-most reference and cv-qualifiers from a given type
template<typename T> struct remove_reference_const_and_volatile { typedef typename remove_reference<typename remove_const_and_volatile< T >::type>::type type; };

//
// Decay
//

/// Removes the top-most reference and cv-qualifiers from a given type
template<typename T>
struct Decay
{
  // TODO: Replace implementation with std::decay
  typedef typename remove_reference_const_and_volatile<T>::type Type;
};

//
// Move Semantics (2)
//

/// Provides a constant defined as true if T is a MoveReference<T> type, else defined as false
template<typename T> struct is_move_reference                     : public false_type {};
template<typename T> struct is_move_reference< MoveReference<T> > : public true_type  {};

/// Determines how ZeroMove is applied to types
template <typename T, typename Enable = void>
struct ZeroMoveHelper;

// Is built-in type?
template <typename T>
struct ZeroMoveHelper<T, TC_ENABLE_IF(is_scalar<typename remove_const_and_volatile< T >::type>::value)>
{
  typedef T type;
};

// Is user type?
template <typename T>
struct ZeroMoveHelper<T, TC_ENABLE_IF(!is_scalar<typename remove_const_and_volatile< T >::type>::value)>
{
  typedef MoveReference<typename remove_reference_const_and_volatile< T >::type> type;
};

/// Creates a move reference
template <typename T>
typename ZeroMoveHelper<T>::type ZeroMove(T& value)
{
  return (typename ZeroMoveHelper<T>::type)(value);
}

//
// has_default_constructor
//

/// has_default_constructor_helper helper class
template<typename T>
struct has_default_constructor_helper
{
  template<typename T2>
  static yes Test(int param[sizeof( ::new T2() )]);
  template<typename T2>
  static no Test(...);

  static const bool value = (sizeof(Test<T>(0)) == sizeof(yes));
};

/// Provides a constant defined as true if T has a default constructor, else defined as false
template<typename T>
struct has_default_constructor : public integral_constant<bool, (has_default_constructor_helper<T>::value) > {};

//
// has_copy_constructor
//

/// has_copy_constructor_helper helper class
template<typename T>
struct has_copy_constructor_helper
{
  template<typename T2>
  static yes Test(int param[sizeof( ::new T2(*((T2*)nullptr)) )]);
  template<typename T2>
  static no Test(...);

  static const bool value = (sizeof(Test<T>(0)) == sizeof(yes));
};

/// Provides a constant defined as true if T has a copy constructor, else defined as false
template<typename T>
struct has_copy_constructor : std::is_copy_constructible<T> {}; //: public integral_constant<bool, (has_copy_constructor_helper<T>::value) > {};

//
// has_move_constructor
//

/// has_move_constructor_helper helper class
template<typename T>
struct has_move_constructor_helper
{
  template<typename T2>
  static yes Test(int param[sizeof( ::new T2(*((MoveReference<T2>*)nullptr)) )]);
  template<typename T2>
  static no Test(...);

  static const bool value = (sizeof(Test<T>(0)) == sizeof(yes));
};

/// Provides a constant defined as true if T has a move constructor, else defined as false
template<typename T>
struct has_move_constructor : public integral_constant<bool, (has_move_constructor_helper<T>::value) > {};

//
// has_destructor
//

/// has_destructor_helper helper class
template<typename T>
struct has_destructor_helper
{
  template<typename T2>
  static yes Test(int param[sizeof( ((T2*)nullptr)->~T2(), 5 /*arbitrary value for param size*/ )]);
  template<typename T2>
  static no Test(...);

  static const bool value = (sizeof(Test<T>(0)) == sizeof(yes));
};

/// Provides a constant defined as true if T has a destructor, else defined as false
template<typename T>
struct has_destructor : public integral_constant<bool, (has_destructor_helper<T>::value) > {};

//
// Type Index
//

/// Sortable wrapper around std::type_info
/// Designed to be used as a key in associative containers
class TypeIndex
{
public:
  /// Default Constructor (Note: Creates an invalid TypeIndex!)
  TypeIndex()                               : mTypeInfo(nullptr)        {}
  /// Constructor
  TypeIndex(const std::type_info& typeInfo) : mTypeInfo(&typeInfo)      {}
  /// Copy Constructor
  TypeIndex(const TypeIndex& rhs)           : mTypeInfo(rhs.mTypeInfo)  {}
  /// Move Constructor (Behaves like a copy constructor)
  TypeIndex(MoveReference<TypeIndex> rhs)   : mTypeInfo(rhs->mTypeInfo) {}

  /// Copy Assignment Operator
  TypeIndex& operator =(const TypeIndex& rhs)         { mTypeInfo = rhs.mTypeInfo; return *this;  }
  /// Move Assignment Operator (Behaves like a copy assignment operator)
  TypeIndex& operator =(MoveReference<TypeIndex> rhs) { mTypeInfo = rhs->mTypeInfo; return *this; }

  /// Comparison Operators (compares their type_infos)
  bool operator ==(const TypeIndex& rhs) const { return *mTypeInfo == *rhs.mTypeInfo;           }
  bool operator !=(const TypeIndex& rhs) const { return !(*this == rhs);                        }
  bool operator  <(const TypeIndex& rhs) const { return mTypeInfo->before(*rhs.mTypeInfo) != 0; }
  bool operator >=(const TypeIndex& rhs) const { return !(*this < rhs);                         }
  bool operator  >(const TypeIndex& rhs) const { return rhs < *this;                            }
  bool operator <=(const TypeIndex& rhs) const { return !(rhs < *this);                         }

  /// Returns the type_info name
  const char* Name() const { return mTypeInfo->name(); }

private:
  /// Underlying type_info
  const std::type_info* mTypeInfo;
};

} // namespace Zero
