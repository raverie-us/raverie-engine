///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2014-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(GenericPhysicsMesh, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterProperty(Vertices);
  ZilchBindGetterProperty(Indices);
  ZilchBindMethod(Validate);
  ZilchBindMethod(UpdateAndNotifyIfModified);
}

GenericPhysicsMesh::GenericPhysicsMesh()
{
  mModified = false;
  mIsValid = true;

  mLocalAabb.Zero();
  mLocalCenterOfMass.ZeroOut();
  mLocalVolume = real(0.0);

  // The bound mesh data is tied 1-to-1 with this mesh so just create and set
  // them up once (they're handle types so when we die so do they)
  mVertexData.mOwner = this;
  mVertexData.mBoundArray = &mVertices;
  mIndexData.mOwner = this;
  mIndexData.mBoundArray = &mIndices;
}

void GenericPhysicsMesh::Save(StringParam filename)
{
  SaveToDataFile(*this, filename, DataFileFormat::Binary);
}

void GenericPhysicsMesh::Serialize(Serializer& stream)
{
  SerializeName(mVertices);
  SerializeName(mIndices);
}

void GenericPhysicsMesh::Initialize()
{
  ForceRebuild();
}

void GenericPhysicsMesh::Unload()
{
  mVertices.Deallocate();
  mIndices.Deallocate();
  mInfoMap.Deallocate();
}

void GenericPhysicsMesh::ResourceModified()
{
  // If we're already modified we don't need to do any extra logic
  if(GetModified())
    return;

  mModified = true;
  OnResourceModified();
}

void GenericPhysicsMesh::Upload(const Vec3Array& points, const IndexArray& indices)
{
  mVertices = points;
  mIndices = indices;
  ResourceModified();
  UpdateAndNotifyIfModified();
}

void GenericPhysicsMesh::ForceRebuild()
{
  // @JoshD: This needs to be updated later to deal with zero volume
  // (requires re-factoring how scaled values are currently computed)
  if(GetValid() == false || mVertices.Empty())
  {
    mLocalVolume = 0;
    mLocalCenterOfMass = Vec3::cZero;
    mLocalAabb.SetInvalid();
    return;
  }

  ComputeLocalVolume();
  ComputeLocalCenterOfMass();
  mLocalAabb.Compute(mVertices);

  RebuildMidPhase();
  GenerateInternalEdgeData();
}

void GenericPhysicsMesh::GenerateInternalEdgeData()
{
  GenerateInternalEdgeInfo(this, &mInfoMap);
}

PhysicsMeshVertexData* GenericPhysicsMesh::GetVertices()
{
  return &mVertexData;
}

PhysicsMeshIndexData* GenericPhysicsMesh::GetIndices()
{
  return &mIndexData;
}

uint GenericPhysicsMesh::GetTriangleCount()
{
  return mIndices.Size() / 3;
}

Triangle GenericPhysicsMesh::GetTriangle(uint index)
{
  return GetTriangleFromIndexBufferIndex(index * 3);
}

bool GenericPhysicsMesh::GetModified()
{
  return mModified;
}

bool GenericPhysicsMesh::GetValid()
{
  return mIsValid;
}

bool GenericPhysicsMesh::Validate(bool throwExceptionIfInvalid)
{
  mIsValid = true;
  // If we find an index outside the range of our vertices then we're invalid
  for(size_t i = 0; i < mIndices.Size(); ++i)
  {
    uint index = mIndices[i];
    if(index >= mVertices.Size())
    {
      mIsValid = false;
      break;
    }
  }

  if(!mIsValid && throwExceptionIfInvalid)
  {
    DoNotifyException("Invalid Mesh", "Physics Mesh contains invalid indices");
    mLocalAabb.SetInvalid();
  }

  return mIsValid;
}

void GenericPhysicsMesh::UpdateAndNotifyIfModified()
{
  if(!GetModified())
    return;

  mModified = false;
  Validate(false);
  ForceRebuild();

  // As we have just changed values like center of mass and aabb, we need to notify any collider
  // who used this mesh that we've changed so they can update world-space values.
  ResourceEvent toSend;
  toSend.EventResource = this;
  DispatchEvent(Events::ResourceModified, &toSend);
}

void GenericPhysicsMesh::DrawEdges(Mat4Param transform, ByteColor color)
{
  uint triCount = GetTriangleCount();
  for(uint i = 0; i < triCount; ++i)
  {
    Triangle tri = GetTriangle(i);
    tri.p0 = Math::TransformPoint(transform, tri.p0);
    tri.p1 = Math::TransformPoint(transform, tri.p1);
    tri.p2 = Math::TransformPoint(transform, tri.p2);

    gDebugDraw->Add(Debug::Line(tri.p0, tri.p1).Color(color));
    gDebugDraw->Add(Debug::Line(tri.p1, tri.p2).Color(color));
    gDebugDraw->Add(Debug::Line(tri.p2, tri.p0).Color(color));
  }
}

void GenericPhysicsMesh::DrawFaces(Mat4Param transform, ByteColor color)
{
  uint triCount = GetTriangleCount();
  for(uint i = 0; i < triCount; ++i)
  {
    Triangle tri = GetTriangle(i);
    tri.p0 = Math::TransformPoint(transform, tri.p0);
    tri.p1 = Math::TransformPoint(transform, tri.p1);
    tri.p2 = Math::TransformPoint(transform, tri.p2);

    ByteColor alphaColor = color;
    SetAlphaByte(alphaColor, 50);
    gDebugDraw->Add(Debug::Triangle(tri).Color(alphaColor));
  }
}

void GenericPhysicsMesh::DrawFaceNormals(Mat4Param transform, ByteColor color)
{
  uint triCount = GetTriangleCount();
  for(uint i = 0; i < triCount; ++i)
  {
    Triangle tri = GetTriangle(i);
    Triangle worldTri = tri.Transform(transform);
    Vec3 normal = worldTri.GetNormal();
    Vec3 center = worldTri.GetCenter();

    // Use a percentage of the area of the triangle as the length of the normal
    real triArea = worldTri.GetArea();
    static real cAreaScalar = real(1.3f);
    triArea *= cAreaScalar;
    triArea = Math::Min(triArea, 0.5f);

    gDebugDraw->Add(Debug::Line(center, center + normal * triArea).Color(color).HeadSize(triArea * real(0.25f)));
  }
}

bool GenericPhysicsMesh::CastRayTriangle(const Ray& localRay, const Triangle& tri, int triIndex, ProxyResult& result, BaseCastFilter& filter)
{
  // Check the ray for intersection with the triangle
  Intersection::IntersectionPoint pointData;
  Intersection::IntersectionType tResult;
  tResult = Intersection::RayTriangle(localRay.Start, localRay.Direction, tri[0], tri[1], tri[2], &pointData);
  // If there is a collision (clean up intersection library's wonkyness later)
  if(tResult >= (Intersection::Type)0)
  {
    // If this collision is after our current best result then skip this triangle
    real distance = pointData.T;
    if(distance > result.mDistance)
      return false;

    // Copy over the new best result (closest result)
    result.mPoints[0] = pointData.Points[0];
    result.mPoints[1] = pointData.Points[1];
    result.mDistance = distance;
    result.ShapeIndex = triIndex;

    // If the filter is set to retrieve the normal of the surface
    if(filter.IsSet(BaseCastFilterFlags::GetContactNormal))
    {
      Vec3 normal = Geometry::NormalFromPointOnTriangle(result.mPoints[0], tri[0], tri[1], tri[2]);

      // The normal returned should always be positive in the y (local space), but if the ray
      // was cast from below the triangle, we want to negate it.
      if(Dot(normal, tri[0] - localRay.Start) > 0)
        normal *= real(-1.0f);

      result.mContactNormal = normal;
    }
    return true;
  }
  return false;
}

bool GenericPhysicsMesh::CastRayGeneric(const Ray& localRay, ProxyResult& result, BaseCastFilter& filter)
{
  bool triangleHit = false;
  result.mTime = Math::PositiveMax();

  // Check all triangles for collision
  size_t triangleCount = GetTriangleCount();
  for(size_t triIndex = 0; triIndex < triangleCount; ++triIndex)
  {
    Triangle tri = GetTriangle(triIndex);
    triangleHit |= CastRayTriangle(localRay, tri, triIndex, result, filter);
  }
  return triangleHit;
}

void GenericPhysicsMesh::Support(const Vec3Array points, Vec3Param localDirection, Vec3Ptr support) const
{
  real longestDistance = -Math::PositiveMax();
  // Initialize the support point to a large, unlikely to be hit value in case there are no points
  // (the aabb will currently be around this point and gjk will treat this like a single point collision)
  *support = Vec3(Math::PositiveMax() * 0.5f);

  for(size_t i = 0; i < points.Size(); ++i)
  {
    Vec3Param curr = points[i];
    real dist = Math::Dot(localDirection, curr);
    if(dist > longestDistance)
    {
      longestDistance = dist;
      *support = curr;
    }
  }
}

void GenericPhysicsMesh::Support(Vec3Param localDirection, Vec3Ptr support) const
{
  Support(mVertices, localDirection, support);
}

Triangle GenericPhysicsMesh::GetTriangleFromIndexBufferIndex(uint index)
{
  Vec3 p0 = mVertices[mIndices[index]];
  Vec3 p1 = mVertices[mIndices[index + 1]];
  Vec3 p2 = mVertices[mIndices[index + 2]];
  return Triangle(p0, p1, p2);
}

TriangleInfoMap* GenericPhysicsMesh::GetInfoMap()
{
  return &mInfoMap;
}

void GenericPhysicsMesh::ComputeLocalVolume()
{
  if(mVertices.Empty() || mIndices.Empty() || !GetValid())
    return;

  uint triCount = GetTriangleCount();
  Vec3* verts = mVertices.Begin();
  uint* indices = mIndices.Begin();
  mLocalVolume = Geometry::CalculateTriMeshVolume(verts, indices, triCount);
}

void GenericPhysicsMesh::ComputeLocalCenterOfMass()
{
  if(mVertices.Empty() || mIndices.Empty() || !GetValid())
    return;

  uint triCount = GetTriangleCount();
  Vec3* verts = mVertices.Begin();
  uint* indices = mIndices.Begin();
  mLocalCenterOfMass = Geometry::CalculateTriMeshCenterOfMass(verts, indices, triCount);
}

Vec3 GenericPhysicsMesh::ComputeScaledCenterOfMass(Vec3Param worldScale)
{
  return mLocalCenterOfMass * worldScale;
}

real GenericPhysicsMesh::ComputeScaledVolume(Vec3Param worldScale)
{
  real scalar = worldScale.x * worldScale.y * worldScale.z;
  return mLocalVolume * scalar;
}

Mat3 GenericPhysicsMesh::ComputeScaledInvInertiaTensor(Vec3Param worldScale, real worldMass)
{
  if(mVertices.Empty() || mIndices.Empty() || !GetValid())
    return Mat3::cIdentity;

  uint triCount = GetTriangleCount();
  const uint* triIndices = (uint*)(&mIndices.Front());

  Mat3 inertiaTensor;
  Vec3* verts = mVertices.Begin();
  uint* indices = mIndices.Begin();
  Vec3 scaledCenterOfMass = mLocalCenterOfMass * worldScale;
  Geometry::CalculateTriMeshInertiaTensor(verts, indices, triCount, scaledCenterOfMass, &inertiaTensor, worldScale);

  inertiaTensor *= worldMass;
  return Math::Inverted(inertiaTensor);
}

void GenericPhysicsMesh::CopyTo(GenericPhysicsMesh* destination)
{
  destination->mVertices = mVertices;
  destination->mIndices = mIndices;
  destination->mInfoMap = mInfoMap;

  destination->mLocalAabb = mLocalAabb;
  destination->mLocalCenterOfMass = mLocalCenterOfMass;
  destination->mLocalVolume = mLocalVolume;
}

const Vec3Array& GenericPhysicsMesh::GetVertexArray() const
{
  return mVertices;
}

const IndexArray& GenericPhysicsMesh::GetIndexArray() const
{
  return mIndices;
}

}//namespace Zero
