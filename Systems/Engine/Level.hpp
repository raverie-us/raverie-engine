///////////////////////////////////////////////////////////////////////////////
///
/// \file Level.hpp
/// 
/// 
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// A level is resource that stores a set of objects that can be loaded into
/// a space. Level is different from most resource types in that it does
/// not really store the level data on the object but always loads the 
/// data from the file system.
class Level : public Resource
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  //Resource interface
  void UpdateContentItem(ContentItem* contentItem) override;

  //Save the current contents of the space into the level.
  void SaveSpace(Space* space);

  //Load the level contents into the space.
  void LoadSpace(Space* space);

  String GetLoadPath();

  ///Path to level file.
  String LoadPath;
};

/// Resource Manager for Levels.
class LevelManager : public ResourceManager
{
public:
  DeclareResourceManager(LevelManager, Level);
  LevelManager(BoundType* resourceType);
};

}
