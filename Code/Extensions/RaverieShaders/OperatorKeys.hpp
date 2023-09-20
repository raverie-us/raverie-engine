// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

/// Represents a unique hashable identifier for a type-cast. Currently
/// type-casts are not function calls in raverie so we can't use a function
/// pointer to resolve them.
struct TypeCastKey
{
  TypeCastKey();
  TypeCastKey(Raverie::Type* fromType, Raverie::Type* toType);

  size_t Hash() const;
  bool operator==(const TypeCastKey& rhs) const;

  Pair<Raverie::Type*, Raverie::Type*> mPair;
};

/// Represents a unique hashable identifier for unary operators. Currently
/// operators are not function calls in raverie so we can't use a function pointer
/// to resolve them.
struct UnaryOperatorKey
{
  UnaryOperatorKey();
  UnaryOperatorKey(Raverie::Type* type, Raverie::Grammar::Enum op);

  size_t Hash() const;
  bool operator==(const UnaryOperatorKey& rhs) const;

  Pair<Raverie::Type*, int> mPair;
};

/// Represents a unique hashable identifier for binary operators. Currently
/// operators are not function calls in raverie so we can't use a function pointer
/// to resolve them.
struct BinaryOperatorKey
{
  BinaryOperatorKey();
  BinaryOperatorKey(Raverie::Type* type1, Raverie::Type* type2, Raverie::Grammar::Enum op);

  size_t Hash() const;
  bool operator==(const BinaryOperatorKey& rhs) const;

  typedef Pair<Raverie::Type*, Raverie::Type*> RaverieTypePair;
  Pair<RaverieTypePair, int> mPair;
};

} // namespace Raverie
