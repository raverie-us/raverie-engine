///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(MeshCollider, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);

  ZeroBindInterface(Collider);
  ZeroBindDocumented();

  ZilchBindGetterSetterProperty(PhysicsMesh);
  ZilchBindFieldProperty(mDrawEdges);
  ZilchBindFieldProperty(mDrawFaces);
  ZilchBindFieldProperty(mDrawFaceNormals);
}

MeshCollider::MeshCollider()
{
  mType = cMesh;
}

void MeshCollider::Serialize(Serializer& stream)
{
  Collider::Serialize(stream);
  SerializeResourceName(mPhysicsMesh, PhysicsMeshManager);
  SerializeNameDefault(mDrawEdges, true);
  SerializeNameDefault(mDrawFaces, false);
  SerializeNameDefault(mDrawFaceNormals, false);
}

void MeshCollider::Initialize(CogInitializer& initializer)
{
  Collider::Initialize(initializer);

  // Listen for this resource being modified
  PhysicsMesh* mesh = mPhysicsMesh;
  ConnectThisTo(mesh, Events::ResourceModified, OnMeshModified);
}

void MeshCollider::DebugDraw()
{
  Collider::DebugDraw();

  // Draw the mesh in world-space
  Mat4 worldTransform = GetWorldTransform()->GetWorldMatrix();
  
  ByteColor color = Color::LightSteelBlue;
  SetAlphaByte(color, 80);

  if(mDrawEdges)
    mPhysicsMesh->DrawEdges(worldTransform, color);
  if(mDrawFaceNormals)
    mPhysicsMesh->DrawFaceNormals(worldTransform, Color::White);
  if(mDrawFaces)
    mPhysicsMesh->DrawFaces(worldTransform, color);
}

void MeshCollider::ComputeWorldAabbInternal()
{
  Vec3 supportPoints[6];
  Support(Vec3::cXAxis, &supportPoints[0]);
  Support(-Vec3::cXAxis, &supportPoints[1]);
  Support(Vec3::cYAxis, &supportPoints[2]);
  Support(-Vec3::cYAxis, &supportPoints[3]);
  Support(Vec3::cZAxis, &supportPoints[4]);
  Support(-Vec3::cZAxis, &supportPoints[5]);
  mAabb.Compute(supportPoints, 6);
}

real MeshCollider::ComputeWorldVolumeInternal()
{
  Vec3 scale = GetWorldScale();
  real worldVolume = mPhysicsMesh->ComputeScaledVolume(scale);
  return worldVolume;
}

void MeshCollider::ComputeLocalInverseInertiaTensor(real mass, Mat3Ref localInvInertia)
{
  Vec3 worldScale = GetWorldScale();
  localInvInertia = mPhysicsMesh->ComputeScaledInvInertiaTensor(worldScale, mass);
}

void MeshCollider::Support(Vec3Param direction, Vec3Ptr support) const
{
  // Bring the support direction into local space (normalize for safety),
  // call the local-space support function then transform the result back into world space
  Vec3 localSpaceDir = TransformSupportDirectionToLocal(direction);
  localSpaceDir.Normalize();
  mPhysicsMesh->Support(localSpaceDir, support);
  *support = TransformSupportPointToWorld(*support);
}

void MeshCollider::RebuildModifiedResources()
{
  Collider::RebuildModifiedResources();
  mPhysicsMesh->UpdateAndNotifyIfModified();
}

PhysicsMesh* MeshCollider::GetPhysicsMesh()
{
  return mPhysicsMesh;
}

void MeshCollider::SetPhysicsMesh(PhysicsMesh* physicsMesh)
{
  if(physicsMesh == nullptr)
    return;

  // Disconnect from events on the old mesh and connect on the new mesh (if they're different)
  PhysicsMesh* oldMesh = mPhysicsMesh;
  if(oldMesh != physicsMesh)
  {
    if(oldMesh != nullptr)
      DisconnectAll(oldMesh, this);
    if(physicsMesh != nullptr)
      ConnectThisTo(physicsMesh, Events::ResourceModified, OnMeshModified);
  }

  mPhysicsMesh = physicsMesh;
  InternalSizeChanged();
}

void MeshCollider::OnMeshModified(Event* e)
{
  InternalSizeChanged();
}

MeshCollider::RangeType MeshCollider::GetOverlapRange(Aabb& localAabb)
{
  PhysicsMesh* mesh = GetPhysicsMesh();
  RangeType range;
  mesh->GetOverlappingTriangles(localAabb, range.mTriangles, range.mIndices);
  range.Initialize();
  return range;
}

bool MeshCollider::Cast(const Ray& localRay, ProxyResult& result, BaseCastFilter& filter)
{
  PhysicsMesh* mesh = GetPhysicsMesh();
  return mesh->CastRay(localRay, result, filter);
}

}//namespace Zero
