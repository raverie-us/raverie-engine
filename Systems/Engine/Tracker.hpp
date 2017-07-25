///////////////////////////////////////////////////////////////////////////////
///
/// \file Tracker.hpp
/// Declaration of the Tracker system.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

///Creates and manages object CogIds
class Tracker : public System
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  Tracker();
  ~Tracker();
  
  //Create the tracking system
  static Tracker* StaticInitialize();

  ///Id object and store it in the object map.
  CogId IdGameObject(Cog* gameObject);

  ///Get the game object with given id. This function will return NULL if the
  ///object has been destroyed.
  Cog* GetObjectWithId(CogId id);
  Cog* RawFind(u32 id);

  //Update the Tracker deleting objects that are queued for destructions.
  void ClearDeletedObjects();

  ///Destroy all the Cogs in the world. Used for final shutdown.
  void DestroyAllObjects();

  ///Add a Cog to the destroy list for delayed destruction.
  void Destroy(Cog* gameObject);

  virtual cstr GetName() { return "Tracker"; }

  uint GetObjectCount(){return mObjectMap.Size();}

private:

  void AddToDestroyList(Cog* object);

  ///Used to incrementally generate unique id's.
  unsigned mLastGameObjectId;

  struct SlotPolicy
  {
    static CogId& AccessId(Cog* object) { return object->mObjectId; }
  };

  ///Map of Cog to their Ids used for safe referencing of game objects
  typedef Slotmap<CogId, Cog*, SlotPolicy> ObjectMapType;
  ObjectMapType mObjectMap;

  Array<Cog*> mDestroyArray;
};

namespace Z
{
  extern Tracker* gTracker;
}

}//namespace Zero
