///////////////////////////////////////////////////////////////////////////////
///
///  \file HeightMapResource.cpp
///  Implementation of the HeightMap resource.
///
///  Authors: Trevor Sundberg
///  Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
ZilchDefineType(HeightMapSource, builder, type)
{
  ZeroBindDocumented();
}

PatchLayer::PatchLayer()
{
  LayerType = 0;
  Width = 0;
  Height = 0;
  ElementSize = 0;
  Data = 0;
}

PatchLayer::~PatchLayer()
{
  if(Data)
    zDeallocate(Data);
}

uint PatchLayer::Size()
{
  return Width * Height * ElementSize;
}

void PatchLayer::Allocate()
{
  if(Data == nullptr)
    Data = (byte*)zAllocate(Size());
}

PatchData::PatchData()
{
}

PatchData::~PatchData()
{
  DeleteObjectsInContainer(Layers);
}

HeightMapSource::HeightMapSource()
{
  mVersion = 0;
}

HeightMapSource::~HeightMapSource()
{
  DeleteObjectsInContainer(mData);
}

PatchLayer* HeightMapSource::GetLayerData(PatchIndex index, uint layerIndex)
{
  PatchData* data = GetPatchData(index);
  PatchLayer* layer = data->Layers.FindValue(layerIndex, nullptr);
  if(layer == nullptr)
  {
    layer = new PatchLayer();
    layer->LayerType = layerIndex;
    data->Layers.Insert(layerIndex, layer);
  }
  return layer;
}

PatchData* HeightMapSource::GetPatchData(PatchIndex index)
{
  PatchData* data = mData.FindValue(index, nullptr);
  if(data == nullptr)
  {
    data = new PatchData();
    data->Index = index;
    mData.Insert(index, data);
  }
  return data;
}

void HeightMapSource::RemovePatch(PatchIndex index)
{
  PatchData* data = mData.FindValue(index, nullptr);
  if(data)
  {
    delete data;
    mData.Erase(index);
  }
}

struct HeightMapSourceLoadPattern
{
  template<typename readerType>
  static void Load(HeightMapSource* heightMapSource, readerType& file)
  {
    uint numberOfPatches = 0;
    uint version = 0;

    file.Read(version);
    file.Read(numberOfPatches);

    heightMapSource->mVersion = version;

    //Read data for each patch
    for(uint i=0;i<numberOfPatches;++i)
    {
      PatchData * data = new PatchData();
      file.Read(data->Index.x);
      file.Read(data->Index.y);

      uint numberOfLayers = 0;

      file.Read(numberOfLayers);

      for(uint l=0;l<numberOfLayers;++l)
      {
        // Read data for each layer
        PatchLayer* layer = new PatchLayer();

        file.Read(layer->LayerType);
        file.Read(layer->Width);
        file.Read(layer->Height);
        file.Read(layer->ElementSize);

        uint size = layer->Size();
        layer->Data = (byte*)zAllocate(size);
        file.ReadArray(layer->Data, size);

        data->Layers.Insert(layer->LayerType, layer);
      }

      heightMapSource->mData.Insert(data->Index, data);
    }
  }

  static void Save(HeightMapSource* heightMapSource, ChunkFileWriter& file)
  {
    file.Write(heightMapSource->mVersion);
    file.Write(u32(heightMapSource->mData.Size()));

    forRange(PatchData* data,  heightMapSource->mData.Values())
    {
      file.Write(data->Index.x);
      file.Write(data->Index.y);
      file.Write(u32(data->Layers.Size()));

      forRange(PatchLayer* layer,  data->Layers.Values())
      {
        file.Write(layer->LayerType);
        file.Write(layer->Width);
        file.Write(layer->Height);
        file.Write(layer->ElementSize);

        uint size = layer->Size();
        file.Write(layer->Data, size);
      }
    }
  }
};

ImplementResourceManager(HeightMapSourceManager, HeightMapSource);

HeightMapSourceManager::HeightMapSourceManager(BoundType* resourceType)
  :ResourceManager(resourceType)
{
  this->mNoFallbackNeeded = true;
  mExtension = "bin";
  AddLoader("HeightMapSource", new ChunkFileLoader<HeightMapSourceManager, HeightMapSourceLoadPattern>());
}

void HeightMapSource::Save(StringParam filename)
{
  ChunkFileWriter file;
  file.Open(filename);
  HeightMapSourceLoadPattern::Save(this, file);
}

void HeightMapSource::Unload()
{
 DeleteObjectsInContainer(mData);
}

}//namespace Zero
