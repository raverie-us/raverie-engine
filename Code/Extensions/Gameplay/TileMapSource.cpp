// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

RaverieDefineType(TileMapSource, builder, type)
{
  RaverieBindDocumented();
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
TileMapSourceManager::TileMapSourceManager(BoundType* resourceType) : ResourceManager(resourceType)
{
  this->mNoFallbackNeeded = true;
  mExtension = "bin";
  AddLoader("TileMapSource", new ChunkFileLoader<TileMapSourceManager, TileMapSourceLoadPattern>());
}

} // namespace Raverie
