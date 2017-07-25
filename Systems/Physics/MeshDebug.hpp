///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class GenericPhysicsMesh;
class HeightMapCollider;

/// A helper class to test internal edges in script.
/// Shouldn't really be committed but I need this for further testing.
class MeshDebug : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  void CacheData();

  void Initialize(CogInitializer& initializer) override;
  void DrawTriangles();
  void DrawVoronoiRegion(Vec3Param p0, Vec3Param p1, real angle, Vec3Param triNormal, Mat4Param transform);
  void DrawTriangleVoronoiRegion(uint triIndex);
  void DrawVectorAtPoint(Vec3Param start, Vec3Param dir, Vec3Param vec);
  void DrawAllTriangleVoronoiRegions();
  void TestManifoldPoint(Vec3Param start, Vec3Param dir, Vec3Param contactNormal);

  uint GetTriangleIndexFromCast(Vec3Param start, Vec3Param dir);
  Vec3 GetTriangleNormal(uint index);
  Vec3 GetPointFromCastOnTriangle(uint index, Vec3Param start, Vec3Param dir);
  void DrawTriangleFromCast(Vec3Param start, Vec3Param dir);
  void GenerateInfoForTriangles(Vec3Param start1, Vec3Param dir1, Vec3Param start2, Vec3Param dir2);

  GenericPhysicsMesh* mMesh;
  HeightMapCollider* mHeightMap;
};

}//namespace Zero
