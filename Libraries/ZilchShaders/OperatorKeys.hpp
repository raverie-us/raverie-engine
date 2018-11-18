///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//-------------------------------------------------------------------TypeCastKey
/// Represents a unique hashable identifier for a type-cast. Currently type-casts
/// are not function calls in zilch so we can't use a function pointer to resolve them.
struct TypeCastKey
{
  TypeCastKey();
  TypeCastKey(Zilch::Type* fromType, Zilch::Type* toType);

  size_t Hash() const;
  bool operator==(const TypeCastKey& rhs) const;

  Pair<Zilch::Type*, Zilch::Type*> mPair;
};

//-------------------------------------------------------------------UnaryOperatorKey
/// Represents a unique hashable identifier for unary operators. Currently operators
/// are not function calls in zilch so we can't use a function pointer to resolve them.
struct UnaryOperatorKey
{
  UnaryOperatorKey();
  UnaryOperatorKey(Zilch::Type* type, Zilch::Grammar::Enum op);

  size_t Hash() const;
  bool operator==(const UnaryOperatorKey& rhs) const;

  Pair<Zilch::Type*, int> mPair;
};

//-------------------------------------------------------------------BinaryOperatorKey
/// Represents a unique hashable identifier for binary operators. Currently operators
/// are not function calls in zilch so we can't use a function pointer to resolve them.
struct BinaryOperatorKey
{
  BinaryOperatorKey();
  BinaryOperatorKey(Zilch::Type* type1, Zilch::Type* type2, Zilch::Grammar::Enum op);

  size_t Hash() const;
  bool operator==(const BinaryOperatorKey& rhs) const;

  typedef Pair<Zilch::Type*, Zilch::Type*> ZilchTypePair;
  Pair<ZilchTypePair, int> mPair;
};

}//public Zero
