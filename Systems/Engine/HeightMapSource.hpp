/////////////////////////////////////////////////////////////////////////////////
/////
///// \file HeightMapResource.hpp
///// Declaration of the HeightMap resource.
/////
///// Authors: Trevor Sundberg
///// Copyright 2010-2011, DigiPen Institute of Technology
/////
/////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

struct PatchLayer;
DeclareEnum2(PatchLayerType, Height, Weights);

// Collection of layers of data for a Patch.
struct PatchData
{
  PatchData();
  ~PatchData();
  PatchIndex Index;
  HashMap<uint, PatchLayer*> Layers;
};

// Layer of data stored on PatchData
struct PatchLayer
{
  PatchLayer();
  ~PatchLayer();
  uint Size();
  void Allocate();

  //Type of the layer. See PatchLayerType.
  uint LayerType;

  //Width and height of the data
  uint Width;
  uint Height;

  //Size of each element
  uint ElementSize;

  //Data
  byte* Data;
};

/// HeightMapSource stores data needed for height maps. Data is stored on PatchData which are indexed by x,y.
/// Each PatchData stores layers that represent different data. The primary layer is height and paint values
/// but additional layers can be added to store data like foliage or custom game data.
class HeightMapSource : public Resource
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  HeightMapSource();
  ~HeightMapSource();

  //Get or Create data for a particular layer.
  PatchLayer* GetLayerData(PatchIndex index, uint layerIndex);
  //Get or create data for a patch.
  PatchData* GetPatchData(PatchIndex index);
  //Remove a patch.
  void RemovePatch(PatchIndex index);

  // Save to a data file
  void Save(StringParam filename) override;
  void Unload() override;

  uint mVersion;
  HashMap<PatchIndex,  PatchData*> mData;
};

class HeightMapSourceManager : public ResourceManager
{
public:
  DeclareResourceManager(HeightMapSourceManager, HeightMapSource);
  HeightMapSourceManager(BoundType* resourceType);
};

}//namespace Zero
