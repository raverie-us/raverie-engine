/////////////////////////////////////////////////////////////////////////////////
/////
///// \file TilePaletteSource.hpp
///// Declaration of the TilePalette resource.
/////
///// Authors: Nathan Carlson
///// Copyright 2013, DigiPen Institute of Technology
/////
/////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class TilePaletteSource : public Resource
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  TilePaletteSource();
  ~TilePaletteSource();

  // Save to a data file
  void Save(StringParam filename) override;
  void Unload() override;
  IntVec2 GetTileDimensions();

  uint mVersion;
  TileMap::TileHashMap mData;
  typedef TileMap::TileHashMap::value_type value_type;
};

class TilePaletteSourceManager : public ResourceManager
{
public:
  DeclareResourceManager(TilePaletteSourceManager, TilePaletteSource);

  TilePaletteSourceManager(BoundType* resourceType);
};

}//namespace Zero
