///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Ryan Edgemon
/// Copyright 2016-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.hpp"


namespace Zero
{


//---------------------------------------------------- ModifiedHeightMapCell ---

/******************************************************************************/
ModifiedHeightMapCell::ModifiedHeightMapCell(const HeightMapCell& cell, float originalHeight, float height)
{
  PatchIndex = cell.Patch->Index;

  Index = cell.Index;
  OriginalHeight = originalHeight;
  AppliedHeight = height - originalHeight;
}

/******************************************************************************/
void ModifiedHeightMapCell::Set(const HeightMapCell& cell, float originalHeight, float height)
{
  PatchIndex = cell.Patch->Index;

  Index = cell.Index;
  OriginalHeight = originalHeight;
  AppliedHeight = height - originalHeight;
}


//-------------------------------------------------------- HeightMapUndoRedo ---

/******************************************************************************/
HeightMapUndoRedo::HeightMapUndoRedo(HeightMap* heightMap, StringParam name)
{
  mName = name;

  mHeightMap = heightMap;
  mHeightMapObject = heightMap->GetOwner();
}

/******************************************************************************/
void HeightMapUndoRedo::SetAABB(const HeightMapCellRange& cells)
{
  mMin = cells.mAabbMin;
  mMax = cells.mAabbMax;
}

/******************************************************************************/
void HeightMapUndoRedo::UpdateAABB(const HeightMapCellRange& additionalCells)
{
  mMin = Math::Min(additionalCells.mAabbMin, mMin);
  mMax = Math::Max(additionalCells.mAabbMax, mMax);
}

/******************************************************************************/
void HeightMapUndoRedo::AddCell(const HeightMapCell& cell, float preDeltaHeight, float height)
{
  AbsoluteIndex index = mHeightMap->GetAbsoluteIndex(cell.Patch->Index, cell.Index);
  YAxisCells::range result = mModifiedCells[index.x].Find(index.y);

  if(!result.Empty( ))
    result.Front( ).second.AppliedHeight = height - result.Front( ).second.OriginalHeight;
  else
    mModifiedCells[index.x][index.y].Set(cell, preDeltaHeight, height);
}

/******************************************************************************/
void HeightMapUndoRedo::ApplyHeightHelper(int useAppliedHeight)
{
  HeightMap* map = *mHeightMap;

  forRange(YAxisCells yAxis, mModifiedCells.Values( ))
  {
    forRange(ModifiedHeightMapCell cell, yAxis.Values( ))
    {
      float& height = map->GetPatchAtIndex(cell.PatchIndex)->GetHeight(cell.Index);
      height = cell.OriginalHeight + useAppliedHeight * cell.AppliedHeight;
    }

  }

  Vec2 position = mMax - mMin;
  position = mMin + position / 2.0f;

  HeightMapCellRange cells(map, position, (mMax.x - mMin.x) / 2.0f, 0.0f); 
  cells.SignalPatchesModified();
}

/******************************************************************************/
void HeightMapUndoRedo::Undo( )
{
  ApplyHeightHelper(0);
}

/******************************************************************************/
void HeightMapUndoRedo::Redo( )
{
  ApplyHeightHelper(1);
}


//------------------------------------------------------ HeightPatchUndoRedo ---

/******************************************************************************/
HeightPatchUndoRedo::HeightPatchUndoRedo(HeightMap* heightMap, StringParam name)
{
  mName = name;

  mUsePerlinNoise = true;
  mBaseHeight = 0.0f;
  mPerlinFrequency = 1.0f;
  mPerlinAmplitude = 10.0f;

  mHeightMapObject = heightMap->GetOwner();
}

/******************************************************************************/
void HeightPatchUndoRedo::AddPatch(bool create, PatchIndexParam index)
{
  mPatches.PushBack(ModifiedPatch(create, index));
}

/******************************************************************************/
void HeightPatchUndoRedo::SetNoise(bool usePerlin, float baseHeight, float frequency, float amplitude)
{
  mUsePerlinNoise = usePerlin;
  mBaseHeight = baseHeight;
  mPerlinFrequency = frequency;
  mPerlinAmplitude = amplitude;
}

/******************************************************************************/
void HeightPatchUndoRedo::Create(PatchIndex& index)
{
  Cog* object = mHeightMapObject;
  HeightMap* map = object->has(HeightMap);
  HeightPatch* patch = map->CreatePatchAtIndex(index);

  if(mUsePerlinNoise)
  {
    map->ApplyNoiseToPatch(patch, mBaseHeight, mPerlinFrequency, mPerlinAmplitude);
  }
  else
  {
    for(size_t i = 0; i < HeightPatch::TotalSize; ++i)
      patch->Heights[i] = mBaseHeight;
  }

  map->SignalPatchModified(patch);
}

/******************************************************************************/
void HeightPatchUndoRedo::Destroy(PatchIndex& index)
{
  Cog* object = mHeightMapObject;
  object->has(HeightMap)->DestroyPatchAtIndex(index);
}

/******************************************************************************/
void HeightPatchUndoRedo::Undo( )
{
  forRange(ModifiedPatch& i, mPatches.All( ))
  {
    if(i.Create)
      Destroy(i.Index);
    else
      Create(i.Index);
  }
}

/******************************************************************************/
void HeightPatchUndoRedo::Redo( )
{
  forRange(ModifiedPatch& i, mPatches.All( ))
  {
    if(i.Create)
      Create(i.Index);
    else
      Destroy(i.Index);
  }

}

//--------------------------------------------------- ModifiedWeightMapPixel ---

/******************************************************************************/
ModifiedWeightMapPixel::ModifiedWeightMapPixel(PatchIndexParam index, uint x, uint y, ByteColor originalWeight, ByteColor weight)
{
  PatchIndex = index;

  Coord.Set(x, y);
  OriginalWeight = originalWeight;
  AppliedWeight = weight;
}

/******************************************************************************/
void ModifiedWeightMapPixel::Set(PatchIndexParam index, uint x, uint y, ByteColor originalWeight, ByteColor weight)
{
  PatchIndex = index;

  Coord.Set(x, y);
  OriginalWeight = originalWeight;
  AppliedWeight = weight;
}


//------------------------------------------------------ WeightPatchUndoRedo ---

/******************************************************************************/
WeightMapUndoRedo::WeightMapUndoRedo(HeightMap* heightMap, StringParam name)
{
  mName = name;

  mHeightMapObject = heightMap->GetOwner();
}

/******************************************************************************/
void WeightMapUndoRedo::AddPixel(PatchIndexParam index, uint x, uint y, ByteColor preDeltaWeight, ByteColor weight)
{
  YAxisPixels::range result = mModifiedPixels[x].Find(y);

  if(!result.Empty())
    result.Front().second.AppliedWeight = weight - result.Front( ).second.OriginalWeight;
  else
    mModifiedPixels[x][y].Set(index, x, y, preDeltaWeight, weight);
}

/******************************************************************************/
void WeightMapUndoRedo::ApplyWeightHelper(ByteColor useAppliedWeight)
{
  Cog* object = mHeightMapObject;

  HeightMap* map = object->has(HeightMap);
  HeightMapModel* model = object->has(HeightMapModel);

  HashSet<GraphicalHeightPatch*> buffers;
  
  forRange(YAxisPixels yAxis, mModifiedPixels.Values( ))
  {
    forRange(ModifiedWeightMapPixel pixel, yAxis.Values( ))
    {
      HeightPatch* patch = map->GetPatchAtIndex(pixel.PatchIndex);
      GraphicalHeightPatch* graphicalPatch = model->mGraphicalPatches.FindPointer(patch, nullptr);

      buffers.Insert(graphicalPatch);

      ByteColor weight = pixel.OriginalWeight + useAppliedWeight * pixel.AppliedWeight;
      graphicalPatch->mWeightTexture->SetPixel(pixel.Coord.x, pixel.Coord.y, weight);
    }

  }

  forRange(GraphicalHeightPatch* patch, buffers.All())
  {
    patch->mWeightTexture->Upload( );
  }

}

/******************************************************************************/
void WeightMapUndoRedo::Undo( )
{
  ApplyWeightHelper(0);
}

/******************************************************************************/
void WeightMapUndoRedo::Redo( )
{
  ApplyWeightHelper(1);
}

}//namespace Zero