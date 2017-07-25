///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

WorldTransformation::WorldTransformation()
{
  mWorldOffset.ZeroOut();
  //set values to something that won't explode later
  mBodyToWorld.SetIdentity();
  mWorldScale = Vec3(1, 1, 1);
  mWorldRotation.SetIdentity();
  mWorldTranslation.ZeroOut();

  mLocalScale = Vec3(1, 1, 1);
  mLocalRotation.SetIdentity();
  mLocalTranslation.ZeroOut();

  mOldRotation.ZeroOut();
  mOldTranslation.ZeroOut();

  mTransform = nullptr;
}

void WorldTransformation::SetTransform(Transform* transform)
{
  mTransform = transform;
}

void WorldTransformation::ReadTransform(PhysicsNode* owner)
{
  ErrorIf(!mTransform, "No transform object ever set on the physics world transform.");

  PhysicsNode* parent = owner->mParent;
  RigidBody* body = owner->mBody;
  Collider* collider = owner->mCollider;

  // Read the raw transform values
  if(mTransform->GetInWorld())
  {
    mLocalScale = mTransform->GetWorldScale();
    mLocalRotation = Math::ToMatrix3(mTransform->GetWorldRotation().Normalized());
    mLocalTranslation = mTransform->GetWorldTranslation();
  }
  else
  {
    mLocalScale = mTransform->GetLocalScale();
    mLocalRotation = Math::ToMatrix3(mTransform->GetLocalRotation().Normalized());
    mLocalTranslation = mTransform->GetLocalTranslation();
  }

  //if we have a collider, make sure to transform
  //the translation offset into world space
  if(collider)
    mWorldOffset = TransformNormal(collider->mTranslationOffset);
}
    
void WorldTransformation::ComputeTransformation(WorldTransformation* parent, 
                                                PhysicsNode* owner)
{
  //if we are marked as in world, we simply just build
  //up our world and body to world values from our local
  if(mTransform->GetInWorld())
  {
    mWorldScale = mLocalScale;
    mWorldRotation = mLocalRotation;  
    mWorldTranslation = mLocalTranslation;
    mBodyToWorld = Math::BuildTransform(mWorldTranslation, mWorldRotation, mWorldScale);
    if(owner->mCollider)
      mWorldOffset = TransformNormal(owner->mCollider->mTranslationOffset);
    return;
  }

  //otherwise, we have to build ourself up from our parent's body to world.
  //however, we may not have a parent node or we may have cog's between us
  //and our parent node. Therefore we have to build up the transform from
  //us to our nearest parentNode (or until the root if we have no parentNode).
  //Our parent node will already have it's body to world cached so we can early out there

  Mat4 toWorldTransform = Math::BuildTransform(mLocalTranslation,mLocalRotation,mLocalScale);

  //get the cog of the parent node (if we have a parent node)
  Cog* parentNodeCog = nullptr;
  if(parent)
    parentNodeCog = parent->mTransform->GetOwner();

  //loop over all of the in between nodes (or just up to the root if parent was null)
  Cog* parentCog = owner->GetTransform()->mTransform->GetOwner()->GetParent();
  while(parentCog != parentNodeCog)
  {
    Transform* parentT = parentCog->has(Transform);
    parentCog = parentCog->GetParent();

    if(parentT == nullptr)
      continue;

    Mat4 parentLocalMatrix = parentT->GetLocalMatrix();
    toWorldTransform = parentLocalMatrix * toWorldTransform;
  }

  //if we had a parent node, we have to do the final
  //step of applying it's body to world transform)
  if(parent)
    toWorldTransform = parent->mBodyToWorld * toWorldTransform;

  //now just build our body to world, decompose it to the world values and also
  //build the offset value
  mBodyToWorld = toWorldTransform;
  mBodyToWorld.Decompose(&mWorldScale, &mWorldRotation, &mWorldTranslation);
  if(owner->mCollider)
    mWorldOffset = TransformNormal(owner->mCollider->mTranslationOffset);
}

Vec3 WorldTransformation::GetWorldScale() const
{
  return mWorldScale;
}

Mat3 WorldTransformation::GetWorldRotation() const
{
  return mWorldRotation;
}

Vec3 WorldTransformation::GetWorldTranslation() const
{
  return mWorldTranslation + mWorldOffset;
}

Vec3 WorldTransformation::GetPublishedTranslation() const
{
  return mWorldTranslation;
}

Vec3 WorldTransformation::GetLocalScale() const
{
  return mLocalScale;
}

Mat3 WorldTransformation::GetLocalRotation() const
{
  return mLocalRotation;
}

Vec3 WorldTransformation::GetLocalTranslation() const
{
  return mLocalTranslation;
}

Mat4 WorldTransformation::GetWorldMatrix() const
{
  Mat4 matrix;
  matrix.BuildTransform(mWorldTranslation + mWorldOffset, mWorldRotation, mWorldScale);
  return matrix;
}

void WorldTransformation::SetScale(Vec3Param scale)
{
  mLocalScale = scale;
  mWorldScale = scale;
}

void WorldTransformation::SetRotation(Mat3Param rotation)
{
  mLocalRotation = rotation;
  mWorldRotation = rotation;
}

void WorldTransformation::SetTranslation(Vec3Param translation)
{
  mLocalTranslation = translation - mWorldOffset;
  mWorldTranslation = translation - mWorldOffset;
}

Vec3 WorldTransformation::TransformPoint(Vec3Param point) const
{
  Vec3 result = point * mWorldScale;
  result = Math::Transform(mWorldRotation, result);
  return result + mWorldTranslation + mWorldOffset;
}

Vec3 WorldTransformation::TransformNormal(Vec3Param normal) const
{
  Vec3 result = normal * mWorldScale;
  return Math::Transform(mWorldRotation, result);
}

Vec3 WorldTransformation::TransformSurfaceNormal(Vec3Param direction) const
{
  Vec3 result = Math::Transform(mWorldRotation, direction);
  return result / mWorldScale;
}

Vec3 WorldTransformation::InverseTransformPoint(Vec3Param point) const
{
  Vec3 result = point - mWorldTranslation - mWorldOffset;
  result = Math::TransposedTransform(mWorldRotation, result);
  return result / mWorldScale;
}

Vec3 WorldTransformation::InverseTransformNormal(Vec3Param normal) const
{
  Vec3 result = Math::TransposedTransform(mWorldRotation, normal);
  return result / mWorldScale;
}

Vec3 WorldTransformation::InverseTransformSurfaceNormal(Vec3Param direction) const
{
  Vec3 result = Math::TransposedTransform(mWorldRotation, direction);
  return result * mWorldScale;
}

void WorldTransformation::ComputeOldValues()
{
  mOldTranslation = mWorldTranslation + mWorldOffset;
  mOldRotation = mWorldRotation;
}

Vec3 WorldTransformation::GetOldTranslation()
{
  return mOldTranslation;
}

Mat3 WorldTransformation::GetOldRotation()
{
  return mOldRotation;
}

bool WorldTransformation::IsUniformlyScaled()
{
  return mWorldScale.x == mWorldScale.y && mWorldScale.y == mWorldScale.z;
}

}//namespace Zero
