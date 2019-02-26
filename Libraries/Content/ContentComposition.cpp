// MIT Licensed (see LICENSE.md).
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

ZilchDefineType(ContentMetaComposition, builder, type)
{
}

ContentMetaComposition::ContentMetaComposition() : MetaComposition(ZilchTypeId(ContentComponent))
{
}

uint ContentMetaComposition::GetComponentCount(HandleParam owner)
{
  ContentComposition* comp = owner.Get<ContentComposition*>(GetOptions::AssertOnNull);
  return comp->mComponents.Size();
}

Handle ContentMetaComposition::GetComponent(HandleParam owner, BoundType* componentType)
{
  ContentComposition* comp = owner.Get<ContentComposition*>(GetOptions::AssertOnNull);
  return comp->mComponentMap.FindValue(componentType, nullptr);
}

Handle ContentMetaComposition::GetComponentAt(HandleParam owner, uint index)
{
  ContentComposition* comp = owner.Get<ContentComposition*>(GetOptions::AssertOnNull);
  return comp->mComponents[index];
}

Handle ContentMetaComposition::MakeObject(BoundType* typeToCreate)
{
  return ZilchAllocate(ContentComponent, typeToCreate);
}

void ContentMetaComposition::AddComponent(HandleParam owner,
                                          HandleParam componentToAdd,
                                          int index,
                                          bool ignoreDependencies,
                                          MetaCreationContext* creationContext)
{
  ContentComposition* cc = owner.Get<ContentComposition*>(GetOptions::AssertOnNull);
  ContentComponent* component = componentToAdd.Get<ContentComponent*>();

  // set up the initializer for that component with our current content item
  ContentInitializer initializer;
  initializer.Filename = cc->Filename;
  initializer.Success = true;
  initializer.Name = FilePath::GetFileNameWithoutExtension(cc->Filename);
  initializer.Item = cc;
  component->mOwner = cc;

  // Have the content set its defaults and generate ids.
  component->Generate(initializer);

  cc->AddComponent(component);
  component->Initialize(cc);
}

void ContentMetaComposition::RemoveComponent(HandleParam owner, HandleParam componentToRemove, bool ignoreDependencies)
{
  ContentComposition* cc = owner.Get<ContentComposition*>(GetOptions::AssertOnNull);
  ContentComponent* component = componentToRemove.Get<ContentComponent*>();
  ReturnIf(component->mOwner != cc, , "Component belongs to different owner");
  cc->Builders.EraseValue(component);
  cc->mComponents.EraseValue(component);
  cc->mComponentMap.EraseEqualValues(component);
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
  forRange (ContentComponent* component, mComponents.All())
    delete component;

  mComponents.Clear();
  mComponentMap.Clear();
  Builders.Clear();
}

void SerializeComponents(Serializer& stream, ContentComposition* contentItem)
{
  if (stream.GetMode() == SerializerMode::Saving)
  {
    forRange (ContentComponent* component, contentItem->mComponents.All())
      stream.SerializePolymorphic(*component);
  }

  if (stream.GetMode() == SerializerMode::Loading)
  {
    PolymorphicNode node;
    while (stream.GetPolymorphic(node))
    {
      // create the component from the name of the node
      ContentComponent* rc = Z::gContentSystem->ComponentFactory.CreateFromName(node.TypeName);
      if (rc)
      {
        rc->Serialize(stream);
        contentItem->AddComponent(rc);
        stream.EndPolymorphic();
      }
      else
      {
        String componentName = node.TypeName;
        ZPrintFilter(Filter::DefaultFilter,
                     "Content component %s was in the data file but could not "
                     "be created (most likely deprecated)\n",
                     componentName.c_str());
        stream.EndPolymorphic();
      }
    }
  }
}

void ContentComposition::AddComponent(ContentComponent* cc)
{
  mComponents.PushBack(cc);
  mComponentMap.Insert(ZilchVirtualTypeId(cc), cc);
  cc->mOwner = this;

  ObjectEvent e(this);
  GetDispatcher()->Dispatch(Events::ComponentsModified, &e);
}

bool ContentComposition::AnyNeedsBuilding(BuildOptions& options)
{
  forRange (BuilderComponent* bc, Builders.All())
  {
    if (bc->NeedsBuilding(options))
      return true;
  }
  return false;
}

void ContentComposition::BuildContentItem(BuildOptions& options)
{
  bool anyBuilt = false;
  forRange (BuilderComponent* bc, Builders.All())
  {
    if (bc->NeedsBuilding(options))
    {
      anyBuilt = true;
      bc->BuildContent(options);
    }
  }

  if (anyBuilt)
    ZPrint("Built %s\n", Filename.c_str());
}

void ContentComposition::Serialize(Serializer& stream)
{
  SerializeComponents(stream, this);
}

void ContentComposition::BuildListing(ResourceListing& listing)
{
  forRange (BuilderComponent* bc, Builders.All())
  {
    bc->BuildListing(listing);
  }
}

ContentComponent* ContentComposition::QueryComponentId(BoundType* typeId)
{
  return mComponentMap.FindValue(typeId, nullptr);
}

void ContentComposition::RemoveComponent(BoundType* componentType)
{
  if (ContentComponent* component = QueryComponentId(componentType))
  {
    mComponents.EraseValue(component);
    mComponentMap.EraseEqualValues(component);
  }
}

void ContentComposition::OnInitialize()
{
  forRange (ContentComponent* component, mComponents.All())
    component->Initialize(this);

  mResourceIsContentItem = Builders.Size() == 1;
}

void ContentComponent::Initialize(ContentComposition* item)
{
  mOwner = item;
}

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

} // namespace Zero
