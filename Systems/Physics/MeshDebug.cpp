///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(MeshDebug, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZilchBindMethod(DrawTriangles);
  ZilchBindMethod(DrawTriangleVoronoiRegion);
  ZilchBindMethod(DrawAllTriangleVoronoiRegions);
  ZilchBindMethod(GetTriangleIndexFromCast);
  ZilchBindMethod(GetTriangleNormal);
  ZilchBindMethod(GetPointFromCastOnTriangle);
  ZilchBindMethod(DrawTriangleFromCast);
  ZilchBindMethod(TestManifoldPoint);
  ZilchBindMethod(DrawVectorAtPoint);
  ZilchBindMethod(GenerateInfoForTriangles);
}

void MeshDebug::CacheData()
{
  mMesh = nullptr;
  mHeightMap = nullptr;

  ConvexMeshCollider* cMesh = GetOwner()->has(ConvexMeshCollider);
  if(cMesh)
    mMesh = cMesh->GetConvexMesh();
  MeshCollider* pMesh = GetOwner()->has(MeshCollider);
  if(pMesh)
    mMesh = pMesh->GetPhysicsMesh();

  if (!mMesh)
  {
    mHeightMap = GetOwner()->has(HeightMapCollider);
  }
}

void MeshDebug::Initialize(CogInitializer& initializer)
{
  CacheData();
}

void MeshDebug::DrawTriangles()
{
  CacheData();

  if (!mMesh) return;

  Transform* t = GetOwner()->has(Transform);
  Mat4 mat = t->GetWorldMatrix();

  ByteColor color = Color::LightSteelBlue;
  mMesh->DrawEdges(mat,color);

  SetAlphaByte(color, 80);
  mMesh->DrawFaces(mat,color);
}

void MeshDebug::DrawVoronoiRegion(Vec3Param p0, Vec3Param p1, real angle, Vec3Param triNormal, Mat4Param transform)
{
  if(angle < Math::cTwoPi)
  {
    Vec3 edge = p0 - p1;
    Vec3 axis = edge.Normalized();
    Mat3 rot = Math::ToMatrix3(axis,angle);
    Vec3 rotNormal = Math::Transform(rot,triNormal);

    Vec3 midPoint = p1 + axis * edge.Length() * real(.5);

    midPoint = Math::TransformPoint(transform,midPoint);
    rotNormal = Math::TransformNormal(transform,rotNormal).Normalized() * real(.25);
    gDebugDraw->Add(Debug::Line(midPoint,midPoint + rotNormal).Color(Color::Red));
  }
}

void MeshDebug::DrawTriangleVoronoiRegion(uint triIndex)
{
  CacheData();

  Transform* t = GetOwner()->has(Transform);
  Mat4 mat = t->GetWorldMatrix();

  Triangle tri;
  TriangleInfoMap* infoMap = nullptr;

  if (mMesh)
  {
    tri = mMesh->GetTriangle(triIndex);
    infoMap = mMesh->GetInfoMap();
  }
  else if (mHeightMap)
  {
    tri = mHeightMap->GetTriangle(triIndex);
    infoMap = mHeightMap->GetInfoMap();
  }

  if (!infoMap) return;

  MeshTriangleInfo* info = infoMap->FindPointer(triIndex);
  if(info == nullptr)
    return;

  Vec3 normal = tri.GetNormal();
  DrawVoronoiRegion(tri[0],tri[1],info->mEdgeV0V1Angle,normal,mat);
  DrawVoronoiRegion(tri[2],tri[0],info->mEdgeV2V0Angle,normal,mat);
  DrawVoronoiRegion(tri[1],tri[2],info->mEdgeV1V2Angle,normal,mat);
}

void MeshDebug::DrawAllTriangleVoronoiRegions()
{
  CacheData();

  if (!mMesh) return;

  uint triCount = mMesh->GetTriangleCount();

  for(uint i = 0; i < triCount; ++i)
    DrawTriangleVoronoiRegion(i);
}

void MeshDebug::DrawVectorAtPoint(Vec3Param start, Vec3Param dir, Vec3Param vec)
{
  uint triIndex = GetTriangleIndexFromCast(start,dir);
  Vec3 contactPoint = GetPointFromCastOnTriangle(triIndex,start,dir);

  gDebugDraw->Add(Debug::Sphere(contactPoint,real(.01)).Color(Color::Green));
  gDebugDraw->Add(Debug::Line(contactPoint,contactPoint + vec).Color(Color::Blue));
}

void MeshDebug::TestManifoldPoint(Vec3Param start, Vec3Param dir, Vec3Param contactNormal)
{
  uint triIndex = GetTriangleIndexFromCast(start,dir);
  Vec3 contactPoint = GetPointFromCastOnTriangle(triIndex,start,dir);

  Transform* t = GetOwner()->has(Transform);
  Mat4 mat = t->GetWorldMatrix();

  Vec3 cPoint = Math::TransformPoint(mat.Inverted(),contactPoint);
  Physics::ManifoldPoint point,oldPoint;
  point.BodyPoints[0] = point.BodyPoints[1] = cPoint;
  point.Normal = contactNormal;
  point.Penetration = real(0.0);
  oldPoint = point;

  CorrectInternalEdgeNormal(point,0,GetOwner()->has(Collider),triIndex);

  ByteColor lineColor = Color::Blue;
  ByteColor pointColor = Color::Green;
  Vec3 p = Math::TransformPoint(mat,cPoint);
  Vec3 n = contactNormal.Normalized();
  if(oldPoint.Normal != point.Normal)
  {
    lineColor = Color::AliceBlue;
    pointColor = Color::Lime;

    gDebugDraw->Add(Debug::Sphere(p,real(.01)).Color(Color::Green));
    gDebugDraw->Add(Debug::Line(p,p + n).Color(Color::Blue));
  }

  // p = Math::TransformPoint(mat,point.BodyPoints[0]);
  n = point.Normal;

  gDebugDraw->Add(Debug::Sphere(p,real(.01)).Color(pointColor));
  gDebugDraw->Add(Debug::Line(p,p + n).Color(lineColor));
}

uint MeshDebug::GetTriangleIndexFromCast(Vec3Param start, Vec3Param dir)
{
  PhysicsSpace* space = GetSpace()->has(PhysicsSpace);
  Physics::CollisionManager* cManager = space->GetCollisionManager();
  Collider* collider = GetOwner()->has(Collider);
  if(collider == nullptr)
    return(uint)-1;

  CastData castData(start,dir);
  ProxyResult result;
  CastFilter filter;
  bool didHit = cManager->TestRayVsObject(collider,castData,result,filter);
  if(!didHit)
    return (uint)-1;

  return result.ShapeIndex;
}

Vec3 MeshDebug::GetTriangleNormal(uint index)
{
  if (mMesh) return mMesh->GetTriangle(index).GetNormal();
  else       return mHeightMap->GetTriangle(index).GetNormal();
}

Vec3 MeshDebug::GetPointFromCastOnTriangle(uint index, Vec3Param start, Vec3Param dir)
{
  CacheData();

  Triangle tri;
  if (mMesh) tri = mMesh->GetTriangle(index);
  else       tri = mHeightMap->GetTriangle(index);

  PhysicsSpace* space = GetSpace()->has(PhysicsSpace);
  Physics::CollisionManager* cManager = space->GetCollisionManager();
  Collider* collider = GetOwner()->has(Collider);
  if(collider == nullptr)
    return Vec3::cZero;

  CastData castData(start,dir);
  ProxyResult result;
  CastFilter filter;
  bool didHit = cManager->TestRayVsObject(collider,castData,result,filter);
  if(!didHit)
    return Vec3::cZero;

  return result.mPoints[0];
}

void MeshDebug::DrawTriangleFromCast(Vec3Param start, Vec3Param dir)
{
  CacheData();

  uint index = GetTriangleIndexFromCast(start,dir);
  if(index == (uint)-1)
    return;

  Transform* t = GetOwner()->has(Transform);
  Mat4 mat = t->GetWorldMatrix();

  Triangle tri;
  if (mMesh) tri = mMesh->GetTriangle(index);
  else       tri = mHeightMap->GetTriangle(index);

  ByteColor color = Color::LightSteelBlue;
  tri.p0 = Math::TransformPoint(mat, tri.p0);
  tri.p1 = Math::TransformPoint(mat, tri.p1);
  tri.p2 = Math::TransformPoint(mat, tri.p2);
  gDebugDraw->Add(Debug::Triangle(tri).Color(color));

  DrawTriangleVoronoiRegion(index);
}

void MeshDebug::GenerateInfoForTriangles(Vec3Param start1, Vec3Param dir1, Vec3Param start2, Vec3Param dir2)
{
  CacheData();

  uint index1 = GetTriangleIndexFromCast(start1,dir1);
  uint index2 = GetTriangleIndexFromCast(start2,dir2);
  if(index1 == (uint)-1 || index2 == (uint)-1)
    return;

  if(index1 == index2)
    return;

  if (mMesh)
  {
    Triangle triA = mMesh->GetTriangle(index1);
    Triangle triB = mMesh->GetTriangle(index2);
    ComputeEdgeInfoForTriangleA(triA,index1,triB,mMesh->GetInfoMap());
  }
  else if (mHeightMap)
  {
    Triangle triA = mHeightMap->GetTriangle(index1);
    Triangle triB = mHeightMap->GetTriangle(index2);
    ComputeEdgeInfoForTriangleA(triA,index1,triB,mHeightMap->GetInfoMap());
  }
}

}//namespace Zero
