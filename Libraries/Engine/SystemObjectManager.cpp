///////////////////////////////////////////////////////////////////////////////
///
/// \file Engine.cpp
/// Implementation of the SystemObjectManager.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Z
{
  SystemObjectManager* gSystemObjects;
}

SystemObjectManager::SystemObjectManager()
{
}

SystemObjectManager::~SystemObjectManager()
{
  forRange(ObjectInstance& objectInstance, Objects.Values())
  {
    if(objectInstance.Cleanup == ObjectCleanup::AutoDelete)
    {
      delete objectInstance.Instance;
    }
  }
}

void SystemObjectManager::Add(Object* object, BoundType* metaType, ObjectCleanup::Enum cleanup)
{
  ObjectInstance none = {object, metaType, cleanup};
  Objects.InsertOrError(metaType->Name, none);
}

void SystemObjectManager::Add(Object* object, ObjectCleanup::Enum cleanup)
{
  Add(object, ZilchVirtualTypeId(object), cleanup);
}

void StartSystemObjects()
{
  Z::gSystemObjects = new SystemObjectManager();
}

void CleanUpSystemObjects()
{
  delete Z::gSystemObjects;
}

}
