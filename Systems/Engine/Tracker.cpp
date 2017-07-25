///////////////////////////////////////////////////////////////////////////////
///
/// \file Tracker.cpp
/// Implementation of the game engine Tracker class.
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
  Tracker* gTracker = nullptr;
}

ZilchDefineType(Tracker, builder, type)
{
}

Tracker* Tracker::StaticInitialize()
{
  ErrorIf(Z::gTracker != nullptr, "Tracker already created.");
  Z::gTracker = new Tracker();
  return Z::gTracker;
}

Tracker::Tracker()
{
  mLastGameObjectId = 0;
}

Tracker::~Tracker()
{

}

CogId Tracker::IdGameObject(Cog* gameObject)
{
  ErrorIf(gameObject->mObjectId != cInvalidCogId, "Object Id is already set");

  //Just increment the last id used.
  ++mLastGameObjectId;

  //Set the id
  gameObject->mObjectId.Id = mLastGameObjectId;

  //Store the game object in the global object id map
  mObjectMap.Insert(gameObject);
  return gameObject->mObjectId;
}

Cog* Tracker::GetObjectWithId(CogId id)
{
  ObjectMapType::range range = mObjectMap.Find(id);
  if(!range.Empty())
    return range.Front();
  else
    return nullptr;
}

void Tracker::ClearDeletedObjects()
{
  //Swap out a temp to prevent errors
  //when deleted an object results in other objects
  //being deleted.
  Array<Cog*> tempArray;
  tempArray.Swap(mDestroyArray);

  while(!tempArray.Empty())
  {
    Array<Cog*>::range range = tempArray.All();
    for(;!range.Empty();range.PopFront())
    {
      Cog* objectToBeDeleted = range.Front();

      //Delete the object
      objectToBeDeleted->OnDestroy();

      if(objectToBeDeleted->mObjectId != cInvalidCogId)
      {
        mObjectMap.Erase(objectToBeDeleted->mObjectId);
      }

      delete objectToBeDeleted;
    }

    //All objects to be delete have been deleted
    tempArray.Clear();

    //swap back into place if objects
    //have been deleted loop will continue.
    tempArray.Swap(mDestroyArray);
  }

}

void Tracker::AddToDestroyList(Cog* gameObject)
{
  ErrorIf(gameObject->mFlags.IsSet(CogFlags::Destroyed), "Double delete error.");
  //Add the object to the to be deleted list they will be deleted
  //when the factory is updated
  mDestroyArray.PushBack(gameObject);
  gameObject->mFlags.SetFlag(CogFlags::Destroyed);
}

void Tracker::Destroy(Cog* gameObject)
{
  if(gameObject->mFlags.IsSet(CogFlags::Destroyed))
  {
    //Double destroy do not do anything.
    return;
  }
  else if(gameObject->mObjectId == cInvalidCogId)
  {
    //Object was never given an Id.
    AddToDestroyList(gameObject);
  }
  else
  {
    //Check to make sure object is in the map.
    ObjectMapType::range range = mObjectMap.Find(gameObject->mObjectId);
    if(!range.Empty())
    {
      //Remove it the map and invalidate its id
      AddToDestroyList(gameObject);
    }
    else
    {
      ErrorIf(gameObject->mObjectId.Id == 0xfeeefeee, "Attempting to destroy a "
              "deleted object. Memory corruption or engine failure.");
      ErrorIf(true, "Object is being destroyed but does not have a valid id. "
              "Memory Corruption or engine failure.");
    }
  }
}

Cog* Tracker::RawFind(u32 id)
{
  ObjectMapType::range range = mObjectMap.All();
  while(!range.Empty())
  {
    if(range.Front()->mObjectId.Id == id)
      return range.Front();
    range.PopFront();
  }
  return nullptr;
}

void Tracker::DestroyAllObjects()
{
  ObjectMapType::range range = mObjectMap.All();
  while(!range.Empty())
  {
    Cog* object = range.Front();
    
    //if(Space* space = object->has(Space))
    //{
    //  space->DestroyAll();
    //  object->Destroy();
    //}
    range.PopFront();
  }

  ClearDeletedObjects();

  range = mObjectMap.All();
  while(!range.Empty())
  {
    Cog* object = range.Front();
    if(object->mObjectId != cInvalidCogId)
      mObjectMap.Erase(object->mObjectId);

    DebugPrint("Warning Global Object %d\n", object->mObjectId.Id);
    delete object;
    range.PopFront();
  }

  mObjectMap.Clear();
}

}//namespace Zero
