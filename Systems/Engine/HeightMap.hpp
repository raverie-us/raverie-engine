///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg, Nathan Carlson, Ryan Edgemon
/// Copyright 2010-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{
// Forward declarations
class HeightMap;
struct HeightPatch;
struct HeightMapRayRange;
struct HeightMapAabbRange;
class HeightMapSource;

// Type-defines
typedef IntVec2       PatchIndex;
typedef IntVec2Param  PatchIndexParam;
typedef IntVec2       CellIndex;
typedef IntVec2Param  CellIndexParam;
typedef IntVec2       AbsoluteIndex;
typedef IntVec2Param  AbsoluteIndexParam;

//------------------------------------------------------------------- Events

namespace Events
{
  DeclareEvent(HeightMapPatchAdded);
  DeclareEvent(HeightMapPatchRemoved);
  DeclareEvent(HeightMapPatchModified);
  DeclareEvent(HeightMapSave);
}//namespace Events

/// Used by any height map event
struct HeightMapEvent : public Event
{
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// The height map
  HeightMap* Map;

  /// The associated patch (if it applies)
  HeightPatch* Patch;

  /// The height Map source to save to
  HeightMapSource* Source;
};

//------------------------------------------------------------------- HeightPatch

/// A large 2d block of height data
struct HeightPatch
{
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// The size of each patch (in height cells)
  static const size_t Size      = 32;
  static const size_t TotalSize = Size * Size;

  /// We actually have as many quads as we have height cells, though our vertices
  /// are placed in the middle of a height cell. This means we have extra quads, which
  /// we use to seam between patches (they stick out to the right and up)
  static const size_t NumQuadsPerSide     = Size;
  static const size_t NumQuadsTotal       = NumQuadsPerSide * NumQuadsPerSide;
  static const size_t NumVerticesPerSide  = (Size + 1);
  static const size_t NumVerticesTotal    = NumVerticesPerSide * NumVerticesPerSide;
  static const size_t NumTrianglesPerSide = NumQuadsPerSide * 2;
  static const size_t NumTrianglesTotal   = NumQuadsTotal * 2;
  static const size_t NumIndicesPerSide   = NumTrianglesPerSide * 3;
  static const size_t NumIndicesTotal     = NumTrianglesTotal * 3;

  static const size_t PaddedNumVerticesPerSide  = NumVerticesPerSide + 2;
  static const size_t PaddedNumVerticesTotal    = PaddedNumVerticesPerSide * PaddedNumVerticesPerSide;

  /// Constructor
  HeightPatch();

  /// Deep Copy Constructor
  HeightPatch(const HeightPatch& rhs);

  /// Sample the height patch (returns a reference so it can be modified)
  float& GetHeight(CellIndex index);

  /// Set the height of a given cell
  void SetHeight(CellIndex index, float height);

  /// An intrusive index into the height map
  PatchIndex Index;

  /// All the heights in this patch
  typedef float HeightValueType;
  HeightValueType Heights[TotalSize];
  HeightValueType MinHeight;
  HeightValueType MaxHeight;
};

//------------------------------------------------------------------- PatchMap

/// Type-defines
typedef Pair<PatchIndex, HeightPatch*> PatchMapPair;
typedef HashMap<PatchIndex, HeightPatch*> PatchMap;
typedef HashMap<PatchIndex, HeightPatch> PatchMapCopy;


//------------------------------------------------------------------- CellRange

struct HeightMapCell
{
  /// The index of the cell in a patch. Note that this is not necessarily related
  /// to a height in the patch
  CellIndex Index;

  /// Typically cells are returned in queries (many of those queries use feathering)
  /// This is the computed influence for this particular cell
  float Influence;

  /// The patch that this cell relates to. Note that the Index not necessarily related
  /// to a height in the patch
  HeightPatch* Patch;
};

class HeightMapCellRange
{
public:
  typedef HeightMapCell FrontResult;
  HeightMapCellRange(HeightMap* heightMap, Vec2 toolPosition, real radius, real feather);
  HeightMapCellRange(PatchMapCopy& patchMap, Vec2 toolPosition, real radius, real feather);

  void Reset( );

  HeightMapCell Front();
  void PopFront();
  bool Empty();

  void GetNextPatch();
  void SkipDeadPatches();
  void GetNextCell();
  void GetInfluence();

  void SignalPatchesModified();

  HeightMap* mHeightMap;
  HeightPatch* mPatch;
  Vec2 mToolPosition;
  Vec2 mAabbMin;
  Vec2 mAabbMax;
  Vec2 mPatchOffset;
  real mRadius;
  real mFeather;
  real mInfluence;
  real mCellSize;

  PatchIndex mPatchIndexMin;
  PatchIndex mPatchIndexMax;
  PatchIndex mPatchIndex;

  CellIndex mCellIndexMin;
  CellIndex mCellIndexMax;
  CellIndex mCellIndex;
};

//-------------------------------------------------------------------- HeightMap

/// A common class that represents height map data
class HeightMap : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  // Friends
  template <typename T>
  friend struct PatchRange;

  static const Vec3 UpVector;

  /// Constructor
  HeightMap();

  /// Destructor
  ~HeightMap();

  /// Component interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  /// Get source for height map
  HeightMapSource* GetSource();

  /// Gets/sets the number of units per patch
  float GetUnitsPerPatch();
  void SetUnitsPerPatch(float value);

  /// Updates all patches
  void SignalAllPatchesModified();

  /// Sample a height given an absolute index
  /// Absolute indices are determined using the PatchIndex * HeightPatch::Size + CellIndex
  float SampleHeight(AbsoluteIndexParam absoluteIndex, float defaultValue);

  /// Sample the height using a local space position
  float SampleHeight(Vec2Param localPosition, float defaultValue, Vec3* worldNormal = nullptr);

  /// Sample the height using a world space position
  float SampleHeight(Vec3Param worldPosition, float defaultValue, Vec3* worldNormal = nullptr);

  /// Get the height of a given point relative to the height map
  /// Note that this function does NOT sample the height map
  float GetWorldPointHeight(Vec3Param worldPosition);

  /// Get the world space up vector
  Vec3 GetWorldUp();

  /// Populates the given array with the vertex data that represents the patch
  /// Will generate the vertex data if it is not cached
  /// Adjacent vertices are padded on all sides for gradient calculations
  void GetPaddedHeightPatchVertices(HeightPatch* patch, Array<Vec3>& outVertices);
  // Non padded is only used by debug drawer currently
  void GetHeightPatchVertices(HeightPatch* patch, Array<Vec3>& outVertices);

  Aabb GetPatchLocalAabb(HeightPatch* patch);
  Aabb GetPatchAabb(HeightPatch* patch);
  void UpdatePatch(HeightPatch* patch);

  /// Generate the indices for a particular LOD set
  static void GenerateIndices(Array<uint>& outIndices, uint lod);

  /// Get the index for a particular world position
  PatchIndex GetPatchIndexFromWorld(Vec3Param worldPosition);

  /// Get the index for a particular local position
  PatchIndex GetPatchIndexFromLocal(Vec2Param localPosition);

  /// Get the absolute index for a particular world position
  AbsoluteIndex GetNearestAbsoluteIndexFromWorld(Vec3Param worldPosition);

  /// Get the absolute index for a particular local position
  AbsoluteIndex GetNearestAbsoluteIndexFromLocal(Vec2Param localPosition);
  AbsoluteIndex GetFlooredAbsoluteIndexFromLocal(Vec2Param localPosition);

  Vec2 GetLocalPositionFromAbsoluteIndex(AbsoluteIndexParam absoluteIndex);

  /// Converts the patch and cell index to an absolute index
  AbsoluteIndex GetAbsoluteIndex(PatchIndex patchIndex, CellIndex cellIndex);

  PatchIndex GetPatchIndex(AbsoluteIndexParam absoluteIndex);

  CellIndex GetCellIndexFromWorld(Vec3Param worldPosition);
  CellIndex GetCellIndexFromLocal(Vec2Param localPosition);
  CellIndex GetCellIndex(AbsoluteIndexParam absoluteIndex);
  CellIndex GetCellIndex(AbsoluteIndexParam absoluteIndex, PatchIndexParam patchIndex);

  void GetPatchAndCellIndex(AbsoluteIndexParam absoluteIndex, PatchIndex& patchIndex, CellIndex& cellIndex);

  /// Get the world position from a local position
  Vec3 GetWorldPosition(Vec2Param localPosition);

  /// Get the local position from a world position
  Vec2 GetLocalPosition(Vec3Param worldPosition);

  /// Get the world position
  Vec3 GetWorldPosition(PatchIndexParam index);

  /// Get the local position
  Vec2 GetLocalPosition(PatchIndexParam index);


  /// Create a patch at a particular index
  HeightPatch* CreatePatchAtIndex(PatchIndexParam index);

  /// Apply a noise function to a patch (generate terrain)
  void ApplyNoiseToPatch(HeightPatch* patch, float baseHeight, float frequency, float amplitude);

  /// Destroy a patch at a given index
  void DestroyPatchAtIndex(PatchIndexParam index);

  /// Get a patch at a particular index
  HeightPatch* GetPatchAtIndex(PatchIndexParam index);

  void GetQuadAtIndex(AbsoluteIndex index, Triangle triangles[2], uint& count);

  /// Signal that a particular patch was modified (typically updates physics, graphics, etc)
  void SignalPatchModified(HeightPatch* patch);
  void SignalPatchModified(HeightPatch* patch, Vec2 min, Vec2 max);

  /// Get all of the patches
  PatchMap::valuerange GetAllPatches();

  HeightMapRayRange CastLocalRay(const Ray& ray, float maxT = Math::PositiveMax());
  HeightMapRayRange CastWorldRay(const Ray& ray, float maxT = Math::PositiveMax());
  HeightMapAabbRange GetLocalAabbRange(const Aabb& aabb, real thickness);
  HeightMapAabbRange GetWorldAabbRange(const Aabb& aabb, real thickness);

  /// Save and load from SaveToHeightMapSource
  void SaveToHeightMapSource(Serializer& stream);
  void LoadFromHeightMapSource(Serializer& stream);
  void OnLevelLoaded(ObjectEvent* event);

  // Mark as modified for serialization and editor save detection
  void Modified();

private:
  static const CellIndex sCellIndexMin;
  static const CellIndex sCellIndexMax;

  /// Sends out a patch event
  void SendPatchEvent(StringParam eventType, HeightPatch* patch);

  /// Computes vertices for the whole patch
  void UpdatePatchVertices(HeightPatch* patch);

  /// Computes only the vertices in the bound given by min/max
  void UpdatePatchVertices(HeightPatch* patch, Vec2 min, Vec2 max);

  /// Updates adjacent patch vertices
  void UpdateAdjacentPatches(HeightPatch* patch);

  /// Pre-fetches all patch and adjacent patch heights that are needed to compute
  /// tangents and bitangents for this patch to prevent any hashes or branching when computing vertices
  void MakePaddedHeightBuffer(HeightPatch* patch, real* heights);

  /// Computes the patch's vertex data and stores it in outVertices
  /// outVertices must already be the correct size, it is assumed that not all vertices always need to be computed
  void ComputePaddedHeightPatchVertices(HeightPatch* patch, Array<Vec3>& outVertices, CellIndex min = sCellIndexMin, CellIndex max = sCellIndexMax);

public:
  friend struct CellRayRange;
  friend struct PatchRayRange;
  friend struct HeightMapRayRange;
  friend struct HeightMapAabbRange;

  /// A pointer to the transform of the height map
  Transform* mTransform;

  /// The units per patch (basically the size in world if the scale is [1,1,1])
  float mUnitsPerPatch;

  /// A global map of all height patches
  PatchMap mPatches;

  /// The Modified Flag for editing
  bool mModified;

  /// Source for height map data
  HandleOf<HeightMapSource> mSource;

  HashMap< PatchIndex, Array<Vec3> > mCachedPatchVertices;

public:
  static void SaveToObj(StringParam fileName, HeightMap* heightMap);
};

/// A range that iterates through the cells hit by a ray on a given patch. Used
/// during actual raycasting to determine which cells to check the triangles
/// against the ray. Note: the cell position used is not necessarily where the
/// height map defines a cell. The map defines a cell as the vertex, this range
/// and raycasting use the center of the cell.
struct CellRayRange
{
  CellRayRange() {};
  CellRayRange(HeightMap* map, PatchIndex index, Vec2Param rayStart, Vec2Param rayDir, real maxT);

  void Set(HeightMap* map, PatchIndex index, Vec2Param rayStart, Vec2Param rayDir, real maxT);

  Vec2 GetCurrentCellCenter();
  Vec2 GetNextTValues();

  /// Range interface
  void PopFront();
  CellIndex Front();
  bool Empty();

  //static because both the cell and patch range need this function
  static Vec2 IntersectPlane(Vec2Param planeDistance, Vec2Param rayStart, Vec2Param rayDir);

private:
  Vec2 mRayStart;
  Vec2 mRayDir;
  int mStepX,mStepY;
  real mMaxT;
  real mCurrT;
  Vec2 mPatchStart;

  CellIndex mCellIndex;
  PatchIndex mPatchIndex;
  HeightMap* mMap;
};

struct HeightMapQueryCache
{
  HeightMapQueryCache();
  float SampleHeight(HeightMap* map, PatchIndexParam patchIndex, CellIndexParam cellIndex);

  HeightPatch* mCachedPatch;
};

/// A range to iterate through all of the patches that a ray hits. Used during
/// actual raycasting to determine which patches we should used the cell range on.
struct PatchRayRange
{
  PatchRayRange() {};
  PatchRayRange(HeightMap* map, Vec2Param rayStart, Vec2Param rayDir, real minT, real maxT);

  void Set(HeightMap* map, Vec2Param rayStart, Vec2Param rayDir, real minT, real maxT);
  /// Helper function to clear the range so it will return empty.
  void SetEmpty();

  /// Range interface
  void PopFront();
  bool Empty() const;
  PatchIndex Front();

private:
  void GetNextPatch();
  void SkipDeadPatches();

  PatchIndex mCurrPatchIndex;

  Vec2 mRayStart;
  Vec2 mRayDir;
  real mCurrT;
  real mMaxT;
  int mStepX;
  int mStepY;

  HeightMap* mMap;
};

/// A range for performing raycasting against a height map. Uses DDA to iterate
/// efficiently through only the patches and cells that the ray actually touches.
/// Returns a structure containing the triangle hit and the intersection
/// info about how it was hit.
struct HeightMapRayRange
{
  /// Data returned by this range. Contains the triangle
  /// hit and the intersection info about how.
  struct TriangleInfo
  {
    Triangle mLocalTri;
    Intersection::IntersectionPoint mIntersectionInfo;
  };

  HeightMapRayRange();

  void SetLocal(HeightMap* map, const Ray& ray, float maxT = Math::PositiveMax());
  void SetWorld(HeightMap* map, const Ray& ray, float maxT = Math::PositiveMax());
  void SetUp(float maxT);
  void GetTMinMaxRange(Vec3Param localRayStart, Vec3Param localRayDir,
                       Vec2Param rayStart, Vec2Param rayDir, float& minT, float& maxT);

  /// Range interface
  void PopFront();
  bool Empty() const;
  TriangleInfo& Front();

  bool TrianglesToProcess();
  void LoadUntilValidTriangles();
  void LoadNext();
  void LoadTriangles();

  /// The non projected ray, need for the triangle ray test
  Vec3 mLocalRayStart;
  Vec3 mLocalRayDir;
  /// The projected ray, needed to iterate through cells with
  Vec2 mProjectedRayStart;
  Vec2 mProjectedRayDir;
  real mMaxT;

  HeightMap* mMap;
  HeightMapQueryCache mPatchCache;
  PatchRayRange mPatchRange;
  CellRayRange mCellRange;

  /// Flag primarily for the debug draw tool
  bool mSkipNonCollidingCells;

  /// Data pertaining to how many triangles we have hit in a cell, where we hit
  /// them and which triangle we should return the next time front is called
  TriangleInfo mTriangleData[2];
  uint mTriangleIndex;
  uint mTriangleCount;
};

/// A range to iterate through all triangles that an aabb should consider
/// checking collision against. Projects the extents of the aabb onto the
/// grid to determine which patches and cells to even consider.
/// Returns a structure containing the local triangle hit and the patch and cell index.
struct HeightMapAabbRange
{
  struct TriangleInfo
  {
    Triangle mLocalTri;
    PatchIndex mPatchIndex;
    CellIndex mCellIndex;
  };

  HeightMapAabbRange();
  void SetLocal(HeightMap* map, const Aabb& aabb, real thickness);
  void SetWorld(HeightMap* map, const Aabb& aabb, real thickness);
  void SetUp();

  /// Range interface
  void PopFront();
  bool Empty() const;
  TriangleInfo& Front();

  void GetCellIndices();
  void GetNextPatch();
  void SkipDeadPatches();
  void LoadTriangles();
  bool TrianglesToProcess();
  bool TrianglesWorthChecking();
  void GetNextCell();
  void SkipDeadCells();

  HeightMap* mMap;
  bool mSkipNonCollidingCells;
  //the local and projected aabb info. The non-projected aabb is needed to determine
  //if a triangle can be skipped (performs the remaining aabb check)
  Vec3 mLocalAabbMin, mLocalAabbMax;
  Vec2 mProjAabbMin, mProjAabbMax;

  //the range of values that need to be searched through
  HeightPatch* mCurrentPatch;
  PatchIndex mMinPatch,mMaxPatch;
  CellIndex mMinCell,mMaxCell;

  uint mTriangleIndex;
  TriangleInfo mCurrentTriangle;
  uint mTriangleCount;
  Triangle mTriangles[2];

  // This is currently only a property on the collider, however this can't know
  // about the collider so it must be passed through. Maybe clean this up and
  // put thickness on the height map itself?
  real mThickness;
};

float FeatherInfluence(float distance, float radius, float featherRadius);

}//namespace Zero
