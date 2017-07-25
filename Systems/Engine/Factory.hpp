///////////////////////////////////////////////////////////////////////////////
///
/// \file Factory.hpp
/// Declaration of the Factory class for Cogs.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//Forward declarations.
class Tracker;
class Space;
class MetaEventConnection;

///Game object factory. The game object factory creates composition objects 
///from data streams.
class Factory : public System
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  Factory(Engine* engine, Tracker* tracker);
  ~Factory();

  //Create the factory
  static Factory* StaticInitialize(Engine* engine, Tracker* tracker);

  ///Create initialize and Id a Cog from the data file.
  Cog* Create(Space* space, StringParam filename, uint flags, GameSession* gameSession);
  Cog* CreateFromStream(Space* space, Serializer& stream, uint flags, GameSession* gameSession);
  Space* CreateSpace(StringParam name, uint flags, GameSession* gameSession);
  Space* CreateSpaceFromStream(Serializer& stream, uint flags, GameSession* gameSession);

  ///Creates an object of an expected type. Used to create a GameSession or
  ///something else but still properly check to make sure the filename we're
  ///opening has the correct type in it.
  Cog* CreateCheckedType(BoundType* expectedType, Space* space, StringParam filename, uint flags, GameSession* gameSession);

  //Create an object engine error if it could not be found.
  Cog* CreateRequired(Space* space, StringParam filename, uint flags, GameSession* gameSession);

  ///Add a Cog to the destroy list for delayed destruction.
  void Destroy(Cog* gameObject);

  //Default update.
  virtual void Update();

  ///Name of the system is factory.
  virtual cstr GetName() { return "Factory"; }

  ///Build a composition and serialize from the data file 
  ///but does not initialize the Cog
  Cog* BuildAndSerialize(BoundType* expected, StringParam source);
  Cog* BuildAndSerialize(BoundType* expected, StringParam source, CogCreationContext* context);

  Cog* BuildFromStream(CogCreationContext* context, Serializer& stream);
  Cog* BuildFromFile(BoundType* expectedMetaType, StringParam source, CogCreationContext* context);
  Cog* BuildFromArchetype(BoundType* expectedMetaType, Archetype* archtype, CogCreationContext* context);

private:
  Tracker* mTracker;
  Engine* mEngine;

  friend class CogMetaComposition;
};

namespace Z
{
extern Factory* gFactory;
}

}//namespace Zero
