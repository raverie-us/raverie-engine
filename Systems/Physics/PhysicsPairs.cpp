///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
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

  if(Top->mId < Bot->mId)
    Math::Swap(Top,Bot);
}

Vec3 ColliderPair::GetPointSeperatingVelocity(Vec3Param point) const
{
  return Top->ComputePointVelocityInternal(point) - Bot->ComputePointVelocityInternal(point);
}

real ColliderPair::GetMixedRestiution() const
{
  PhysicsMaterial* topMaterial = Top->GetMaterial();
  PhysicsMaterial* botMaterial = Bot->GetMaterial();
  if(topMaterial->mHighPriority && !botMaterial->mHighPriority)
    return topMaterial->mRestitution;
  if(botMaterial->mHighPriority && !topMaterial->mHighPriority)
    return botMaterial->mRestitution;
  return Math::Max(topMaterial->mRestitution, botMaterial->mRestitution);
}

real ColliderPair::GetMixedFriction() const
{
  return Math::Sqrt(Top->GetMaterial()->mDynamicFriction * Bot->GetMaterial()->mDynamicFriction);
}

real ColliderPair::GetMinRestitution() const
{
  return Math::Min(Top->GetMaterial()->mRestitution, Bot->GetMaterial()->mRestitution);
}

real ColliderPair::GetMinStaticFriction() const
{
  return Math::Min(Top->GetMaterial()->mStaticFriction, Bot->GetMaterial()->mStaticFriction);
}

real ColliderPair::GetMinDynamicFriction() const
{
  return Math::Min(Top->GetMaterial()->mDynamicFriction, Bot->GetMaterial()->mDynamicFriction);
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

}//namespace Physics

}//namespace Zero
