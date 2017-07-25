/////////////////////////////////////////////////////////////////////////////////
/////
///// \file TileMapSource.hpp
///// Declaration of the TileMap resource.
/////
///// Authors: Nathan Carlson
///// Copyright 2013, DigiPen Institute of Technology
/////
/////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class TileMapSource : public Resource
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

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

} // namespace Zero
