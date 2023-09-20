// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

/// A level is resource that stores a set of objects that can be loaded into
/// a space. Level is different from most resource types in that it does
/// not really store the level data on the object but always loads the
/// data from the file system.
class Level : public Resource
{
public:
  RaverieDeclareType(Level, TypeCopyMode::ReferenceType);

  Level();
  ~Level();

  // Resource interface
  void UpdateContentItem(ContentItem* contentItem) override;

  // Save the current contents of the space into the level.
  void SaveSpace(Space* space);

  // Load the level contents into the space.
  void LoadSpace(Space* space);

  String GetLoadPath();

  /// Path to level file.
  String LoadPath;
  DataNode* mCacheTree;
};

/// Resource Manager for Levels.
class LevelManager : public ResourceManager
{
public:
  DeclareResourceManager(LevelManager, Level);
  LevelManager(BoundType* resourceType);

  static void ClearCachedLevels();
};

} // namespace Raverie
