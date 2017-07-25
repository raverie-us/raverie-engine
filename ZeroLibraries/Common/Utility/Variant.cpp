///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//---------------------------------------------------------------------------------//
//                                  Variant                                        //
//---------------------------------------------------------------------------------//

Variant::Variant()
  : mNativeType(nullptr)
{
  InternalZeroLocalBuffer();
}

Variant::Variant(NativeType* nativeType)
  : mNativeType(nullptr)
{
  InternalZeroLocalBuffer();
  DefaultConstruct(nativeType);
}

Variant::Variant(NativeType* nativeType, const void* data)
  : mNativeType(nullptr)
{
  InternalZeroLocalBuffer();
  Assign(nativeType, data);
}

Variant::Variant(const Variant& rhs)
  : mNativeType(nullptr)
{
  InternalZeroLocalBuffer();
  Assign(rhs);
}
Variant::Variant(MoveReference<Variant> rhs)
  : mNativeType(nullptr)
{
  InternalZeroLocalBuffer();
  Assign(ZeroMove(rhs));
}

Variant::~Variant()
{
  Clear();
}

//
// Member Operators
//

Variant& Variant::operator=(const Variant& rhs)
{
  Assign(rhs);
  return *this;
}
Variant& Variant::operator=(MoveReference<Variant> rhs)
{
  Assign(ZeroMove(rhs));
  return *this;
}

bool Variant::operator==(const Variant& rhs) const
{
  // Variants are storing different types?
  if(mNativeType != rhs.mNativeType)
    return false;
  // Variants are storing the same types?
  else
  {
    // Both variants are empty? (rhs empty check is made redundant by this + previous if check)
    if(mNativeType == nullptr/* && rhs.mNativeType == nullptr*/)
      return true;
    // Both variants are non-empty?
    else
      return InternalEqualToValue(rhs.InternalGetData());
  }
}

bool Variant::operator!=(const Variant& rhs) const
{
  return !(*this == rhs);
}

//
// Stored Value Access
//

bool Variant::Is(NativeType* nativeType) const
{
  return (mNativeType == nativeType);
}

void* Variant::GetData() const
{
  // Variant is empty?
  if(mNativeType == nullptr)
    return nullptr;

  return InternalGetData();
}

size_t Variant::Hash() const
{
  // Variant is empty?
  if(mNativeType == nullptr)
    return 0;

  return InternalHashStoredValue();
}

String Variant::ToString(bool shortFormat) const
{
  // Variant is empty?
  if(mNativeType == nullptr)
    return String();

  return InternalStoredValueToString(shortFormat);
}

void Variant::ToValue(StringRange range, NativeType* nativeType)
{
  DefaultConstruct(nativeType);
  ToValue(range);
}
void Variant::ToValue(StringRange range)
{
  // Variant is empty?
  if(mNativeType == nullptr)
    return;

  return InternalStringToStoredValue(range);
}

bool Variant::IsEmpty() const
{
  return (mNativeType == nullptr);
}
bool Variant::IsNotEmpty() const
{
  return !IsEmpty();
}

bool Variant::IsZeroed() const
{
  return BufferIsZeroed(mData);
}
bool Variant::IsNotZeroed() const
{
  return !IsZeroed();
}

bool Variant::IsSmallType() const
{
  return (mNativeType && IsSmallType(mNativeType));
}
bool Variant::IsLargeType() const
{
  return (mNativeType && IsLargeType(mNativeType));
}

bool Variant::IsSmallType(NativeType* nativeType)
{
  return (nativeType->mTypeSize <= sizeof(Variant::mData));
}
bool Variant::IsLargeType(NativeType* nativeType)
{
  return !IsSmallType(nativeType);
}

NativeType* Variant::GetNativeType() const
{
  return mNativeType;
}
NativeTypeId Variant::GetNativeTypeId() const
{
  // Variant is empty?
  if(mNativeType == nullptr)
    return cInvalidNativeTypeId;

  return mNativeType->mTypeId;
}

//
// Stored Value Management
//

void Variant::DefaultConstruct(NativeType* nativeType)
{
  // New stored type is null?
  if(nativeType == nullptr)
  {
    // Clear this variant
    Clear();
    return;
  }

  // Type is not both copy constructible and destructible?
  // (Unfortunately we must do this check at runtime as we don't have the static C++ type here)
  if(nativeType->mCopyConstructObjectFn == nullptr
  || nativeType->mDestructObjectFn      == nullptr)
  {
    Error(String::Format("Unable to assign type '%s' to variant. - "
                         "Types assigned to variant must have both an accessible copy constructor and destructor.", nativeType->mDebugTypeName).c_str());

    // Clear this variant
    Clear();
    return;
  }

  // Destroy current stored value, if any
  // (Does not free memory or forget the current stored type)
  InternalDestructStoredValue();

  // Replace current stored type, if any, with this new stored type
  // (Prepares internal buffers for the new stored value)
  NativeType* newStoredType = nativeType;
  InternalReplaceStoredType(newStoredType);

  // Default construct the new stored value
  InternalDefaultConstructValue();

  // (Sanity check)
  Assert(GetNativeType() == newStoredType);
}

void Variant::Assign(NativeType* nativeType, const void* data)
{
  // New stored type is null?
  if(nativeType == nullptr)
  {
    // Clear this variant
    Clear();
    return;
  }

  // Type is not both copy constructible and destructible?
  // (Unfortunately we must do this check at runtime as we don't have the static C++ type here)
  if(nativeType->mCopyConstructObjectFn == nullptr
  || nativeType->mDestructObjectFn      == nullptr)
  {
    Error(String::Format("Unable to assign type '%s' to variant. - "
                         "Types assigned to variant must have both an accessible copy constructor and destructor.", nativeType->mDebugTypeName).c_str());

    // Clear this variant
    Clear();
    return;
  }

  // Destroy current stored value, if any
  // (Does not free memory or forget the current stored type)
  InternalDestructStoredValue();

  // Replace current stored type, if any, with this new stored type
  // (Prepares internal buffers for the new stored value)
  NativeType* newStoredType = nativeType;
  InternalReplaceStoredType(newStoredType);

  // Copy construct the new stored value
  InternalCopyConstructValue(data);

  // (Sanity check)
  Assert(GetNativeType() == newStoredType);
}

void Variant::Assign(const Variant& rhs)
{
  // Their variant is empty?
  if(rhs.IsEmpty())
  {
    // Clear this variant
    Clear();
    return;
  }

  // Destroy current stored value, if any
  // (Does not free memory or forget the current stored type)
  InternalDestructStoredValue();

  // Replace current stored type, if any, with this new stored type
  // (Prepares internal buffers for the new stored value)
  NativeType* newStoredType = rhs.GetNativeType();
  InternalReplaceStoredType(newStoredType);

  // Copy construct the new stored value
  const void* unqualifiedValue = rhs.InternalGetData();
  InternalCopyConstructValue(unqualifiedValue);

  // (Sanity check)
  Assert(GetNativeType() == newStoredType);
}
void Variant::Assign(MoveReference<Variant> rhs)
{
  // Their variant is empty?
  if(rhs->IsEmpty())
  {
    // Clear both variants
    Clear();
    rhs->Clear();
    return;
  }

  // Their variant is storing a large type?
  if(rhs->IsLargeType())
  {
    // Clear this variant
    Clear();

    // Steal their stored value instead of move constructing a new one
    mNativeType = rhs->mNativeType;
    mDataAsPointer  = rhs->mDataAsPointer;

    // "Clear" their variant (we stole their data, they're no longer managing it)
    rhs->mNativeType = nullptr;
    rhs->mDataAsPointer  = nullptr;
  }
  // Their variant is storing a small type?
  else
  {
    // Destroy current stored value, if any
    // (Does not free memory or forget the current stored type)
    InternalDestructStoredValue();

    // Replace current stored type, if any, with this new stored type
    // (Prepares internal buffers for the new stored value)
    NativeType* newStoredType = rhs->GetNativeType();
    InternalReplaceStoredType(newStoredType);

    // Move construct the new stored value
    void* unqualifiedValue = rhs->InternalGetData();
    InternalMoveConstructValue(unqualifiedValue);

    // Clear their variant (so we properly adhere to expected move semantics)
    rhs->Clear();

    // (Sanity check)
    Assert(GetNativeType() == newStoredType);
  }

  // (Their variant should be completely empty and zeroed)
  Assert(rhs->IsEmpty() && rhs->IsZeroed());
}

void Variant::Clear()
{
  // Destroy current stored value, if any
  // (Does not free memory or forget the current stored type)
  InternalDestructStoredValue();

  // Variant is storing a large type?
  if(IsLargeType())
  {
    // Free the heap buffer we allocated to fit our large stored value
    InternalFreeHeapBuffer();
  }

  // Clear stored type
  mNativeType = nullptr;

  // Zero local buffer
  InternalZeroLocalBuffer();

  // (Sanity check)
  Assert(IsEmpty() && IsZeroed());
}

//
// Internal Helper Functions
//

void* Variant::InternalGetData() const
{
  // (Variant should have a stored type)
  Assert(mNativeType != nullptr);

  // Small stored type?
  if(IsSmallType(mNativeType))
  {
    // Value is stored in local buffer
    return const_cast<byte*>(mData);
  }
  // Large stored type?
  else
  {
    // (Heap buffer should be allocated)
    Assert(mDataAsPointer != nullptr);

    // Value is stored in heap buffer
    return mDataAsPointer;
  }
}

void Variant::InternalZeroLocalBuffer()
{
  // Zero local buffer
  memset(mData, 0, sizeof(mData));

  // (Local buffer should now be zeroed)
  Assert(IsZeroed());
}

void Variant::InternalZeroHeapBuffer()
{
  // (Variant should have a large stored type)
  Assert(IsLargeType());

  // (Variant should have already allocated a heap buffer)
  Assert(mDataAsPointer != nullptr);

  // Zero heap buffer
  memset(mDataAsPointer, 0, mNativeType->mTypeSize);
}

void Variant::InternalAllocateHeapBuffer()
{
  // (Variant should have a large stored type)
  Assert(IsLargeType());

  // (Variant should not have already allocated a heap buffer)
  Assert(mDataAsPointer == nullptr);

  // Allocate heap buffer large enough to fit the stored type
  mDataAsPointer = new byte [mNativeType->mTypeSize];
}

void Variant::InternalFreeHeapBuffer()
{
  // (Variant should have a large stored type)
  Assert(IsLargeType());

  // (Variant should have already allocated a heap buffer)
  Assert(mDataAsPointer != nullptr);

  // Free heap buffer
  delete [] mDataAsPointer;
  mDataAsPointer = nullptr;

  // (Local buffer should now be zeroed, albeit incidentally)
  Assert(IsZeroed());
}

void Variant::InternalDestructStoredValue()
{
  // Variant is empty?
  if(mNativeType == nullptr)
    return;

  // Destruct stored value
  mNativeType->mDestructObjectFn(InternalGetData());
}

void Variant::InternalReplaceStoredType(NativeType* newStoredType)
{
  Assert(newStoredType != nullptr);

  // We have a heap buffer and can reuse it for the new stored type?
  bool canReuseHeapBuffer = false;

  // Current stored type is empty or small? (Uses local buffer?)
  if(mNativeType == nullptr || IsSmallType(mNativeType))
  {
    // Zero local buffer
    InternalZeroLocalBuffer();
  }
  // Current stored type is large? (Uses heap buffer?)
  else
  {
    Assert(IsLargeType(mNativeType));

    // New stored type coincidentally has the exact same size?
    if(mNativeType->mTypeSize == newStoredType->mTypeSize)
    {
      Assert(IsLargeType(newStoredType));

      // Zero heap buffer
      InternalZeroHeapBuffer();

      // We can reuse our heap buffer for the new stored type
      // (Since the types are the same size. This may be a coincidence or they may even be the same type.)
      canReuseHeapBuffer = true;
    }
    // New stored type has a different size? (New stored type is either small or a different-sized large type)
    else
    {
      // Free heap buffer
      InternalFreeHeapBuffer();
    }
  }

  // Set new stored type
  mNativeType = newStoredType;

  // New stored type is large
  if(IsLargeType(newStoredType) && !canReuseHeapBuffer)
  {
    // Allocate heap buffer
    InternalAllocateHeapBuffer();

    // Zero heap buffer
    InternalZeroHeapBuffer();
  }
}

void Variant::InternalDefaultConstructValue()
{
  // Stored type is missing a default constructor?
  if(mNativeType->mDefaultConstructObjectFn == nullptr)
  {
#if VARIANT_ERROR_ON_MISSING_DEFAULT_CONSTRUCTOR
    Error(String::Format("Unable to perform default construction for variant stored type '%s'. Will be zero-initialized instead. - "
                          "An accessible default constructor was not defined and could not be generated for the given type. "
                          "(Note: Zero-initialized is likely not a valid state for complex types! This can result in undefined behavior!)", mNativeType->mDebugTypeName).c_str());
#endif

    // Fallback behavior
    // (The previous step, InternalReplaceStoredType, already zeroed the stored value's buffer
    // so there's nothing to do here. The stored value is already zero-initialized)
    return;
  }

  // Default construct into the stored value buffer
  mNativeType->mDefaultConstructObjectFn(InternalGetData());
}

void Variant::InternalCopyConstructValue(const void* source)
{
  // Copy construct source value into the stored value buffer
  mNativeType->mCopyConstructObjectFn(source, InternalGetData());
}

void Variant::InternalMoveConstructValue(void* source)
{
  // Stored type is missing a move constructor?
  if(mNativeType->mMoveConstructObjectFn == nullptr)
  {
#if VARIANT_ERROR_ON_MISSING_MOVE_CONSTRUCTOR
    Error(String::Format("Unable to perform move construction for variant stored type '%s'. Will perform copy construction instead. - "
                          "An accessible move constructor was not defined and could not be generated for the given type.", mNativeType->mDebugTypeName).c_str());
#endif

    // Fallback behavior
    InternalCopyConstructValue(source);
    return;
  }

  // Move construct source value into the stored value buffer
  mNativeType->mMoveConstructObjectFn(source, InternalGetData());
}

bool Variant::InternalEqualToValue(const void* rhs) const
{
  // Stored type is missing a valid compare policy?
  if(mNativeType->mEqualToObjectFn == nullptr)
  {
#if VARIANT_ERROR_ON_MISSING_COMPARE_POLICY
    Error(String::Format("Unable to perform equality comparison for variant stored type '%s'. Will return false instead. - "
                          "A valid ComparePolicy was not defined and could not be generated for the given type "
                          "(Adding an operator== function to the given type or defining a valid ComparePolicy for the given type should solve this problem).", mNativeType->mDebugTypeName).c_str());
#endif

    // Fallback behavior
    return false;
  }

  // Perform equality comparison between stored value and rhs
  return mNativeType->mEqualToObjectFn(InternalGetData(), rhs);
}

size_t Variant::InternalHashStoredValue() const
{
  // Stored type is missing a valid hash policy?
  if(mNativeType->mHashObjectFn == nullptr)
  {
#if VARIANT_ERROR_ON_MISSING_HASH_POLICY
    Error(String::Format("Unable to perform hash operation for variant stored type '%s'. Will return 0 instead. - "
                          "A valid HashPolicy was not defined and could not be generated for the given type "
                          "(Adding a Hash function to the given type or defining a valid HashPolicy for the given type should solve this problem).", mNativeType->mDebugTypeName).c_str());
#endif

    // Fallback behavior
    return 0;
  }

  // Perform hash operation on stored value
  return mNativeType->mHashObjectFn(InternalGetData());
}

String Variant::InternalStoredValueToString(bool shortFormat) const
{
  // Stored type is missing a global to string function?
  if(mNativeType->mObjectToStringFn == nullptr)
  {
#if VARIANT_ERROR_ON_MISSING_TO_STRING_FUNCTION
    Error(String::Format("Unable to perform object to string conversion for variant stored type '%s'. Will return String() instead. - "
                          "A global ToString function was not defined and could not be generated for the given type.", mNativeType->mDebugTypeName).c_str());
#endif

    // Fallback behavior
    return String();
  }

  // Convert value to string
  return mNativeType->mObjectToStringFn(InternalGetData(), shortFormat);
}

void Variant::InternalStringToStoredValue(StringRange range)
{
  // Stored type is missing a global to value function?
  if(mNativeType->mStringToObjectFn == nullptr)
  {
#if VARIANT_ERROR_ON_MISSING_TO_VALUE_FUNCTION
    Error(String::Format("Unable to perform string to object conversion for variant stored type '%s'. Will clear the variant instead. - "
                          "A global ToValue function was not defined and could not be generated for the given type.", mNativeType->mDebugTypeName).c_str());
#endif

    // Fallback behavior
    Clear();
    return;
  }

  // Convert string to value
  return mNativeType->mStringToObjectFn(range, InternalGetData());
}

} // namespace Zero
