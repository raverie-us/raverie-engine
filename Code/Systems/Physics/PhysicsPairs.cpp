// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

namespace Physics
{

ColliderPair::ColliderPair()
{
  Top = Bot = nullptr;
}

ColliderPair::ColliderPair(Collider* a, Collider* b)
{
  Top = a;
  Bot = b;

  if (Top->mId < Bot->mId)
    Math::Swap(Top, Bot);
}

Vec3 ColliderPair::GetPointSeperatingVelocity(Vec3Param point) const
{
  return Top->ComputePointVelocityInternal(point) - Bot->ComputePointVelocityInternal(point);
}

real ColliderPair::GetMixedRestitution() const
{
  PhysicsMaterial* topMaterial = Top->GetMaterial();
  PhysicsMaterial* botMaterial = Bot->GetMaterial();
  if (topMaterial->mRestitutionImportance > botMaterial->mRestitutionImportance)
    return topMaterial->mRestitution;
  if (botMaterial->mRestitutionImportance > topMaterial->mRestitutionImportance)
    return botMaterial->mRestitution;
  return Math::Max(topMaterial->mRestitution, botMaterial->mRestitution);
}

real ColliderPair::GetMixedFriction() const
{
  PhysicsMaterial* topMaterial = Top->GetMaterial();
  PhysicsMaterial* botMaterial = Bot->GetMaterial();
  if (topMaterial->mFrictionImportance > botMaterial->mFrictionImportance)
    return topMaterial->mDynamicFriction;
  if (botMaterial->mFrictionImportance > topMaterial->mFrictionImportance)
    return botMaterial->mDynamicFriction;
  return Math::Sqrt(topMaterial->mDynamicFriction * botMaterial->mDynamicFriction);
}

u64 ColliderPair::GetId() const
{
  return GetLexicographicId(Top->mId, Bot->mId);
}

Collider* ColliderPair::operator[](uint index) const
{
  return mObjects[index];
}

Collider*& ColliderPair::operator[](uint index)
{
  return mObjects[index];
}

bool ColliderPair::operator>(const ColliderPair& rhs) const
{
  return GetId() > rhs.GetId();
}

} // namespace Physics

} // namespace Raverie
