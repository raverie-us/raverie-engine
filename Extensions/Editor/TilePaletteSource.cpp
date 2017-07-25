///////////////////////////////////////////////////////////////////////////////
///
///  \file TilePaletteSource.cpp
///  Implementation of the TileMap resource.
///
///  Authors: Nathan Carlson
///  Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(TilePaletteSource, builder, type)
{
  ZeroBindDocumented();
}

TilePaletteSource::TilePaletteSource()
{
  mVersion = 0;
}

TilePaletteSource::~TilePaletteSource()
{
}

void TilePaletteSource::Save(StringParam filename)
{
  ChunkFileWriter file;
  file.Open(filename);
  TileMapSourceLoadPattern::Save(this, file);
}

void TilePaletteSource::Unload()
{
  mData.Clear();
}

IntVec2 TilePaletteSource::GetTileDimensions()
{
  // returns 1,1 to not divide by 0 for empty tile palette previews
  if (mData.Empty())
    return IntVec2(1, 1);

  TileMap::TileRange range = mData.All();
  int maxX, maxY;
  maxX = maxY = Math::IntegerNegativeMin();
  while (!range.Empty())
  {
    value_type tilePair = range.Front();
    range.PopFront();
    maxX = Math::Max(tilePair.first.x, maxX);
    maxY = Math::Max(tilePair.first.y, maxY);
  }
  // +1 as tiles are 0 indexed
  return IntVec2(maxX + 1, maxY + 1);
}

ImplementResourceManager(TilePaletteSourceManager, TilePaletteSource);

TilePaletteSourceManager::TilePaletteSourceManager(BoundType* resourceType)
  :ResourceManager(resourceType)
{
  this->mNoFallbackNeeded = true;
  mExtension = "bin";
  mCanAddFile = true;
  mOpenFileFilters.PushBack(FileDialogFilter("Tile Palette (*.bin)", "*.bin"));
  AddLoader("TilePaletteSource", new ChunkFileLoader<TilePaletteSourceManager, TileMapSourceLoadPattern>());
  mCanCreateNew = true;
}

}//namespace Zero
