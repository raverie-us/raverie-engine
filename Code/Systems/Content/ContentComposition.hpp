// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

class ContentComposition;
void SerializeComponents(Serializer& stream, ContentComposition* contentItem);

/// Any add on data for a content item. This includes the builders and
/// any extra data such as notes, copyright and history.
class ContentComponent : public EventObject
{
public:
  ZilchDeclareType(ContentComponent, TypeCopyMode::ReferenceType);
  ContentComponent(){};
  virtual ~ContentComponent(){};
  ContentComposition* mOwner;

  // ContentComponent Interface.

  /// Serialize the content component
  virtual void Serialize(Serializer& stream)
  {
  }

  /// Initialize the content component
  virtual void Initialize(ContentComposition* item);

  /// Content component has been added, generate a new id and set to defaults.
  virtual void Generate(ContentInitializer& initializer)
  {
  }
};

class ContentMetaComposition : public MetaComposition
{
public:
  ZilchDeclareType(ContentMetaComposition, TypeCopyMode::ReferenceType);

  ContentMetaComposition();

  // MetaComposition interface
  uint GetComponentCount(HandleParam owner) override;
  Handle GetComponent(HandleParam owner, BoundType* componentType) override;
  Handle GetComponentAt(HandleParam owner, uint index) override;
  Handle MakeObject(BoundType* typeToCreate) override;
  void AddComponent(HandleParam owner,
                    HandleParam component,
                    int index = -1,
                    bool ignoreDependencies = false,
                    MetaCreationContext* creationContext = nullptr) override;
  void RemoveComponent(HandleParam owner, HandleParam component, bool ignoreDependencies = false) override;
};

/// A content item that allows composited items.
/// Allows flexibility in what is stored on per content item.
class ContentComposition : public ContentItem
{
public:
  ZilchDeclareType(ContentComposition, TypeCopyMode::ReferenceType);
  DeclareSafeIdHandle(u64);

  ContentComposition();
  ~ContentComposition();

  void ClearComponents();

  /// Determines if any builder of this content item needs to build resources
  bool AnyNeedsBuilding(BuildOptions& options);

  // Content Item Interface
  void AddComponent(ContentComponent* cc) override;
  void BuildContentItem(BuildOptions& options) override;
  void Serialize(Serializer& stream) override;
  void BuildListing(ResourceListing& listing) override;
  void OnInitialize() override;
  ContentComponent* QueryComponentId(BoundType* typeId) override;

  void RemoveComponent(BoundType* componentType);

  typedef Array<ContentComponent*> ComponentArray;
  typedef ComponentArray::range ComponentRange;
  typedef ArrayMultiMap<BoundType*, ContentComponent*> ComponentMapType;
  typedef ComponentMapType::valueRange ComponentMapRange;

  // Content Components
  ComponentArray mComponents;
  ComponentMapType mComponentMap;

  // Content Builders. Builders are currently in both the builders and
  // components list. Some times explicit iteration through the builders
  // is desired, other times all components (including builders)
  // need to undergo an operation.
  Array<BuilderComponent*> Builders;
};

/// Builder component is a content component that builds resources.
class BuilderComponent : public ContentComponent
{
public:
  ZilchDeclareType(BuilderComponent, TypeCopyMode::ReferenceType);
  // ContentComponent Interface
  void Initialize(ContentComposition* item) override;

  // Does this builder need building?
  virtual bool NeedsBuilding(BuildOptions& options)
  {
    return false;
  }

  // Build content resources.
  virtual void BuildContent(BuildOptions& buildOptions)
  {
  }

  // Add built resources to listing.
  virtual void BuildListing(ResourceListing& listing);

  // Rename resource generated from builder.
  virtual void Rename(StringParam newName);

  // Allows a builder to Append command line arguments.
  virtual void AppendCommands(StringBuilder& builder)
  {
  }

  virtual void SetShowInEditor(bool state)
  {
  }
  virtual String GetTag()
  {
    return String();
  }

  virtual String GetResourceOwner()
  {
    return String();
  }
  virtual void SetResourceOwner(StringParam owner)
  {
  }
};

} // namespace Zero
