///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Returns the basic native type corresponding to the specified zilch type (may be null), else nullptr
NativeType* ZilchTypeToBasicNativeType(Zilch::Type* zilchType);

/// Returns the zilch type corresponding to the specified basic native type (may be null), else nullptr
Zilch::Type* BasicNativeTypeToZilchType(NativeType* nativeType);
Zilch::Type* BasicNativeTypeToZilchType(NativeTypeId nativeTypeId);

/// Returns a variant containing the stored value of the specified any if the any's stored value is a basic native type, else Variant()
Variant ConvertBasicAnyToVariant(const Any& anyValue);
/// Returns an any containing the stored value of the specified variant if the variant's stored value is a basic native type, else Any()
Any ConvertBasicVariantToAny(const Variant& variantValue);

} // namespace Zero
