////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2016, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//------------------------------------------------------------------------------------------ Helpers
void SetCogFlag(Cog* cog, CogFlags::Enum flag, cstr flagName, bool state);
bool CogIsModifiedFromArchetype(Cog* cog, bool ignoreOverrideProperties);
void ClearCogModifications(Cog* rootCog, Cog* cog, ObjectState::ModifiedProperties& cachedMemory,
                           bool retainOverrideProperties, bool retainChildArchetypeModifications);
void ClearCogModifications(Cog* root, bool retainChildArchetypeModifications);
void AssignChildIds(Cog* parent);
template<typename type>
void eraseEqualValues(Array<type>& mArray, type value);

//------------------------------------------------------------------------------------------- Events
namespace Events
{
  DefineEvent(CogNameChanged);
  DefineEvent(TransformUpdated);
  DefineEvent(CogDestroy);
  DefineEvent(CogDelayedDestroy);
}

//----------------------------------------------------------------------------------------- Cog Meta
//**************************************************************************************************
Handle CogGetOwner(HandleParam object)
{
  Cog* cog = object.Get<Cog*>();

  // Hierarchy is technically our Owner according to Meta (Hierarchy has a meta composition)
  if(Cog* parent = cog->GetParent())
    return parent->has(Hierarchy);
  return nullptr;
}

//**************************************************************************************************
String CogToString(const BoundType* type, const byte* data)
{
  Cog* cog = (Cog*)(data);
  return cog->GetDescription();
}

//**************************************************************************************************
String CogDisplayName(HandleParam object)
{
  return object.StoredType->HasInherited<MetaDisplay>( )->GetName(object);
}

//---------------------------------------------------------------------------------------------- Cog
Memory::Heap* Cog::sHeap = new Memory::Heap("Cogs", Memory::GetNamedHeap("Objects"));

//**************************************************************************************************
ZilchDefineType(Cog, builder, type)
{
  type->HandleManager = ZilchManagerId(CogHandleManager);

  //METAREFACTOR Componentize this stuff
  //type->ShortDescription = GetShortDescriptionCog;
  //type->SerializeProperty = MetaSerializeCog;
  //meta->mFlags.SetFlag(MetaTypeFlags::AsPropertyUseCustomSerialization);

  type->ToStringFunction = CogToString;

  type->Add(new MetaOwner(CogGetOwner));
  type->Add(new CogMetaComposition());
  type->Add(new CogMetaDataInheritance());
  type->Add(new CogMetaTransform());
  type->Add(new CogSerializationFilter());
  type->Add(new CogMetaOperations());
  type->Add(new CogMetaDisplay());

  type->AddAttribute(ObjectAttributes::cStoreLocalModifications);

  ZeroBindDocumented();

  ZilchBindDefaultConstructor();

  // Properties
  ZilchBindGetterSetterProperty(Name)->AddAttribute(PropertyAttributes::cLocalModificationOverride);
  ZilchBindGetterSetterProperty(Archetype)->Add(new CogArchetypeExtension());
  ZilchBindGetter(BaseArchetype);
  ZilchBindGetterProperty(BaseArchetypeName);

  ZilchBindGetter(Space);
  ZilchBindGetter(LevelSettings);
  ZilchBindGetter(GameSession);
  ZilchBindGetter(Actions);
  ZilchBindGetter(RuntimeId);
  ZilchBindGetterSetter(Transient);
  ZilchBindGetterSetter(Persistent);
  ZilchBindGetterSetter(EditorViewportHidden);
  ZilchBindGetterSetter(ObjectViewHidden);
  ZilchBindGetterSetter(Locked);

  ZilchBindMethod(Destroy);
  ZilchBindMethod(Clone);
  
  // Components
  ZilchBindMethod(AddComponentByType);
  ZilchBindMethod(AddComponentByName);

  ZilchBindMethod(GetComponentByName);
  ZilchBindMethod(GetComponentByIndex);
  ZilchBindGetter(ComponentCount);
  ZilchBindMethod(GetComponentIndex);

  ZilchBindMethod(RemoveComponentByType);
  ZilchBindMethod(RemoveComponentByName);

  // Children
  ZilchBindGetter(Parent);
  ZilchBindMethod(FindRoot);
  ZilchBindGetter(Children);

  ZilchBindMethod(AttachTo);
  ZilchBindMethod(AttachToRelative);
  ZilchBindMethod(Detach);
  ZilchBindMethod(DetachRelative);

  ZilchBindMethod(FindChildByName);
  ZilchBindMethod(FindAllChildrenByName);

  ZilchBindMethod(IsDescendant);

  ZilchBindMethod(FindNextSibling);
  ZilchBindMethod(FindPreviousSibling);
  ZilchBindMethod(FindNextInOrder);
  ZilchBindMethod(FindPreviousInOrder);

  ZilchBindMethod(PlaceBeforeSibling);
  ZilchBindMethod(PlaceAfterSibling);
  ZilchBindMethod(ReplaceChild);
  
  // Archetypes
  ZilchBindMethod(IsModifiedFromArchetype);
  ZilchBindMethod(ClearArchetype);
  ZilchBindMethod(UploadToArchetype);
  ZilchBindMethod(FindNearestArchetype);
  ZilchBindMethod(FindRootArchetype);

  // Events
  ZilchBindMethod(DispatchEvent);
  ZilchBindMethod(DispatchUp);
  ZilchBindMethod(DispatchDown);

  // Other
  ZilchBindGetter(MarkedForDestruction);
  ZilchBindMethod(DebugDraw);

  ZeroBindEvent(Events::CogNameChanged, ObjectEvent);
  ZeroBindEvent(Events::TransformUpdated, ObjectEvent);
  ZeroBindEvent(Events::CogDestroy, ObjectEvent);
  ZeroBindEvent(Events::CogDelayedDestroy, ObjectEvent);
}

//**************************************************************************************************
void* Cog::operator new(size_t size)
{
  return sHeap->Allocate(size);
}

//**************************************************************************************************
void Cog::operator delete(void* pMem, size_t size)
{
  return sHeap->Deallocate(pMem, size);
}

//**************************************************************************************************
Cog::Cog()
{
  mObjectId = cInvalidCogId;
  mSpace = nullptr;
  mFlags = 0;
  mActionList = nullptr;
  mArchetype = nullptr;
  mHierarchyParent = nullptr;
  mSubContextId = 0;
  mChildId = PolymorphicNode::cInvalidUniqueNodeId;
}

//**************************************************************************************************
Cog::~Cog()
{
  //Remove from action list before component shutdown
  //(may be a space)
  SafeRelease(mActionList);
  DeleteComponents();
}

//**************************************************************************************************
void Cog::Destroy()
{
  if (!mFlags.IsSet(CogFlags::Protected))
    ForceDestroy();
}

//**************************************************************************************************
void Cog::ForceDestroy()
{
  // First queue up any children to be destroyed.
  if (Hierarchy* hierarchy = this->has(Hierarchy))
    hierarchy->DestroyChildren();

  // Clean up local modifications
  LocalModifications* modifications = LocalModifications::GetInstance();
  modifications->ClearModifications(this, false, false);
  forRange(Component* component, GetComponents())
    modifications->ClearModifications(component, false, false);

  // Signal the factory that this object needs to be destroyed
  // this will happen at the end of the frame. It's important
  // that this is called before destroying our children as we
  // want ourselves to be marked for deletion so our children
  // don't send events to us saying they were detached
  Z::gFactory->Destroy(this);

  ObjectEvent toSend;
  toSend.Source = this;
  DispatchEvent(Events::CogDestroy, &toSend);

  // Send an event to our parent saying that we were detached
  //if(HierarchyParent)
  //{
  //  // If our parent is also being destroyed, don't bother sending the event
  //  if(!HierarchyParent->MarkedForDeletion())
  //  {
  //    HierarchyEvent e;
  //    e.Parent = HierarchyParent;
  //    e.Child = this;
  //    HierarchyParent->DispatchEvent(Events::ChildDetached, &e);
  //  }
  //}
}

//**************************************************************************************************
String Cog::GetName( )
{
  return mName;
}

//**************************************************************************************************
void Cog::SetName(StringParam newName)
{
  if(!mName.Empty( ) && this->mFlags.IsSet(CogFlags::Protected))
    return;

  String sanatizedName = SanatizeName(newName);
  if(sanatizedName != newName)
    DoNotifyWarning("Invalid Symbols in Cog Name", "Invalid symbols were removed from cog name");

  if(mSpace)
  {
    mSpace->RemoveFromNameMap(this, mName);
    mSpace->AddToNameMap(this, sanatizedName);

    mSpace->ChangedObjects( );
  }

  mName = sanatizedName;

  Event event;
  DispatchEvent(Events::CogNameChanged, &event);
}

//**************************************************************************************************
void Cog::OnDestroy()
{
  if (mSpace)
    mSpace->RemoveFromNameMap(this, mName);

  ComponentRange range = mComponents.All();
  for (; !range.Empty(); range.PopFront())
    range.Front()->OnDestroy();

  ObjectEvent toSend;
  toSend.Source = this;
  DispatchEvent(Events::CogDelayedDestroy, &toSend);
}

//**************************************************************************************************
Space* Cog::GetSpace()
{
  return mSpace;
}

//**************************************************************************************************
GameSession* Cog::GetGameSession()
{
  Space* space = GetSpace();
  if (space != nullptr)
    return space->GetGameSession();
  return nullptr;
}

//**************************************************************************************************
Cog* Cog::GetLevelSettings()
{
  Space* space = GetSpace();
  if (space)
  {
    Cog* levelSettings = GetSpace()->FindObjectByName(SpecialCogNames::LevelSettings);

    return levelSettings;
  }

  return nullptr;
}

//**************************************************************************************************
Cog* Cog::Clone()
{
  // If we have modifications, we want to save to string to properly copy over
  // modified properties. Binary does not support property patching
  if (true)//IsModifiedFromArchetype())
  {
    String stringData = CogSerialization::SaveToStringForCopy(this);
    Cog* clonedObject = CreateFromString(GetSpace(), stringData);

    if (Cog* parent = GetParent())
      clonedObject->AttachTo(parent);

    return clonedObject;
  }

  DataBlock data = SaveToDataBlock(this);
  return CreateFromDataBlock(GetSpace(), data);
}

//**************************************************************************************************
void Cog::Serialize(Serializer& stream)
{
  CogSavingContext* context = static_cast<CogSavingContext*>(stream.GetSerializationContext());

  ErrorIf(context != nullptr && context->CurrentContextMode != ContextMode::Saving, "Not a saving context");

  // We need to save out the name differently in the legacy format (to support old projects)
  bool legacy = false;
  if (stream.GetType() == SerializerType::Text)
  {
    TextSaver& saver = *(TextSaver*)&stream;
    legacy = (saver.mVersion == DataVersion::Legacy);
  }

  if (legacy)
  {
    if (!mName.Empty())
    {
      Named named;
      named.Name = mName;
      stream.SerializePolymorphic(named);
    }
  }
  else
  {
    SerializeName(mName);
  }

  //If the object Id is valid this object may be reference
  //so save out the id as a component.
  if (!(mObjectId == cInvalidCogId) && context)
  {
    uint linkId = context->ToContextId(mObjectId.Id);
    stream.SerializeField("LinkId", linkId);
  }

  // We only want to save hidden and locked
  if (mFlags.IsSet(CogFlags::EditorViewportHidden | CogFlags::Locked))
  {
    EditorFlags flags;
    flags.mHidden = mFlags.IsSet(CogFlags::EditorViewportHidden);
    flags.mLocked = mFlags.IsSet(CogFlags::Locked);
    stream.SerializePolymorphic(flags);
  }

  ComponentCheckFn shouldSerializeCallback = nullptr;
  if (context != nullptr)
  {
    shouldSerializeCallback = context->ShouldSerializeComponentCallback;
  }

  ComponentRange range = mComponents.All();
  for (; !range.Empty(); range.PopFront())
  {
    Component* component = range.Front();
    //Should we serialize the component?
    bool shouldSerialize = range.Front()->ShouldSerialize() &&
      (!shouldSerializeCallback ||
        shouldSerializeCallback(this, component) == SerializeCheck::Serialized);

    if (shouldSerialize)
      stream.SerializePolymorphic(*component);
  }

  //Always save hierarchy last.
  if (Hierarchy* hierarchy = has(Hierarchy))
  {
    bool shouldSerialize = !shouldSerializeCallback ||
      shouldSerializeCallback(this, hierarchy) == SerializeCheck::Serialized;

    if (shouldSerialize)
    {
      LocalModifications* modifications = LocalModifications::GetInstance();

      PolymorphicInfo info;
      info.mRuntimeType = ZilchTypeId(Hierarchy);
      info.mTypeName = info.mRuntimeType->Name.c_str();

      bool validChildId = (mChildId != PolymorphicNode::cInvalidUniqueNodeId);
      if (validChildId && modifications->IsChildOrderModified(this))
        info.mFlags.SetFlag(PolymorphicSaveFlags::ChildOrderOverride);

      // Serialize the Hierarchy
      stream.SerializePolymorphic(info, *hierarchy);
    }
  }
}

//**************************************************************************************************
DataBlock Cog::SaveToDataBlock(Cog* cog)
{
  // Save to the buffer
  BinaryBufferSaver saver;
  saver.Open();
  saver.SerializePolymorphic(*cog);

  // Extract the data
  return saver.ExtractAsDataBlock();
}

//**************************************************************************************************
Cog* Cog::CreateFromDataBlock(Space* space, DataBlock& block)
{
  // Load the data
  BinaryBufferLoader loader;
  loader.SetBuffer(block.Data, block.Size);

  return CreateAndInitializeFromStream(space, loader);
}

//**************************************************************************************************
Cog* Cog::CreateFromString(Space* space, StringParam stringData)
{
  // Load the data
  ObjectLoader loader;
  Status status;
  loader.OpenBuffer(status, stringData);

  if (status.Failed())
    return nullptr;

  return CreateAndInitializeFromStream(space, loader);
}

//**************************************************************************************************
Cog* Cog::CreateAndInitializeFromStream(Space* space, Serializer& stream)
{
  // Create the object
  CogCreationContext context;
  context.mSpace = space;
  if (space)
    context.mGameSession = space->GetGameSession();
  Cog* cog = Z::gFactory->BuildFromStream(&context, stream);

  CogInitializer initializer(space);
  initializer.Context = &context;
  cog->Initialize(initializer);
  initializer.AllCreated();

  return cog;
}

//**************************************************************************************************
void Cog::Initialize(CogInitializer& initializer)
{
  ErrorIf(mSpace != nullptr, "Object has already been initialized.");

  initializer.CreationList.PushBack(this);
  ++initializer.AddCount;

  //If this composition is part of space
  if (initializer.mSpace != nullptr)
  {
    // Do not add to space list AllCreated will move when done.

    Space* space = initializer.mSpace;
    if (space != nullptr)
    {
      mSpace = space;

      //Level name not empty create from level
      if (!initializer.mLevel.Empty())
        this->mFlags.SetFlag(CogFlags::CreatedFromLevel);

      if (mHierarchyParent == nullptr)
      {
        mSpace->mRoots.PushBack(this);
        ++mSpace->mRootCount;
      }

      //Map the name
      mSpace->AddToNameMap(this, mName);
    }
  }

  //Initialize all components on this
  //composition.

  // Use indexes so components can be added during
  // Initialization.
  uint componentCount = mComponents.Size();
  for (uint i = 0; i<componentCount; ++i)
  {
    Component* component = mComponents[i];
    component->mOwner = this;
    component->Initialize(initializer);

    BoundType* componentType = ZilchVirtualTypeId(component);
    forRange(CogComponentMeta* meta, componentType->HasAll<CogComponentMeta>())
    {
      // Add component interfaces
      forRange(BoundType* interfaceType, meta->mInterfaces)
        AddComponentInterface(interfaceType, component);
    }
  }

  mFlags.SetFlag(CogFlags::Initialized);
}

//**************************************************************************************************
void Cog::OnAllObjectsCreated(CogInitializer& initializer)
{
  //Set a context id if available
  uint currentContextId = 0;
  if (initializer.Context)
  {
    currentContextId = initializer.Context->mCurrentSubContextId;
    if (this->mSubContextId)
      initializer.Context->mCurrentSubContextId = this->mSubContextId;
  }

  for (size_t i = 0; i < mComponents.Size(); ++i)
  {
    mComponents[i]->OnAllObjectsCreated(initializer);
  }

  if (initializer.Context)
    initializer.Context->mCurrentSubContextId = currentContextId;
}

//**************************************************************************************************
void Cog::ScriptInitialize(CogInitializer& initializer)
{
  for (size_t i = 0; i < mComponents.Size(); ++i)
  {
    mComponents[i]->ScriptInitialize(initializer);
  }
}

//**************************************************************************************************
bool Cog::IsInitialized() const
{
  return mFlags.IsSet(CogFlags::Initialized);
}

//**************************************************************************************************
bool Cog::AddComponent(Component* component, int index)
{
  // We need to check for missing dependencies and possible duplicates
  BoundType* componentType = ZilchVirtualTypeId(component);
  if (CheckForAdditionWithNotify(componentType) == false)
    return false;

  ForceAddComponent(component, index);
  return true;
}

//**************************************************************************************************
void Cog::ForceAddComponent(Component* component, int index)
{
  BoundType* componentType = ZilchVirtualTypeId(component);

  AddComponentInternal(componentType, component, index);
  CogInitializer initializer(mSpace);

  initializer.mParent = mHierarchyParent;
  initializer.Flags |= CreationFlags::DynamicallyAdded;
  component->mOwner = this;
  component->Initialize(initializer);

  //Do this now
  component->OnAllObjectsCreated(initializer);

  ComponentRange range = mComponents.All();
  for (; !range.Empty(); range.PopFront())
    range.Front()->ComponentAdded(componentType, component);

  Event e;
  GetDispatcher()->Dispatch(Events::ComponentsModified, &e);
}

//**************************************************************************************************
bool Cog::AddComponentByType(BoundType* componentType)
{
  ReturnIf(componentType == nullptr, false, "Invalid meta");

  // Check for the component
  Component* current = this->QueryComponentType(componentType);

  // Component already added
  if (current)
    return false;

  // Create the object (we know it must be a component because it was registered as one)
  uint flags = 0;
  if (GetSpace() && GetSpace()->IsEditorMode())
    flags |= CreationFlags::Editing;

  // Checks to see if the component type is valid to add
  if (CheckForAdditionWithNotify(componentType) == false)
    return false;

  // Create the component
  Component* component = ZilchAllocate(Component, componentType, HeapFlags::NonReferenceCounted);

  // If the returned component is null, most likely a script was
  // proxied and they're trying to create it by name.
  if (component == nullptr)
  {
    String msg = String::Format("Cannot add component '%s', component has most likely been proxied.", componentType->Name.c_str());
    DoNotifyException("Invalid component addition", msg);
    return false;
  }

  // Sets the defaults or serializes
  SetUpObject(component);

  // Add the component and initialize it
  return AddComponent(component);
}

//**************************************************************************************************
bool Cog::AddComponentByName(StringParam name)
{
  // Find the meta by name
  BoundType* componentType = MetaDatabase::GetInstance()->FindType(name);

  // As long as we have a meta...
  if (componentType != nullptr)
    return AddComponentByType(componentType);

  String message = "Attempt to add a component by name, but the type was not found";
  DoNotifyException("Could not add Component", message);
  return false;
}

//**************************************************************************************************
Component * Cog::QueryComponentType(BoundType* componentType)
{
  return mComponentMap.FindValue(componentType, nullptr);
}

//**************************************************************************************************
Component* Cog::GetComponentByName(StringParam componentTypeName)
{
  BoundType* componentType = MetaDatabase::GetInstance()->FindType(componentTypeName);

  if (componentType)
    return QueryComponentType(componentType);

  return nullptr;
}

//**************************************************************************************************
Component* Cog::GetComponentByIndex(size_t index)
{
  if (index >= mComponents.Size())
  {
    DoNotifyException("Cog", "When attempting to get a component, the index given was out of range");
    return nullptr;
  }

  return mComponents[index];
}

//**************************************************************************************************
size_t Cog::GetComponentCount()
{
  return mComponents.Size();
}

//**************************************************************************************************
uint Cog::GetComponentIndex(BoundType* componentType)
{
  for (uint i = 0; i < mComponents.Size(); ++i)
  {
    Component* component = mComponents[i];
    if (ZilchVirtualTypeId(component) == componentType)
      return i;
  }

  return uint(-1);
}

//**************************************************************************************************
Cog::ComponentRange Cog::GetComponents()
{
  return mComponents.All();
}

//**************************************************************************************************
bool Cog::RemoveComponent(Component* component)
{
  ErrorIf(component->mOwner != this, "Removing a component from a cog it doesn't belong to.");

  if (CheckForRemovalWithNotify(ZilchVirtualTypeId(component)) == false)
    return false;

  ForceRemoveComponent(component);

  return true;
}

//**************************************************************************************************
void Cog::ForceRemoveComponent(Component* component)
{
  ErrorIf(component->mOwner != this, "Removing a component from a cog it doesn't belong to.");

  // Set modified on the space and this object
  if (Space* space = GetSpace())
    space->ChangedObjects();

  // Disconnect any events sent to this
  // component because components use the cog's
  // EventReceiver
  mReceiver.Disconnect((Object*)component->GetEventThisObject());

  // Inform all the other components that this component
  // is being removed
  BoundType* typeId = ZilchVirtualTypeId(component);
  ComponentRange range = mComponents.All();
  for (; !range.Empty(); range.PopFront())
    range.Front()->ComponentRemoved(typeId, component);

  // Let the component do clean up
  component->OnDestroy(DestroyFlags::DynamicallyDestroyed);

  RemoveComponentInternal(component);

  Event e;
  GetDispatcher()->Dispatch(Events::ComponentsModified, &e);
}

//**************************************************************************************************
bool Cog::RemoveComponentByType(BoundType* componentType)
{
  // Error checking
  if(componentType == nullptr)
  {
    DoNotifyException("Could not remove Component", "null type given");
    return false;
  }

  // This check isn't really necessary as it will fail to find the Component, but it may
  // be useful for the user to see a more informative error message
  if(componentType->IsA(ZilchTypeId(Component)) == false)
  {
    String message = String::Format("Type of name '%s' is not a Component type",
                                    componentType->Name.c_str());
    DoNotifyException("Could not remove Component", message);
    return false;
  }

  // Query for the component
  Component* component = this->QueryComponentType(componentType);

  if (component != nullptr)
    return RemoveComponent(component);

  return false;
}

//**************************************************************************************************
bool Cog::RemoveComponentByName(StringParam typeName)
{
  // Find the meta by name
  BoundType* componentType = MetaDatabase::GetInstance()->FindType(typeName);

  // Error checking
  if(componentType == nullptr)
  {
    String message = String::Format("Type of name '%s' doesn't exist", typeName.c_str());
    DoNotifyException("Could not remove Component", message);
    return false;
  }

  return RemoveComponentByType(componentType);
}

//**************************************************************************************************
void Cog::MoveComponentBefore(uint componentToMove, uint destination)
{
  Component* component = mComponents[componentToMove];

  // Remove it and re-Insert it at the new location
  mComponents.EraseAt(componentToMove);

  if (componentToMove < destination && componentToMove != (destination - 1))
    mComponents.InsertAt(destination - 1, component);
  else
    mComponents.InsertAt(destination, component);

  Event e;
  GetDispatcher()->Dispatch(Events::ComponentsModified, &e);
}

//**************************************************************************************************
void Cog::AddComponentInterface(BoundType* alternateType, Component* component)
{
  mComponentMap.Insert(alternateType, component);

  // DO NOT add it the component list as it's just for looking it up by type
}

//**************************************************************************************************
void Cog::AddComponentInternal(BoundType* typeId, Component* component, int index)
{
  // Safeguard against bad indices
  if (index == -1 || index >= (int)mComponents.Size())
    mComponents.PushBack(component);
  else
    mComponents.InsertAt(index, component);

  mComponentMap.Insert(typeId, component);
  component->mOwner = this;

  BoundType* componentType = ZilchVirtualTypeId(component);
  forRange(CogComponentMeta* meta, componentType->HasAll<CogComponentMeta>())
  {
    // Add component interfaces
    forRange(BoundType* interfaceType, meta->mInterfaces)
      AddComponentInterface(interfaceType, component);
  }

  // Move Hierarchy to end
  uint hierarchyIndex = GetComponentIndex(ZilchTypeId(Hierarchy));
  if(hierarchyIndex != uint(-1))
  {
    Component* hierarchy = mComponents[hierarchyIndex];
    mComponents.EraseAt(hierarchyIndex);
    mComponents.PushBack(hierarchy);
  }
}

//**************************************************************************************************
void Cog::RemoveComponentInternal(Component* component)
{
  // Un map the component
  mComponentMap.EraseEqualValues(component);
  eraseEqualValues(mComponents, component);

  // Delete the component
  component->Delete();
}

//**************************************************************************************************
bool CheckForAddition(Cog* cog, BoundType* componentMeta, AddInfo& info)
{
  BoundType* cogType = ZilchVirtualTypeId(cog);

  bool canBeAdded = false;

  if (componentMeta != nullptr)
  {
    MetaComposition* composition = cogType->HasInherited<MetaComposition>();
    canBeAdded = composition->CanAddComponent(cog, componentMeta, &info);
  }

  return canBeAdded;
}

//**************************************************************************************************
bool Cog::CheckForAddition(BoundType* metaOfComponent)
{
  AddInfo info;
  return Zero::CheckForAddition(this, metaOfComponent, info);
}

//**************************************************************************************************
bool Cog::CheckForAdditionWithNotify(BoundType* componentType)
{
  AddInfo info;
  if (Zero::CheckForAddition(this, componentType, info) == false)
  {
    DoNotifyWarning("Can't Add Component.", info.Reason);
    return false;
  }

  return true;
}

//**************************************************************************************************
bool Cog::CheckForRemovalWithNotify(BoundType* componentType)
{
  if (componentType == nullptr)
    return true;

  // Query our Composition to see if we can be removed

  BoundType* cogType = ZilchVirtualTypeId(this);
  MetaComposition* composition = cogType->Has<MetaComposition>();

  String reason;
  if (composition->CanRemoveComponent(this, componentType, reason))
  {
    DoNotifyWarning("Can't Remove Component.", reason);
    return false;
  }

  composition->RemoveComponent(this, componentType);
  return true;
}

//**************************************************************************************************
void Cog::DeleteComponents()
{
  //When components are deleted we have to destroy the receivers
  mReceiver.DestroyConnections();

  //Delete each component using the component's virtual destructor
  //takes care of all resources and memory.
  ComponentRange range = mComponents.All();
  uint numberOfComponents = mComponents.Size();
  for (uint i = 0; i < numberOfComponents; ++i)
    mComponents[numberOfComponents - 1 - i]->Delete();

  //Clear all components
  mComponents.Clear();
  mComponentMap.Clear();

  if (mHierarchyParent)
  {
    //If this composition has a parent 
    //remove from the parents list and NULL it
    HierarchyList::Unlink(this);
    mHierarchyParent = nullptr;
  }
  else if (mSpace)
  {
    //otherwise object is a root object
    HierarchyList::Unlink(this);
    --mSpace->mRootCount;
  }

  if (mSpace != nullptr)
  {
    //Remove from space
    mSpace->RemoveObject(this);
    mSpace = nullptr;
  }
}

//**************************************************************************************************
Cog* Cog::GetParent()
{
  return mHierarchyParent;
}

//**************************************************************************************************
Cog* Cog::FindRoot()
{
  Cog* root = this;
  while (root->GetParent())
    root = root->GetParent();
  return root;
}

//**************************************************************************************************
HierarchyList::range Cog::GetChildren()
{
  Hierarchy* hierarchy = this->has(Hierarchy);
  if (hierarchy)
    return hierarchy->GetChildren();

  return HierarchyList::range();
}

//**************************************************************************************************
uint Cog::GetChildCount()
{
  uint count = 0;
  forRange(Cog& child, GetChildren())
    ++count;

  return count;
}

//**************************************************************************************************
void Cog::AttachTo(Cog* parent)
{
  if (parent == nullptr)
  {
    DoNotifyException("Invalid attachment", "Cannot attach to a null object.");
    return;
  }

  if (parent == this)
  {
    DoNotifyException("Invalid attachment", "Cannot attach to ourself.");
    return;
  }

  if (parent == mHierarchyParent)
  {
    DoNotifyException("Invalid attachment", "Already attached to our own parent.");
    return;
  }

  if (this->mFlags.IsSet(CogFlags::Protected))
  {
    DoNotifyException("Invalid attachment", "Cannot attach protected objects to other objects.");
    return;
  }

  // Don't bother doing anything if we're being destroyed
  if (GetMarkedForDestruction())
    return;

  // Check to make sure that the passed in parent is not already a child of us
  Cog* otherRootParent = parent;
  while (otherRootParent->GetParent())
  {
    otherRootParent = otherRootParent->GetParent();
    if (otherRootParent == this)
    {
      DoNotifyException("Invalid attachment", "Cannot attach to our own child.");
      return;
    }
  }

  // Deal with already having a parent
  if (GetParent() != parent)
    Detach();

  if (mHierarchyParent == nullptr)
  {
    Hierarchy* hierarchy = HasOrAdd<Hierarchy>(parent);

    // Remove from space
    HierarchyList::Unlink(this);
    --mSpace->mRootCount;

    // Add to new Hierarchy
    hierarchy->Children.PushBack(this);
    mHierarchyParent = parent;
  }

  // Attaching to an object may mean that children who are in-world need to transform,
  // first cache the current world matrix if we have one
  Mat4 oldMat;
  Transform* transform = this->has(Transform);
  if (transform != nullptr)
    oldMat = transform->GetWorldMatrix();

  // Signal attachment to all components
  AttachmentInfo info;
  info.Parent = parent;
  info.Child = this;

  ComponentRange range = mComponents.All();
  for (; !range.Empty(); range.PopFront())
    range.Front()->AttachTo(info);

  // Now apply the delta to the hierarchy after we've been attached (have to split this because we need old and new values)
  if (transform != nullptr)
    transform->UpdateAll(oldMat);

  // Also send an event to the object
  HierarchyEvent e;
  e.Parent = parent;
  e.Child = this;
  DispatchEvent(Events::Attached, &e);

  parent->DispatchEvent(Events::ChildAttached, &e);

  // Assign a new child id if we're under an archetype
  if (mChildId == PolymorphicNode::cInvalidUniqueNodeId && IsChildOfArchetype())
    mChildId = GenerateUniqueId64();

  // Space has now changed
  mSpace->ChangedObjects();
}

//**************************************************************************************************
void Cog::AttachToRelative(Cog* parent)
{
  if (parent == nullptr || parent == this)
    return;

  // Don't bother doing anything if we're being destroyed
  if (GetMarkedForDestruction())
    return;

  Transform* childTransform = this->has(Transform);

  // If the child has no Transform, there's no relative attachment needed
  if (childTransform == nullptr)
  {
    AttachTo(parent);
    return;
  }

  // Cannot attach an object with a Transform to an object without a Transform
  Transform* parentTransform = parent->has(Transform);
  if (parentTransform == nullptr)
    return;

  // Bring the child's transformation into the parent's space
  Mat4 parentTransformation = parentTransform->GetWorldMatrix();
  Mat4 childTransformation = childTransform->GetWorldMatrix();
  Mat4 relativeTransformation = parentTransformation.Inverted() * childTransformation;

  // Extract the transformation elements from the matrix
  Vec3 scale, translation;
  Mat3 rotation;
  relativeTransformation.Decompose(&scale, &rotation, &translation);

  // Attach the child
  AttachTo(parent);

  // Set his new transformation
  //if the child wants to be in world after the attachment, don't reset
  //the transform since he's staying where he was in the world
  if (!childTransform->GetInWorld())
  {
    //set his new transformation
    childTransform->SetScale(scale);
    childTransform->SetRotation(Math::ToQuaternion(rotation));
    childTransform->SetTranslation(translation);
  }
}

//**************************************************************************************************
void Cog::Detach()
{
  Cog* parent = mHierarchyParent;
  if (parent == nullptr)
    return;

  // Don't bother doing anything if we're being destroyed
  if (GetMarkedForDestruction())
    return;

  // Move from child list to root list
  HierarchyList::Unlink(this);

  // Clear hierarchy parent
  mHierarchyParent = nullptr;

  // Add to space root list
  mSpace->mRoots.PushBack(this);
  ++mSpace->mRootCount;

  // Detaching from an object may mean that children who are in-world need to transform,
  // first cache the current world matrix if we have one
  Mat4 oldMat;
  Transform* transform = this->has(Transform);
  if (transform != nullptr)
    oldMat = transform->GetWorldMatrix();

  // Signal detachment to all components
  AttachmentInfo info;
  info.Parent = parent;
  info.Child = this;

  ComponentRange range = mComponents.All();
  for (; !range.Empty(); range.PopFront())
    range.Front()->Detached(info);

  // Now apply the delta to the hierarchy after we've been attached (have to split this because we need old and new values)
  if (transform != nullptr)
    transform->UpdateAll(oldMat);

  // Also send an event to the object
  HierarchyEvent e;
  e.Parent = parent;
  e.Child = this;
  DispatchEvent(Events::Detached, &e);

  parent->DispatchEvent(Events::ChildDetached, &e);

  // Clear child id
  mChildId = PolymorphicNode::cInvalidUniqueNodeId;

  // Space has now changed
  mSpace->ChangedObjects();
}

//**************************************************************************************************
void Cog::DetachRelative()
{
  // Can't do anything if there's nothing to detach from
  if (mHierarchyParent == nullptr)
    return;

  // Don't bother doing anything if we're being destroyed
  if (GetMarkedForDestruction())
    return;

  Transform* childTransform = this->has(Transform);
  if (childTransform == nullptr)
  {
    Detach();
    return;
  }

  // Bring the child's transformation into the parent's space
  Mat4 childTransformation = childTransform->GetWorldMatrix();

  // Extract the transformation elements from the matrix
  Vec3 scale, translation;
  Mat3 rotation;
  childTransformation.Decompose(&scale, &rotation, &translation);

  // Detach the child
  Detach();

  // Set his new transformation
  childTransform->SetScale(scale);
  childTransform->SetRotation(Math::ToQuaternion(rotation));
  childTransform->SetTranslation(translation);
}

//**************************************************************************************************
Cog* Cog::FindChildByName(StringParam name)
{
  // Attempt to grab the hierarchy component
  Hierarchy* hierarchy = this->has(Hierarchy);

  // If we have a hierarchy
  if (hierarchy != nullptr)
  {
    // Loop through all the children
    forRange(Cog& child, hierarchy->Children.All())
    {
      // Get the name of the object and compare it
      if (child.GetName() == name)
      {
        // It's the object we wanted!
        return &child;
      }
      else
      {
        // Otherwise, search the children of this object...
        Cog* found = child.FindChildByName(name);

        // If we found it... return it
        if (found != nullptr)
          return found;
      }
    }
  }

  // Otherwise, we found nothing in this hierarchy
  return nullptr;
}

//**************************************************************************************************
HierarchyNameRange Cog::FindAllChildrenByName(StringParam name)
{
  // Create the policy and set the name
  NameCondition policy;
  policy.Name = name;

  // Return a conditional range that stores the hierarchy range and policy
  return HierarchyNameRange(HierarchyRange::SubTree(this), policy);
}

//**************************************************************************************************
Cog* Cog::FindChildByChildId(Guid childId)
{
  forRange(Cog& child, GetChildren())
  {
    if (child.mChildId == childId)
      return &child;
  }

  return nullptr;
}

//**************************************************************************************************
bool Cog::IsDescendant(Cog* cog)
{
  while (cog)
  {
    cog = cog->GetParent();

    if (cog == this)
      return true;
  }

  return false;
}

//**************************************************************************************************
Cog* Cog::FindNextSibling()
{
  HierarchyList* parentHierarchyList = GetParentHierarchyList();
  Cog* next = (Cog*)HierarchyList::Next(this);
  if (parentHierarchyList && next != parentHierarchyList->End())
    return next;
  return nullptr;
}

//**************************************************************************************************
Cog* Cog::FindPreviousSibling()
{
  HierarchyList* parentHierarchyList = GetParentHierarchyList();
  Cog* prev = (Cog*)HierarchyList::Prev(this);
  if (parentHierarchyList && prev != parentHierarchyList->End())
    return prev;
  return nullptr;
}

//**************************************************************************************************
Cog* Cog::FindNextInOrder()
{
  // If this object has children return the first child
  HierarchyList* hierarchyList = GetHierarchyList();
  if (hierarchyList && !hierarchyList->Empty())
    return &hierarchyList->Front();

  // Return next sibling if there is one
  Cog* nextSibling = FindNextSibling();
  if (nextSibling)
    return nextSibling;

  // Loop until the root or a parent has a sibling
  Cog* parent = GetParent();
  while (parent != nullptr)
  {
    Cog* parentSibling = parent->FindNextSibling();
    if (parentSibling)
      return parentSibling;
    else
      parent = parent->GetParent();
  }
  return nullptr;
}

//**************************************************************************************************
Cog* Cog::FindPreviousInOrder()
{
  // get prev sibling if there is one
  Cog* prevSibling = FindPreviousSibling();

  // If this is first node of a child it is previous node
  if (prevSibling == nullptr)
    return GetParent();

  // return the last child of the sibling
  return prevSibling->FindLastDeepestChild();
}

//**************************************************************************************************
void Cog::PlaceBeforeSibling(Cog* sibling)
{
  if (sibling->GetParent() != GetParent())
  {
    DoNotifyException("Cannot move object", "The objects must have the same parent.");
    return;
  }

  HierarchyList* list = GetParentHierarchyList();
  list->Erase(this);

  list->InsertBefore(sibling, this);

  Event eventToSend;
  if (GetParent())
    GetParent()->DispatchEvent(Events::ChildrenOrderChanged, &eventToSend);

  GetSpace()->ChangedObjects();
}

//**************************************************************************************************
void Cog::PlaceAfterSibling(Cog* sibling)
{
  if (sibling->GetParent() != GetParent())
  {
    DoNotifyException("Cannot move object", "The objects must have the same parent.");
    return;
  }

  HierarchyList* list = GetParentHierarchyList();
  list->Erase(this);

  list->InsertAfter(sibling, this);

  Event eventToSend;
  if (GetParent())
    GetParent()->DispatchEvent(Events::ChildrenOrderChanged, &eventToSend);

  GetSpace()->ChangedObjects();
}

//**************************************************************************************************
void Cog::ReplaceChild(Cog* oldChild, Cog* newChild)
{
  if (oldChild == nullptr)
  {
    DoNotifyException("Cannot replace child", "'oldChild' parameter is null.");
    return;
  }

  if (newChild == nullptr)
  {
    DoNotifyException("Cannot replace child", "'newChild' parameter is null.");
    return;
  }

  // Confirm the old child is actually our child
  Hierarchy* hierarchy = this->has(Hierarchy);
  if(hierarchy == nullptr || oldChild->GetParent() != this)
  {
    DoNotifyException("Cannot replace child", "'oldChild' is not a child of this object.");
    return;
  }

  // Attach the new child
  newChild->AttachTo(this);

  // Move to same location
  // When we attached the new child, it was added to the child list,
  // so we need to remove it and then place it back in after the old child
  hierarchy->Children.Erase(newChild);
  hierarchy->Children.InsertAfter(oldChild, newChild);

  oldChild->Detach();

  Event eventToSend;
  DispatchEvent(Events::ChildrenOrderChanged, &eventToSend);

  GetSpace()->ChangedObjects();
}

//**************************************************************************************************
uint Cog::GetHierarchyIndex()
{
  if (HierarchyList* list = GetParentHierarchyList())
    return list->FindIndex(this);
  return 0;
}

//**************************************************************************************************
void Cog::PlaceInHierarchy(uint index)
{
  HierarchyList* list = GetParentHierarchyList();
  uint currIndex = list->FindIndex(this);
  list->Erase(this);

  // We have to compensate for removing ourself from the list
  if (currIndex < index)
    list->InsertAt(index - 1, this);
  else
    list->InsertAt(index, this);

  Event eventToSend;
  if (GetParent())
    GetParent()->DispatchEvent(Events::ChildrenOrderChanged, &eventToSend);

  GetSpace()->ChangedObjects();
}

//**************************************************************************************************
Cog* Cog::FindLastDeepestChild()
{
  HierarchyList* hierarchyList = GetHierarchyList();
  if (hierarchyList && !hierarchyList->Empty())
    return hierarchyList->Back().FindLastDeepestChild();
  else
    return this;
}

//**************************************************************************************************
HierarchyList* Cog::GetHierarchyList()
{
  Hierarchy* hierarchy = this->has(Hierarchy);
  if (hierarchy)
    return &hierarchy->Children;
  return nullptr;
}

//**************************************************************************************************
HierarchyList* Cog::GetParentHierarchyList()
{
  Cog* parent = GetParent();
  if (parent == nullptr)
  {
    if (Space* space = GetSpace())
      return &space->mRoots;
    return nullptr;
  }
  return parent->GetHierarchyList();
}

//**************************************************************************************************
Archetype* Cog::GetArchetype()
{
  return mArchetype;
}

//**************************************************************************************************
void Cog::SetArchetype(Archetype* archetype)
{
  mArchetype = archetype;
}

//**************************************************************************************************
Archetype* Cog::GetBaseArchetype()
{
  if (Archetype* archetype = mArchetype)
    return ArchetypeManager::FindOrNull(archetype->mBaseResourceIdName);

  return nullptr;
}

//**************************************************************************************************
String Cog::GetBaseArchetypeName()
{
  if (Archetype* baseArchetype = GetBaseArchetype())
    return baseArchetype->Name;

  return "";
}

//**************************************************************************************************
bool Cog::IsModifiedFromArchetype()
{
  // Only ignore override properties if we're the root context (either root Archetype or a locally
  // added Archetype).
  Cog* nearestContext = FindNearestArchetypeContext();

  // If we aren't an Archetype and there isn't one above us, we're not modified
  if (nearestContext == nullptr)
    return false;

  bool ignoreOverrideProperties = (nearestContext == this);
  return CogIsModifiedFromArchetype(this, ignoreOverrideProperties);
}

//**************************************************************************************************
void Cog::ClearArchetype()
{
  // Clear all modifications
  LocalModifications* modifications = LocalModifications::GetInstance();
  modifications->ClearModifications(this, true, false);

  mArchetype = nullptr;
}

//**************************************************************************************************
void Cog::MarkNotModified()
{
  ClearCogModifications(this, false);
}

//**************************************************************************************************
void Cog::UploadToArchetype()
{
  if (!mArchetype)
  {
    DoNotifyError("No archetype", "No archetype to upload to.");
    return;
  }

  // No children can have this archetype
  // search all children for this archetype
  HierarchyRange r = HierarchyRange::SubTree(this);
  for (; !r.Empty(); r.PopFront())
  {
    Cog* child = &r.Front();
    if (child != this && child->mArchetype == this->mArchetype)
    {
      String message = String::Format("Archetype is already in this Hierarchy. "
        "Uploading will cause an infinite Hierarchy.");
      DoNotifyError("Can not upload", message);
      return;
    }
  }

  Archetype* archetype = mArchetype;

  // If we're a child of another Archetype, we may need to keep around some local changes after
  // we upload to Archetype.
  CachedModifications overlappingModifications;

  Cog* archetypeContextCog = FindNearestArchetypeContext();
  if (archetypeContextCog != this)
  {
    Archetype* archetypeContext = archetypeContextCog->GetArchetype();
    CachedModifications& cachedModifications = archetypeContext->GetAllCachedModifications();
    CachedModifications::ObjectNode* thisChildNode = cachedModifications.FindChildNode(archetypeContextCog, this);
    if (thisChildNode)
      overlappingModifications.StoreOverlappingModifications(this, thisChildNode);
  }

  // All local modifications on this object are no longer considered modifications because
  // they're becoming part of the Archetype.
  // This is true UNLESS we inherit from another Archetype. In that case, our modifications from
  // the base Archetype should remain.
  if (archetype->mBaseResourceIdName.Empty())
  {
    // We want to retain child archetype modifications because they are needed when saving out
    // those Archetypes
    ClearCogModifications(this, true);

    overlappingModifications.ApplyModificationsToObject(this);
  }

  // Local modifications are contextual. When we upload a Cog instance to the Archetype
  // definition, we're switching contexts.
  //
  // Example:
  // Lets say we have an Archetype called Enemy. This Archetype has a child object
  // that is the Gun Archetype, with the Damage locally modified from the Gun Archetype
  // within the context of the Enemy.
  // 
  // In our level, we have an instance of Enemy. LocalModifications on this Cog are modifications
  // from the Enemy Archetype; they do not include any modifications to the Gun that are part 
  // of the Enemy Archetype (such as the modified Damage).
  // 
  // If we upload to Archetype on this instance in our level, it will not have the modified Damage
  // stored in LocalModifications, and thus won't save out that modification, losing data.
  // This is because that modification is in a different context.
  // In a way, we're switching contexts, so we need to combine our modifications with the
  // modifications that are local to the Enemy Archetype (the changed Damage).
  //
  // If we only ever allowed people to modify the Archetype in its own window, this would not be
  // an issue.
  CachedModifications& archetypeModifications = archetype->GetAllCachedModifications();
  archetypeModifications.ApplyModificationsToObject(this, true);

  // Assign all children child-id's if they don't already have them
  AssignChildIds(this);

  // Update the resource and save to library
  // If uploaded this will also clear modified
  ArchetypeManager::GetInstance()->SaveToContent(this, mArchetype);

  // We have to clear again for two reasons:
  // 1. Locally added child Archetypes won't get their modifications cleared because they were
  //    required when saving out this Archetype definition. We want to clear them on the instance
  //    because those modifications are now part of the Archetype's context, not the instance
  // 2. Any cached modifications we applied to the object before saving. See comment above
  //    applying the archetypes cached modifications in this function.
  ClearCogModifications(this, false);

  overlappingModifications.ApplyModificationsToObject(this);

  // Send out an event to rebuild all archetypes
  ArchetypeManager::GetInstance()->ArchetypeModified(mArchetype);
}

//**************************************************************************************************
void Cog::RevertToArchetype()
{
  // We can only be reverted if we are an Archetype or are a child of an Archetype

  // When we revert to Archetype on an object, we want to clear all modifications, except override
  // properties that are on the root Archetype.
  // This is slightly incorrect...
  // Lets say we have an 'Enemy' Archetype, and we locally added a 'Gun' Archetype as a child. If we
  // were to revert the 'Gun' Archetype, we want it to retain its override properties, otherwise it
  // would move to the translation specified in the 'Gun' Archetype definition.
  // Because of this, we call the 'FindNearestArchetypeContext' instead of 'FindRootArchetype'.
  if (Cog* nearestArchetypeContext = FindNearestArchetypeContext())
  {
    // No need to do anything if we aren't already modified from archetype
    if (IsModifiedFromArchetype())
    {
      // Marking this object as not modified will cause
      // the saving system to ignore all other properties.
      MarkNotModified();

      // Apply the changes
      ArchetypeRebuilder::RebuildCog(nearestArchetypeContext);
    }
  }
}

//**************************************************************************************************
void Cog::MarkTransformModified()
{
  if (Transform* t = this->has(Transform))
  {
    LocalModifications* modifications = LocalModifications::GetInstance();
    if (ObjectState* state = modifications->GetObjectState(t, true))
    {
      const PropertyPath cTranslation("Translation");
      state->SetPropertyModified(cTranslation, true);

      const PropertyPath cRotation("Rotation");
      state->SetPropertyModified(cRotation, true);

      const PropertyPath cScale("Scale");
      state->SetPropertyModified(cScale, true);
    }
  }
}

//**************************************************************************************************
Cog* Cog::FindNearestArchetypeContext()
{
  LocalModifications* modifications = LocalModifications::GetInstance();

  // This could possibly be made generic in meta
  Cog* rootArchetype = nullptr;
  Cog* current = this;

  while (current)
  {
    if (current->GetArchetype())
      rootArchetype = current;

    Cog* parent = current->GetParent();

    // Stop if the current object is locally added
    if (parent && modifications->IsChildLocallyAdded(parent->has(Hierarchy), current))
      break;

    current = parent;
  }

  return rootArchetype;
}


//**************************************************************************************************
Cog* Cog::FindNearestParentArchetype()
{
  Cog* parent = GetParent();
  while (parent)
  {
    if (parent->GetArchetype())
      return parent;
    parent = parent->GetParent();
  }

  return nullptr;
}

//**************************************************************************************************
Cog* Cog::FindNearestArchetype()
{
  if (GetArchetype())
    return this;
  return FindNearestParentArchetype();
}

//**************************************************************************************************
Cog* Cog::FindRootArchetype()
{
  Cog* rootArchetype = nullptr;
  Cog* current = this;
  while (current)
  {
    if (current->GetArchetype())
      rootArchetype = current;

    current = current->GetParent();
  }

  return rootArchetype;
}

//**************************************************************************************************
bool Cog::IsChildOfArchetype()
{
  return (FindNearestParentArchetype() != nullptr);
}

//**************************************************************************************************
void Cog::DispatchEvent(StringParam eventId, Event* event)
{
  GetDispatcher()->Dispatch(eventId, event);
}

//**************************************************************************************************
void Cog::DispatchUp(StringParam eventId, Event* event)
{
  Cog* parent = GetParent();
  if (parent)
  {
    parent->DispatchEvent(eventId, event);
    parent->DispatchUp(eventId, event);
  }
}

//**************************************************************************************************
void Cog::DispatchDown(StringParam eventId, Event* event)
{
  Hierarchy* hierarchy = this->has(Hierarchy);
  if (hierarchy)
  {
    forRange(HierarchyList::sub_reference child, hierarchy->GetChildren())
    {
      child.DispatchEvent(eventId, event);
      child.DispatchDown(eventId, event);
    }
  }
}

//**************************************************************************************************
EventDispatcher* Cog::GetDispatcherObject()
{
  return GetDispatcher();
}

//**************************************************************************************************
EventReceiver* Cog::GetReceiverObject()
{
  return GetReceiver();
}

//**************************************************************************************************
EventDispatcher* Cog::GetDispatcher()
{
  return &mDispatcher;
}

//**************************************************************************************************
EventReceiver* Cog::GetReceiver()
{
  return &mReceiver;
}

//**************************************************************************************************
bool Cog::IsEditorMode()
{
  Space* space = this->GetSpace();
  if (space)
    return space->IsEditorMode();
  else
    return false;
}

//**************************************************************************************************
bool Cog::IsPreviewMode()
{
  Space* space = this->GetSpace();
  if (space)
    return space->IsPreviewMode();
  else
    return false;
}

//**************************************************************************************************
bool Cog::IsEditorOrPreviewMode()
{
  Space* space = this->GetSpace();
  if (space)
    return space->IsEditorOrPreviewMode();
  else
    return false;
}

//**************************************************************************************************
void Cog::DebugDraw()
{
  for (size_t i = 0; i < mComponents.Size(); ++i)
    mComponents[i]->DebugDraw();
}

//**************************************************************************************************
String Cog::SanatizeName(StringParam newName)
{
  // 'FixIdentifier' will return "empty" if the string is empty, so we need to special case it here
  // because we allow for empty names
  if (newName.Empty())
    return newName;

  return LibraryBuilder::FixIdentifier(newName, TokenCheck::None, '\0');
}

//**************************************************************************************************
CogId Cog::GetId()
{
  if (mObjectId == cInvalidCogId)
  {
    Z::gTracker->IdGameObject(this);
    return mObjectId;
  }
  else
  {
    return mObjectId;
  }
}

//**************************************************************************************************
u32 Cog::GetRuntimeId()
{
  CogId cogId = GetId();
  return cogId.Id;
}

//**************************************************************************************************
void Cog::SetEditorOnly()
{
  mFlags.SetFlag(CogFlags::Protected | CogFlags::Transient | CogFlags::Persistent);
}

//**************************************************************************************************
bool Cog::GetTransient()
{
  return mFlags.IsSet(CogFlags::Transient);
}

//**************************************************************************************************
void Cog::SetTransient(bool state)
{
  SetCogFlag(this, CogFlags::Transient, "Transient", state);
  forRange(Cog& child, GetChildren().All())
  {
    child.SetTransient(state);
  }
}

//**************************************************************************************************
bool Cog::GetPersistent()
{
  return mFlags.IsSet(CogFlags::Persistent);
}

//**************************************************************************************************
void Cog::SetPersistent(bool state)
{
  SetCogFlag(this, CogFlags::Persistent, "Persistent", state);
}

//**************************************************************************************************
bool Cog::GetEditorViewportHidden()
{
  return mFlags.IsSet(CogFlags::EditorViewportHidden);
}

//**************************************************************************************************
void Cog::SetEditorViewportHidden(bool state)
{
  SetCogFlag(this, CogFlags::EditorViewportHidden, "Editor Viewport Hidden", state);
  // set the hidden state for all the children of this cog
  forRange(Cog& child, GetChildren().All())
  {
    child.SetEditorViewportHidden(state);
  }
}

//**************************************************************************************************
bool Cog::GetObjectViewHidden()
{
  return mFlags.IsSet(CogFlags::ObjectViewHidden);
}

//**************************************************************************************************
void Cog::SetObjectViewHidden(bool state)
{
  SetCogFlag(this, CogFlags::ObjectViewHidden, "Object View Hidden", state);
  forRange(Cog& child, GetChildren().All())
  {
    child.SetObjectViewHidden(state);
  }
}

//**************************************************************************************************
bool Cog::GetLocked()
{
  return mFlags.IsSet(CogFlags::Locked);
}

//**************************************************************************************************
void Cog::SetLocked(bool state)
{
  SetCogFlag(this, CogFlags::Locked, ":Locked", state);
  // set the locked state for all the children of this cog
  forRange(Cog& child, GetChildren().All())
  {
    child.SetLocked(state);
  }
}

//**************************************************************************************************
void Cog::TransformUpdate(TransformUpdateInfo& info)
{
  ComponentRange range = mComponents.All();
  for (; !range.Empty(); range.PopFront())
  {
    Component* component = range.Front();
    component->TransformUpdate(info);
  }

  ObjectEvent toSend;
  toSend.Source = this;
  DispatchEvent(Events::TransformUpdated, &toSend);
}

//**************************************************************************************************
bool Cog::GetMarkedForDestruction() const
{
  return mFlags.IsSet(CogFlags::Destroyed);
}

//**************************************************************************************************
void Cog::WriteDescription(StringBuilder& builder)
{
  BoundType* type = ZilchVirtualTypeId(this);
  builder << type->Name;
  if (!mName.Empty())
    builder << " '" << mName << "'";
  if (mArchetype)
    builder << " (" << mArchetype->Name << ")";
  builder << " [" << GetRuntimeId() << "]";
}

//**************************************************************************************************
String Cog::GetDescription()
{
  StringBuilder builder;
  builder << "<";
  WriteDescription(builder);
  builder << ">";
  return builder.ToString();
}

//**************************************************************************************************
Actions* Cog::GetActions()
{
  if (mActionList == nullptr)
  {
    Cog* space = GetSpace();
    if (!space)
      return nullptr;

    mActionList = new Actions(space->has(ActionSpace));
    mActionList->AddReference();
  }
  return mActionList;
}

//------------------------------------------------------------------------------------------ Helpers
//**************************************************************************************************
void SetCogFlag(Cog* cog, CogFlags::Enum flag, cstr flagName, bool state)
{
  if(cog->mFlags.IsSet(CogFlags::Protected))
  {
    String title = BuildString("Cannot Set ", flagName);
    DoNotifyException("Cannot Set Persistent", "Object is protected");
    return;
  }

  cog->mFlags.SetState(flag, state);
  cog->GetSpace()->ChangedObjects();
}

//**************************************************************************************************
// Similar to clearing Cog modifications, we have to do this in a custom way due to how child
// properties work in an Archetype. See the comment for the 'ClearCogModifications' function for
// a more detailed explanation. The reasoning is the same for this operation.
// This could possibly be made generic in meta.
bool CogIsModifiedFromArchetype(Cog* cog, bool ignoreOverrideProperties)
{
  LocalModifications* modifications = LocalModifications::GetInstance();

  // Check any modifications on ourself (name change, added / removed components)
  if(modifications->IsModified(cog, false, ignoreOverrideProperties))
    return true;

  // Check modifications to our children list (added / removed Cogs)
  if(Hierarchy* hierarchy = cog->has(Hierarchy))
  {
    if(modifications->IsModified(hierarchy, false, ignoreOverrideProperties))
      return true;
  }

  // Check for modifications on our components
  forRange(Component* component, cog->GetComponents())
  {
    // Ignore the Hierarchy Component as we're handling child objects manually
    if(ZilchVirtualTypeId(component) == ZilchTypeId(Hierarchy))
      continue;

    if(modifications->IsModified(component, true, ignoreOverrideProperties))
      return true;
  }

  // Check for modifications on our children
  // Never retain child override properties (see example about the enemy holding a gun)
  forRange(Cog& child, cog->GetChildren())
  {
    // Never retain child override properties (see example about the enemy holding a gun)
    if(CogIsModifiedFromArchetype(&child, false))
      return true;
  }

  return false;
}

//**************************************************************************************************
// The reason for a custom method of clearing modifications on a Cog is because of how override
// properties are handled in hierarchies. The 'Translation' property on a Cog is marked as
// 'LocalModificationOverride'. This is because when we revert an objects modified properties,
// we want the object to stay in the same location.
// This is true for all properties on the root object in the Archetype. However, it's not true
// for child objects in the Archetype. If there is an enemy in the world, we want him to stay
// at the same position when reverted. If the enemy is holding a gun and we move the gun,
// we want the gun to go back to its original position relative to the enemy when reverted.
// This could possibly be made generic in meta.
void ClearCogModifications(Cog* rootCog, Cog* cog, ObjectState::ModifiedProperties& cachedMemory,
                           bool retainOverrideProperties, bool retainChildArchetypeModifications)
{
  LocalModifications* modifications = LocalModifications::GetInstance();

  // When uploading to Archetype, modifications are cleared, then the object is saved to 
  // the Archetype file. We still want to save out modifications to locally added children,
  // so don't touch them
  if(cog->GetArchetype() != nullptr)
  {
    bool isRoot = (rootCog == cog);
    if(retainChildArchetypeModifications && !isRoot)
      return;
  }

  // Clear modifications on ourself (Components added / removed)
  modifications->ClearModifications(cog, false, retainOverrideProperties, cachedMemory);

  forRange(Component* component, cog->GetComponents())
  {
    // Ignore the Hierarchy Component as we're handling child objects manually
    if(ZilchVirtualTypeId(component) == ZilchTypeId(Hierarchy))
      continue;

    modifications->ClearModifications(component, true, retainOverrideProperties, cachedMemory);
  }

  // Never retain child override properties (see example about the enemy holding a gun)
  forRange(Cog& child, cog->GetChildren())
    ClearCogModifications(rootCog, &child, cachedMemory, false, retainChildArchetypeModifications);

  // Clear modifications to our children list (added / removed Cogs)
  // The hierarchy modifications should be cleared after recursing because we need this state
  // to determine if our children are locally added
  if(Hierarchy* hierarchy = cog->has(Hierarchy))
    modifications->ClearModifications(hierarchy, false, retainOverrideProperties);
}

//**************************************************************************************************
void ClearCogModifications(Cog* root, bool retainChildArchetypeModifications)
{
  // Store override properties. This is passed down to re-use allocated memory
  ObjectState::ModifiedProperties cachedMemory;

  // Only keep around override properties if this is the root Archetype
  Cog* nearestArchetypeContext = root->FindNearestArchetypeContext();
  bool retainOverride = (nearestArchetypeContext == root);

  ClearCogModifications(root, root, cachedMemory, retainOverride, retainChildArchetypeModifications);
}

//**************************************************************************************************
void AssignChildIds(Cog* parent)
{
  forRange(Cog& child, parent->GetChildren())
  {
    if(child.mChildId == PolymorphicNode::cInvalidUniqueNodeId)
      child.mChildId = GenerateUniqueId64();

    // Assign to children as long as we don't enter a new Archetype (they should
    // already have child id's assigned to them)
    if (child.mArchetype == nullptr)
      AssignChildIds(&child);
  }
}

//**************************************************************************************************
template<typename type>
void eraseEqualValues(Array<type>& mArray, type value)
{
  uint index = 0;
  for(uint i = 0; i < mArray.Size();)
  {
    if(mArray[i] == value)
      mArray.Erase(&mArray[i]);
    else
      ++i;
  }
}

//------------------------------------------------------------------------------- Cog Handle Manager
//**************************************************************************************************
void CogHandleManager::Allocate(BoundType* type, Handle& handleToInitialize, size_t customFlags)
{
  handleToInitialize.Flags |= HandleFlags::NoReferenceCounting;

  CogHandleData& data = *(CogHandleData*)(handleToInitialize.Data);
  data.mCogId = CogId();
  data.mRawObject = zAllocate(type->Size);
}

//**************************************************************************************************
void CogHandleManager::ObjectToHandle(const byte* object, BoundType* type, Handle& handleToInitialize)
{
  if (object == nullptr)
    return;

  Cog* cog = (Cog*)object;
  CogHandleData& data = *(CogHandleData*)(handleToInitialize.Data);
  data.mCogId = cog;
  data.mRawObject = nullptr;
}

//**************************************************************************************************
byte* CogHandleManager::HandleToObject(const Handle& handle)
{
  const CogHandleData& data = *(const CogHandleData*)(handle.Data);

  if (data.mRawObject)
  {
    Cog* cog = (Cog*)data.mRawObject;
    return (byte*)cog;
  }

  return (byte*)(Cog*)data.mCogId;
}

//**************************************************************************************************
void CogHandleManager::Delete(const Handle& handle)
{
  Cog* cog = handle.Get<Cog*>();
  if (cog)
    cog->Destroy();
}

//**************************************************************************************************
bool CogHandleManager::CanDelete(const Handle& handle)
{
  return true;
}

//**************************************************************************************************
size_t CogHandleManager::Hash(const Handle& handle)
{
  const CogHandleData& data = *(const CogHandleData*)(handle.Data);
  return (int)data.mCogId.Hash();
}

}//namespace Zero
