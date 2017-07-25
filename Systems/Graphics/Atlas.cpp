///////////////////////////////////////////////////////////////////////////////
///
/// \file Atlas.cpp
///
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(Atlas, builder, type)
{
  type->AddAttribute(ObjectAttributes::cHidden);
}

Atlas::Atlas()
{
  NeedsBuilding = false;
}

Atlas::~Atlas()
{
  ClearTextures();
  while(!Sources.Empty())
  {
    SpriteSource* spriteSource = &Sources.Front();
    Sources.Erase(spriteSource);
    spriteSource->mAtlas = NULL;
  }
}

void Atlas::Unload()
{
  Textures.Clear();
}

void Atlas::ClearTextures()
{
  Textures.Clear();
}

void Atlas::AddSpriteSource(SpriteSource* source)
{
  ErrorIf(source->mAtlas != NULL);
  Sources.PushBack(source);
  source->mAtlas = this;
  NeedsBuilding = true;
}

//------------------------------------------------------------
ImplementResourceManager(AtlasManager, Atlas);

AtlasManager::AtlasManager(BoundType* resourceType)
  : ResourceManager(resourceType)
{
  AddLoader("Atlas", new TextDataFileLoader<AtlasManager>());
  mNoFallbackNeeded = true;
  DefaultResourceName = "Default";
  mExtension = DataResourceExtension;
}

AtlasManager::~AtlasManager()
{

}

}
