// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class TileMapSource : public Resource
{
public:
  RaverieDeclareType(TileMapSource, TypeCopyMode::ReferenceType);

  TileMapSource();
  ~TileMapSource();

  // Save to a data file
  void Save(StringParam filename) override;
  void Unload() override;

  uint mVersion;
  TileMap::TileHashMap mData;
  typedef TileMap::TileHashMap::value_type value_type;
};

class TileMapSourceManager : public ResourceManager
{
public:
  DeclareResourceManager(TileMapSourceManager, TileMapSource);

  TileMapSourceManager(BoundType* resourceType);
};

} // namespace Raverie
