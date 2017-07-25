///////////////////////////////////////////////////////////////////////////////
///
/// \file MarchingCubes.hpp
/// Declaration of the MarchingCubes class.
/// 
/// Authors: Trevor Sundberg
/// Copyright 2011-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//----------------------------------------------------------------- Edge Indices
struct EdgeIndicies
{
  s32 P1;
  s32 P2;
};

//------------------------------------------------------------------------- Cell
struct Cell
{
  s32 x;
  s32 y;
  s32 z;
};

//-------------------------------------------------------------------- Cell Info
struct CellInfo : public Cell
{
  Vec3 p[8];
};

//---------------------------------------------------------------- Triangle Info
struct TriangleInfo
{
  TriangleInfo() { numTrianglesWritten = 0; }

  u8 numTrianglesWritten;
  u32 firstIndices[5];
};

//----------------------------------------------------------- Hash Policy (Vec3)
template<>
struct HashPolicy<Vec3>
{
  inline size_t operator()(Vec3Param value) const
  {
    return HashUint(*(unsigned int*)&value.x) +
           HashUint(*(unsigned int*)&value.y) +
           HashUint(*(unsigned int*)&value.z);
  }
  inline bool Equal(Vec3Param left, Vec3Param right) const
  {
    return left == right;
  }
};

//----------------------------------------------------------- Hash Policy (Cell)
template<>
struct HashPolicy<Cell>
{
  inline size_t operator()(const Cell& cell) const
  {
    return HashUint((unsigned)cell.x) +
           HashUint((unsigned)cell.y) +
           HashUint((unsigned)cell.z);
  }
  inline bool Equal(const Cell& left, const Cell& right) const
  {
    return left.x == right.x && left.y == right.y && left.z == right.z;
  }
};

//--------------------------------------------------------------- Marching Cubes
class MarchingCubes
{
public:
  typedef float (*DensitySamplerFn)(Vec3Param position, void* userData);
  typedef void (*VertexWriterFn)(Vec3 positions[], Vec3 normals[],
                                 u32 firstIndicesOut[], u32 numTriangles, void* userData);
  typedef void (*InvalidatorFn)(u32 firstIndices[], u32 numTriangles, void* userData);

  float SurfaceLevel;
  Vec3 SampleDistances;
  void* UserData;

  DensitySamplerFn DensitySampler;
  VertexWriterFn VertexWriter;
  InvalidatorFn Invalidator;

  float NormalSampleDelta;

  MarchingCubes();

  void GenerateMesh();

  void AddInvalidatedCell(s32 x, s32 y, s32 z);

  // Clear the invalidated cells, as well as the stored triangle information
  void ClearHistory();

private:

  HashSet<CellInfo, HashPolicy<Cell> > InvalidatedCells;

  HashMap<Vec3, float> Values;

  HashMap<Cell, TriangleInfo, HashPolicy<Cell> > TriangleInfos;

  Vec3 InterpolatedVertices[12];
  u32 EdgeFlags;

  HashMap<Vec3, Vec3> CachedNormals;


  // This table is indexed by a special bit-masked index known as the 'cube-index'
  // It's purpose is to define which edges are being split by the iso-surface
  // Each bit in this array of 12-bit numbers represents which edges get split (1 for split, 0 for not)
  static u32 EdgeTable[256];

  static s32 TriangleTable[256][16];

  static EdgeIndicies EdgeIndexOffsets[12];

  s32 IsVertexInsideSurface(s32 index, s32 x, s32 y, s32 z);

  void GenerateCell(CellInfo& cell);

  Vec3 SampleNormal(Vec3 position);

  void ComputeInterpolatedVertexForEdge(s32 edgeIndex, CellInfo& cell);

  static Vec3 Interpolate(float surfaceLevel, Vec3 p1, Vec3 p2, float val1, float val2);
};

} // namespace Zero
