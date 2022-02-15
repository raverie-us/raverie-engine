// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

class HeightMapBrushStroke
{
public:
  ZilchDeclareType(HeightMapBrushStroke, TypeCopyMode::ReferenceType);

  HeightMapBrushStroke()
  {
  }
  HeightMapBrushStroke(float radius, float featherRadius);

  void AddPoint(Vec2Param point);
  // void Apply(HeightManipulationTool* tool, PatchMapCopy& patchMap);

private:
  float mRadius;
  float mFeatherRadius;

  Array<Vec2> mPoints;
};

class HeightMapKeyFrame
{
public:
  void CopyHeightMapData(HeightMap* map);

public:
  float mUnitsPerPatch;
  PatchMapCopy mPatches;

  // InList?
  Array<HeightMapBrushStroke*> mBrushStrokes;
};

class HeightMapStateManager
{
public:
  HeightMapStateManager()
  {
  }
  HeightMapStateManager(HeightMap* map);

  void StartBrushStroke(float radius, float featherRadius);
  void EndBrushStroke();

  void AddPointToStroke(Vec2Param point);

  void InitializeHeightMapState();

public:
  HeightMap* mHeightMap;

  HeightMapKeyFrame* mCurrentFrame;
  HeightMapBrushStroke* mCurrentStroke;

  // InList?
  Array<HeightMapKeyFrame*> mFrames;

private:
  uint mFramesBetweenKeys;
};

} // namespace Zero
