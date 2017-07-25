///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2014-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------MultiConvexMeshRange::ConvexMeshObject
void MultiConvexMeshRange::ConvexMeshObject::Support(Vec3Param direction, Vec3Ptr support) const
{
  MultiConvexMesh* mesh = mCollider->mMesh;
  SubConvexMesh* subMesh = mesh->mMeshes[Index];

  // Bring the support direction into local space (normalize for safety),
  // call the local-space support function then transform the result back into world space
  Vec3 localSpaceDir = mCollider->TransformSupportDirectionToLocal(direction);
  localSpaceDir.Normalize();
  subMesh->Support(mesh->mVertices, localSpaceDir, support);
  *support = mCollider->TransformSupportPointToWorld(*support);
}

void MultiConvexMeshRange::ConvexMeshObject::GetCenter(Vec3Ref worldCenter) const
{
  // Get the local-space center of the current sub-mesh
  SubConvexMesh* subMesh = mCollider->mMesh->mMeshes[Index];
  Vec3 localCenter = subMesh->GetCenter();

  // Transform the center to world-space
  WorldTransformation* transform = mCollider->GetWorldTransform();
  worldCenter = transform->TransformPoint(localCenter);
}

//-------------------------------------------------------------------MultiConvexMeshRange
MultiConvexMeshRange::MultiConvexMeshRange(MultiConvexMeshCollider* collider, const Aabb& worldAabb)
{
  mIndex = 0;
  mCollider = collider;

  // Bring the aabb to local space
  WorldTransformation* transform = mCollider->GetWorldTransform();
  Mat4 invWorldTransform = transform->GetWorldMatrix().Inverted();
  mLocalAabb = worldAabb.TransformAabb(invWorldTransform);

  // Find the first sub-mesh that is hitting the local space aabb
  // (we may be an empty range after this but that's fine)
  SkipDead();
}

MultiConvexMeshRange::ConvexMeshObject& MultiConvexMeshRange::Front()
{
  // Load the sub shape
  obj.mCollider = mCollider;
  obj.Index = mIndex;
  obj.Shape = Intersection::MakeSupport(&obj, false);
  obj.Shape.mWorldAabb = mCollider->mAabb;

  return obj;
}

void MultiConvexMeshRange::PopFront()
{
  // Start search for the next possible sub-mesh
  ++mIndex;
  SkipDead();
}

bool MultiConvexMeshRange::Empty()
{
  return mIndex >= mCollider->mMesh->mMeshes.Size();
}

void MultiConvexMeshRange::SkipDead()
{
  // Find the next shape index that is hit by the local space query aabb
  MultiConvexMesh* mesh = mCollider->mMesh;
  while(!Empty())
  {
    SubConvexMesh* subMesh = mesh->mMeshes[mIndex];
    if(subMesh->mAabb.Overlap(mLocalAabb) == true)
      return;

    ++mIndex;
  }
}

//-------------------------------------------------------------------MultiConvexMeshCollider
ZilchDefineType(MultiConvexMeshCollider, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindInterface(Collider);
  ZeroBindDocumented();

  ZilchBindGetterSetterProperty(Mesh);
}

MultiConvexMeshCollider::MultiConvexMeshCollider()
{
  mType = cMultiConvexMesh;
}

void MultiConvexMeshCollider::Serialize(Serializer& stream)
{
  Collider::Serialize(stream);

  SerializeResourceName(mMesh, MultiConvexMeshManager);
}

void MultiConvexMeshCollider::Initialize(CogInitializer& initalizer)
{
  Collider::Initialize(initalizer);

  // Listen for this resource being modified
  MultiConvexMesh* mesh = mMesh;
  ConnectThisTo(mesh, Events::ResourceModified, OnMeshModified);
}

void MultiConvexMeshCollider::DebugDraw()
{
  Collider::DebugDraw();

  MultiConvexMesh* mesh = GetMesh();
  if(mesh == nullptr)
    return;

  Mat4 worldMat = GetWorldTransform()->GetWorldMatrix();
  mesh->Draw(worldMat);
}

void MultiConvexMeshCollider::ComputeWorldAabbInternal()
{
  mAabb.Zero();
  MultiConvexMesh* mesh = mMesh;
  if(mesh->mMeshes.Size() == 0)
    return;

  // Transform the mesh's aabb to world space
  Vec3 scale = GetWorldScale();
  Mat3 rotation = GetWorldRotation();
  Vec3 translation = GetWorldTranslation();
  mAabb = mesh->mAabb.TransformAabb(scale, rotation, translation);
}

real MultiConvexMeshCollider::ComputeWorldVolumeInternal()
{
  Vec3 worldScale = GetWorldScale();
  real worldVolume = mMesh->GetWorldVolume(worldScale);
  return worldVolume;
}

void MultiConvexMeshCollider::ComputeLocalInverseInertiaTensor(real mass, Mat3Ref localInvInertia)
{
  Vec3 worldScale = GetWorldScale();
  localInvInertia = mMesh->ComputeInvInertiaTensor(worldScale, mass);
}

Vec3 MultiConvexMeshCollider::GetColliderLocalCenterOfMass() const
{
  return mMesh->GetCenterOfMass();
}

void MultiConvexMeshCollider::RebuildModifiedResources()
{
  Collider::RebuildModifiedResources();
  mMesh->UpdateAndNotifyIfModified();
}

MultiConvexMesh* MultiConvexMeshCollider::GetMesh()
{
  return mMesh;
}

void MultiConvexMeshCollider::SetMesh(MultiConvexMesh* mesh)
{
  if(mesh == nullptr)
    return;

  // Disconnect from events on the old mesh and connect on the new mesh (if they're different)
  MultiConvexMesh* oldMesh = mMesh;
  if(oldMesh != mesh)
  {
    if(oldMesh != nullptr)
      DisconnectAll(oldMesh, this);
    if(mesh != nullptr)
      ConnectThisTo(mesh, Events::ResourceModified, OnMeshModified);
  }

  mMesh = mesh;
  OnMeshModified(nullptr);
}

void MultiConvexMeshCollider::OnMeshModified(Event* e)
{
  InternalSizeChanged();
}

MultiConvexMeshCollider::RangeType MultiConvexMeshCollider::GetOverlapRange(Aabb& worldAabb)
{
  return RangeType(this, worldAabb);
}

bool MultiConvexMeshCollider::Cast(const Ray& worldRay, ProxyResult& result, BaseCastFilter& filter)
{
  WorldTransformation* transform = GetWorldTransform();
  Ray localRay = worldRay.TransformInverse(transform->GetWorldMatrix());

  bool meshWasHit = mMesh->CastRay(localRay, result, filter);
  // If the mesh was hit then transform the local space data into world-space
  //(do this only once instead of per hit triangle)
  if(meshWasHit)
  {
    result.mPoints[0] = transform->TransformPoint(result.mPoints[0]);
    result.mPoints[1] = transform->TransformPoint(result.mPoints[1]);
    Vec3 worldNormal = transform->TransformSurfaceNormal(result.mContactNormal);
    result.mContactNormal = worldNormal.AttemptNormalized();
  }

  return meshWasHit;
}

}//namespace Zero
