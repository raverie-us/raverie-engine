///////////////////////////////////////////////////////////////////////////////
///
/// \file Level.cpp
///
/// 
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

class LevelLoader : public ResourceLoader
{
public:
  HandleOf<Resource> LoadFromFile(ResourceEntry& entry)
  {
    Level* level = new Level();
    level->LoadPath = entry.FullPath;
    LevelManager::GetInstance()->AddResource(entry, level);
    return level;
  }

  HandleOf<Resource> LoadFromBlock(ResourceEntry& entry)
  {
    WriteToFile(entry.FullPath.c_str(), entry.Block.Data, entry.Block.Size);
    return LoadFromFile(entry);
  }
};

ZilchDefineType(Level, builder, type)
{
  ZeroBindDocumented();
}

void Level::UpdateContentItem(ContentItem* contentItem)
{
  mContentItem = contentItem;
  LoadPath = contentItem->GetFullPath();
}

String Level::GetLoadPath()
{
  // If valid content item (editor mode)
  // always use the content item path
  if(mContentItem)
    LoadPath = mContentItem->GetFullPath();
  return LoadPath;
}

void Level::SaveSpace(Space* space)
{
  // If the space has a level load pending do not save.
  if(Level* pending = space->mPendingLevel)
  {
    DoNotifyError("Saving Error", "Attempted to save level file while loading.");
    return;
  }

  // If the space has a different level loaded then this level.
  Level* loadedLevel = space->mLevelLoaded;
  if(loadedLevel != this)
  {
    DoNotifyError("Saving Error", "Space has not loaded this level.");
    return;
  }

  if(!IsWritable())
  {
    DoNotifyError("Saving Error", "Can not save to read only default level.");
    return;
  }

  // When saving levels in the editor change the resource to directly point at the file. 
  // Normally it loads from the built content directory.
  if(mContentItem)
  {
    String filename = mContentItem->GetFullPath();
    this->LoadPath = filename;
  }

  // Auto back up level files
  BackUpFile( Z::gContentSystem->GetHistoryPath(mContentItem->mLibrary), mContentItem->GetFullPath() );

  // Save level to file
  space->SaveLevelFile(this->LoadPath);

  ZPrintFilter(Filter::ResourceFilter, "Saved level file '%s'.\n", Name.c_str());

  // No longer modified
  space->MarkNotModified();
}

void Level::LoadSpace(Space* space)
{
  space->LoadLevel(this);
}

ImplementResourceManager(LevelManager, Level);

LevelManager::LevelManager(BoundType* resourceType)
  :ResourceManager(resourceType)
{
  AddLoader("Level", new LevelLoader());
  mCanCreateNew = true;
  mCanAddFile = true;
  mOpenFileFilters.PushBack(FileDialogFilter("*.Level.data"));
  mCanDuplicate = true;
  DefaultResourceName = "EmptyLevel";
  mExtension = DataResourceExtension;
}

}
