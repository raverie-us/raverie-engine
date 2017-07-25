///////////////////////////////////////////////////////////////////////////////
///
/// \file ContentComposition.cpp
/// 
/// 
/// Authors: Chris Peters
/// Copyright 2011-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
ZilchDefineType(ContentComponent, builder, type)
{
  type->HandleManager = ZilchManagerId(PointerManager);
  type->Add(new MetaSerialization());
}

ZilchDefineType(BuilderComponent, builder, type)
{
  type->AddAttribute(ObjectAttributes::cCore);
}

DefineSafeIdHandle(ContentComposition);
ZilchDefineType(ContentComposition, builder, type)
{
  ZeroBindHandle();
  type->Add(new ContentMetaComposition());
}

//-------------------------------------------------- ContentMetaComposition
ZilchDefineType(ContentMetaComposition, builder, type)
{
}

ContentMetaComposition::ContentMetaComposition() : MetaComposition(ZilchTypeId(ContentComponent))
{
}

uint ContentMetaComposition::GetComponentCount(HandleParam owner)
{
  ContentComposition* comp = owner.Get<ContentComposition*>(GetOptions::AssertOnNull);
  return comp->Components.Size();
}

Handle ContentMetaComposition::GetComponent(HandleParam owner, BoundType* componentType)
{
  ContentComposition* comp = owner.Get<ContentComposition*>(GetOptions::AssertOnNull);
  return comp->Components.FindValue(componentType, nullptr);
}

Handle ContentMetaComposition::GetComponentAt(HandleParam owner, uint index)
{
  ContentComposition* comp = owner.Get<ContentComposition*>(GetOptions::AssertOnNull);
  return comp->Components[index].second;
}

Handle ContentMetaComposition::MakeObject(BoundType* typeToCreate)
{
  return ZilchAllocate(ContentComponent, typeToCreate);
}

void ContentMetaComposition::AddComponent(HandleParam owner, HandleParam componentToAdd, int index,
                                          bool ignoreDependencies)
{
  ContentComposition* cc = owner.Get<ContentComposition*>(GetOptions::AssertOnNull);
  ContentComponent* component = componentToAdd.Get<ContentComponent*>();

  //set up the initializer for that component with our current content item
  ContentInitializer initializer;
  initializer.Filename = cc->Filename;
  initializer.Success = true;
  initializer.Name = FilePath::GetFileNameWithoutExtension(cc->Filename);
  initializer.Item = cc;
  component->mOwner = cc;

  //Have the content set its defaults and generate ids.
  component->Generate(initializer);

  cc->AddComponent(component);
  component->Initialize(cc);
}

void ContentMetaComposition::RemoveComponent(HandleParam owner, HandleParam componentToRemove,
                                             bool ignoreDependencies)
{
  ContentComposition* comp = owner.Get<ContentComposition*>(GetOptions::AssertOnNull);
  ContentComponent* component = componentToRemove.Get<ContentComponent*>();
  ReturnIf(component->mOwner != comp, , "Component belongs to different owner");
  comp->Components.EraseEqualValues(component);
}

ContentComposition::ContentComposition()
{
  ConstructSafeIdHandle();
}

ContentComposition::~ContentComposition()
{
  ClearComponents();
  DestructSafeIdHandle();
}

void ContentComposition::ClearComponents()
{
  forRange(ComponentMapType::value_type entry, Components.All())
  {
    ContentComponent* cc = entry.second;
    delete cc;
  }
  Components.Clear();
  Builders.Clear();
}

void SerializeComponents(Serializer& stream, ContentComposition* contentItem)
{
  if(stream.GetMode() == SerializerMode::Saving)
  {
    forRange(ContentComposition::ComponentMapType::value_type entry, contentItem->Components.All())
    {
      ContentComponent* component = entry.second;
      stream.SerializePolymorphic(*component);
    }
  }

  if(stream.GetMode() == SerializerMode::Loading)
  {
    PolymorphicNode node;
    while(stream.GetPolymorphic(node))
    {
      //create the component from the name of the node
      ContentComponent* rc = Z::gContentSystem->ComponentFactory.CreateFromName(node.TypeName);
      if(rc)
      {
        rc->Serialize(stream);
        contentItem->AddComponent(rc);
        stream.EndPolymorphic();
      }
      else
      {
        String componentName = node.TypeName;
        ZPrintFilter(Filter::ErrorFilter, "Failed to create content component %s\n", 
               componentName.c_str());
        stream.EndPolymorphic();
      }
    }
  }
}

void ContentComposition::AddComponent(ContentComponent* cc)
{
  this->Components.Insert(ZilchVirtualTypeId(cc), cc);
  cc->mOwner = this;

  Event e;
  GetDispatcher()->Dispatch(Events::ComponentsModified, &e);
}

bool ContentComposition::AnyNeedsBuilding(BuildOptions& options)
{
  forRange(BuilderComponent* bc, Builders.All())
  {
    if(bc->NeedsBuilding(options))
      return true;
  }
  return false;
}

void ContentComposition::BuildContent(BuildOptions& options)
{
  forRange(BuilderComponent* bc, Builders.All())
  {
    if(bc->NeedsBuilding(options))
      bc->BuildContent(options);
  }
}

void ContentComposition::Serialize(Serializer& stream)
{
  SerializeComponents(stream, this);
}

void ContentComposition::BuildListing(ResourceListing& listing)
{
  forRange(BuilderComponent* bc, Builders.All())
  {
    bc->BuildListing(listing);
  }
}

ContentComponent* ContentComposition::QueryComponentId(BoundType* typeId)
{
  return Components.FindValue(typeId, nullptr);
}

void ContentComposition::OnInitialize()
{
  forRange(ComponentMapType::value_type entry, Components.All())
  {
    ContentComponent* cc = entry.second;
    cc->Initialize(this);
  }

  mResourceIsContentItem = Builders.Size() == 1;
}

void ContentComponent::Initialize(ContentComposition* item)
{
  mOwner = item;
}

//------------------------------------------------------------ Builder

void BuilderComponent::BuildListing(ResourceListing& listing)
{

}

void BuilderComponent::Initialize(ContentComposition* item)
{
  ContentComponent::Initialize(item);
  item->Builders.PushBack(this);
}

void BuilderComponent::Rename(StringParam newName)
{

}

}
