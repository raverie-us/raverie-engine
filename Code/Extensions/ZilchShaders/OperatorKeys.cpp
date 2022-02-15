// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

TypeCastKey::TypeCastKey()
{
  mPair.first = nullptr;
  mPair.second = nullptr;
}

TypeCastKey::TypeCastKey(Zilch::Type* fromType, Zilch::Type* toType)
{
  mPair.first = fromType;
  mPair.second = toType;
}

size_t TypeCastKey::Hash() const
{
  return mPair.Hash();
}

bool TypeCastKey::operator==(const TypeCastKey& rhs) const
{
  return mPair == rhs.mPair;
}

UnaryOperatorKey::UnaryOperatorKey()
{
  mPair.first = nullptr;
  mPair.second = (int)Zilch::Grammar::Invalid;
}

UnaryOperatorKey::UnaryOperatorKey(Zilch::Type* type, Zilch::Grammar::Enum op)
{
  mPair.first = type;
  mPair.second = (int)op;
}

size_t UnaryOperatorKey::Hash() const
{
  return mPair.Hash();
}

bool UnaryOperatorKey::operator==(const UnaryOperatorKey& rhs) const
{
  return mPair == rhs.mPair;
}

BinaryOperatorKey::BinaryOperatorKey()
{
  mPair.first.first = nullptr;
  mPair.first.second = nullptr;
  mPair.second = (int)Zilch::Grammar::Invalid;
}

BinaryOperatorKey::BinaryOperatorKey(Zilch::Type* lhsType, Zilch::Type* rhsType, Zilch::Grammar::Enum op)
{
  mPair.first.first = lhsType;
  mPair.first.second = rhsType;
  mPair.second = (int)op;
}

size_t BinaryOperatorKey::Hash() const
{
  return mPair.Hash();
}

bool BinaryOperatorKey::operator==(const BinaryOperatorKey& rhs) const
{
  return mPair == rhs.mPair;
}

} // namespace Zero
