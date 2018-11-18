///////////////////////////////////////////////////////////////////////////////
///
///  \file TileMapSource.cpp
///  Implementation of the TileMap resource.
///
///  Authors: Nathan Carlson
///  Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(TileMapSource, builder, type)
{
  ZeroBindDocumented();
}

TileMapSource::TileMapSource()
{
  mVersion = 0;
}

TileMapSource::~TileMapSource()
{
}

void TileMapSource::Save(StringParam filename)
{
  ChunkFileWriter file;
  file.Open(filename);
  TileMapSourceLoadPattern::Save(this, file);
}

void TileMapSource::Unload()
{
  mData.Clear();
}

ImplementResourceManager(TileMapSourceManager, TileMapSource);
TileMapSourceManager::TileMapSourceManager(BoundType* resourceType)
  :ResourceManager(resourceType)
{
  this->mNoFallbackNeeded = true;
  mExtension = "bin";
  AddLoader("TileMapSource", new ChunkFileLoader<TileMapSourceManager, TileMapSourceLoadPattern>());
}

} // namespace Zero
