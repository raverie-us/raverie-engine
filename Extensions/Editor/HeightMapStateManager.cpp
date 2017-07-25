////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Ryan Edgemon
/// Copyright 2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////

#include "Precompiled.hpp"

namespace Zero
{

//----------------------------------------------------- HeightMapBrushStroke ---

ZilchDefineType(HeightMapBrushStroke, builder, type)
{
  ZilchBindConstructor();
  ZilchBindMethod(AddPoint);
}

/******************************************************************************/
HeightMapBrushStroke::HeightMapBrushStroke(float radius, float featherRadius)
  : mRadius(radius), mFeatherRadius(featherRadius)
{
}

/******************************************************************************/
void HeightMapBrushStroke::AddPoint(Vec2Param point)
{
  mPoints.PushBack(point);
}

/******************************************************************************/
// Should move this to Undo/Redo?
// Operation should know the Tool [ie, brush], HeightMap, and maybe keyframe
//void HeightMapBrushStroke::Apply(HeightManipulationTool* tool, PatchMapCopy& patchMap)
//{
//  forRange(Vec2& position, mPoints.All( ))
//  {
//    HeightMapCellRange range(patchMap, position, mRadius, mFeatherRadius);
//    //mTool->ApplyToCells(range, );
//  }
//
//}

//-------------------------------------------------------- HeightMapKeyFrame ---

/******************************************************************************/
void HeightMapKeyFrame::CopyHeightMapData(HeightMap* map)
{
  mUnitsPerPatch = map->mUnitsPerPatch;

  forRange(PatchMapPair& patch, map->mPatches.All( ))
  {
    mPatches[patch.first] = *patch.second;
  }

}

//---------------------------------------------------- HeightMapStateManager ---

/******************************************************************************/
HeightMapStateManager::HeightMapStateManager(HeightMap* map)
  : mHeightMap(map)
{
  mCurrentFrame = nullptr;
  mCurrentStroke = nullptr;
}

/******************************************************************************/
void HeightMapStateManager::StartBrushStroke(float radius, float featherRadius)
{
  mCurrentStroke = new HeightMapBrushStroke(radius, featherRadius);

  if(mCurrentFrame == nullptr)
  {
    mCurrentFrame = new HeightMapKeyFrame();
    mCurrentFrame->mBrushStrokes.PushBack(mCurrentStroke);

    mFrames.PushBack(mCurrentFrame);

    InitializeHeightMapState( );
  }
  else if(mCurrentFrame->mBrushStrokes.Size( ) == mFramesBetweenKeys)
  {
    // memory optimization: only store diff of HeightMap state now vs prev keyframe
    //   - note: currently copying out entire HeightMap state
    InitializeHeightMapState( );
  }

}

/******************************************************************************/
void HeightMapStateManager::EndBrushStroke( )
{
  mCurrentStroke = nullptr;
}

/******************************************************************************/
void HeightMapStateManager::AddPointToStroke(Vec2Param point)
{
  mCurrentStroke->AddPoint(point);
}

/******************************************************************************/
void HeightMapStateManager::InitializeHeightMapState( )
{
  mCurrentFrame->CopyHeightMapData(mHeightMap);
}

}//namespace Zero
