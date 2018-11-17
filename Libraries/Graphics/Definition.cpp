///////////////////////////////////////////////////////////////////////////////
///
/// \file Definition.cpp
/// Implementation of the Display object base definition classes.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ZilchDefineType(DefinitionSet, builder, type)
{
}

DefinitionSet::DefinitionSet()
{
  mParent = nullptr;
}

DefinitionSet::~DefinitionSet()
{
  DeleteObjectsInContainer(DefinitionMap);
}

void DefinitionSet::Unload()
{
  forRange(BaseDefinition* definition, DefinitionMap.Values())
    definition->Unload();
}

void DefinitionSet::SetParent(DefinitionSet* set)
{
  mParent = set;
}

void DefinitionSet::Append(DefinitionSet* child)
{
  DefinitionMapType::range r = child->DefinitionMap.All();
  while(!r.Empty())
  {
    DefinitionMap.InsertOrError(r.Front().first, r.Front().second);
    r.Front().second->SetParent(this);
    r.PopFront();
  }
}

void DefinitionSet::AddToSet(StringParam id, BaseDefinition* definition)
{
  definition->SetParent(this);
  DefinitionMap.InsertOrError(id, definition);
}

void DefinitionSet::Initialize()
{
  DefinitionMapType::range r = DefinitionMap.All();

  while(!r.Empty())
  {
    r.Front().second->Initialize();
    r.PopFront();
  }

  if(mParent == nullptr)
  {
    DefinitionSetManager::GetInstance()->Main->Append(this);
  }
}

BaseDefinition* DefinitionSet::GetDefinitionOrNull(StringParam id)
{
  DefinitionMapType::range r = DefinitionMap.Find(id);
  if(r.Empty())
  {
    if(mParent)
      return mParent->GetDefinitionOrNull(id);
    else
      return nullptr;
  }
  else
  {
    return r.Front().second;
  }
}

BaseDefinition* DefinitionSet::GetDefinition(StringParam id)
{
  BaseDefinition* def = GetDefinitionOrNull(id);
  ErrorIf(def == nullptr, "Failed to find definition '%s'", id.c_str());
  return def;
}

DefinitionSet* DefinitionSet::GetDefinitionSet(StringParam id)
{
  return (DefinitionSet*)GetDefinition(id);
}

void DefinitionSet::Serialize(Serializer& stream)
{
  PolymorphicNode definitionNode;
  while(stream.GetPolymorphic(definitionNode))
  {
    DefinitionSetManager::CreatorMapType::range r = DefinitionSetManager::GetInstance()->CreatorMap.Find(definitionNode.TypeName);
    DefinitionCreator* c = r.Front().second;
    BaseDefinition* def = c->Create();
    def->Serialize(stream);
    def->Name = definitionNode.Name;
    AddToSet(definitionNode.Name, def);
    stream.EndPolymorphic();
  }
}


ImplementResourceManager(DefinitionSetManager, DefinitionSet);

DefinitionSetManager::DefinitionSetManager(BoundType* resourceType)
  :ResourceManager(resourceType)
{
  Main = new DefinitionSet();
  AddLoader("DefinitionSet", new TextDataFileLoader<DefinitionSetManager>());
  AddDefinitionCreator<SlicedDefinition>();
  AddDefinitionCreator<ImageDefinition>();
  AddDefinitionCreator<TextDefinition>();
  AddDefinitionCreator<DefinitionSet>();
  mNoFallbackNeeded = true;
}

DefinitionSetManager::~DefinitionSetManager()
{
  //Main definitions are store in other definition sets.
  Main->DefinitionMap.Clear();
  SafeDelete(Main);
  DeleteObjectsInContainer(CreatorMap);
}

}//namespace Zero
