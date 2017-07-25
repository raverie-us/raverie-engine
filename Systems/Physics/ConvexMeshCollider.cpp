///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(ConvexMeshCollider, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindInterface(Collider);
  ZeroBindDocumented();

  ZilchBindGetterSetterProperty(ConvexMesh);
}

ConvexMeshCollider::ConvexMeshCollider(void)
{
  mType = cConvexMesh;
}

void ConvexMeshCollider::Serialize(Serializer& stream)
{
  Collider::Serialize(stream);
  SerializeResourceName(mConvexMesh, ConvexMeshManager);
}

void ConvexMeshCollider::Initialize(CogInitializer& initalizer)
{
  Collider::Initialize(initalizer);

  // Listen for this resource being modified
  ConvexMesh* mesh = mConvexMesh;
  ConnectThisTo(mesh, Events::ResourceModified, OnMeshModified);
}

void ConvexMeshCollider::DebugDraw()
{
  Collider::DebugDraw();

  // Draw the mesh in world-space
  Mat4 worldTransform = GetWorldTransform()->GetWorldMatrix();
  mConvexMesh->Draw(worldTransform);

  // Mostly for debugging, draw the center of mass in world-space
  Vec3 center;
  GetCenter(center);
  gDebugDraw->Add(Debug::Sphere(center, real(.01f)));
}

void ConvexMeshCollider::ComputeWorldAabbInternal()
{
  // Compute a more tight-fitting aabb if the vertex count is low enough?
  //static const uint vertexThreshold = 9999;
  //if(mMesh->GetVertexCount() <= vertexThreshold)
  {
    Vec3 supportPoints[6];
    Support(Vec3::cXAxis, &supportPoints[0]);
    Support(-Vec3::cXAxis, &supportPoints[1]);
    Support(Vec3::cYAxis, &supportPoints[2]);
    Support(-Vec3::cYAxis, &supportPoints[3]);
    Support(Vec3::cZAxis, &supportPoints[4]);
    Support(-Vec3::cZAxis, &supportPoints[5]);
    mAabb.Compute(supportPoints, 6);
    return;
  }

  // Proper aabb of transformed aabb code. Maybe use later?
  //Vec3 scale = GetWorldScale();
  //Mat3 rotation = GetWorldRotation();
  //Vec3 translation = GetWorldTranslation();
  //mAabb = mMesh->GetAabb();
  //mAabb = mAabb.TransformAabb(scale, rotation, translation);
}

real ConvexMeshCollider::ComputeWorldVolumeInternal()
{
  Vec3 worldScale = GetWorldScale();
  return mConvexMesh->ComputeScaledVolume(worldScale);
}

void ConvexMeshCollider::ComputeLocalInverseInertiaTensor(real mass, Mat3Ref localInvInertia)
{
  Vec3 worldScale = GetWorldScale();
  localInvInertia = mConvexMesh->ComputeScaledInvInertiaTensor(worldScale, mass);
}

void ConvexMeshCollider::RebuildModifiedResources()
{
  Collider::RebuildModifiedResources();
  mConvexMesh->UpdateAndNotifyIfModified();
}

Vec3 ConvexMeshCollider::GetColliderLocalCenterOfMass(void) const
{
  // We currently need true world-space as the caller of this will transform to world space.
  // To do this we must return the local space center of mass.
  // @JoshD: Refactor later to remove this?
  return mConvexMesh->mLocalCenterOfMass;
}

void ConvexMeshCollider::Support(Vec3Param direction, Vec3Ptr support) const
{
  // Bring the support direction into local space (normalize for safety),
  // call the local-space support function then transform the result back into world space
  Vec3 localSpaceDir = TransformSupportDirectionToLocal(direction);
  localSpaceDir.Normalize();
  mConvexMesh->Support(localSpaceDir, support);
  *support = TransformSupportPointToWorld(*support);
}

void ConvexMeshCollider::GetCenter(Vec3Ref center) const
{
  Vec3 localCenterOfMass = mConvexMesh->mLocalCenterOfMass;
  center = GetWorldTransform()->TransformPoint(localCenterOfMass);
}

ConvexMesh* ConvexMeshCollider::GetConvexMesh()
{
  return mConvexMesh;
}

void ConvexMeshCollider::SetConvexMesh(ConvexMesh* convexMesh)
{
  if(convexMesh == nullptr)
    return;

  // Disconnect from events on the old mesh and connect on the new mesh (if they're different)
  ConvexMesh* oldMesh = mConvexMesh;
  if(oldMesh != convexMesh)
  {
    if(oldMesh != nullptr)
      DisconnectAll(oldMesh, this);
    if(convexMesh != nullptr)
      ConnectThisTo(convexMesh, Events::ResourceModified, OnMeshModified);
  }

  mConvexMesh = convexMesh;
  OnMeshModified(nullptr);
}

void ConvexMeshCollider::OnMeshModified(Event* e)
{
  InternalSizeChanged();
}

ConvexMeshCollider::RangeType ConvexMeshCollider::GetOverlapRange(Aabb& localAabb)
{
  // Return a range containing the mesh info (vertices and indices)
  ConvexMesh* mesh = GetConvexMesh();
  return RangeType(&mesh->GetVertexArray(), &mesh->GetIndexArray(), localAabb);
}

}//namespace Zero
