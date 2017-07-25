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
//                                  Variant                                        //
//---------------------------------------------------------------------------------//

template <typename T>
Variant::Variant(const T& rhs)
  : mNativeType(nullptr)
{
  InternalZeroLocalBuffer();
  Assign<T>(rhs);
}
template <typename T>
Variant::Variant(MoveReference<T> rhs)
  : mNativeType(nullptr)
{
  InternalZeroLocalBuffer();
  Assign<T>(ZeroMove(rhs));
}

//
// Member Operators
//

template <typename T>
Variant& Variant::operator=(const T& rhs)
{
  Assign<T>(rhs);
  return *this;
}
template <typename T>
Variant& Variant::operator=(MoveReference<T> rhs)
{
  Assign<T>(ZeroMove(rhs));
  return *this;
}

template <typename T>
Variant::operator T() const
{
  return GetOrError<T>();
}

template <typename T>
bool Variant::operator==(const T& rhs) const
{
  // Variant is not storing a value of the same type? (Is empty or different type?)
  if(!Is<T>())
    return false;

  // Perform equality comparison
  return InternalEqualToValue(&rhs);
}

template <typename T>
bool Variant::operator!=(const T& rhs) const
{
  return !(*this == rhs);
}

//
// Stored Value Access
//

template <typename T>
bool Variant::Is() const
{
  return Is(NativeTypeOf(T));
}

template <typename T, typename UnqualifiedType>
UnqualifiedType& Variant::GetOrError() const
{
  // Get stored value
  if(UnqualifiedType* value = GetOrNull<UnqualifiedType>()) // Successful?
    return *value;

  Error("%s%s", String::Format("Unable to get stored value of specified type '%s' from Variant - ", NativeTypeOf(T)->mDebugTypeName).c_str(),
                (mNativeType == nullptr ? "Variant is empty" : String::Format("Variant's stored value is type '%s'", mNativeType->mDebugTypeName).c_str()));
  return GetInvalidObject<UnqualifiedType>();
}
template <typename T, typename UnqualifiedType>
UnqualifiedType& Variant::GetOrDefault(const UnqualifiedType& defaultValue) const
{
  // Get stored value
  if(UnqualifiedType* value = GetOrNull<UnqualifiedType>()) // Successful?
    return *value;

  return (UnqualifiedType&)defaultValue;
}
template <typename T, typename UnqualifiedType>
UnqualifiedType* Variant::GetOrNull() const
{
  // Variant is not storing a value of type T?
  if(!Is<UnqualifiedType>())
    return nullptr;

  // Get stored value
  UnqualifiedType* value = reinterpret_cast<UnqualifiedType*>(GetData());
  return value;
}

template <typename T>
void Variant::ToValue(StringRange range)
{
  DefaultConstruct<T>();
  ToValue(range);
}

template <typename T>
bool Variant::IsSmallType()
{
  return IsSmallType(NativeTypeOf(T));
}
template <typename T>
bool Variant::IsLargeType()
{
  return IsLargeType(NativeTypeOf(T));
}

//
// Stored Value Management
//

template <typename T>
void Variant::DefaultConstruct()
{
  // Unqualify type
  typedef typename Decay<T>::Type UnqualifiedType;

  // (Verify type is copy constructible and destructible, else fail to compile)
  StaticAssert(VariantTypeRequiresCopyConstructorAndDestructor0,
               has_copy_constructor<UnqualifiedType>::value && has_destructor<UnqualifiedType>::value,
               "Types assigned to variant must have both an accessible copy constructor and destructor.");

  // Destroy current stored value, if any
  // (Does not free memory or forget the current stored type)
  InternalDestructStoredValue();

  // Replace current stored type, if any, with this new stored type
  // (Prepares internal buffers for the new stored value)
  NativeType* newStoredType = NativeTypeOf(T);
  InternalReplaceStoredType(newStoredType);

  // Default construct the new stored value
  InternalDefaultConstructValue();

  // (Sanity check)
  Assert(Is<T>());
}

template <typename T>
void Variant::Assign(const T& rhs)
{
  // Unqualify type
  typedef typename Decay<T>::Type UnqualifiedType;

  // (Verify type is copy constructible and destructible, else fail to compile)
  StaticAssert(VariantTypeRequiresCopyConstructorAndDestructor1,
               has_copy_constructor<UnqualifiedType>::value && has_destructor<UnqualifiedType>::value,
               "Types assigned to variant must have both an accessible copy constructor and destructor.");

  // Destroy current stored value, if any
  // (Does not free memory or forget the current stored type)
  InternalDestructStoredValue();

  // Replace current stored type, if any, with this new stored type
  // (Prepares internal buffers for the new stored value)
  NativeType* newStoredType = NativeTypeOf(T);
  InternalReplaceStoredType(newStoredType);

  // Copy construct the new stored value
  const UnqualifiedType* unqualifiedValue = static_cast<const UnqualifiedType*>(&rhs);
  InternalCopyConstructValue(unqualifiedValue);

  // (Sanity check)
  Assert(Is<T>());
}
template <typename T>
void Variant::Assign(MoveReference<T> rhs)
{
  // Unqualify type
  typedef typename Decay<T>::Type UnqualifiedType;

  // (Verify type is copy constructible and destructible, else fail to compile)
  StaticAssert(VariantTypeRequiresCopyConstructorAndDestructor2,
               has_copy_constructor<UnqualifiedType>::value && has_destructor<UnqualifiedType>::value,
               "Types assigned to variant must have both an accessible copy constructor and destructor.");

  // Destroy current stored value, if any
  // (Does not free memory or forget the current stored type)
  InternalDestructStoredValue();

  // Replace current stored type, if any, with this new stored type
  // (Prepares internal buffers for the new stored value)
  NativeType* newStoredType = NativeTypeOf(T);
  InternalReplaceStoredType(newStoredType);

  // Move construct the new stored value
  UnqualifiedType* unqualifiedValue = static_cast<UnqualifiedType*>(&rhs);
  InternalMoveConstructValue(unqualifiedValue);

  // (Sanity check)
  Assert(Is<T>());
}

//
// Primitive Member Access (Arithmetic Types Only)
//

template <typename T, typename UnqualifiedType,
            TF_ENABLE_IF_DEF(IsBasicNativeTypeArithmetic<UnqualifiedType>::Value),
            typename PrimitiveType>
PrimitiveType& Variant::GetPrimitiveMemberOrError(size_t index) const
{
  // Get stored value's primitive member at the specified index
  if(PrimitiveType* primitiveValue = GetPrimitiveMemberOrNull<T>(index)) // Successful?
    return *primitiveValue;

  Error("%s%s", String::Format("Unable to get primitive member on stored value of specified type '%s' from Variant - ", NativeTypeOf(T)->mDebugTypeName).c_str(),
                (mNativeType == nullptr ? "Variant is empty" : String::Format("Variant's stored value is type '%s'", mNativeType->mDebugTypeName).c_str()));
  return GetInvalidObject<PrimitiveType>();
}

template <typename T, typename UnqualifiedType,
            TF_ENABLE_IF_DEF(IsBasicNativeTypeArithmetic<UnqualifiedType>::Value),
            typename PrimitiveType>
PrimitiveType& Variant::GetPrimitiveMemberOrDefault(size_t index, const PrimitiveType& defaultValue) const
{
  // Get stored value's primitive member at the specified index
  if(PrimitiveType* primitiveValue = GetPrimitiveMemberOrNull<T>(index)) // Successful?
    return *primitiveValue;

  return (PrimitiveType&)defaultValue;
}

template <typename T, typename UnqualifiedType,
            TF_ENABLE_IF_DEF(IsBasicNativeTypeArithmetic<UnqualifiedType>::Value),
            typename PrimitiveType>
PrimitiveType* Variant::GetPrimitiveMemberOrNull(size_t index) const
{
  // Primitive member info
  static const size_t PrimitiveCount = BasicNativeTypePrimitiveMembers<Decay<T>::Type>::Count;

  // (Specified index should not be out of bounds)
  ErrorIf(index >= PrimitiveCount, "Specified index is out of bounds while attempting to access a primitive member on variant's stored value");

  // Variant is not storing a value of type T?
  if(!Is<T>())
    return nullptr;

  // Get a pointer to stored value's primitive member at the specified index
  PrimitiveType* primitiveValues = reinterpret_cast<PrimitiveType*>(GetData());
  return (primitiveValues + index);
}

//
// Variant ToString / ToValue
//

inline String ToString(const Variant& value, bool shortFormat)
{
  // Convert stored value to string
  return value.ToString(shortFormat);
}

template <typename T>
inline void ToValue(StringRange range, Variant& value)
{
  // Parse string as specified T type
  return value.ToValue<T>(range);
}
inline void ToValue(StringRange range, Variant& value, NativeType* nativeType)
{
  // Parse string as specified native type
  return value.ToValue(range, nativeType);
}
inline void ToValue(StringRange range, Variant& value)
{
  // Parse string as variant's stored type
  return value.ToValue(range);
}

} // namespace Zero
