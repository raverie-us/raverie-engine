///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg, Nathan Carlson, Ryan Edgemon
/// Copyright 2010-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
DefineEvent(HeightMapPatchAdded);
DefineEvent(HeightMapPatchRemoved);
DefineEvent(HeightMapPatchModified);
DefineEvent(HeightMapSave);
}//namespace Events

/// The icon we use
const String cHeightMapIcon("HeightMap");

//------------------------------------------------------------------- Events
ZilchDefineType(HeightMapEvent, builder, type)
{
  ZeroBindDocumented();
}

//------------------------------------------------------------------- HeightPatch
ZilchDefineType(HeightPatch, builder, type)
{
}

HeightPatch::HeightPatch( )
  : MinHeight(0), MaxHeight(0)
{
  memset(Heights, 0, sizeof(Heights));
}

HeightPatch::HeightPatch(const HeightPatch& rhs)
{
  Index = rhs.Index;

  MinHeight = rhs.MinHeight;
  MaxHeight = rhs.MaxHeight;

  for(int i = 0; i < TotalSize; ++i)
    Heights[i] = rhs.Heights[i];
}

float& HeightPatch::GetHeight(CellIndex index)
{
  size_t linearIndex = index.x + index.y * HeightPatch::Size;
  ErrorIf(linearIndex > TotalSize, "Invalid height index");
  return Heights[linearIndex];
}

void HeightPatch::SetHeight(CellIndex index, float height)
{
  size_t linearIndex = index.x + index.y * HeightPatch::Size;
  ErrorIf(linearIndex > TotalSize, "Invalid height index");
  Heights[linearIndex] = height;
}

//------------------------------------------------------------------- CellRange

HeightMapCellRange::HeightMapCellRange(HeightMap* heightMap, Vec2 position, real radius, real feather)
  : mHeightMap(heightMap), mToolPosition(position), mRadius(radius), mFeather(feather)
{
  mCellSize = mHeightMap->mUnitsPerPatch / HeightPatch::Size;

  Vec2 extents(mRadius + mFeather + mCellSize, mRadius + mFeather + mCellSize);
  mAabbMin = mToolPosition - extents;
  mAabbMax = mToolPosition + extents;

  mPatchIndexMin = mHeightMap->GetPatchIndexFromLocal(mAabbMin);
  mPatchIndexMax = mHeightMap->GetPatchIndexFromLocal(mAabbMax);

  Reset();
}

HeightMapCellRange::HeightMapCellRange(PatchMapCopy& patchMap, Vec2 toolPosition, real radius, real feather)
{
}

void HeightMapCellRange::Reset( )
{
  mPatchIndex = mPatchIndexMin;
  mPatch = mHeightMap->GetPatchAtIndex(mPatchIndex);
  SkipDeadPatches( );
}

HeightMapCell HeightMapCellRange::Front()
{
  HeightMapCell cell = {mCellIndex, mInfluence, mPatch};
  return cell;
}

void HeightMapCellRange::PopFront()
{
  GetNextCell();
}

bool HeightMapCellRange::Empty()
{
  return mPatchIndex.y > mPatchIndexMax.y;
}

void HeightMapCellRange::GetNextPatch()
{
  ++mPatchIndex.x;
  if (mPatchIndex.x > mPatchIndexMax.x)
  {
    mPatchIndex.x = mPatchIndexMin.x;
    ++mPatchIndex.y;
  }

  mPatch = mHeightMap->GetPatchAtIndex(mPatchIndex);
}

void HeightMapCellRange::SkipDeadPatches()
{
  while (!mPatch)
  {
    GetNextPatch();
    if (Empty())
      return;
  }

  Vec2 patchPos = mHeightMap->GetLocalPosition(mPatchIndex);
  Vec2 patchMin = patchPos - Vec2(1, 1) * mCellSize * (HeightPatch::Size / 2);
  Vec2 patchMax = patchMin + Vec2(1, 1) * mCellSize * (HeightPatch::Size - 1);

  mPatchOffset = patchMin;

  Vec2 cellMin = (Math::Max(patchMin, mAabbMin) - mPatchOffset) / mCellSize;
  Vec2 cellMax = (Math::Min(patchMax, mAabbMax) - mPatchOffset) / mCellSize;

  mCellIndexMin = CellIndex(int(Math::Ceil(cellMin.x)), int(Math::Ceil(cellMin.y)));
  mCellIndexMax = CellIndex(int(Math::Floor(cellMax.x)), int(Math::Floor(cellMax.y)));

  mCellIndexMin = Math::Min(mCellIndexMin, CellIndex(HeightPatch::Size - 1, HeightPatch::Size - 1));
  mCellIndexMax = Math::Max(mCellIndexMax, CellIndex(0, 0));

  mCellIndex = mCellIndexMin;
  GetInfluence();
}

void HeightMapCellRange::GetNextCell()
{
  ++mCellIndex.x;
  if (mCellIndex.x > mCellIndexMax.x)
  {
    mCellIndex.x = mCellIndexMin.x;
    ++mCellIndex.y;
  }
  if (mCellIndex.y > mCellIndexMax.y)
  {
    GetNextPatch();
    SkipDeadPatches();
  }
  else
  {
    GetInfluence();
  }
}

void HeightMapCellRange::GetInfluence()
{
  Vec2 cellPosition = Vec2((real)mCellIndex.x, (real)mCellIndex.y) * mCellSize + mPatchOffset;
  real distance = Math::Distance(cellPosition, mToolPosition);
  mInfluence = FeatherInfluence(distance, mRadius, mFeather);
}

void HeightMapCellRange::SignalPatchesModified()
{
  while (!Empty())
  {
    mHeightMap->SignalPatchModified(mPatch, mAabbMin, mAabbMax);

    GetNextPatch();
    while (!mPatch)
    {
      GetNextPatch();
      if (Empty())
        break;
    }
  }

  Reset();
}

//------------------------------------------------------------------- HeightMap
const Vec3 HeightMap::UpVector = Vec3(0, 1, 0);
// Cell index limits account for generating padded vertex data
const CellIndex HeightMap::sCellIndexMin = CellIndex(0, 0);
const CellIndex HeightMap::sCellIndexMax = CellIndex(HeightPatch::PaddedNumVerticesPerSide - 1, HeightPatch::PaddedNumVerticesPerSide - 1);

ZilchDefineType(HeightMap, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDocumented();
  ZeroBindDependency(Transform);
  ZilchBindGetterProperty(Source);
  ZilchBindGetterSetterProperty(UnitsPerPatch);
}

/// Constructor
HeightMap::HeightMap()
{
  mUnitsPerPatch = 50.0f;
  mModified = false;
}

/// Destructor
HeightMap::~HeightMap()
{
  DeleteObjectsInContainer(mPatches);
}

void HeightMap::Serialize(Serializer& stream)
{
  if(stream.GetMode() == SerializerMode::Saving)
    SaveToHeightMapSource(stream);

  SerializeNameDefault(mUnitsPerPatch, (float)HeightPatch::NumQuadsPerSide);
  SerializeResourceName(mSource, HeightMapSourceManager);

  if(stream.GetMode() == SerializerMode::Loading)
    LoadFromHeightMapSource(stream);
}

void HeightMap::Initialize(CogInitializer& initializer)
{
  if(Z::gRuntimeEditor)
    Z::gRuntimeEditor->Visualize(this, cHeightMapIcon);

  // Get a pointer to the transform
  mTransform = GetOwner()->has(Transform);
}

HeightMapSource* HeightMap::GetSource()
{
  return mSource;
}

float HeightMap::GetUnitsPerPatch()
{
  return mUnitsPerPatch;
}

void HeightMap::SetUnitsPerPatch(float value)
{
  // Set the units per patch internally
  mUnitsPerPatch = value;

  mCachedPatchVertices.Clear();

  // We need to update all patches since this value changed
  SignalAllPatchesModified();
}

void HeightMap::SignalAllPatchesModified()
{
  // Grab a range of all patches
  PatchMap::range patches = mPatches.All();

  // Loop through all the patches
  while (patches.Empty() == false)
  {
    // Grab the current patch and iterate to the next
    HeightPatch* patch = patches.Front().second;
    patches.PopFront();

    // Signal that the patch was modified
    SignalPatchModified(patch);
  }
}

float HeightMap::SampleHeight(Vec3Param worldPosition, float defaultValue, Vec3* worldNormal)
{
  return SampleHeight(GetLocalPosition(worldPosition), defaultValue, worldNormal);
}

float HeightMap::GetWorldPointHeight(Vec3Param worldPosition)
{
  // Transform the world point into local space
  Vec3 localPosition = mTransform->TransformPointInverse(worldPosition);
  return localPosition.y;
}

Vec3 HeightMap::GetWorldUp()
{
  return mTransform->TransformNormal(UpVector);
}

float HeightMap::SampleHeight(Vec2Param localPosition, float defaultValue, Vec3* worldNormal)
{
  // 0-------3
  // | \     |
  // |   \   |
  // |     \ |
  // 1-------2

  AbsoluteIndex i0, i1, i2, i3;

  i0 = GetFlooredAbsoluteIndexFromLocal(localPosition);
  i1 = i0 + IntVec2(0, 1);
  i2 = i0 + IntVec2(1, 1);
  i3 = i0 + IntVec2(1, 0);

  // Get the interpolant
  float x = localPosition.x / mUnitsPerPatch * HeightPatch::Size;
  float y = localPosition.y / mUnitsPerPatch * HeightPatch::Size;

  float tX = x - Math::Floor(x);
  float tY = y - Math::Floor(y);

  Vec2 localPos0 = GetLocalPositionFromAbsoluteIndex(i0);
  Vec2 localPos2 = GetLocalPositionFromAbsoluteIndex(i2);

  Vec2 from0To2 = localPos2 - localPos0;
  Vec2 from0ToPosition = localPosition - localPos0;

  float finalHeight;
  bool lowerTriangle = true;

  float height0 = SampleHeight(i0, defaultValue);
  float height2 = SampleHeight(i2, defaultValue);

  // Are we in the lower triangle?
  if (Math::Cross(from0To2, from0ToPosition) > 0)
  {
    float height1 = SampleHeight(i1, defaultValue);

    float xHeight = Math::Lerp(height1, height2, tX);
    finalHeight = Math::Lerp(height0, xHeight, tY);
  }
  else // Upper triangle
  {
    lowerTriangle = false;

    float height3 = SampleHeight(i3, defaultValue);

    float xHeight = Math::Lerp(height0, height3, tX);
    finalHeight = Math::Lerp(xHeight, height2, tY);
  }

  // Calculate the normal if requested
  if (worldNormal != nullptr)
  {
    Vec3 localHeightPos0(localPos0.x, height0, localPos0.y);
    Vec3 localHeightPos2(localPos2.x, height2, localPos2.y);
    Vec3 localHeightPosition(localPosition.x, finalHeight, localPosition.y);

    if(lowerTriangle)
      *worldNormal = Math::Cross(localHeightPos2 - localHeightPos0, localHeightPosition - localHeightPos0);
    else
      *worldNormal = Math::Cross(localHeightPosition - localHeightPos0, localHeightPos2 - localHeightPos0);

    worldNormal->AttemptNormalize();
    *worldNormal = mTransform->TransformNormal(*worldNormal);
  }

  return finalHeight;
}

float HeightMap::SampleHeight(AbsoluteIndexParam absoluteIndex, float defaultValue)
{
  // Determine the patch index
  PatchIndex patchIndex;
  patchIndex.x = (int)Math::Round(absoluteIndex.x / (float)HeightPatch::Size);
  patchIndex.y = (int)Math::Round(absoluteIndex.y / (float)HeightPatch::Size);

  // Determine the cell index
  CellIndex cellIndex;
  cellIndex.x = absoluteIndex.x - patchIndex.x * HeightPatch::Size + HeightPatch::Size / 2;
  cellIndex.y = absoluteIndex.y - patchIndex.y * HeightPatch::Size + HeightPatch::Size / 2;

  HeightPatch* patch = mPatches.FindValue(patchIndex, nullptr);
  // If the given patch exists then sample its height
  if(patch != nullptr)
    return patch->GetHeight(cellIndex);
  
  if(cellIndex.x == 0)
  {
    patchIndex.x -= 1;
    cellIndex.x = HeightPatch::Size - 1;
  }
  if(cellIndex.y == 0)
  {
    patchIndex.y -= 1;
    cellIndex.y = HeightPatch::Size - 1;
  }

  patch = mPatches.FindValue(patchIndex, nullptr);
  if(patch != nullptr)
    return patch->GetHeight(cellIndex);
  return defaultValue;
}

Aabb HeightMap::GetPatchLocalAabb(HeightPatch* patch)
{
  // Get the height of the patch using min and max
  float heightSize = patch->MaxHeight - patch->MinHeight;

  // Compute the size of the patch
  Vec3 patchSize = Vec3(mUnitsPerPatch, heightSize, mUnitsPerPatch);

  // The patch index is already in the center of the patch
  // however the height needs to be offset so it is the center
  // of the aabb.
  Vec3 patchCenter = Vec3(
    float(patch->Index.x) * mUnitsPerPatch,
    patch->MinHeight + heightSize * 0.5f,
    float(patch->Index.y) * mUnitsPerPatch);

  Aabb localAabb(patchCenter, patchSize * 0.5f);

  return localAabb;
}

Aabb HeightMap::GetPatchAabb(HeightPatch* patch)
{
  // bring localAabb to world
  return GetPatchLocalAabb(patch).TransformAabb(mTransform->GetWorldMatrix());
}

void HeightMap::UpdatePatch(HeightPatch* patch)
{
  float min = Math::PositiveMax();
  float max = -Math::PositiveMax();
  for(uint i=0;i<HeightPatch::Size*HeightPatch::Size;++i)
  {
    min = Math::Min(min, patch->Heights[i]);
    max = Math::Max(max, patch->Heights[i]);
  }

  patch->MinHeight = min;
  patch->MaxHeight = max;
}

void HeightMap::GenerateIndices(Array<uint>& outIndices, uint lod)
{
  // HACK
  outIndices.Resize(HeightPatch::NumIndicesTotal);

  // Get a pointer to the index data for ease of use
  uint* out = outIndices.Data();

  // Loop through all vertices vertically
  for (int y = 0; y < HeightPatch::NumQuadsPerSide; ++y)
  {
    // Loop through all vertices horizontally
    for (int x = 0; x < HeightPatch::NumQuadsPerSide; ++x)
    {
      // The base vertex that we're making triangles from
      uint base = x + y * HeightPatch::NumVerticesPerSide;

      // Make the first triangle
      *out++ = base;
      *out++ = base + HeightPatch::NumVerticesPerSide;
      *out++ = base + HeightPatch::NumVerticesPerSide + 1;

      // Make the second triangle
      *out++ = base;
      *out++ = base + HeightPatch::NumVerticesPerSide + 1;
      *out++ = base + 1;
    }
  }
}

PatchIndex HeightMap::GetPatchIndexFromWorld(Vec3Param worldPosition)
{
  // Now get the index from the local space position and return it
  return GetPatchIndexFromLocal(GetLocalPosition(worldPosition));
}

PatchIndex HeightMap::GetPatchIndexFromLocal(Vec2Param localPosition)
{
  // In order to determine the patch index, we simply take the local
  // space position and divide by the units per patch, then integerize it
  PatchIndex index;
  index.x = (int)Math::Round(localPosition.x / mUnitsPerPatch);
  index.y = (int)Math::Round(localPosition.y / mUnitsPerPatch);

  // Return the found index
  return index;
}

AbsoluteIndex HeightMap::GetNearestAbsoluteIndexFromWorld(Vec3Param worldPosition)
{
  // Now get the index from the local space position and return it
  return GetNearestAbsoluteIndexFromLocal(GetLocalPosition(worldPosition));
}

AbsoluteIndex HeightMap::GetNearestAbsoluteIndexFromLocal(Vec2Param localPosition)
{
  // Get the x and y in patch space
  float x = localPosition.x / mUnitsPerPatch;
  float y = localPosition.y / mUnitsPerPatch;

  // Return the index
  return AbsoluteIndex((int)Math::Round(x * HeightPatch::Size), (int)Math::Round(y * HeightPatch::Size));
}

AbsoluteIndex HeightMap::GetFlooredAbsoluteIndexFromLocal(Vec2Param localPosition)
{
  // Get the x and y in patch space
  float x = localPosition.x / mUnitsPerPatch;
  float y = localPosition.y / mUnitsPerPatch;

  // Return the index
  return AbsoluteIndex((int)Math::Floor(x * HeightPatch::Size), (int)Math::Floor(y * HeightPatch::Size));
}

Vec2 HeightMap::GetLocalPositionFromAbsoluteIndex(AbsoluteIndexParam absoluteIndex)
{
  Vec2 localPosition((float)absoluteIndex.x, (float)absoluteIndex.y);
  localPosition /= (float)HeightPatch::Size;
  localPosition *= (float)mUnitsPerPatch;
  return localPosition;
}

AbsoluteIndex HeightMap::GetAbsoluteIndex(PatchIndex patchIndex, CellIndex cellIndex)
{
  // Compute the absolute index
  return AbsoluteIndex(patchIndex * HeightPatch::Size +
    CellIndex(cellIndex.x - HeightPatch::Size / 2, cellIndex.y - HeightPatch::Size / 2));
}

PatchIndex HeightMap::GetPatchIndex(AbsoluteIndexParam absoluteIndex)
{
  PatchIndex patchIndex;
  patchIndex.x = (int)Math::Round(absoluteIndex.x / (float)HeightPatch::Size);
  patchIndex.y = (int)Math::Round(absoluteIndex.y / (float)HeightPatch::Size);

  return patchIndex;
}

CellIndex HeightMap::GetCellIndexFromWorld(Vec3Param worldPosition)
{
  return GetCellIndex(GetNearestAbsoluteIndexFromWorld(worldPosition));
}

CellIndex HeightMap::GetCellIndexFromLocal(Vec2Param localPosition)
{
  return GetCellIndex(GetNearestAbsoluteIndexFromLocal(localPosition));
}

CellIndex HeightMap::GetCellIndex(AbsoluteIndexParam absoluteIndex)
{
  PatchIndex patchIndex = GetPatchIndex(absoluteIndex);

  // Determine the cell index
  CellIndex cellIndex;
  cellIndex.x = absoluteIndex.x - patchIndex.x * HeightPatch::Size + HeightPatch::Size / 2;
  cellIndex.y = absoluteIndex.y - patchIndex.y * HeightPatch::Size + HeightPatch::Size / 2;
  return cellIndex;
}

CellIndex HeightMap::GetCellIndex(AbsoluteIndexParam absoluteIndex, PatchIndexParam patchIndex)
{
  // Determine the cell index
  CellIndex cellIndex;
  cellIndex.x = absoluteIndex.x - patchIndex.x * HeightPatch::Size + HeightPatch::Size / 2;
  cellIndex.y = absoluteIndex.y - patchIndex.y * HeightPatch::Size + HeightPatch::Size / 2;
  return cellIndex;
}

void HeightMap::GetPatchAndCellIndex(AbsoluteIndexParam absoluteIndex, PatchIndex& patchIndex, CellIndex& cellIndex)
{
  patchIndex = GetPatchIndex(absoluteIndex);

  // Determine the cell index
  cellIndex.x = absoluteIndex.x - patchIndex.x * HeightPatch::Size + HeightPatch::Size / 2;
  cellIndex.y = absoluteIndex.y - patchIndex.y * HeightPatch::Size + HeightPatch::Size / 2;
}

Vec3 HeightMap::GetWorldPosition(Vec2Param localPosition)
{
  // Sample the height at the local position
  float height = SampleHeight(localPosition, 0);

  // The y will be the height of the map
  return mTransform->TransformPoint(Vec3(localPosition.x, height, localPosition.y));
}

Vec2 HeightMap::GetLocalPosition(Vec3Param worldPosition)
{
  // Transform the world point into local space
  Vec3 localPosition = mTransform->TransformPointInverse(worldPosition);

  // Now get the local space position and return it
  return Vec2(localPosition.x, localPosition.z);
}

Vec3 HeightMap::GetWorldPosition(PatchIndexParam index)
{
  Vec2 localPosition = GetLocalPosition(index);
  return mTransform->TransformPoint(Vec3(localPosition.x, 0.0f, localPosition.y));
}

Vec2 HeightMap::GetLocalPosition(PatchIndexParam index)
{
  return Vec2(index.x * mUnitsPerPatch,
              index.y * mUnitsPerPatch);
}

void HeightMap::ApplyNoiseToPatch(HeightPatch* patch, float baseHeight, float frequency, float amplitude)
{
  // We scale the frequency because we're using whole units (integers) rather than floats
  const float FrequencyScale = 0.005f;

  // Loop through all cells
  for (size_t y = 0; y < HeightPatch::Size; ++y)
  {
    for (size_t x = 0; x < HeightPatch::Size; ++x)
    {
      // Compute the cell and absolute index
      CellIndex cellIndex(x, y);
      AbsoluteIndex absoluteIndex = patch->Index * HeightPatch::Size + cellIndex;

      // Compute the height based off the noise function
      float noiseHeight = PerlinNoise
      (
        (float)absoluteIndex.x * frequency * FrequencyScale,
        (float)absoluteIndex.y * frequency * FrequencyScale
      );

      // Add in the base height
      float height = baseHeight + noiseHeight * amplitude;

      // Set the height of the cell
      patch->SetHeight(cellIndex, height);
    }
  }
}

HeightPatch* HeightMap::CreatePatchAtIndex(PatchIndexParam index)
{
  // Grab the patch at that location
  HeightPatch*& patch = mPatches[index];

  // Only create the patch if we have none yet
  if (patch == NULL)
  {
    // Create a new height patch
    patch = new HeightPatch();
    patch->Index = index;

    // Send out an event that the height map patch was added (and update adjacents)
    SendPatchEvent(Events::HeightMapPatchAdded, patch);

    // Update adjacent patches *after* we add the patch
    UpdateAdjacentPatches(patch);
  }

  // Return the created patch
  return patch;
}

void HeightMap::DestroyPatchAtIndex(PatchIndexParam index)
{
  // Get the patch by index
  HeightPatch* patch = mPatches.FindValue(index, NULL);

  // If a patch actually exists at that index
  if (patch != NULL)
  {
    // Send out an event that the height map patch was removed
    SendPatchEvent(Events::HeightMapPatchRemoved, patch);

    // Remove the patch from the map
    mPatches.Erase(index);
    mCachedPatchVertices.Erase(index);

    // Remove from height map source
    if(mSource)
      mSource->RemovePatch(index);

    // Update adjacent patches *after* we remove the patch
    // from the list, but before we delete it
    UpdateAdjacentPatches(patch);

    // Delete the patch at the given index
    delete patch;
  }
}

HeightPatch* HeightMap::GetPatchAtIndex(PatchIndexParam index)
{
  // Return the found patch
  return mPatches.FindValue(index, NULL);
}

void HeightMap::GetQuadAtIndex(AbsoluteIndex index, Triangle triangles[2], uint& count)
{
  // Break down absolute index
  PatchIndex patchIndex;
  CellIndex cellIndex;
  GetPatchAndCellIndex(index, patchIndex, cellIndex);

  //get the center, size and start (bottom left) of this patch
  Vec2 patchPos = GetLocalPosition(patchIndex);
  Vec2 patchHalfExtents = Vec2(mUnitsPerPatch,mUnitsPerPatch) * .5f;
  Vec2 patchStart = patchPos - patchHalfExtents;
  //To get a cell center, we have to start at the patch bottom left, offset by
  //the cell index scaled by each cell size. This gets us the bottom left of a cell,
  //to get the center we then have to offset by half a cell.
  real cellSizeScalar = mUnitsPerPatch / HeightPatch::Size;
  Vec2 cellSize = Vec2(cellSizeScalar,cellSizeScalar) * .5f;
  Vec2 currCell = Vec2((float)cellIndex.x,(float)cellIndex.y);
  Vec2 cellStart = patchStart + currCell * cellSizeScalar;
  Vec2 cellCenter = cellStart + cellSize;

  Vec2 cellPos = cellCenter;

  //get the heights of the four corners of the cell
  real h00 = SampleHeight(GetAbsoluteIndex(patchIndex, cellIndex + CellIndex(0,0)), Math::cInfinite);
  real h01 = SampleHeight(GetAbsoluteIndex(patchIndex, cellIndex + CellIndex(0,1)), Math::cInfinite);
  real h10 = SampleHeight(GetAbsoluteIndex(patchIndex, cellIndex + CellIndex(1,0)), Math::cInfinite);
  real h11 = SampleHeight(GetAbsoluteIndex(patchIndex, cellIndex + CellIndex(1,1)), Math::cInfinite);

  //create local vertices for the four corners of the cell
  Vec3 p00 = Vec3(cellPos.x - cellSize.x,h00,cellPos.y - cellSize.y);
  Vec3 p01 = Vec3(cellPos.x - cellSize.x,h01,cellPos.y + cellSize.y);
  Vec3 p10 = Vec3(cellPos.x + cellSize.x,h10,cellPos.y - cellSize.y);
  Vec3 p11 = Vec3(cellPos.x + cellSize.x,h11,cellPos.y + cellSize.y);

  //edge triangles and holes are defined by vertices set to infinite, so determine if any of the vertices are infinite
  bool b01 = h01 != Math::cInfinite;
  bool b00 = h00 != Math::cInfinite;
  bool b10 = h10 != Math::cInfinite;
  bool b11 = h11 != Math::cInfinite;

  count = 0;
  //we need to determine how many of the triangles were actually hit,
  //store them, and in t first order. Also, if any triangle is invalid
  //we should skip it (has an infinite point).
  if(b01 && b00 && b10)
  {
    triangles[0] = Triangle(p00,p01,p10);

    if(b01 && b10 && b11)
    {
      triangles[1] = Triangle(p10,p01,p11);
      count = 2;
    }
    else
      count = 1;
  }
  else if(b01 && b10 && b11)
  {
    triangles[0] = Triangle(p10,p01,p11);
    count = 1;
  }
}

void HeightMap::SignalPatchModified(HeightPatch* patch)
{
  // Dirty
  Modified();

  UpdatePatch(patch);

  UpdatePatchVertices(patch);

  // Send out an event that the height map patch was modified
  SendPatchEvent(Events::HeightMapPatchModified, patch);

  // Since this patch was modified, we need to update adjacent patches (seams)
  UpdateAdjacentPatches(patch);
}

void HeightMap::SignalPatchModified(HeightPatch* patch, Vec2 min, Vec2 max)
{
  // Dirty
  Modified();

  UpdatePatch(patch);

  UpdatePatchVertices(patch, min, max);

  // Send out an event that the height map patch was modified
  SendPatchEvent(Events::HeightMapPatchModified, patch);
}

void HeightMap::GetPaddedHeightPatchVertices(HeightPatch* patch, Array<Vec3>& outVertices)
{
  Array<Vec3>* vertices = mCachedPatchVertices.FindPointer(patch->Index);
  if (vertices)
  {
    outVertices = *vertices;
  }
  else
  {
    outVertices.Resize(HeightPatch::PaddedNumVerticesTotal);
    ComputePaddedHeightPatchVertices(patch, outVertices);
  }
}

void HeightMap::GetHeightPatchVertices(HeightPatch* patch, Array<Vec3>& outVertices)
{
  const uint paddedWidth = HeightPatch::PaddedNumVerticesPerSide;
  const uint paddedSize = HeightPatch::PaddedNumVerticesTotal;
  real heights[paddedSize] = {};
  MakePaddedHeightBuffer(patch, heights);

  const uint width = HeightPatch::NumVerticesPerSide;
  const uint size = HeightPatch::NumVerticesTotal;
  outVertices.Resize(size);

  // Get the local position of the given patch
  Vec2 localPosition = GetLocalPosition(patch->Index);

  // Get the number of patches per
  real unitsPerCell = mUnitsPerPatch / HeightPatch::NumQuadsPerSide;

  // Compute half of the patch size
  real halfPatchSize = mUnitsPerPatch / 2.0f;
  real halfPaddedPatchSize = halfPatchSize + unitsPerCell;

  for (int y = 1; y < paddedWidth - 1; ++y)
  {
    for (int x = 1; x < paddedWidth - 1; ++x)
    {
      int heightIndex = x + y * paddedWidth;
      int vertexIndex = (x - 1) + (y - 1) * width;

      // Compute the position from the local position and the height
      Vec3& vertex = outVertices[vertexIndex];
      vertex.x = localPosition.x + unitsPerCell * x - halfPaddedPatchSize;
      vertex.z = localPosition.y + unitsPerCell * y - halfPaddedPatchSize;
      vertex.y = heights[heightIndex];
    }
  }
}

PatchMap::valuerange HeightMap::GetAllPatches()
{
  return mPatches.All();
}

HeightMapRayRange HeightMap::CastLocalRay(const Ray& ray, float maxT)
{
  HeightMapRayRange range;
  range.SetLocal(this,ray,maxT);
  return range;
}

HeightMapRayRange HeightMap::CastWorldRay(const Ray& ray, float maxT)
{
  HeightMapRayRange range;
  range.SetWorld(this,ray,maxT);
  return range;
}

HeightMapAabbRange HeightMap::GetLocalAabbRange(const Aabb& aabb, real thickness)
{
  HeightMapAabbRange range;
  range.SetLocal(this,aabb,thickness);
  return range;
}

HeightMapAabbRange HeightMap::GetWorldAabbRange(const Aabb& aabb, real thickness)
{
  HeightMapAabbRange range;
  range.SetWorld(this,aabb,thickness);
  return range;
}

void HeightMap::SaveToHeightMapSource(Serializer& stream)
{
  HeightMapSourceManager* manager = HeightMapSourceManager::GetInstance();

  CogSavingContext* context = (CogSavingContext*)stream.GetSerializationContext();
  Archetype* archetype = context ? (Archetype*)context->SavingArchetype : 0;

  if(Z::gRuntimeEditor)
    mSource = (HeightMapSource*)Z::gRuntimeEditor->NewResourceOnWrite(HeightMapSourceManager::GetInstance(), ZilchTypeId(HeightMap), "Source", GetSpace(), mSource, archetype, mModified);

  if (mSource)
  {
    forRange(PatchMap::value_type patchPair, mPatches.All())
    {
      HeightPatch* patch = patchPair.second;
      PatchLayer* layer = mSource->GetLayerData(patchPair.first, PatchLayerType::Height);
      layer->LayerType = PatchLayerType::Height;
      layer->ElementSize = sizeof(float);
      layer->Width = HeightPatch::Size;
      layer->Height = HeightPatch::Size;
      uint size = layer->Size();
      layer->Allocate();
      memcpy(layer->Data, patch->Heights, size);
    }

    HeightMapEvent event;
    event.Source = mSource;
    DispatchEvent(Events::HeightMapSave, &event);

    if (mSource->mContentItem)
      mSource->mContentItem->SaveContent();
  }

  mModified = false;
}

void HeightMap::LoadFromHeightMapSource(Serializer& stream)
{
  HeightMapSource* source = mSource;

  if(!mSource)
    return;

  forRange(PatchData* data, mSource->mData.Values())
  {
    PatchLayer* layer = data->Layers.FindValue(PatchLayerType::Height, NULL);
    if(layer)
    {
      HeightPatch*& patch = mPatches[data->Index];
      patch = new HeightPatch();
      patch->Index = data->Index;

      uint memorySize = sizeof(HeightPatch::HeightValueType) * HeightPatch::Size * HeightPatch::Size;
      memcpy(patch->Heights, layer->Data, memorySize);

      UpdatePatch(patch);
    }
  }

  // Get space object is being created in
  CogCreationContext* context = (CogCreationContext*)stream.GetSerializationContext();
  Space* space = context->mSpace;
  if (!space)
    return;

  if (!mSource->mBuilder)
    return;

  // Get the level that owns this resource
  String resourceIdName = mSource->mBuilder->GetResourceOwner();
  LevelManager* levelManager = LevelManager::GetInstance();
  Resource* levelOwner = levelManager->GetResource(resourceIdName, ResourceNotFound::ReturnNull);

  // If being loaded into a different level, mark space as modified after load
  if (levelOwner && levelOwner != space->GetCurrentLevel())
    ConnectThisTo(space, Events::SpaceLevelLoaded, OnLevelLoaded);
}

void HeightMap::OnLevelLoaded(ObjectEvent* event)
{
  // Mark space as modified so resource can be copied
  GetOwner()->GetSpace()->MarkModified();
}

void HeightMap::Modified()
{
  mModified = true;
  GetSpace()->MarkModified();
}

void HeightMap::SendPatchEvent(StringParam eventType, HeightPatch* patch)
{
  HeightMapEvent e;
  e.Map = this;
  e.Patch = patch;
  DispatchEvent(eventType, &e);
}

void HeightMap::UpdatePatchVertices(HeightPatch* patch)
{
  Array<Vec3>* vertices = mCachedPatchVertices.FindPointer(patch->Index);
  if (!vertices)
  {
    vertices = &mCachedPatchVertices[patch->Index];
    vertices->Resize(HeightPatch::PaddedNumVerticesTotal);
  }

  ComputePaddedHeightPatchVertices(patch, *vertices);
}

void HeightMap::UpdatePatchVertices(HeightPatch* patch, Vec2 min, Vec2 max)
{
  // Get the number of patches per
  real unitsPerCell = mUnitsPerPatch / HeightPatch::NumQuadsPerSide;

  // Get the local position of the given patch
  Vec2 localPosition = GetLocalPosition(patch->Index);

  Array<Vec3>* vertices = mCachedPatchVertices.FindPointer(patch->Index);
  if (vertices)
  {
    // Plus one quad for updating padded area
    Vec2 patchMin = localPosition - Vec2(1, 1) * unitsPerCell * (HeightPatch::NumQuadsPerSide / 2 + 1);
    Vec2 patchMax = localPosition + Vec2(1, 1) * unitsPerCell * (HeightPatch::NumQuadsPerSide / 2 + 1);

    Vec2 cellMin = (Math::Max(patchMin, min) - patchMin) / unitsPerCell;
    Vec2 cellMax = (Math::Min(patchMax, max) - patchMin) / unitsPerCell;

    CellIndex cellIndexMin = CellIndex(int(Math::Ceil(cellMin.x)), int(Math::Ceil(cellMin.y)));
    CellIndex cellIndexMax = CellIndex(int(Math::Floor(cellMax.x)), int(Math::Floor(cellMax.y)));

    ComputePaddedHeightPatchVertices(patch, *vertices, cellIndexMin, cellIndexMax);
  }
  else
  {
    vertices = &mCachedPatchVertices[patch->Index];
    vertices->Resize(HeightPatch::PaddedNumVerticesTotal);

    ComputePaddedHeightPatchVertices(patch, *vertices);
  }
}

void HeightMap::UpdateAdjacentPatches(HeightPatch* patch)
{
  // Store an array of adjacent indices
  const size_t NumAdjacentPatches = 8;
  PatchIndex adjacentIndices[NumAdjacentPatches];

  /// Determine all adjacent indices
  adjacentIndices[0] = patch->Index + PatchIndex(+1, +0);
  adjacentIndices[1] = patch->Index + PatchIndex(+0, +1);
  adjacentIndices[2] = patch->Index + PatchIndex(-1, +0);
  adjacentIndices[3] = patch->Index + PatchIndex(+0, -1);

  adjacentIndices[4] = patch->Index + PatchIndex(+1, +1);
  adjacentIndices[5] = patch->Index + PatchIndex(-1, +1);
  adjacentIndices[6] = patch->Index + PatchIndex(+1, -1);
  adjacentIndices[7] = patch->Index + PatchIndex(-1, -1);

  // Get patch bounds so adjacent patches only update their appropriate seems
  Vec2 localPosition = GetLocalPosition(patch->Index);
  real unitsPerCell = mUnitsPerPatch / HeightPatch::NumQuadsPerSide;
  Vec2 patchMin = localPosition - Vec2(1, 1) * unitsPerCell * (HeightPatch::NumQuadsPerSide / 2 + 1);
  Vec2 patchMax = localPosition + Vec2(1, 1) * unitsPerCell * (HeightPatch::NumQuadsPerSide / 2 + 1);

  // Loop through all adjacent patches
  for (size_t i = 0; i < NumAdjacentPatches; ++i)
  {
    // Grab the current adjacent patch
    HeightPatch* adjacentPatch = GetPatchAtIndex(adjacentIndices[i]);

    // If we actually found an adjacent patch..
    if (adjacentPatch != NULL)
    {
      UpdatePatchVertices(adjacentPatch, patchMin, patchMax);
      // Send out an event that the adjacent patch was modified (we do not need to update adjacent patches)
      SendPatchEvent(Events::HeightMapPatchModified, adjacentPatch);
    }
  }
}

void HeightMap::MakePaddedHeightBuffer(HeightPatch* patch, real* heights)
{
  uint size = HeightPatch::Size;
  uint paddedSize = HeightPatch::PaddedNumVerticesPerSide;

  PatchIndex patchIndex = patch->Index;

  // Shortcuts to specific buffer locations that need to be written to
  real* nwStart = heights;
  real* nStart = nwStart + 1;
  real* neStart1 = nStart + size;
  real* neStart2 = neStart1 + 1;

  real* wStart = heights + paddedSize;
  real* mStart = wStart + 1;
  real* eStart1 = mStart + size;
  real* eStart2 = eStart1 + 1;

  real* swStart1 = heights + (paddedSize - 2) * paddedSize;
  real* swStart2 = swStart1 + paddedSize;
  real* sStart1 = swStart1 + 1;
  real* sStart2 = swStart2 + 1;
  real* seStart1 = sStart1 + size;
  real* seStart2 = sStart2 + size;

  // Adjacent patches
  HeightPatch* nwPatch = mPatches.FindValue(patchIndex + PatchIndex(-1,-1), nullptr);
  HeightPatch* nPatch  = mPatches.FindValue(patchIndex + PatchIndex( 0,-1), nullptr);
  HeightPatch* nePatch = mPatches.FindValue(patchIndex + PatchIndex( 1,-1), nullptr);
  HeightPatch* wPatch  = mPatches.FindValue(patchIndex + PatchIndex(-1, 0), nullptr);
  HeightPatch* ePatch  = mPatches.FindValue(patchIndex + PatchIndex( 1, 0), nullptr);
  HeightPatch* swPatch = mPatches.FindValue(patchIndex + PatchIndex(-1, 1), nullptr);
  HeightPatch* sPatch  = mPatches.FindValue(patchIndex + PatchIndex( 0, 1), nullptr);
  HeightPatch* sePatch = mPatches.FindValue(patchIndex + PatchIndex( 1, 1), nullptr);

  // middle
  {
    for (uint i = 0; i < size; ++i)
      memcpy(mStart + i * paddedSize, patch->Heights + i * size, size * sizeof(real));
  }

  // north edge
  {
    real* adjHeights;
    if (nPatch)
      adjHeights = nPatch->Heights + (size - 1) * size;
    else
      adjHeights = mStart;
    memcpy(nStart, adjHeights, size * sizeof(real));

    // north-west corner
    {
      if (nwPatch)
        nwStart[0] = nwPatch->Heights[size * size - 1];
      else
        nwStart[0] = nStart[0];
    }

    // north-east corners
    {
      if (nePatch)
      {
        neStart1[0] = nePatch->Heights[(size - 1) * size];
        neStart2[0] = nePatch->Heights[(size - 1) * size + 1];
      }
      else if (ePatch)
      {
        neStart1[0] = ePatch->Heights[0];
        neStart2[0] = ePatch->Heights[1];
      }
      else
      {
        neStart1[0] = nStart[size - 1];
        neStart2[0] = nStart[size - 1];
      }
    }
  }

  // west edge
  {
    real* adjHeights;
    uint rowSize;
    if (wPatch)
    {
      adjHeights = wPatch->Heights + size - 1;
      rowSize = size;
    }
    else
    {
      adjHeights = mStart;
      rowSize = paddedSize;
    }
    for (uint i = 0; i < size; ++i)
      wStart[i * paddedSize] = adjHeights[i * rowSize];
  }

  // east edges
  {
    real* adjHeights1;
    real* adjHeights2;
    uint rowSize;
    if (ePatch)
    {
      adjHeights1 = ePatch->Heights;
      adjHeights2 = ePatch->Heights + 1;
      rowSize = size;
    }
    else
    {
      adjHeights1 = mStart + size - 1;
      adjHeights2 = adjHeights1;
      rowSize = paddedSize;
    }
    for (uint i = 0; i < size; ++i)
    {
      eStart1[i * paddedSize] = adjHeights1[i * rowSize];
      eStart2[i * paddedSize] = adjHeights2[i * rowSize];
    }
  }

  // south edges
  {
    real* adjHeights1;
    real* adjHeights2;
    if (sPatch)
    {
      adjHeights1 = sPatch->Heights;
      adjHeights2 = sPatch->Heights + size;
    }
    else
    {
      adjHeights1 = mStart + (size - 1) * paddedSize;
      adjHeights2 = adjHeights1;
    }
    memcpy(sStart1, adjHeights1, size * sizeof(real));
    memcpy(sStart2, adjHeights2, size * sizeof(real));

    // south-west corners
    {
      if (swPatch)
      {
        swStart1[0] = swPatch->Heights[size - 1];
        swStart2[0] = swPatch->Heights[size - 1 + size];

        // prioritize top right corners over bottom left corners
        if (!sPatch)
        {
          sStart1[0] = swPatch->Heights[size - 1];
          sStart2[0] = swPatch->Heights[size - 1 + size];
        }
      }
      else
      {
        swStart1[0] = sStart1[0];
        swStart2[0] = sStart2[0];
      }
    }

    // south-east corners
    {
      if (sePatch)
      {
        seStart1[0] = sePatch->Heights[0];
        seStart1[1] = sePatch->Heights[1];
        seStart2[0] = sePatch->Heights[size];
        seStart2[1] = sePatch->Heights[size + 1];
      }
      // check south patch before east patch (top right priority)
      else if (sPatch)
      {
        seStart1[0] = sPatch->Heights[size - 1];
        seStart1[1] = sPatch->Heights[size - 1 + size];
        seStart2[0] = sPatch->Heights[size - 1];
        seStart2[1] = sPatch->Heights[size - 1 + size];
      }
      else if (ePatch)
      {
        seStart1[0] = ePatch->Heights[(size - 1) * size];
        seStart1[1] = ePatch->Heights[(size - 1) * size + 1];
        seStart2[0] = ePatch->Heights[(size - 1) * size];
        seStart2[1] = ePatch->Heights[(size - 1) * size + 1];
      }
      else
      {
        seStart1[0] = sStart1[size - 1];
        seStart1[1] = sStart1[size - 1];
        seStart2[0] = sStart2[size - 1];
        seStart2[1] = sStart2[size - 1];
      }
    }
  }
}

void HeightMap::ComputePaddedHeightPatchVertices(HeightPatch* patch, Array<Vec3>& outVertices, CellIndex min, CellIndex max)
{
  const uint paddedWidth = HeightPatch::PaddedNumVerticesPerSide;
  const uint paddedSize = HeightPatch::PaddedNumVerticesTotal;
  real heights[paddedSize] = {};
  MakePaddedHeightBuffer(patch, heights);

  outVertices.Resize(paddedSize);

  // Get the local position of the given patch
  Vec2 localPosition = GetLocalPosition(patch->Index);

  // Get the number of patches per
  real unitsPerCell = mUnitsPerPatch / HeightPatch::NumQuadsPerSide;

  // Compute half of the patch size
  real halfPatchSize = mUnitsPerPatch / 2.0f;
  real halfPaddedPatchSize = halfPatchSize + unitsPerCell;

  for (int y = min.y; y <= max.y; ++y)
  {
    for (int x = min.x; x <= max.x; ++x)
    {
      int index = x + y * paddedWidth;

      // Compute the position from the local position and the height
      Vec3& vertex = outVertices[index];
      vertex.x = localPosition.x + unitsPerCell * x - halfPaddedPatchSize;
      vertex.z = localPosition.y + unitsPerCell * y - halfPaddedPatchSize;
      vertex.y = heights[index];
    }
  }
}

float FeatherInfluence(float distance, float radius, float featherRadius)
{
  // If it's within the inner most radius...
  if (distance < radius)
  {
    // Full influence
    return 1.0f;
  }
  else if (distance < radius + featherRadius)
  {
    // Feather the influence
    return  1.0f - ((distance - radius) / featherRadius);
  }
  else
  {
    return 0.0f;
  }
}

CellRayRange::CellRayRange(HeightMap* map, PatchIndex index, Vec2Param rayStart, Vec2Param rayDir, real maxT)
{
  Set(map,index,rayStart,rayDir,maxT);
}

void CellRayRange::Set(HeightMap* map, PatchIndex index, Vec2Param rayStart, Vec2Param rayDir, real maxT)
{
  mMaxT = maxT;
  mMap = map;
  mRayStart = rayStart;
  mRayDir = rayDir;
  mPatchIndex = index;
  mStepX = static_cast<int>(Math::GetSign(mRayDir.x));
  mStepY = static_cast<int>(Math::GetSign(mRayDir.y));

  //get the center, size and start (bottom left) of this patch
  Vec2 patchPos = mMap->GetLocalPosition(mPatchIndex);
  Vec2 patchHalfExtents = Vec2(mMap->mUnitsPerPatch,mMap->mUnitsPerPatch) * .5f;
  Vec2 patchStart = patchPos - patchHalfExtents;
  Vec2 patchEnd = patchPos + patchHalfExtents;
  mPatchStart = patchStart;

  //find out when we start intersecting the box of this patch
  Intersection::Interval interval;
  Intersection::RayAabb(mRayStart,mRayDir,patchStart,patchEnd,&interval);
  mCurrT = interval.Min;

  //our start point on the ray is where we start hitting this patch
  Vec2 startPoint = mRayStart + mRayDir * mCurrT;
  //bring that starting point from map space to patch space (defined from the bottom left of the patch)
  Vec2 localPoint = startPoint - patchStart;

  //now convert that position to a cell index and clamp between
  //our min and max bounds for a cell (to deal with floating point)
  real cellSizeScalar = mMap->mUnitsPerPatch / HeightPatch::Size;
  mCellIndex.x = Math::Clamp((int)Math::Floor(localPoint.x / cellSizeScalar),0,31);
  mCellIndex.y = Math::Clamp((int)Math::Floor(localPoint.y / cellSizeScalar),0,31);
}

Vec2 CellRayRange::GetCurrentCellCenter()
{
  //To get a cell center, we have to start at the patch bottom left, offset by
  //the cell index scaled by each cell size. This gets us the bottom left of a cell,
  //to get the center we then have to offset by half a cell.
  real cellSizeScalar = mMap->mUnitsPerPatch / HeightPatch::Size;
  Vec2 cellSize = Vec2(cellSizeScalar,cellSizeScalar);
  Vec2 currCell = Vec2((float)mCellIndex.x,(float)mCellIndex.y);
  Vec2 cellStart = mPatchStart + currCell * cellSizeScalar;
  Vec2 cellCenter = cellStart + cellSize * .5f;
  return cellCenter;
}

Vec2 CellRayRange::GetNextTValues()
{
  //we have to check the plane boundaries from this cell to the next one
  //to do this, we need to get the center of a cell in map local space
  //(the patch start pos + the cell index times it's size + half a cell
  //Size(to get to the center)) but then we have to offset this in the
  //direction we are searching by half a cell so we get the edges
  real cellSizeScalar = mMap->mUnitsPerPatch / HeightPatch::Size;
  Vec2 cellRayDirOffset = Vec2(cellSizeScalar * mStepX,cellSizeScalar * mStepY) * .5f;
  Vec2 cellPos = GetCurrentCellCenter() + cellRayDirOffset;

  Vec2 t = CellRayRange::IntersectPlane(cellPos,mRayStart,mRayDir);
  return t;
}

void CellRayRange::PopFront()
{
  //get the t values to intersect the next plane along the x and y direction
  Vec2 t = GetNextTValues();
  //choose whichever we hit first and advance to that next cell
  if(t.x < t.y)
  {
    mCurrT = t.x;
    mCellIndex.x += mStepX;
  }
  else
  {
    mCurrT = t.y;
    mCellIndex.y += mStepY;
  }
}

CellIndex CellRayRange::Front()
{
  ErrorIf(Empty(), "Calling front on an empty range.");
  return mCellIndex;
}

bool CellRayRange::Empty()
{
  //if we are out of the patch bounds or we have exceeded our max tValue, then we are empty
  if(mCellIndex.x < 0 || mCellIndex.y < 0 ||
     mCellIndex.x >= HeightPatch::Size || mCellIndex.y >= HeightPatch::Size ||
     mCurrT >= mMaxT)
    return true;
  return false;
}

Vec2 CellRayRange::IntersectPlane(Vec2Param planeDistance, Vec2Param rayStart, Vec2Param rayDir)
{
  //the numerator in computing the distance
  Vec2 num = planeDistance - rayStart;
  Vec2 t = Vec2(Math::PositiveMax(),Math::PositiveMax());

  //safeguard from zero divisions, if we would zero divide leave the distance at
  //infinity since we aren't going to hit that axis any time soon
  if(rayDir.x != 0)
    t.x = num.x / rayDir.x;
  if(rayDir.y != real(0.0))
    t.y = num.y / rayDir.y;

  return t;
}

HeightMapQueryCache::HeightMapQueryCache()
{
  mCachedPatch = NULL;
}

float HeightMapQueryCache::SampleHeight(HeightMap* map, PatchIndexParam patchIndex, CellIndexParam cellIndex)
{
  AbsoluteIndex absIndex = map->GetAbsoluteIndex(patchIndex,cellIndex);
  PatchIndex realPatchIndex;
  CellIndex realCellIndex;
  map->GetPatchAndCellIndex(absIndex,realPatchIndex,realCellIndex);

  if(mCachedPatch == NULL || mCachedPatch->Index != realPatchIndex)
  {
    mCachedPatch = map->GetPatchAtIndex(realPatchIndex);
    if(mCachedPatch == NULL)
      return Math::cInfinite;
  }

  return mCachedPatch->GetHeight(realCellIndex);
}

PatchRayRange::PatchRayRange(HeightMap* map, Vec2Param rayStart, Vec2Param rayDir, real minT, real maxT)
{
  Set(map,rayStart,rayDir,minT, maxT);
}

void PatchRayRange::Set(HeightMap* map, Vec2Param rayStart, Vec2Param rayDir, real minT, real maxT)
{
  mMap = map;

  mRayStart = rayStart;
  mRayDir = rayDir;
  mMaxT = maxT;
  mCurrT = minT;

  //used to determine which direction we are stepping in
  mStepX = static_cast<int>(Math::GetSign(mRayDir.x));
  mStepY = static_cast<int>(Math::GetSign(mRayDir.y));

  //get the starting "patch" (might not actually be a patch there)
  mCurrPatchIndex = mMap->GetPatchIndexFromLocal(mRayStart + mRayDir * mCurrT);
  //now walk until our current patch is a valid one (no point in checking patches that don't exist)
  SkipDeadPatches();
}

void PatchRayRange::SetEmpty()
{
  mCurrT = real(1.0);
  mMaxT = real(0.0);
}

void PatchRayRange::PopFront()
{
  //skip to the next patch and then skip over all patches that don't exist
  GetNextPatch();
  SkipDeadPatches();
}

bool PatchRayRange::Empty() const
{
  //we're done when we run out of length to explore on the ray (since patches
  //are in a hash map, there's no easy way to know when we reached the end of
  //the grid apart from a max t value which approximates the bounds of the grid)
  return mCurrT > mMaxT;
}

PatchIndex PatchRayRange::Front()
{
  ErrorIf(Empty(), "Calling front on an empty range.");
  return mCurrPatchIndex;
}

void PatchRayRange::GetNextPatch()
{
  real unitsPerPatch = mMap->mUnitsPerPatch;
  //get the local center of the next patch
  Vec2 localPos = mMap->GetLocalPosition(mCurrPatchIndex + PatchIndex(mStepX,mStepY));
  //we need to compare t values with the start of the patch though, so offset by
  //half the patch size in the opposite direction we are searching from
  Vec2 halfOffset = Vec2(unitsPerPatch * mStepX,unitsPerPatch * mStepY)* .5f;
  Vec2 tValues = CellRayRange::IntersectPlane(localPos - halfOffset,mRayStart,mRayDir);
  //advance in the direction that we hit first
  if(tValues.x < tValues.y)
  {
    mCurrT = tValues.x;
    mCurrPatchIndex.x += mStepX;
  }
  else
  {
    mCurrT = tValues.y;
    mCurrPatchIndex.y += mStepY;
  }
}

void PatchRayRange::SkipDeadPatches()
{
  //if our current patch is invalid then get the next patch indices and check
  //again to see if it is valid. Keep this up until we get a valid patch or run
  //out of patches to check.
  HeightPatch* patch = mMap->GetPatchAtIndex(mCurrPatchIndex);
  while(patch == NULL && !Empty())
  {
    //if the ray we got was straight down, we'll never manage to continue
    //the loop so set the range to be empty and bail
    if(mRayDir.x == real(0.0) && mRayDir.y == real(0.0))
    {
      SetEmpty();
      break;
    }
    GetNextPatch();
    patch = mMap->GetPatchAtIndex(mCurrPatchIndex);
  }
}

HeightMapRayRange::HeightMapRayRange()
{
  mMap = NULL;
  mSkipNonCollidingCells = true;
}

void HeightMapRayRange::SetLocal(HeightMap* map, const Ray& ray, float maxT)
{
  mMap = map;
  mLocalRayStart = ray.Start;
  mLocalRayDir = ray.Direction;
  SetUp(maxT);
}

void HeightMapRayRange::SetWorld(HeightMap* map, const Ray& ray, float maxT)
{
  mMap = map;
  mLocalRayStart = mMap->mTransform->TransformPointInverse(ray.Start);
  mLocalRayDir = mMap->mTransform->TransformNormalInverse(ray.Direction);
  SetUp(maxT);
}

void HeightMapRayRange::SetUp(float maxT)
{
  //project the ray onto the x-z plane where the height map resides
  mProjectedRayStart = Vec2(mLocalRayStart.x,mLocalRayStart.z);
  mProjectedRayDir = Vec2(mLocalRayDir.x,mLocalRayDir.z);

  mTriangleIndex = 0;
  mTriangleCount = 0;

  //the min and max range that we will consider against this
  //map then clamp the max to what was passed in
  float mapMin, mapMax;
  GetTMinMaxRange(mLocalRayStart, mLocalRayDir, mProjectedRayStart, mProjectedRayDir, mapMin, mapMax);
  mMaxT = Math::Min(mapMax,maxT);

  //set up the initial patch range
  mPatchRange.Set(mMap,mProjectedRayStart,mProjectedRayDir,mapMin,mMaxT);

  //if we hit any patches, set up the cell range and then
  //iterate until we actually have triangles we hit
  if(!Empty())
  {
    mCellRange.Set(mMap,mPatchRange.Front(),mProjectedRayStart,mProjectedRayDir,mMaxT);
    LoadTriangles();
    LoadUntilValidTriangles();
  }
}

void HeightMapRayRange::GetTMinMaxRange(Vec3Param localRayStart, Vec3Param localRayDir,
                                        Vec2Param rayStart, Vec2Param rayDir, float& minT, float& maxT)
{
  //if the raycast is straight down, then the projected ray will cause some bad things to happen.
  //Just set a simple set of bounds for the min/max so that we'll check at least once
  //(we should be prevented from infinite looping at a different spot)
  if(mProjectedRayDir.Length() == real(0))
  {
    minT = 0;
    maxT = 1;
    return;
  }

  //since the patches are in a hash map, we have no way to know what the max
  //bounds are for the raycast. So we need to somehow figure out what the aabb
  //of the height map is and clip the ray against that. At the moment we don't
  //have the aabb, so compute it now by iterating over all of the patches
  //(fix later to make faster)
  PatchIndex min = PatchIndex(0,0);
  PatchIndex max = PatchIndex(0,0);
  real maxHeight = -Math::PositiveMax();
  real minHeight = Math::PositiveMax();
  for(PatchMap::valuerange range = mMap->GetAllPatches(); !range.Empty(); range.PopFront())
  {
    if(range.Front() == NULL)
      continue;

    HeightPatch* patch = range.Front();
    PatchIndex index = patch->Index;
    min.x = Math::Min(min.x,index.x);
    min.y = Math::Min(min.y,index.y);
    max.x = Math::Max(max.x,index.x);
    max.y = Math::Max(max.y,index.y);
    minHeight = Math::Min(minHeight, patch->MinHeight);
    maxHeight = Math::Max(maxHeight, patch->MaxHeight);
  }

  //there were no patches so just set the range to invalid and return
  if(minHeight > maxHeight)
  {
    minT = 1;
    maxT = 0;
    return;
  }

  //we compute the min and max indices, convert those to positions (accounting for the fact the position of min and max is the patch center)
  Vec2 patchSize = Vec2(mMap->mUnitsPerPatch,mMap->mUnitsPerPatch) * .5f;
  Vec2 minLocal = mMap->GetLocalPosition(min) - patchSize;
  Vec2 maxLocal = mMap->GetLocalPosition(max) + patchSize;

  Intersection::Interval interval;
  if(Intersection::RayAabb(rayStart,rayDir,minLocal,maxLocal,&interval) == Intersection::None)
  {
    interval.Min = 0;
    interval.Max = real(-1.0);
  }
  minT = interval.Min;
  maxT = interval.Max;


  Vec3 aabbMin = Vec3(minLocal.x, minHeight, minLocal.y);
  Vec3 aabbMax = Vec3(maxLocal.x, maxHeight, maxLocal.y);
  //cast the local ray against full aabb and then limit to that range
  //(use the interval test because the point test can return p0 = p1 when the ray starts inside the aabb
  if(Intersection::RayAabb(localRayStart, localRayDir, aabbMin, aabbMax, &interval) != Intersection::None)
  {
    Vec3 p0 = localRayStart + (localRayDir * interval.Min);
    Vec3 p1 = localRayStart + (localRayDir * interval.Max);
    Vec2 projectedPoint0 = Vec2(p0.x, p0.z);
    Vec2 projectedPoint1 = Vec2(p1.x, p1.z);
    real projectedMinT = (projectedPoint0 - rayStart).Length() / rayDir.Length();
    real projectedMaxT = (projectedPoint1 - rayStart).Length() / rayDir.Length();
    minT = Math::Max(minT, projectedMinT);
    maxT = Math::Min(maxT, projectedMaxT);
  }
}

void HeightMapRayRange::PopFront()
{
  //advance to the next triangle index and then make sure
  //that we get to a valid triangle
  ++mTriangleIndex;
  LoadUntilValidTriangles();
}

bool HeightMapRayRange::Empty() const
{
  //done when out of patches (having no patches implies having no cells)
  return mPatchRange.Empty();
}

HeightMapRayRange::TriangleInfo& HeightMapRayRange::Front()
{
  return mTriangleData[mTriangleIndex];
}

bool HeightMapRayRange::TrianglesToProcess()
{
  //Determine whether or not we have any triangles left to process in this cell.
  //Obviously, if we have no triangles we're done with this cell.
  if(mTriangleCount == 0)
    return false;

  //otherwise, if we ever reach the index greater than the number of
  //triangles we have, then we don't have any to process
  if(mTriangleIndex >= mTriangleCount)
    return false;

  return true;
}

void HeightMapRayRange::LoadUntilValidTriangles()
{
  //if there are no triangles in this cell to process, we need to load the next
  //cell until we either get valid triangles or until we reach the end of the range
  while(!TrianglesToProcess() && !Empty())
  {
    mTriangleIndex = 0;
    LoadNext();
    //for the debug drawing tool. It's nice to draw all cells,
    //not just the ones with triangles we collide with
    if(mSkipNonCollidingCells == false)
      break;
  }
}

void HeightMapRayRange::LoadNext()
{
  //get the next cell, if there is not another cell then we must go to the next patch
  mCellRange.PopFront();
  if(mCellRange.Empty())
  {
    //get the next patch, if we run out of patches then we are done (our range is now empty)
    mPatchRange.PopFront();
    if(mPatchRange.Empty())
      return;
    //we had a valid patch, set up our cell range for the new patch
    mCellRange.Set(mMap,mPatchRange.Front(),mProjectedRayStart,mProjectedRayDir,mMaxT);
  }
  //we have a new cell to process, load the triangles from it
  LoadTriangles();
}

void HeightMapRayRange::LoadTriangles()
{
  ReturnIf(mPatchRange.Empty(),, "The patch range for HeightMapRayRange was empty and it never should be");
  // This isn't actually an error when you look straight up-down and the maxT value is 0
  if(mCellRange.Empty())
    return;

  PatchIndex patchIndex = mPatchRange.Front();
  CellIndex cellIndex = mCellRange.Front();

  Vec2 cellPos = mCellRange.GetCurrentCellCenter();
  real cellSizeScalar = mMap->mUnitsPerPatch / HeightPatch::Size;
  Vec2 cellSize = Vec2(cellSizeScalar,cellSizeScalar) * .5f;

  //get the heights of the four corners of the cell
  real h00 = mMap->SampleHeight(mMap->GetAbsoluteIndex(patchIndex, cellIndex + CellIndex(0,0)), Math::cInfinite);
  real h01 = mMap->SampleHeight(mMap->GetAbsoluteIndex(patchIndex, cellIndex + CellIndex(0,1)), Math::cInfinite);
  real h10 = mMap->SampleHeight(mMap->GetAbsoluteIndex(patchIndex, cellIndex + CellIndex(1,0)), Math::cInfinite);
  real h11 = mMap->SampleHeight(mMap->GetAbsoluteIndex(patchIndex, cellIndex + CellIndex(1,1)), Math::cInfinite);

  //create local vertices for the four corners of the cell
  Vec3 p00 = Vec3(cellPos.x - cellSize.x,h00,cellPos.y - cellSize.y);
  Vec3 p01 = Vec3(cellPos.x - cellSize.x,h01,cellPos.y + cellSize.y);
  Vec3 p10 = Vec3(cellPos.x + cellSize.x,h10,cellPos.y - cellSize.y);
  Vec3 p11 = Vec3(cellPos.x + cellSize.x,h11,cellPos.y + cellSize.y);

  //edge triangles and holes are defined by vertices set to infinite, so determine if any of the vertices are infinite
  bool b01 = h01 != Math::cInfinite;
  bool b00 = h00 != Math::cInfinite;
  bool b10 = h10 != Math::cInfinite;
  bool b11 = h11 != Math::cInfinite;

  //see if any vertices are bad on either triangle
  bool tri1Valid = b01 && b00 && b10;
  bool tri2Valid = b01 && b10 && b11;

  //set the intersection results to be none by default
  Intersection::Type type1, type2;
  type1 = type2 = Intersection::None;

  //intersect the non-projected ray with both triangles (can optimize later by
  //performing the remaining aabb check against the ray, aka check the height
  //values to see if the ray hits). If the triangle was invalid then don't test
  //(it's set to None so the remaining checks will work)
  real epsilon = real(0.001);
  Intersection::IntersectionPoint point1,point2;
  if(tri1Valid)
    type1 = Intersection::RayTriangle(mLocalRayStart,mLocalRayDir,p00,p01,p10,&point1,epsilon);
  if(tri2Valid)
    type2 = Intersection::RayTriangle(mLocalRayStart,mLocalRayDir,p10,p01,p11,&point2,epsilon);

  //we need to determine how many of the triangles were actually hit,
  //store them, and in t first order.
  if(type1 != Intersection::None)
  {
    mTriangleData[0].mLocalTri = Triangle(p00,p01,p10);
    mTriangleData[0].mIntersectionInfo = point1;

    if(type2 != Intersection::None)
    {
      mTriangleData[1].mLocalTri = Triangle(p10,p01,p11);
      mTriangleData[1].mIntersectionInfo = point2;
      mTriangleCount = 2;

      if(point2.T < point1.T)
        Math::Swap(mTriangleData[0],mTriangleData[1]);
    }
    else
      mTriangleCount = 1;
  }
  else if(type2 != Intersection::None)
  {
    mTriangleData[1].mLocalTri = Triangle(p10,p01,p11);
    mTriangleData[1].mIntersectionInfo = point2;
    mTriangleCount = 2;
    mTriangleIndex = 1;
  }
  else
    mTriangleCount = 0;
}

HeightMapAabbRange::HeightMapAabbRange()
{
  mSkipNonCollidingCells = true;
}

void HeightMapAabbRange::SetLocal(HeightMap* map, const Aabb& aabb, real thickness)
{
  mMap = map;
  mThickness = thickness;

  mLocalAabbMin = aabb.mMin;
  mLocalAabbMax = aabb.mMax;
  SetUp();
}

void HeightMapAabbRange::SetWorld(HeightMap* map, const Aabb& aabb, real thickness)
{
  mMap = map;
  mThickness = thickness;

  //we need to turn the world aabb into a local one. There may be a faster way
  //to do this, but this way works (previous method didn't work with rotation
  //and scale due to sheer terms showing up during decomposition of the inverse matrix.
  //The previous method was to get the obb of the rotated aabb, then take the aabb
  //of that. Decomposition seems to be the problem).

  //compute all 8 aabb points
  Vec3 aabbPoints[8];
  aabbPoints[0] = Vec3(aabb.mMin.x,aabb.mMin.y,aabb.mMin.z);
  aabbPoints[1] = Vec3(aabb.mMax.x,aabb.mMin.y,aabb.mMin.z);
  aabbPoints[2] = Vec3(aabb.mMin.x,aabb.mMax.y,aabb.mMin.z);
  aabbPoints[3] = Vec3(aabb.mMax.x,aabb.mMax.y,aabb.mMin.z);
  aabbPoints[4] = Vec3(aabb.mMin.x,aabb.mMin.y,aabb.mMax.z);
  aabbPoints[5] = Vec3(aabb.mMax.x,aabb.mMin.y,aabb.mMax.z);
  aabbPoints[6] = Vec3(aabb.mMin.x,aabb.mMax.y,aabb.mMax.z);
  aabbPoints[7] = Vec3(aabb.mMax.x,aabb.mMax.y,aabb.mMax.z);
  //bring all of the points back into local space
  Mat4 worldInv = mMap->mTransform->GetWorldMatrix().Inverted();
  for(uint i = 0; i < 8; ++i)
    aabbPoints[i] = Math::TransformPoint(worldInv,aabbPoints[i]);
  //get the aabb of the local space obb
  Aabb localAabb;
  localAabb.Compute(aabbPoints,8);

  //store the local aabb
  mLocalAabbMin = localAabb.mMin;
  mLocalAabbMax = localAabb.mMax;

  SetUp();
}

void HeightMapAabbRange::SetUp()
{
  mTriangleIndex = 0;
  mTriangleCount = 0;
  //project the aabb onto the height map plane (remove y axis)
  mProjAabbMin = Vec2(mLocalAabbMin.x,mLocalAabbMin.z);
  mProjAabbMax = Vec2(mLocalAabbMax.x,mLocalAabbMax.z);
  //convert to patch indices
  mMinPatch = mMap->GetPatchIndexFromLocal(mProjAabbMin);
  mMaxPatch = mMap->GetPatchIndexFromLocal(mProjAabbMax);

  //we start at the min patch, cache the current patch we are using too
  mCurrentTriangle.mPatchIndex = mMinPatch;
  mCurrentPatch = mMap->GetPatchAtIndex(mCurrentTriangle.mPatchIndex);

  //skip all dead patch and load the first set of triangles to get ready
  SkipDeadPatches();
  //if after skipping all dead patches we still don't have a
  //valid patch then don't try to load any triangles
  if(mCurrentPatch != NULL)
    LoadTriangles();
  //for debug drawing, we want to draw even the cells we are skipping, but
  //otherwise skip all of the starting cells we don't even touch
  if(mSkipNonCollidingCells)
    SkipDeadCells();
}

void HeightMapAabbRange::PopFront()
{
  //go to the next triangle and skip all dead cells (if we ran out of
  //triangles, skipping dead cells moves to the next triangle)
  ++mTriangleIndex;
  SkipDeadCells();
}

bool HeightMapAabbRange::Empty() const
{
  //we're done when we exceed the max y patch value
  return mCurrentTriangle.mPatchIndex.y > mMaxPatch.y;
}

HeightMapAabbRange::TriangleInfo& HeightMapAabbRange::Front()
{
  mCurrentTriangle.mLocalTri = mTriangles[mTriangleIndex];
  return mCurrentTriangle;
}

void HeightMapAabbRange::GetCellIndices()
{
  //get the center, size and start (bottom left) of this patch
  Vec2 patchPos = mMap->GetLocalPosition(mCurrentTriangle.mPatchIndex);
  Vec2 patchHalfExtents = Vec2(mMap->mUnitsPerPatch,mMap->mUnitsPerPatch) * .5f;
  Vec2 patchStart = patchPos - patchHalfExtents;
  Vec2 patchEnd = patchPos + patchHalfExtents;
  //convert the aabb positions to the coordinate frame of the patch
  Vec2 localMin = mProjAabbMin - patchStart;
  Vec2 localMax = mProjAabbMax - patchStart;

  //now convert that position to a cell index and clamp between
  //our min and max bounds for a cell (to deal with floating point)
  real cellSizeScalar = mMap->mUnitsPerPatch / HeightPatch::Size;
  mMinCell.x = Math::Clamp((int)Math::Floor(localMin.x / cellSizeScalar),0,31);
  mMinCell.y = Math::Clamp((int)Math::Floor(localMin.y / cellSizeScalar),0,31);
  mMaxCell.x = Math::Clamp((int)Math::Floor(localMax.x / cellSizeScalar),0,31);
  mMaxCell.y = Math::Clamp((int)Math::Floor(localMax.y / cellSizeScalar),0,31);
  //set the starting value to be the min
  mCurrentTriangle.mCellIndex = mMinCell;
}

void HeightMapAabbRange::GetNextPatch()
{
  //get the next column, if we exceed our max get the next row
  ++mCurrentTriangle.mPatchIndex.x;
  if(mCurrentTriangle.mPatchIndex.x > mMaxPatch.x)
  {
    mCurrentTriangle.mPatchIndex.x = mMinPatch.x;
    ++mCurrentTriangle.mPatchIndex.y;
  }
  //now cache the patch at the new index
  mCurrentPatch = mMap->GetPatchAtIndex(mCurrentTriangle.mPatchIndex);
}

void HeightMapAabbRange::SkipDeadPatches()
{
  //as longs as we don't have a valid patch, get the next one
  //and deal with running out of patches to check
  while(mCurrentPatch == NULL)
  {
    GetNextPatch();
    if(Empty())
      return;

    //if we are debug drawing, we want to stop so we can draw
    //every patch we touch (even if it doesn't exist)
    if(!mSkipNonCollidingCells)
    {
      //set some valid values so we won't crash trying to index junk
      mTriangleCount = 0;
      mTriangleIndex = 1;
      GetCellIndices();
      return;
    }
  }
  //since we got a valid cell now, load the valid range of cells
  GetCellIndices();
}

void HeightMapAabbRange::LoadTriangles()
{
  PatchIndex patchIndex = mCurrentTriangle.mPatchIndex;
  CellIndex cellIndex = mCurrentTriangle.mCellIndex;

  //get the center, size and start (bottom left) of this patch
  Vec2 patchPos = mMap->GetLocalPosition(patchIndex);
  Vec2 patchHalfExtents = Vec2(mMap->mUnitsPerPatch,mMap->mUnitsPerPatch) * .5f;
  Vec2 patchStart = patchPos - patchHalfExtents;
  //To get a cell center, we have to start at the patch bottom left, offset by
  //the cell index scaled by each cell size. This gets us the bottom left of a cell,
  //to get the center we then have to offset by half a cell.
  real cellSizeScalar = mMap->mUnitsPerPatch / HeightPatch::Size;
  Vec2 cellSize = Vec2(cellSizeScalar,cellSizeScalar) * .5f;
  Vec2 currCell = Vec2((float)cellIndex.x,(float)cellIndex.y);
  Vec2 cellStart = patchStart + currCell * cellSizeScalar;
  Vec2 cellCenter = cellStart + cellSize;

  Vec2 cellPos = cellCenter;


  //get the heights of the four corners of the cell
  real h00 = mMap->SampleHeight(mMap->GetAbsoluteIndex(patchIndex, cellIndex + CellIndex(0,0)), Math::cInfinite);
  real h01 = mMap->SampleHeight(mMap->GetAbsoluteIndex(patchIndex, cellIndex + CellIndex(0,1)), Math::cInfinite);
  real h10 = mMap->SampleHeight(mMap->GetAbsoluteIndex(patchIndex, cellIndex + CellIndex(1,0)), Math::cInfinite);
  real h11 = mMap->SampleHeight(mMap->GetAbsoluteIndex(patchIndex, cellIndex + CellIndex(1,1)), Math::cInfinite);

  //create local vertices for the four corners of the cell
  Vec3 p00 = Vec3(cellPos.x - cellSize.x,h00,cellPos.y - cellSize.y);
  Vec3 p01 = Vec3(cellPos.x - cellSize.x,h01,cellPos.y + cellSize.y);
  Vec3 p10 = Vec3(cellPos.x + cellSize.x,h10,cellPos.y - cellSize.y);
  Vec3 p11 = Vec3(cellPos.x + cellSize.x,h11,cellPos.y + cellSize.y);

  //edge triangles and holes are defined by vertices set to infinite, so determine if any of the vertices are infinite
  bool b01 = h01 != Math::cInfinite;
  bool b00 = h00 != Math::cInfinite;
  bool b10 = h10 != Math::cInfinite;
  bool b11 = h11 != Math::cInfinite;

  mTriangleIndex = 0;
  //we need to determine how many of the triangles were actually hit,
  //store them, and in t first order. Also, if any triangle is invalid
  //we should skip it (has an infinite point).
  if(b01 && b00 && b10)
  {
    mTriangles[0] = Triangle(p00,p01,p10);

    if(b01 && b10 && b11)
    {
      mTriangles[1] = Triangle(p10,p01,p11);
      mTriangleCount = 2;
    }
    else
      mTriangleCount = 1;
  }
  else if(b01 && b10 && b11)
  {
    mTriangles[0] = Triangle(p10,p01,p11);
    mTriangleCount = 1;
  }
  else
  {
    mTriangleCount = 0;
    return;
  }

  if(!TrianglesWorthChecking())
    mTriangleCount = 0;
}

bool HeightMapAabbRange::TrianglesToProcess()
{
  return !(mTriangleIndex >= mTriangleCount);
}

bool HeightMapAabbRange::TrianglesWorthChecking()
{
  //since this only uses the heights, load triangles could be changed to only
  //read the height values and store the triangle heights and not actually
  //construct all of the triangle info

  //perform the remaining aabb check (compute the min/max y values and compare
  //them against the local aabb). No point in returning the triangles for
  //testing if they aren't even in the aabb
  float maxY,minY;
  maxY = minY = mTriangles[0].p0.y;

  maxY = Math::Max(mTriangles[0].p1.y,maxY);
  maxY = Math::Max(mTriangles[0].p2.y,maxY);
  if(mTriangleCount > 1)
  {
    maxY = Math::Max(mTriangles[1].p0.y,maxY);
    maxY = Math::Max(mTriangles[1].p1.y,maxY);
    maxY = Math::Max(mTriangles[1].p2.y,maxY);
  }

  minY = Math::Min(mTriangles[0].p1.y,minY);
  minY = Math::Min(mTriangles[0].p2.y,minY);
  if(mTriangleCount > 1)
  {
    minY = Math::Min(mTriangles[1].p0.y,minY);
    minY = Math::Min(mTriangles[1].p1.y,minY);
    minY = Math::Min(mTriangles[1].p2.y,minY);
  }

  if(minY - mThickness > mLocalAabbMax.y || maxY < mLocalAabbMin.y)
    return false;
  return true;
}

void HeightMapAabbRange::GetNextCell()
{
  //get the next cell along the x axis, if we overflow go to the next row
  ++mCurrentTriangle.mCellIndex.x;
  if(mCurrentTriangle.mCellIndex.x > mMaxCell.x)
  {
    mCurrentTriangle.mCellIndex.x = mMinCell.x;
    ++mCurrentTriangle.mCellIndex.y;
    //if we overflow past our last row, get the next valid patch
    if(mCurrentTriangle.mCellIndex.y > mMaxCell.y)
    {
      GetNextPatch();
      SkipDeadPatches();
    }
  }
  //we now have a valid cell, load the triangle heights and
  //figure out if any triangles are even valid
  LoadTriangles();
}

void HeightMapAabbRange::SkipDeadCells()
{
  //a cell is dead if it has no triangles to process, so as long as we have
  //no triangles keep getting the next cell (as long as we don't empty out)
  while(!TrianglesToProcess() && !Empty())
  {
    GetNextCell();
    mTriangleIndex = 0;
    //for debug drawing, we want to see every cell that is touched, so stop here
    if(!mSkipNonCollidingCells)
      return;
  }
}

void HeightMap::SaveToObj(StringParam fileName, HeightMap* heightMap)
{
  // Attempt to open the file
  File file;
  bool opened = file.Open(fileName.c_str(), FileMode::Write, FileAccessPattern::Sequential);

  // Can't do anything if it didn't open
  if(!opened)
    return;

  // Turn off smoothing groups
  String sOff = "s off\n";
  file.Write((byte*)sOff.Data(), sOff.SizeInBytes());

  // We want to transform the points by our owners transform
  Mat4 worldTransform = heightMap->GetOwner()->has(Transform)->GetWorldMatrix();

  // Used to name the patches
  uint patchIndex = 0;

  // Used to offset the indices from each patch
  uint indexOffset = 0;

  // Save out each patch
  forRange(PatchMap::value_type patchPair, heightMap->mPatches.All())
  {
    HeightPatch* patch = patchPair.second;

    // Generate the vertices for the current patch
    Array<Vec3> vertices;
    heightMap->GetHeightPatchVertices(patch, vertices);

    // Save each vertex to the file
    for (uint i = 0; i < vertices.Size(); ++i)
    {
      Vec3& vertex = vertices[i];

      // Transform the position to world
      Vec3 pos = Math::TransformPoint(worldTransform, vertex);

      // Write it to the file
      String vertexLine = String::Format("v %f %f %f\n", pos.x, pos.y, pos.z);
      file.Write((byte*)vertexLine.Data(), vertexLine.SizeInBytes());
    }

    // Write out the current patch
    String patchLine = String::Format("\ng Patch_%i\n", patchIndex);
    file.Write((byte*)patchLine.Data(), patchLine.SizeInBytes());

    // Generate the indices for triangles
    Array<uint> indices;
    heightMap->GenerateIndices(indices, 0);

    for (uint i = 0; i < indices.Size() / 3; ++i)
    {
      // Get the indices for this triangle
      uint i0 = indices[(i * 3) + 0];
      uint i1 = indices[(i * 3) + 1];
      uint i2 = indices[(i * 3) + 2];

      // We want to ignore any triangles that have vertices at infinite height
      // (this covers the seam cases)
      if (vertices[i0].y == Math::cInfinite)
        continue;
      if (vertices[i1].y == Math::cInfinite)
        continue;
      if (vertices[i2].y == Math::cInfinite)
        continue;

      // Write the triangle out to the file
      // + 1 is because the .obj file format isn't 0 based
      String triLine = String::Format("f %i %i %i\n",
        i0 + 1 + indexOffset,
        i1 + 1 + indexOffset,
        i2 + 1 + indexOffset);
      file.Write((byte*)triLine.Data(), triLine.SizeInBytes());
    }

    // Offset the indices for the next patch
    indexOffset += vertices.Size();

    // Move on to the next patch
    ++patchIndex;
  }

  // Close and save the file
  file.Close();
}

}//namespace Zero
