///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------HeightMapDebugDrawer
ZilchDefineType(HeightMapDebugDrawer, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDependency(HeightMap);

  ZilchBindFieldProperty(mDrawTriangles)->ZeroSerialize(true);
  ZilchBindFieldProperty(mDrawOffset);
}

HeightMapDebugDrawer::HeightMapDebugDrawer()
{
  mMap = nullptr;
  mTransform = nullptr;
  mDrawOffset = real(0.0);
}

void HeightMapDebugDrawer::Serialize(Serializer& stream)
{
  SerializeNameDefault(mDrawTriangles, true);
  SerializeNameDefault(mDrawOffset, real(0.0));
}

void HeightMapDebugDrawer::Initialize(CogInitializer& initializer)
{
  mMap = GetOwner()->has(HeightMap);
  mTransform = GetOwner()->has(Transform);
}

void HeightMapDebugDrawer::DebugDraw()
{
  if(!mDrawTriangles)
    return;

  Array<Vec3> vertices;
  Array<uint> indices;
  Mat4 worldMat = mTransform->GetWorldMatrix();

  auto range = mMap->GetAllPatches();
  for(; !range.Empty(); range.PopFront())
  {
    vertices.Clear();
    indices.Clear();

    HeightPatch* patch = range.Front();
    if(patch == nullptr)
      continue;

    mMap->GetHeightPatchVertices(patch, vertices);
    mMap->GenerateIndices(indices, 0);

    for(uint i = 0; i < indices.Size(); i += 3)
    {
      Triangle tri;
      tri.p0 = vertices[indices[i + 0]];
      tri.p1 = vertices[indices[i + 1]];
      tri.p2 = vertices[indices[i + 2]];
      if(!tri.p0.Valid() || !tri.p1.Valid() || !tri.p2.Valid())
        continue;

      tri = tri.Transform(worldMat);
      gDebugDraw->Add(Debug::Triangle(tri).Color(Color::Aqua).Border(true).Alpha(50));
    }
  }
}

void HeightMapDebugDrawer::DrawPatch(IntVec2Param patchIndex)
{
  // Get a patch center and half size
  float unitsPerPatch = mMap->GetUnitsPerPatch();
  Vec2 localPos = mMap->GetLocalPosition(patchIndex);
  Vec3 pos = Vec3(localPos.x, mDrawOffset, localPos.y);
  Vec3 halfSize = Vec3(unitsPerPatch * .5f, .1f, unitsPerPatch * .5f);

  // Make the aabb of the patch
  Aabb aabb;
  aabb.SetCenterAndHalfExtents(pos, halfSize);

  // The height map might be scaled and rotated, so bring it to world
  // space and take the obb of that world space aabb
  Obb obb = aabb.Transform(mTransform->GetWorldMatrix());

  gDebugDraw->Add(Debug::Obb(obb));
  // Draw the patch's indices as well
  String str = String::Format("%d, %d", patchIndex.x, patchIndex.y);
  gDebugDraw->Add(Debug::Text(obb.GetCenter(), real(1.0), str));
}

void HeightMapDebugDrawer::DrawCell(IntVec2Param patchIndex, IntVec2Param cellIndex, bool skippedCell)
{
  float unitsPerPatch = mMap->GetUnitsPerPatch();
  Vec2 patchPos = mMap->GetLocalPosition(patchIndex);
  Vec2 patchHalfSize = Vec2(unitsPerPatch * .5f, unitsPerPatch * .5f);
  Vec2 patchStart = patchPos - patchHalfSize;

  // Get a cell's center and half size
  float cellScalar = unitsPerPatch / HeightPatch::Size;
  Vec3 cellHalfSize = Vec3(cellScalar, 0.1f, cellScalar) * .5f;

  Vec2 cellSize = Vec2(cellScalar,cellScalar);
  Vec2 currCell = Vec2((float)cellIndex.x, (float)cellIndex.y);
  Vec2 cellStart = patchStart + currCell * cellScalar;
  Vec2 localCellPos = cellStart + cellSize * .5f;

  Vec3 cellPos = Vec3(localCellPos.x, mDrawOffset, localCellPos.y);

  Aabb cellAabb;
  cellAabb.SetCenterAndHalfExtents(cellPos, cellHalfSize);

  // If this cell had a hit triangle in it, change it's color
  ByteColor color = Color::Black;
  if(!skippedCell)
    color = Color::Orange;

  // Make the aabb of the cell
  Obb cellObb = cellAabb.Transform(mTransform->GetWorldMatrix());
  gDebugDraw->Add(Debug::Obb(cellObb).Color(color));

  // Draw the cell's indices as well
  String cellStr = String::Format("%d, %d", cellIndex.x, cellIndex.y);
  gDebugDraw->Add(Debug::Text(cellObb.GetCenter(), real(.05), cellStr));
}

void HeightMapDebugDrawer::DrawRayProjection(Vec3Param start, Vec3Param dir, float maxT)
{
  Mat3 worldRot = Math::ToMatrix3(mTransform->GetWorldRotation());
  Vec3 worldPos = mTransform->GetWorldTranslation();

  Vec3 end = start + dir * maxT;
  Vec3 normal = worldRot.GetBasis(1);
  Vec3 projStart = start - Math::Dot(start - worldPos, normal) * normal;
  Vec3 projEnd = end - Math::Dot(end - worldPos, normal) * normal;
  gDebugDraw->Add(Debug::Line(projStart, projEnd));
}

void HeightMapDebugDrawer::DrawAabbProjection(const Aabb& aabb)
{
  Mat4 worldMat = mTransform->GetWorldMatrix();
  Mat4 worldMatInv = worldMat.Inverted();

  // Get the aabb
  Vec3 aabbPoints[8];
  aabbPoints[0] = Vec3(aabb.mMin.x, aabb.mMin.y, aabb.mMin.z);
  aabbPoints[1] = Vec3(aabb.mMax.x, aabb.mMin.y, aabb.mMin.z);
  aabbPoints[2] = Vec3(aabb.mMin.x, aabb.mMax.y, aabb.mMin.z);
  aabbPoints[3] = Vec3(aabb.mMax.x, aabb.mMax.y, aabb.mMin.z);
  aabbPoints[4] = Vec3(aabb.mMin.x, aabb.mMin.y, aabb.mMax.z);
  aabbPoints[5] = Vec3(aabb.mMax.x, aabb.mMin.y, aabb.mMax.z);
  aabbPoints[6] = Vec3(aabb.mMin.x, aabb.mMax.y, aabb.mMax.z);
  aabbPoints[7] = Vec3(aabb.mMax.x, aabb.mMax.y, aabb.mMax.z);

  // Bring the aabb into the height maps space (minus scale)
  for(uint i = 0; i < 8; ++i)
    aabbPoints[i] = Math::TransformPoint(worldMatInv, aabbPoints[i]);

  // Compute the actual local aabb now
  Aabb localAabb;
  localAabb.Compute(aabbPoints, 8);

  // Compute the 4 closest points to the y surface
  // (technically this is always the bottom when it could be the top, oh well)
  Vec3 points[4], projPoints[4];
  points[0] = Vec3(localAabb.mMin.x, localAabb.mMin.y, localAabb.mMin.z);
  points[1] = Vec3(localAabb.mMax.x, localAabb.mMin.y, localAabb.mMin.z);
  points[2] = Vec3(localAabb.mMin.x, localAabb.mMin.y, localAabb.mMax.z);
  points[3] = Vec3(localAabb.mMax.x, localAabb.mMin.y, localAabb.mMax.z);
  // Now project the closest points onto the draw surface
  for(uint i = 0; i < 4; ++i)
    projPoints[i] = Vec3(points[i].x, mDrawOffset, points[i].z);

  // Draw lines from the bottom of the aabb to the projected points on the
  // surface, but bring these back into world space to draw them
  for(uint i = 0; i < 4; ++i)
  {
    Vec3 worldPoint = Math::TransformPoint(worldMat, points[i]);
    Vec3 worldProjPoint = Math::TransformPoint(worldMat, projPoints[i]);
    gDebugDraw->Add(Debug::Line(worldPoint, worldProjPoint).Color(Color::Blue));
  }

  gDebugDraw->Add(Debug::Obb(aabb));
}

//-------------------------------------------------------------------HeightMapAabbChecker
ZilchDefineType(HeightMapAabbChecker, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultConstructor);

  ZeroBindDependency(Transform);

  ZilchBindFieldProperty(mDrawHeightMap);
  ZilchBindFieldProperty(mSkipNonCollidingCells);
  ZilchBindFieldProperty(mHeightMapPath);
}

HeightMapAabbChecker::HeightMapAabbChecker()
{
  mDrawHeightMap = false;
  mSkipNonCollidingCells = false;
}

void HeightMapAabbChecker::Serialize(Serializer& stream)
{
  stream.SerializeFieldDefault("HeightMapPath", mHeightMapPath, CogPath());
  SerializeNameDefault(mDrawHeightMap, false);
  SerializeNameDefault(mSkipNonCollidingCells, false);
}

void HeightMapAabbChecker::Initialize(CogInitializer& initializer)
{
  ConnectThisTo(GetSpace(), Events::FrameUpdate, OnFrameUpdate);
}

void HeightMapAabbChecker::OnAllObjectsCreated(CogInitializer& initializer)
{
  mHeightMapPath.RestoreLink(initializer, this, "HeightMapPath");
}

void HeightMapAabbChecker::OnFrameUpdate(UpdateEvent* e)
{
  Draw();

  if(!GetOwner()->IsEditorMode())
    GetOwner()->RemoveComponent(this);
}

void HeightMapAabbChecker::Draw()
{
  Cog* heightMapCog = mHeightMapPath.GetCog();
  if(heightMapCog == nullptr)
    return;

  HeightMap* heightMap = heightMapCog->has(HeightMap);
  HeightMapDebugDrawer* heightMapDrawer = heightMapCog->has(HeightMapDebugDrawer);
  if(heightMap == nullptr || heightMapDrawer == nullptr)
    return;

  if(mDrawHeightMap && heightMapDrawer != nullptr)
    heightMapDrawer->DebugDraw();

  Collider* collider = GetOwner()->has(Collider);
  if(collider == nullptr)
    return;

  Aabb aabb = collider->mAabb;
  heightMapDrawer->DrawAabbProjection(aabb);

  Transform* transform = heightMapCog->has(Transform);
  HeightMapCollider* heightMapCollider = heightMapCog->has(HeightMapCollider);

  real thickness = real(0.0);
  if(heightMapCollider != nullptr)
    thickness = heightMapCollider->GetThickness();

  HeightMapAabbRange range;
  // For debug drawing
  range.mSkipNonCollidingCells = mSkipNonCollidingCells;
  range.SetWorld(heightMap, aabb, thickness);

  PatchIndex lastPatch = PatchIndex(99999999, 9999999);
  PatchIndex lastCell = CellIndex(99999999, 9999999);
  for(; !range.Empty(); range.PopFront())
  {
    HeightMapAabbRange::TriangleInfo& info = range.Front();
    PatchIndex patchIndex = info.mPatchIndex;
    CellIndex cellIndex = info.mCellIndex;

    bool validTriangles = range.TrianglesToProcess();

    // If we hit a triangle, draw it
    if(validTriangles)
    {
      Triangle& localTri = info.mLocalTri;
      Mat4 worldTransform = transform->GetWorldMatrix();
      gDebugDraw->Add(Debug::Triangle(localTri.Transform(worldTransform)).Color(Color::Orange).Border(true).Alpha(50));

      if(heightMapCollider)
      {
        Vec3 dir = HeightMap::UpVector * -heightMapCollider->GetThickness();
        Triangle sweptTri = Triangle(localTri.p0 + dir, localTri.p1 + dir, localTri.p2 + dir);
        gDebugDraw->Add(Debug::Triangle(sweptTri.Transform(worldTransform)).Color(Color::Orange).Border(true).Alpha(50));
      }
    }

    if(cellIndex != lastCell)
      heightMapDrawer->DrawCell(patchIndex, cellIndex, !validTriangles);
    if(patchIndex != lastPatch)
      heightMapDrawer->DrawPatch(patchIndex);

    lastPatch = patchIndex;
    lastCell = cellIndex;
  }
}

}//namespace Zero
