///////////////////////////////////////////////////////////////////////////////
///
/// \file Definition.hpp
/// Declaration of the Display object base definition classes.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class BaseDefinition;
class DefinitionCreator
{
public:
  virtual BaseDefinition* Create()=0;
  virtual ~DefinitionCreator(){};
};

template<typename type>
class DefinitionCreatorType : public DefinitionCreator
{
public:
  virtual BaseDefinition* Create(){ return new type(); }
};

class DefinitionSet;
class DefinitionSetManager;

///Base definition class for all display objects.
class BaseDefinition : public Resource
{
public:
  virtual void Initialize()=0;
  virtual void Serialize(Serializer& stream)=0;
  virtual void SetParent(DefinitionSet* set){};
  virtual ~BaseDefinition(){};
};

/// Base Definition Collection
class DefinitionSet : public BaseDefinition
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  DefinitionSet();
  ~DefinitionSet();

  void Initialize() override;
  void Serialize(Serializer& stream) override;
  void SetParent(DefinitionSet* set) override;
  void Unload() override;

  BaseDefinition* GetDefinition(StringParam id);
  DefinitionSet* GetDefinitionSet(StringParam id);
  BaseDefinition* GetDefinitionOrNull(StringParam id);

  void AddToSet(StringParam id, BaseDefinition* definition);
  void Append(DefinitionSet* child);

private:
  friend class DefinitionSetManager;
  DefinitionSet* mParent;
  typedef HashMap<String, BaseDefinition*> DefinitionMapType;
  DefinitionMapType DefinitionMap;
};

class DefinitionSetManager : public ResourceManager
{
public:
  DeclareResourceManager(DefinitionSetManager, DefinitionSet);

  DefinitionSetManager(BoundType* resourceType);
  ~DefinitionSetManager();

  typedef HashMap<String, DefinitionCreator*> CreatorMapType;
  CreatorMapType CreatorMap;

  DefinitionSet* Main;

  template<typename definitionType>
  void AddDefinitionCreator()
  {
    CreatorMap.Insert( ZilchTypeId(definitionType)->Name, new DefinitionCreatorType<definitionType> );
  }
};


}///namespace Zero
