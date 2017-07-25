///////////////////////////////////////////////////////////////////////////////
///
/// \file Image.cpp
/// Implementation of the image based display object classes.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(SlicedDefinition, builder, type)
{
}

void SlicedDefinition::Initialize()
{
  TexturePtr = TextureManager::Find(mTexture);
  float invTextureSize = 1.0f / TexturePtr->GetSize().x;
  Vec2 uv0 = Uv0;
  Vec2 uvSize = Uv1;
  Vec4 slices = Slices;
  //TexturePtr->SetFiltering(Filtering::LinearNoMip);

  DefaultSize = Uv1;

  Uv0 = uv0 * invTextureSize;
  Uv1 = (uv0 + uvSize) * invTextureSize;
  Slices = slices * invTextureSize;
  Borders = slices;
  ImageMode = cSlice;
  Sliced = true;

  if(Slices[0] == 0.0f)
  {
    Sliced = false;
    ImageMode = 0;
  }
}

void SlicedDefinition::Serialize(Serializer& stream)
{
  SerializeName(mTexture);
  SerializeName(Uv0);
  SerializeName(Uv1);
  SerializeName(Slices);
}

void SlicedDefinition::Unload()
{
  TexturePtr = nullptr;
}

//------------------------------------------------------------------------------
ZilchDefineType(ImageDefinition, builder, type)
{
}

void ImageDefinition::Initialize()
{
  TexturePtr = TextureManager::Find(mTexture);

  // Converting pixel uv's to normalized uvs
  Vec2 pixels = Math::ToVec2(TexturePtr->GetSize());
  Vec2 invTextureSize(1.0f / pixels.x, 1.0f / pixels.y);
  //TexturePtr->SetFiltering(Filtering::LinearNoMip);
  DefaultSize = Uv1;
  Vec2 uv0 = Uv0;
  Vec2 uvSize = Uv1;
  Uv0 = uv0 * invTextureSize;
  Uv1 = (uv0 + uvSize) * invTextureSize;

  Slices = Vec4(0,0,0,0);
  Sliced = false;
}

void ImageDefinition::Serialize(Serializer& stream)
{
  SerializeName(mTexture);
  SerializeName(ImageMode);
  SerializeName(Uv0);
  SerializeName(Uv1);
}

}//namespace Zero
