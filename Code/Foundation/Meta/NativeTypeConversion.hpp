// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

/// Returns the basic native type corresponding to the specified raverie type (may
/// be null), else nullptr
NativeType* RaverieTypeToBasicNativeType(Raverie::Type* raverieType);

/// Returns the raverie type corresponding to the specified basic native type (may
/// be null), else nullptr
Raverie::Type* BasicNativeTypeToRaverieType(NativeType* nativeType);
Raverie::Type* BasicNativeTypeToRaverieType(NativeTypeId nativeTypeId);

/// Returns a variant containing the stored value of the specified any if the
/// any's stored value is a basic native type, else Variant()
Variant ConvertBasicAnyToVariant(const Any& anyValue);
/// Returns an any containing the stored value of the specified variant if the
/// variant's stored value is a basic native type, else Any()
Any ConvertBasicVariantToAny(const Variant& variantValue);

} // namespace Raverie
