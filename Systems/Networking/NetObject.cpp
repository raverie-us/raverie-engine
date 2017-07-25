///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrew Colean.
/// Copyright 2015, DigiPen Institute of Technology.
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//---------------------------------------------------------------------------------//
//                                  NetObject                                      //
//---------------------------------------------------------------------------------//

ZilchDefineType(NetObject, builder, type)
{
  ZeroBindComponent();

  // Bind tags
  ZeroBindTag(Tags::Networking);

  // Bind documentation
  ZeroBindDocumented();

  // Is just a NetObject? (Not a more derived type?)
  if(type == ZilchTypeId(NetObject))
  {
    // Bind dependencies
    ZeroBindDependency(Cog);
  }

  // Bind setup (can be added in the editor)
  ZeroBindSetup(SetupMode::DefaultSerialization);

  // Bind peer interface
  ZilchBindGetterProperty(Role);
  ZilchBindCustomGetter(IsClient);
  ZilchBindCustomGetter(IsServer);
  ZilchBindCustomGetter(IsOffline);
  ZilchBindCustomGetter(IsClientOrOffline);
  ZilchBindCustomGetter(IsServerOrOffline);
  ZilchBindCustomGetter(IsClientOrServer);

  // Bind object interface
  ZilchBindGetterSetterProperty(DetectOutgoingChanges);
  ZilchBindGetterSetterProperty(AcceptIncomingChanges);
  ZilchBindGetterSetterProperty(AllowNapping);

  // Is not a NetPeer or NetSpace? (Accurate timestamps may be configured by the user?)
  if(!type->IsA(ZilchTypeId(NetPeer))
  && !type->IsA(ZilchTypeId(NetSpace)))
  {
    ZilchBindGetterSetterProperty(AccurateTimestampOnOnline);
    ZilchBindGetterSetterProperty(AccurateTimestampOnChange);
    ZilchBindGetterSetterProperty(AccurateTimestampOnOffline);
  }

  ZilchBindGetterProperty(OnlineTimestamp);
  ZilchBindGetterProperty(LastChangeTimestamp);
  ZilchBindGetterProperty(OfflineTimestamp);
  ZilchBindGetterProperty(OnlineTimePassed);
  ZilchBindGetterProperty(LastChangeTimePassed);
  ZilchBindGetterProperty(OfflineTimePassed);
  ZilchBindCustomGetterProperty(IsOnline);
  ReflectionObject* netObjectIdProperty = ZilchBindGetterProperty(NetObjectId);

  /* METAREFACTOR - config
  if(!Z::gEngine->GetConfigCog()->has(DeveloperConfig)) // Not a developer?
  {
    // Hide from property grid
    netObjectIdProperty->AddAttribute(PropertyAttributes::cHidden);
  }
  */
  ZilchBindCustomGetterProperty(IsNapping);
  ZilchBindMethodProperty(SelectRemote);
  ZilchBindMethodProperty(ReplicateNow);
  ZilchBindMethodProperty(WakeUp);
  ZilchBindMethodProperty(TakeNap);
  ZilchBindMethodProperty(Forget);

  // Bind channel interface
  ZilchBindMethod(HasNetChannel);
  ZilchBindMethod(GetNetChannel);

  // Is not a NetPeer or NetSpace? (Is a NetObject that may be conceptually owned?)
  if(!type->IsA(ZilchTypeId(NetPeer))
  && !type->IsA(ZilchTypeId(NetSpace)))
  {
    // Bind ownership interface
    ZilchBindCustomGetter(IsOwnedByAUser);
    ZilchBindCustomGetter(IsNotOwnedByAUser);
    ZilchBindMethod(IsOwnedByUser);
    // ZilchBindMethod(IsOwnedByUserId);
    ZilchBindMethod(IsOwnedByPeer);
    ZilchBindCustomGetterProperty(IsMine);
    ZilchBindCustomGetter(IsNotMine);
    ZilchBindCustomGetter(IsClientAndMine);
    ZilchBindCustomGetter(IsClientButNotMine);
    ZilchBindCustomGetter(IsServerAndMine);
    ZilchBindCustomGetter(IsServerButNotMine);
    ZilchBindCustomGetter(IsOfflineAndMine);
    ZilchBindCustomGetter(IsOfflineButNotMine);
    ZilchBindGetterSetterProperty(NetUserOwner);
    ReflectionObject* netUserOwnerUserIdProperty = ZilchBindGetterProperty(NetUserOwnerUserId);
    ReflectionObject* netUserOwnerPeerIdProperty = ZilchBindGetterProperty(NetUserOwnerPeerId);
    /* METAREFACTOR - DevConfig
    if(!Z::gEngine->GetConfigCog()->has(DeveloperConfig)) // Not a developer?
    {
      // Hide from property grid
      netUserOwnerUserIdProperty->AddAttribute(PropertyAttributes::cHidden);
      netUserOwnerPeerIdProperty->AddAttribute(PropertyAttributes::cHidden);
    }*/
    // ZilchBindMethod(SetNetUserOwnerUpById);
    ZilchBindMethod(SetNetUserOwnerUp);
    // ZilchBindMethod(SetNetUserOwnerDownById);
    ZilchBindMethod(SetNetUserOwnerDown);
  }

  // Bind network dispatch interface
  ZilchBindMethod(DispatchLocal);
  ZilchBindMethod(DispatchRemote);
  ZilchBindMethod(DispatchBroadcast);
  ZilchBindMethod(DispatchLocalAndRemote);
  ZilchBindMethod(DispatchLocalAndBroadcast);

  // Bind channel configuration interface
  ZilchBindGetterSetterProperty(AutomaticChannel);

  // METAREFACTOR handle CreateMetaForContainer - needs to be bound as pointer with MetaContainer and container interface (think Zilch Ranges)
  // Bind property info interface
  //BoundType* containerType = CreateMetaForContainer<NetPropertyInfoArray>("NetProperties");
  ZilchBindFieldProperty(mNetPropertyInfos); // METAREFACTOR array
}

NetObject::NetObject()
  : Replica(),
    Component(),
    mInitLevelResourceIdName(),
    mIsAncestor(false),
    mFamilyTreeId(0),
    mIsOnline(false),
    mNetUserOwnerUserId(0),
    mAutomaticChannel(),
    mNetPropertyInfos()
{
  ResetConfig();
}

NetObject::~NetObject()
{
}

//
// Component Interface
//

void NetObject::Serialize(Serializer& stream)
{
  // Serialize data members
  SerializeNameDefault(mDetectOutgoingChanges, GetDetectOutgoingChanges());
  SerializeNameDefault(mAcceptIncomingChanges, GetAcceptIncomingChanges());
  SerializeNameDefault(mAllowNapping, GetAllowNapping());
  SerializeResourceName(mAutomaticChannel, NetChannelConfigManager);
  SerializeNameDefault(mNetPropertyInfos, NetPropertyInfoArray());
}

void NetObject::Initialize(CogInitializer& initializer)
{
  // Get owner
  Cog* owner = GetOwner();

  // Is editor or preview mode?
  if(owner->IsEditorOrPreviewMode())
    return;

  // Get gamesession
  GameSession* gameSession = owner->GetGameSession();
  if(!gameSession) // Unable?
    return;

  // Get net peer
  NetPeer* netPeer = gameSession->has(NetPeer);
  if(!netPeer) // Unable?
  {
    // (This can happen if the user forgot to add a net peer component to their game session)
    DoNotifyError("Invalid NetObject GameSession",
                  String::Format("Unable to initialize NetObject - NetObject '%s' was created in the GameSession '%s' which does not have a NetPeer component."
                                 " Please add a NetPeer component to the GameSession.",
                                 owner->GetDescription().c_str(), gameSession->GetDescription().c_str()));
    return;
  }

  //
  // Initialize Replica Type
  //

  // Set replica type (archetype resource ID)
  InitializeReplicaType();

  //
  // Determine Initialization Type
  //

  // Get space (if any)
  Space* space = GetSpace();

  // Created by level initialization?
  if(space && space->mIsLoadingLevel)
  {
    // Get the level's resource ID name
    String levelResourceIdName = space->GetCurrentLevel()->ResourceIdName;
    Assert(!levelResourceIdName.Empty());

    // Set the level's resource ID name
    SetInitializationLevelResourceIdName(levelResourceIdName);
    Assert(WasLevelInitialized());
  }
  // Created by cog initialization?
  else
    Assert(WasCogInitialized());

  //
  // Register NetProperties
  //

  // Connect event handlers
  ConnectThisTo(owner, Events::Attached,                 OnAttached);
  ConnectThisTo(owner, Events::Detached,                 OnDetached);
  ConnectThisTo(owner, Events::RegisterCppNetProperties, OnRegisterCppNetProperties);

  // Add C++ component net properties
  AddCppNetProperties();

  // Add script component net properties
  AddScriptNetProperties();

  // Add configured (property grid) component net properties
  AddConfiguredNetProperties();

  // Add net channel authority net properties
  AddNetChannelAuthorityNetProperties();
}

void NetObject::OnAllObjectsCreated(CogInitializer& initializer)
{
  // Get owner
  Cog* owner = GetOwner();

  // Get parent (if any)
  Cog* parent = owner->GetParent();

  // Is editor or preview mode?
  if(owner->IsEditorOrPreviewMode())
    return;

  // Get net peer
  NetPeer* netPeer = GetNetPeer();
  if(!netPeer) // Unable?
    return;

  // Not the net peer?
  if(!IsNetPeer())
  {
    // Get space
    Space* space = owner->GetSpace();
    if(!space) // Unable?
      return;

    // Get net space
    NetSpace* netSpace = GetNetSpace();
    if(!netSpace) // Unable?
    {
      // (This can happen if the user forgot to add a net space component to their space)
      DoNotifyError("Invalid NetObject Space",
                    String::Format("Unable to initialize NetObject - NetObject '%s' was created in the Space '%s' which does not have a NetSpace component."
                                   " Please add a NetSpace component to the Space.",
                                   owner->GetDescription().c_str(), space->GetDescription().c_str()));
      return;
    }
  }

  // Parent is not a net object?
  if(parent && !parent->has(NetObject))
  {
    // (This can happen if the user forgot to add a net object component to their parent)
    DoNotifyError("Invalid NetObject Hierarchy",
                    String::Format("Unable to initialize NetObject - NetObject Child '%s' was attached to Parent '%s' which does not have a NetObject component."
                                   " Please add a NetObject component to the Parent.",
                                   owner->GetDescription().c_str(), parent->GetDescription().c_str()));
    return;
  }

  //
  // Initialize Is Ancestor, Create Context, and Family Tree
  //

  // Set is-ancestor flag
  InitializeIsAncestor();

  // Set create context (space net object ID)
  InitializeCreateContext();

  // Is client or server, and the ancestor?
  if(IsClientOrServer() && IsAncestor())
  {
    // (Ancestor's create context and replica type must be initialized by this point)
    Assert(GetCreateContext().IsNotEmpty() && GetReplicaType().IsNotEmpty());

    // Create the complete family tree representing this ancestor and all of it's descendants
    InitializeFamilyTree();
  }

  //
  // Bring NetObject Online
  //

  // Net peer not open?
  if(!netPeer->IsOpen())
  {
    // (This indicates it is either invalid or unnecessary to bring the net object online here)
    // (If it is unnecessary to bring the net object online here, it will be brought online elsewhere)
    return;
  }

  // Is client and created by cog initialization?
  if(IsClient() && WasCogInitialized())
  {
    // Claim the active replica stream
    if(!netPeer->ClaimActiveReplicaStream(&initializer)) // Unable?
    {
      DoNotifyWarning("Unauthorized NetObject Creation Attempted",
                      String::Format("The Client attempted to illegally create a NetObject '%s'."
                      " Clients are not permitted to spawn NetObjects."
                      " Try creating the NetObject on the Server instead.",
                      owner->GetDescription().c_str()));

      // Remove this net object from it's family tree (if it hasn't been removed already)
      GetNetPeer()->RemoveNetObjectFromFamilyTree(this);

      // Mark object for destruction and return
      owner->Destroy();
      return;
    }

    // Get the active replica stream
    const ReplicaStream* replicaStream = netPeer->GetActiveReplicaStream();
    Assert(replicaStream);

    // Read identification information (such as IsAbsent, ReplicaId, IsCloned, IsEmplaced, EmplaceContext, and EmplaceId) from the replica stream
    bool isAbsent = false;
    ReadIdentificationInfo(replicaStream, isAbsent);

    // Replica is supposed to be absent (destroyed/forgotten)?
    if(isAbsent)
    {
      // Remove this net object from it's family tree (if it hasn't been removed already)
      GetNetPeer()->RemoveNetObjectFromFamilyTree(this);

      // Mark object for destruction and return
      owner->Destroy();
      return;
    }

    // (Should not be marked for destruction at this point)
    Assert(!owner->GetMarkedForDestruction());

    // Read channel data (such as forward and reverse ReplicaChannels) from the replica stream
    ReadChannelData(replicaStream);
  }

  // Get net space
  NetSpace* netSpace = GetNetSpace();

  // We are receiving a net game clone?
  if(netPeer && netSpace && netPeer->IsReceivingNetGame())
  {
    Assert(netPeer->IsClient());

    // Fulfill delayed attachments (if any), now that this object exists locally
    // (This object may be the delayed parent of another object)
    netSpace->FulfillDelayedAttachments(owner);
  }

  // Bring net object online (may be frame-delayed depending on role)
  BringNetObjectOnline();
}

void NetObject::OnDestroy(uint flags)
{
  // Get owner
  Cog* owner = GetOwner();

  // Get gamesession
  GameSession* gameSession = owner->GetGameSession();
  if(!gameSession) // Unable?
    return;

  // Is editor or preview mode?
  if(owner->IsEditorOrPreviewMode())
    return;

  // Get net peer
  NetPeer* netPeer = GetNetPeer();
  if(!netPeer) // Unable?
  {
    // (This can happen if the user forgot to add a net peer component to their game session)
    return;
  }

  //
  // Take NetObject Offline
  //

  // Net peer not open?
  if(!netPeer->IsOpen())
  {
    // (This indicates it is either invalid or unnecessary to take the net object offline here)
    // (If it is unnecessary to take the net object offline here, it will be taken offline elsewhere)
    return;
  }

  // Determine if this was caused by the game quitting
  bool causedByGameQuitting = gameSession->mQuiting;

  // Is client?
  if(netPeer->IsClient())
  {
    //     Net object still online?
    // AND Not caused by the game quitting?
    if(IsOnline() && !causedByGameQuitting)
    {
      DoNotifyWarning("Unauthorized NetObject Destruction",
                      String::Format("The Client illegally destroyed a NetObject '%s'."
                      " Clients are not permitted to destroy NetObjects."
                      " Try destroying the NetObject on the Server instead.",
                      owner->GetDescription().c_str()));
    }
  }

  // Take net object offline
  TakeNetObjectOffline();
}

void NetObject::OnAttached(HierarchyEvent* event)
{
  // Get owner
  Cog* owner = GetOwner();

  // Get parent
  Cog* parent = event->Parent;

  // Parent is not a net object?
  if(!parent->has(NetObject))
  {
    // (This can happen if the user forgot to add a net object component to their parent)
    DoNotifyError("Invalid NetObject Attachment",
                  String::Format("Unable to replicate NetObject attachment - NetObject Child '%s' was attached to Parent '%s' which does not have a NetObject component."
                                 " Please add a NetObject component to the Parent.",
                                 owner->GetDescription().c_str(), parent->GetDescription().c_str()));
    return;
  }

  // Is server?
  if(IsServer())
  {
    // Replicate parent changes now (if any)
    if(NetChannel* netObjectChannel = GetNetChannel("NetObject"))
      netObjectChannel->ReplicateNow();
    else
      Assert(false);
  }
}
void NetObject::OnDetached(HierarchyEvent* event)
{
  // Is server?
  if(IsServer())
  {
    // Replicate parent changes now (if any)
    if(NetChannel* netObjectChannel = GetNetChannel("NetObject"))
      netObjectChannel->ReplicateNow();
    else
      Assert(false);
  }
}

//
// NetProperty Registration
//

void NetObject::AddCppNetProperties()
{
  // Notify C++ components (including ourself) to add their net properties
  RegisterCppNetProperties event;
  GetOwner()->DispatchEvent(Events::RegisterCppNetProperties, &event);
}
void NetObject::OnRegisterCppNetProperties(RegisterCppNetProperties* event)
{
  // Get owner
  Cog* owner = GetOwner();

  // Add 'built-in' net object channel
  NetChannel* netObjectChannel = AddNetChannel("NetObject");
  if(!netObjectChannel) // Unable?
  {
    DoNotifyError("Unable to Add Built-In C++ NetProperties", String::Format("Unable to add built-in 'NetObject' channel on the NetObject '%s'", owner->GetDescription().c_str()));
    return;
  }

  // Configure net object channel
  // (Note: Since we know exactly when the net properties included in this built-in channel are changed,
  // we don't need to wait until the end of the frame to detect changes. So we use manual replication instead.
  // This improves both responsiveness and performance, since we don't need to poll all these net objects constantly)
  netObjectChannel->GetNetChannelType()->SetDetectOutgoingChanges(false);
  netObjectChannel->GetNetChannelType()->SetAcceptIncomingChanges(true);
  netObjectChannel->GetNetChannelType()->SetEventOnOutgoingPropertyChange(false);
  netObjectChannel->GetNetChannelType()->SetEventOnIncomingPropertyChange(true);

  // Add parent net property (replicates parent changes)
  NetProperty* parentProperty = netObjectChannel->AddBasicNetProperty("Parent", Variant(this), NativeTypeOf(NetObjectId), SerializeKnownExtendedVariant, GetNetObjectParentProperty, SetNetObjectParentProperty);
  if(!parentProperty) // Unable?
  {
    DoNotifyError("Unable to Add Built-In C++ NetProperties", String::Format("Unable to add built-in 'Parent' NetProperty to the 'NetObject' channel on the NetObject '%s'", owner->GetDescription().c_str()));
    return;
  }

  // Add owning network user net property (replicates network owner changes)
  NetProperty* netUserOwnerIdProperty = netObjectChannel->AddBasicNetProperty("NetUserOwnerUserId", mNetUserOwnerUserId);
  if(!netUserOwnerIdProperty) // Unable?
  {
    DoNotifyError("Unable to Add Built-In C++ NetProperties", String::Format("Unable to add built-in 'NetUserOwnerUserId' NetProperty to the 'NetObject' channel on the NetObject '%s'", owner->GetDescription().c_str()));
    return;
  }
}
void NetObject::AddScriptNetProperties()
{
  // Get owner
  Cog* owner = GetOwner();

  // For all components
  Cog::ComponentRange components = owner->GetComponents();
  forRange(Component* component, components)
  {
    // Get component type
    BoundType* componentType = ZilchVirtualTypeId(component);
    if(!componentType) // Unable?
    {
      Assert(false);
      continue; // Skip component
    }

    // Not a scripted component?
    if(componentType->Native)
      continue; // Skip component

    // For all properties
    MemberRange<Property> properties = componentType->GetProperties(Members::InheritedInstanceExtension);
    forRange(Property* property, properties)
    {
      // Get net property attribute
      Attribute* netPropertyAttribute = property->HasAttribute(cNetProperty);
      if(!netPropertyAttribute) // Unable? (Is not a net property?)
        continue; // Skip property

      // (Should be a valid net property since we already validate properties with the net property attribute)
      Assert(IsValidNetProperty(property));

      //
      // Get NetPropertyConfig Parameter
      //

      // Get net property type name
      String netPropertyTypeName = "Default";

      // Net property has a "netPropertyConfig" attribute parameter?
      AttributeParameter* netPropertyConfigAttributeParameter = netPropertyAttribute->HasAttributeParameter("netPropertyConfig");
      if(netPropertyConfigAttributeParameter)
      {
        // Set net property type name
        netPropertyTypeName = netPropertyConfigAttributeParameter->StringValue;
      }

      // Find net property config resource by name (if one exists)
      NetPropertyConfig* netPropertyConfig = NetPropertyConfigManager::FindOrNull(netPropertyTypeName);

      //
      // Get NetChannelConfig Parameter
      //

      // Get desired net channel name (assume automatic channel resource name by default)
      String netChannelName = mAutomaticChannel->Name;

      // Net property has a "netChannelConfig" attribute parameter?
      AttributeParameter* netChannelConfigAttributeParameter = netPropertyAttribute->HasAttributeParameter("netChannelConfig");
      if(netChannelConfigAttributeParameter)
      {
        // Set desired net channel name
        netChannelName = netChannelConfigAttributeParameter->StringValue;
      }

      // Find net channel config resource by name (if one exists)
      NetChannelConfig* netChannelConfig = NetChannelConfigManager::FindOrNull(netChannelName);

      //
      // Add NetProperty With Config Parameters
      //

      // Add net property to the specified channel (which will also be added and configured if it doesn't already exist)
      bool result = AddNetPropertyToChannel(component, property, netPropertyTypeName, netPropertyConfig, netChannelName, netChannelConfig);
      Assert(result);
    }
  }
}
void NetObject::AddConfiguredNetProperties()
{
  // Get owner
  Cog* owner = GetOwner();

  // For all net property infos
  forRange(NetPropertyInfo& netPropertyInfo, mNetPropertyInfos.All())
  {
    // Get meta property (specified by component and property name)
    Property* property = netPropertyInfo.mComponentType->GetProperty(netPropertyInfo.mPropertyName);
    if(!property) // Unable?
    {
      DoNotifyWarning("Unable To Add Configured NetProperty",
                      String::Format("Unable to add NetProperty '%s' configured on Component '%s' to the NetChannel '%s' on the NetObject '%s' - Unable to get component property meta",
                      property->Name.c_str(), netPropertyInfo.mComponentType->Name.c_str(), netPropertyInfo.mNetChannelConfig->Name.c_str(), owner->GetDescription().c_str()));
      continue; // Skip net property info
    }

    // Not a valid net property?
    if(!IsValidNetProperty(property))
    {
      Assert(false);
      continue; // Skip net property info
    }

    // Has net property attribute?
    Attribute* netPropertyAttribute = property->HasAttribute(cNetProperty);
    if(netPropertyAttribute)
      continue; // Skip net property info (We've already added the net property above)

    // Get component instance from owner
    Component* component = owner->QueryComponentType(netPropertyInfo.mComponentType);
    if(!component) // Unable?
    {
      // TODO: Uncomment this error message. We're just temporarily ignoring this error.

      // DoNotifyWarning("Unable To Add Configured NetProperty",
      //                 String::Format("Unable to add NetProperty '%s' configured on Component '%s' to the NetChannel '%s' on the NetObject '%s' - Unable to get component instance from owner",
      //                 property->Name.c_str(), netPropertyInfo.mComponentType->TypeName.c_str(), netPropertyInfo.mNetChannelConfig->Name.c_str(), owner->GetDescription().c_str()));
      continue; // Skip net property info
    }

    // Add net property to the specified channel (which will also be added and configured if it doesn't already exist)
    bool result = AddNetPropertyToChannel(component, property, netPropertyInfo.mNetPropertyConfig, netPropertyInfo.mNetChannelConfig);
    Assert(result);
  }
}
void NetObject::AddNetChannelAuthorityNetProperties()
{
  // Get owner
  Cog* owner = GetOwner();

  // Get 'built-in' net object channel
  NetChannel* netObjectChannel = GetNetChannel("NetObject");
  if(!netObjectChannel) // Unable?
  {
    DoNotifyError("Unable to Add Built-In Authority NetProperties", String::Format("Unable to get built-in 'NetObject' channel on the NetObject '%s'", owner->GetDescription().c_str()));
    return;
  }

  // For all replica channels
  forRange(ReplicaChannel* replicaChannel, GetReplicaChannels().All())
  {
    // Get replica channel type
    ReplicaChannelType* replicaChannelType = replicaChannel->GetReplicaChannelType();

    // Replica channel type has dynamic change authority mode?
    if(replicaChannelType->GetAuthorityMode() == AuthorityMode::Dynamic)
    {
      // Create net property name ("ChannelName_Authority")
      String netPropertyName = String::Format("%s_Authority", replicaChannel->GetName().c_str());

      // Add net channel authority net property (replicates net channel authority changes)
      NetProperty* authorityProperty = netObjectChannel->AddBasicNetProperty(netPropertyName, Variant(replicaChannel), NativeTypeOf(Any), SerializeKnownExtendedVariant, GetNetChannelAuthorityProperty, SetNetChannelAuthorityProperty);
      if(!authorityProperty) // Unable?
      {
        DoNotifyError("Unable to Add Built-In Authority NetProperties",
                      String::Format("Unable to add NetProperty '%s' to the built-in 'NetObject' channel on the NetObject '%s' - Error adding NetProperty",
                      netPropertyName.c_str(), owner->GetDescription().c_str()));
        return;
      }
    }
  }
}

//
// NetObject Scope
//

void NetObject::InitializeIsAncestor()
{
  // Net peer not open?
  NetPeer* netPeer = GetNetPeer();
  if(!netPeer || !netPeer->IsOpen())
  {
    // Clear is-ancestor flag
    // (This case indicates the NetObject is going to be emplaced against the game session)
    // (We don't use ancestors/descendants for emplacement so we must set ancestor to false here)
    mIsAncestor = false;
    return;
  }

  // Get owner
  Cog* owner = GetOwner();

  // Determine if we are the ancestor (is root, has an archetype, and was cog initialized)
  bool isRoot            = owner->GetParent()    == nullptr;
  bool hasArchetype      = owner->GetArchetype() != nullptr;
  bool wasCogInitialized = WasCogInitialized();
  Assert(isRoot ? owner->FindRoot() == owner : true);

  // Set is-ancestor flag
  mIsAncestor = (isRoot && hasArchetype && wasCogInitialized);
}
void NetObject::InitializeCreateContext()
{
  // Get create context (space net object ID)
  NetObjectId spaceNetObjectId = 0;

  // Is a net peer (game session)?
  if(IsNetPeer())
  {
    // Use 0 as the create context
    // (We don't use create context for game sessions because we only ever emplace them)
    spaceNetObjectId = 0;
  }
  // Is a net space?
  else if(IsNetSpace())
  {
    // Use 0 as the create context
    // (Indicates a net space is to be created in the game session)
    spaceNetObjectId = 0;
  }
  // Is just a net object?
  else
  {
    // Get our net space
    NetSpace* netSpace = GetNetSpace();
    if(!netSpace) // Unable?
    {
      // (This can happen if the user forgot to add a net space component to their space)
      // (We currently give an error prior to this)
      return;
    }

    // Use our net space's net object ID as the create context
    // (Indicates a net object is to be created in the specified net space)
    spaceNetObjectId = netSpace->GetNetObjectId();

    // Our net space is still invalid by this point?
    if(spaceNetObjectId == 0)
    {
      // (This can happen for net objects created in an emplaced net space, which won't be online yet)
      // (This is intended behavior, not an error. Emplaced net objects created in an emplaced net space will never have a create context!)
      // (This can also happen when a net object is created immediately after a netspace is created, because the net space won't be brought online until next frame)
      return;
    }
  }

  // Set create context
  SetCreateContext(CreateContext(spaceNetObjectId));
}
void NetObject::InitializeReplicaType()
{
  // Get owner
  Cog* owner = GetOwner();

// Using Archetype "ResourceId:Name" String as ReplicaType? (Easier to debug)
#ifdef NETOBJECT_USE_RESOURCE_ID_NAME_STRING

  // Set replica type to archetype resource ID name (if there is an archetype)
  String archetypeResourceIdName = owner->GetArchetype() ? owner->GetArchetype()->ResourceIdName : String();
  Replica::SetReplicaType(ReplicaType(archetypeResourceIdName));

// Using Archetype ResourceId u64 as ReplicaType? (Much more efficient)
#else

  // Set replica type to archetype resource ID (if there is an archetype)
  u64 archetypeResourceId = (u64)(owner->GetArchetype() ? owner->GetArchetype()->mResourceId : ResourceId(0));
  Replica::SetReplicaType(ReplicaType(archetypeResourceId));

#endif
}

void NetObject::InitializeFamilyTree()
{
  Assert(IsClientOrServer());
  Assert(IsAncestor());

  // Add net object root to family tree as ancestor (this will create the family tree)
  // Add net object children recursively to family tree as descendants (in depth-first pre-order traversal order)
  AddDownFamilyTree(this);
}
void NetObject::AddDownFamilyTree(NetObject* ancestor)
{
  Assert(IsClientOrServer());
  Assert(GetFamilyTreeId() == 0);
  Assert(WasCogInitialized());

  // Get owner
  Cog* owner = GetOwner();

  // Get net peer
  NetPeer* netPeer = GetNetPeer();

  // Add this net object to the family tree
  bool result = netPeer->AddNetObjectToFamilyTree(ancestor, this);
  Assert(result);

  // Add this net object's children to the family tree
  forRange(Cog& cog, owner->GetChildren())
    if(NetObject* netObject = cog.has(NetObject))
      netObject->AddDownFamilyTree(ancestor);
}

void NetObject::ReadIdentificationInfo(const ReplicaStream* replicaStream, bool& isAbsent)
{
  // Read identification information (such as IsAbsent, ReplicaId, IsCloned, IsEmplaced, EmplaceContext, and EmplaceId) from the replica stream
  bool result = replicaStream->ReadIdentificationInfo(isAbsent, this);
  Assert(result);
}
void NetObject::ReadChannelData(const ReplicaStream* replicaStream)
{
  // Read channel data (such as forward and reverse ReplicaChannels) from the replica stream
  bool result = replicaStream->ReadChannelData(this);
  Assert(result);
}

void NetObject::BringNetObjectOnline()
{
  // Get owner
  Cog* owner = GetOwner();

  // Get net peer
  NetPeer* netPeer = GetNetPeer();

  // Get net space
  NetSpace* netSpace = GetNetSpace();

  // (Sanity checks)
  Assert(!owner->IsEditorOrPreviewMode());
  Assert(netPeer->IsOpen());
  Assert(Replica::IsInvalid());

  // Is offline role?
  if(IsOffline())
  {
    // Bring net object online next engine update (if it still exists)
    // (We artificially frame-delay this operation to stay consistent with the server role which must naturally frame-delay)
    netSpace->mPendingNetObjects.PushBack(owner->mObjectId);
  }
  // Is client/server role?
  else
  {
    Assert(IsClientOrServer());

    // Was level initialized?
    if(WasLevelInitialized())
    {
      // Is server?
      if(IsServer())
      {
        // Emplace net object by level next engine update (if it still exists)
        // (We naturally frame-delay this operation to allow initial net property values to be set in script)
        netSpace->mPendingNetObjects.PushBack(owner->mObjectId);
      }
      // Is client?
      else
      {
        Assert(IsClient());

        // Emplace net object by level now
        // (We cannot frame-delay this operation, nor is there any benefit to doing so for clients)
        if(!netPeer->EmplaceNetObjectBySpaceAndLevel(owner, owner->GetSpace(), GetInitializationLevelResourceIdName())) // Unable?
          return;
      }
    }
    // Was cog initialized?
    else
    {
      Assert(WasCogInitialized());

      // Is server?
      if(IsServer())
      {
        // Spawn net object by level next engine update (if it still exists)
        // (We naturally frame-delay this operation to allow initial net property values to be set in script)
        netSpace->mPendingNetObjects.PushBack(owner->mObjectId);
      }
      // Is client?
      else
      {
        Assert(IsClient());

        // (Nothing for clients to do here)
      }
    }
  }
}
void NetObject::TakeNetObjectOffline()
{
  // Get owner
  Cog* owner = GetOwner();

  // Get net peer
  NetPeer* netPeer = GetNetPeer();

  // (Sanity checks)
  Assert(!owner->IsEditorOrPreviewMode());
  Assert(netPeer->IsOpen());

  // Determine if this was caused by the game quitting
  bool causedByGameQuitting = GetGameSession()->mQuiting;

  // Determine if this was caused by a level transition
  Level* pendingLevel = GetSpace() ? GetSpace()->mPendingLevel : nullptr;
  bool causedByLevelTransition = pendingLevel;

  // Is offline?
  if(netPeer->IsOffline())
  {
    // Net object still online?
    if(IsOnline())
    {
      // Handle net object going offline
      HandleNetObjectOffline();
    }
  }
  // Is client/server role?
  else
  {
    Assert(IsClientOrServer());

    // Is server?
    if(netPeer->IsServer())
    {
      // Net object still online?
      if(IsOnline())
      {
        // Handle net object going offline
        HandleNetObjectOffline();
      }

      //     Not caused by a level transition?
      // AND Not caused by the game quitting?
      if(!causedByLevelTransition && !causedByGameQuitting)
      {
        // Net object still live?
        if(IsLive())
        {
          // Destroy net object
          if(!netPeer->DestroyNetObject(owner, Route::All)) // Unable?
            return;
        }
      }
      // Caused by a level transition?
      else if(causedByLevelTransition)
      {
        // (Sanity check: Net peers and net spaces shouldn't fall into this case)
        Assert(!IsNetPeer());
        Assert(!IsNetSpace());
      }
    }

    //     Net object still valid?
    // AND We don't already have an uninitialization timestamp? (This can get called *during* a DestroyNetObject command, so we must check this first)
    if(IsValid()
    && Replica::GetUninitializationTimestamp() == cInvalidMessageTimestamp)
    {
      // Forget net object locally
      if(!netPeer->ForgetNetObject(owner, Route::All)) // Unable?
        return;
    }
  }
}

void NetObject::HandleNetObjectOnline()
{
  // (Should not already be online)
  Assert(!mIsOnline);

  // Set net object as online
  mIsOnline = true;

  // Create event
  NetObjectOnline event;
  event.mGameSession       = GetGameSession();
  event.mSpace             = GetSpace();
  event.mObject            = GetOwner();
  event.mIsStartOfLifespan = !Replica::IsCloned(); // TODO: Implement this for network relevance

  // Handle special behavior according to net object derived type before dispatching the online event
  HandleNetObjectOnlinePreDispatch(&event);

  // Get event ID according to net object derived type
  const String& eventId = GetNetObjectOnlineEventId();

  // Dispatch event on object
  if(event.mObject)
    event.mObject->DispatchEvent(eventId, &event);

  // Dispatch event on space
  if(event.mSpace && (event.mObject != event.mSpace))
    event.mSpace->DispatchEvent(eventId, &event);

  // Dispatch event on game session
  if(event.mGameSession && (event.mObject != event.mGameSession))
    event.mGameSession->DispatchEvent(eventId, &event);
}
const String& NetObject::GetNetObjectOnlineEventId() const
{
  return Events::NetObjectOnline;
}
void NetObject::HandleNetObjectOnlinePreDispatch(NetObjectOnline* event)
{
  // (Nothing special to do here for net object base type)
}
void NetObject::HandleNetObjectOffline()
{
  // (Should still be online)
  Assert(mIsOnline);

  // Get net peer
  NetPeer* netPeer = GetNetPeer();

  // Get net space
  NetSpace* netSpace = GetNetSpace();

  // We are receiving a net game clone?
  if(netPeer && netSpace && netPeer->IsReceivingNetGame())
  {
    Assert(netPeer->IsClient());

    // Remove delayed attachment (if any)
    netSpace->RemoveDelayedAttachment(GetNetObjectId());
  }

  // Create event
  NetObjectOffline event;
  event.mGameSession     = GetGameSession();
  event.mSpace           = GetSpace();
  event.mObject          = GetOwner();
  event.mIsEndOfLifespan = true; // TODO: Implement this for network relevance

  // Get event ID according to net object derived type
  const String& eventId = GetNetObjectOfflineEventId();

  // Dispatch event on object
  if(event.mObject)
    event.mObject->DispatchEvent(eventId, &event);

  // Dispatch event on space
  if(event.mSpace && (event.mObject != event.mSpace))
    event.mSpace->DispatchEvent(eventId, &event);

  // Dispatch event on game session
  if(event.mGameSession && (event.mObject != event.mGameSession))
    event.mGameSession->DispatchEvent(eventId, &event);

  // Handle special behavior according to net object derived type after dispatching the offline event
  HandleNetObjectOfflinePostDispatch(&event);

  // Is server or offline?
  if(IsServerOrOffline())
  {
    // Clear net user owner
    SetNetUserOwnerUserId(NetUserId(0));
  }

  // Is client or server, and was cog initialized?
  if(IsClientOrServer() && WasCogInitialized())
  {
    // Remove this net object from it's family tree (if it hasn't been removed already)
    GetNetPeer()->RemoveNetObjectFromFamilyTree(this);
  }

  // Set net object as offline
  mIsOnline = false;
}
const String& NetObject::GetNetObjectOfflineEventId() const
{
  return Events::NetObjectOffline;
}
void NetObject::HandleNetObjectOfflinePostDispatch(NetObjectOffline* event)
{
  // (Nothing special to do here for net object base type)
}

//
// Peer Interface
//

Role::Enum NetObject::GetRole() const
{
  NetPeer* netPeer = GetNetPeer();
  return netPeer ? netPeer->GetRole() : Role::Unspecified;
}
bool NetObject::IsClient() const
{
  return GetRole() == Role::Client;
}
bool NetObject::IsServer() const
{
  return GetRole() == Role::Server;
}
bool NetObject::IsOffline() const
{
  return GetRole() == Role::Offline;
}
bool NetObject::IsClientOrOffline() const
{
  Role::Enum role = GetRole();
  return role == Role::Client
      || role == Role::Offline;
}
bool NetObject::IsServerOrOffline() const
{
  Role::Enum role = GetRole();
  return role == Role::Server
      || role == Role::Offline;
}
bool NetObject::IsClientOrServer() const
{
  Role::Enum role = GetRole();
  return role == Role::Client
      || role == Role::Server;
}

//
// Object Interface
//

void NetObject::ResetConfig()
{
  SetDetectOutgoingChanges();
  SetAcceptIncomingChanges();
  SetAllowNapping();
  SetAccurateTimestampOnOnline();
  SetAccurateTimestampOnChange();
  SetAccurateTimestampOnOffline();
}

void NetObject::SetDetectOutgoingChanges(bool detectOutgoingChanges)
{
  Replica::SetDetectOutgoingChanges(detectOutgoingChanges);
}
bool NetObject::GetDetectOutgoingChanges() const
{
  return Replica::GetDetectOutgoingChanges();
}

void NetObject::SetAcceptIncomingChanges(bool acceptIncomingChanges)
{
  Replica::SetAcceptIncomingChanges(acceptIncomingChanges);
}
bool NetObject::GetAcceptIncomingChanges() const
{
  return Replica::GetAcceptIncomingChanges();
}

void NetObject::SetAllowNapping(bool allowNapping)
{
  Replica::SetAllowNapping(allowNapping);
}
bool NetObject::GetAllowNapping() const
{
  return Replica::GetAllowNapping();
}

void NetObject::SetAccurateTimestampOnOnline(bool accurateTimestampOnOnline)
{
  Replica::SetAccurateTimestampOnInitialization(accurateTimestampOnOnline);
}
bool NetObject::GetAccurateTimestampOnOnline() const
{
  return Replica::GetAccurateTimestampOnInitialization();
}

void NetObject::SetAccurateTimestampOnChange(bool accurateTimestampOnChange)
{
  Replica::SetAccurateTimestampOnChange(accurateTimestampOnChange);
}
bool NetObject::GetAccurateTimestampOnChange() const
{
  return Replica::GetAccurateTimestampOnChange();
}

void NetObject::SetAccurateTimestampOnOffline(bool accurateTimestampOnOffline)
{
  Replica::SetAccurateTimestampOnUninitialization(accurateTimestampOnOffline);
}
bool NetObject::GetAccurateTimestampOnOffline() const
{
  return Replica::GetAccurateTimestampOnUninitialization();
}

float NetObject::GetOnlineTimestamp() const
{
  // Get initialization timestamp
  TimeMs timestamp = Replica::GetInitializationTimestamp();
  if(timestamp == cInvalidMessageTimestamp) // Invalid?
    return 0;

  return TimeMsToFloatSeconds(timestamp);
}
float NetObject::GetLastChangeTimestamp() const
{
  // Get last change timestamp
  TimeMs timestamp = Replica::GetLastChangeTimestamp();
  if(timestamp == cInvalidMessageTimestamp) // Invalid?
    return 0;

  return TimeMsToFloatSeconds(timestamp);
}
float NetObject::GetOfflineTimestamp() const
{
  // Get uninitialization timestamp
  TimeMs timestamp = Replica::GetUninitializationTimestamp();
  if(timestamp == cInvalidMessageTimestamp) // Invalid?
    return 0;

  return TimeMsToFloatSeconds(timestamp);
}

float NetObject::GetOnlineTimePassed() const
{
  // Get replicator
  Replicator* replicator = Replica::GetReplicator();
  if(!replicator) // Unable?
    return 0;

  // Get current time
  TimeMs now = replicator->GetPeer()->GetLocalTime();

  // Get initialization timestamp
  TimeMs timestamp = Replica::GetInitializationTimestamp();
  if(timestamp == cInvalidMessageTimestamp) // Invalid?
    return 0;

  // Compute time passed since initialization (duration between now and initialization timestamp)
  TimeMs timePassed = (now - timestamp);
  return TimeMsToFloatSeconds(timePassed);
}
float NetObject::GetLastChangeTimePassed() const
{
  // Get replicator
  Replicator* replicator = Replica::GetReplicator();
  if(!replicator) // Unable?
    return 0;

  // Get current time
  TimeMs now = replicator->GetPeer()->GetLocalTime();

  // Get last change timestamp
  TimeMs timestamp = Replica::GetLastChangeTimestamp();
  if(timestamp == cInvalidMessageTimestamp) // Invalid?
    return 0;

  // Compute time passed since last change (duration between now and last change timestamp)
  TimeMs timePassed = (now - timestamp);
  return TimeMsToFloatSeconds(timePassed);
}
float NetObject::GetOfflineTimePassed() const
{
  // Get replicator
  Replicator* replicator = Replica::GetReplicator();
  if(!replicator) // Unable?
    return 0;

  // Get current time
  TimeMs now = replicator->GetPeer()->GetLocalTime();

  // Get uninitialization timestamp
  TimeMs timestamp = Replica::GetUninitializationTimestamp();
  if(timestamp == cInvalidMessageTimestamp) // Invalid?
    return 0;

  // Compute time passed since uninitialization (duration between now and uninitialization timestamp)
  TimeMs timePassed = (now - timestamp);
  return TimeMsToFloatSeconds(timePassed);
}

void NetObject::WakeUp()
{
  Replica::WakeUp();
}
void NetObject::TakeNap()
{
  Replica::TakeNap();
}

bool NetObject::ReplicateNow()
{
  // Get owner
  Cog* owner = GetOwner();

  // Is editor or preview mode?
  if(owner->IsEditorOrPreviewMode())
    return false;

  // Get net peer
  NetPeer* netPeer = GetNetPeer();
  if(!netPeer) // Unable?
    return false;

  // Not client or server?
  if(!netPeer->IsClientOrServer())
    return false;

  // Net object not valid?
  if(!IsValid())
    return false;

  // For all replica channels
  bool result = false;
  forRange(ReplicaChannel* replicaChannel, Replica::GetReplicaChannels().All())
  {
    NetChannel* netChannel = static_cast<NetChannel*>(replicaChannel);
    if(netChannel->ReplicateNow())
      result = true;
  }
  return result;
}

bool NetObject::IsNapping() const
{
  return Replica::IsNapping();
}

bool NetObject::IsAncestor() const
{
  return mIsAncestor;
}
bool NetObject::IsDescendant() const
{
  return !IsAncestor();
}

void NetObject::SetFamilyTreeId(FamilyTreeId familyTreeId)
{
  mFamilyTreeId = familyTreeId;
}
FamilyTreeId NetObject::GetFamilyTreeId() const
{
  return mFamilyTreeId;
}

bool NetObject::WasLevelInitialized() const
{
  return !mInitLevelResourceIdName.Empty();
}
bool NetObject::WasCogInitialized() const
{
  return !WasLevelInitialized();
}

void NetObject::SetInitializationLevelResourceIdName(const String& initLevelResourceIdName)
{
  mInitLevelResourceIdName = initLevelResourceIdName;
}
const String& NetObject::GetInitializationLevelResourceIdName() const
{
  return mInitLevelResourceIdName;
}

NetPeer* NetObject::GetNetPeer() const
{
  GameSession* gameSession = GetOwner()->GetGameSession();
  return gameSession ? gameSession->has(NetPeer) : nullptr;
}
NetSpace* NetObject::GetNetSpace() const
{
  Space* space = GetOwner()->GetSpace();
  return space ? space->has(NetSpace) : nullptr;
}

bool NetObject::IsNetPeer() const
{
  return this == GetOwner()->has(NetPeer);
}
bool NetObject::IsNetSpace() const
{
  return this == GetOwner()->has(NetSpace);
}
bool NetObject::IsNetUser() const
{
  return this == GetOwner()->has(NetUser);
}

bool NetObject::IsInvalid() const
{
  return !IsValid();
}
bool NetObject::IsValid() const
{
  if(Replica::IsValid())
    return true;
  else
  {
    // Get net peer
    NetPeer* netPeer = GetNetPeer();
    if(!netPeer) // Unable?
      return false;

    // Is offline?
    return netPeer->IsOffline();
  }
}
bool NetObject::IsLive() const
{
  return Replica::IsLive();
}
bool NetObject::IsOnline() const
{
  return mIsOnline;
}
NetObjectId NetObject::GetNetObjectId() const
{
  return NetObjectId(Replica::GetReplicaId().value());
}

bool NetObject::Forget()
{
  // Get owner
  Cog* owner = GetOwner();

  // Is editor or preview mode?
  if(owner->IsEditorOrPreviewMode())
    return false;

  // Get net peer
  NetPeer* netPeer = GetNetPeer();
  if(!netPeer) // Unable?
    return false;

  // Not client or server?
  if(!netPeer->IsClientOrServer())
    return false;

  // Net object not valid?
  if(!IsValid())
    return false;

  // Forget net object locally and remotely for all relevant peers
  bool result = netPeer->ForgetNetObject(GetOwner(), Route::All);
  Assert(IsInvalid()); // (Net object should be invalid after this call)
  return result;
}

bool NetObject::SelectRemote()
{
  // Get owner
  Cog* owner = GetOwner();

  // Is editor or preview mode?
  if(owner->IsEditorOrPreviewMode())
    return false;

  // Unable to get editor?
  if(!Z::gRuntimeEditor)
    return false;

  // Get net peer
  NetPeer* netPeer = GetNetPeer();
  if(!netPeer) // Unable?
    return false;

  // Not client or server?
  if(!netPeer->IsClientOrServer())
    return false;

  // Net object not valid?
  if(!IsValid())
    return false;

  // Let our target role be the opposite of our local peer's role
  Role::Enum targetRole = (GetRole() == Role::Client)
                        ? Role::Server
                        : Role::Client;

  // Find the first game session with a net peer with our target role
  NetPeer* targetNetPeer = nullptr;
  forRange(GameSession* gameSession, Z::gEngine->GetGameSessions())
  {
    // Get game session's net peer component
    NetPeer* netPeer = gameSession->has(NetPeer);
    if(!netPeer) // Unable?
      continue; // Skip

    // Net peer has our target role?
    if(netPeer->GetRole() == targetRole)
    {
      // Found target peer
      targetNetPeer = netPeer;
      break;
    }
  }

  // Unable to find target peer?
  if(!targetNetPeer)
    return false;

  // Find our remote net object using our local net object's ID
  Cog* remoteNetObjectCog = targetNetPeer->GetNetObject(GetNetObjectId());
  if(!remoteNetObjectCog) // Unable?
    return false;

  // Get editor's active selection
  MetaSelection* activeSelection = Z::gRuntimeEditor->GetActiveSelection();
  if(!activeSelection) // Unable?
    return false;

  // Set the remote net object cog as the editor's active selection
  activeSelection->SelectOnly(remoteNetObjectCog);
  activeSelection->FinalSelectionChanged();

  // Success
  return true;
}

//
// Channel Management
//

bool NetObject::DoesThisNetPropertyAlreadyBelongToAChannel(Component* component, StringParam propertyName) const
{
  // For all replica channels
  forRange(ReplicaChannel* replicaChannel, GetReplicaChannels().All())
  {
    // Try to get replica property by name
    NetProperty* replicaProperty = static_cast<NetChannel*>(replicaChannel)->GetNetProperty(component, propertyName);
    if(replicaProperty) // Found?
    {
      // Get property info
      ComponentPropertyInstanceData componentPropertyMetaData = replicaProperty->GetPropertyData().GetOrError<ComponentPropertyInstanceData>();
      Assert(componentPropertyMetaData.mPropertyName == propertyName);

      // Component matches?
      if(componentPropertyMetaData.mComponent == component)
      {
        // Net property already belongs to a net channel
        return true;
      }
    }
  }

  // Net property does not already belong to a net channel
  return false;
}

bool NetObject::HasNetChannel(const String& netChannelName) const
{
  return Replica::HasReplicaChannel(netChannelName);
}

NetChannel* NetObject::GetNetChannel(const String& netChannelName)
{
  return static_cast<NetChannel*>(Replica::GetReplicaChannel(netChannelName));
}

NetChannel* NetObject::AddNetChannel(const String& netChannelName, NetChannelConfig* netChannelConfig)
{
  // Already valid? (Net object is already online?)
  if(Replica::IsValid())
  {
    DoNotifyError("NetObject", String::Format("Unable to add NetChannel named '%s' - NetObject is already online", netChannelName.c_str()));
    return nullptr;
  }

  // Get net peer
  NetPeer* netPeer = GetNetPeer();
  if(!netPeer) // Unable?
    return nullptr;

  // Get or add corresponding net channel type
  NetChannelType* netChannelType = netPeer->GetOrAddReplicaChannelType(netChannelName, netChannelConfig);

  // Add net channel
  return static_cast<NetChannel*>(Replica::AddReplicaChannel(ReplicaChannelPtr(new NetChannel(netChannelName, netChannelType))));
}

bool NetObject::AddNetPropertyToChannel(Component* component, Property* property, const String& netPropertyTypeName, NetPropertyConfig* netPropertyConfig, const String& netChannelName, NetChannelConfig* netChannelConfig)
{
  // (Net object should not be online yet)
  Assert(!Replica::IsValid());

  // Get owner
  Cog* owner = GetOwner();

  // Get net channel (if it already exists)
  NetChannel* netChannel = GetNetChannel(netChannelName);
  bool addedNetChannel = false;
  if(!netChannel) // Doesn't exist?
  {
    // Add net channel (using provided config settings, if any)
    netChannel = AddNetChannel(netChannelName, netChannelConfig);
    addedNetChannel = true;
  }

  // Unable to get or add net channel?
  if(!netChannel)
  {
    DoNotifyError("Unable To Add NetProperty",
                  String::Format("Unable to add NetProperty '%s' declared in Component '%s' to the NetChannel '%s' on the NetObject '%s' - Error adding NetChannel",
                  property->Name.c_str(), ZilchVirtualTypeId(component)->Name, netChannelName.c_str(), owner->GetDescription().c_str()));
    return false;
  }

  // Add net property
  NetProperty* netProperty = netChannel->AddNetProperty(component, property, netPropertyTypeName, netPropertyConfig);
  if(!netProperty) // Unable?
  {
    DoNotifyError("Unable To Add NetProperty",
                  String::Format("Unable to add NetProperty '%s' declared in Component '%s' to the NetChannel '%s' on the NetObject '%s' - Error adding NetProperty",
                  property->Name.c_str(), ZilchVirtualTypeId(component)->Name, netChannelName.c_str(), owner->GetDescription().c_str()));

    // Net channel was added as a result of this property?
    if(addedNetChannel)
      RemoveNetChannel(netChannel->GetName()); // Remove net channel

    return false;
  }

  // Success
  return true;
}
bool NetObject::AddNetPropertyToChannel(Component* component, Property* property, NetPropertyConfig* netPropertyConfig, NetChannelConfig* netChannelConfig)
{
  return AddNetPropertyToChannel(component, property, netPropertyConfig->GetName(), netPropertyConfig, netChannelConfig->GetName(), netChannelConfig);
}

bool NetObject::RemoveNetChannel(const String& netChannelName)
{
  // Already valid? (Net object is already online?)
  if(Replica::IsValid())
  {
    DoNotifyError("NetObject", String::Format("Unable to remove NetChannel named '%s' - NetObject is already online", netChannelName.c_str()));
    return nullptr;
  }

  // Remove net channel
  return Replica::RemoveReplicaChannel(netChannelName);
}

void NetObject::ClearNetChannels()
{
  // Already valid? (Net object is already online?)
  if(Replica::IsValid())
  {
    DoNotifyError("NetObject", String::Format("Unable to clear NetChannels - NetObject is already online"));
    return;
  }

  // Clear net channels
  Replica::ClearReplicaChannels();
}

//
// Ownership Interface
//

bool NetObject::IsOwnedByAUser() const
{
  return GetNetUserOwnerUserId() != 0;
}
bool NetObject::IsNotOwnedByAUser() const
{
  return !IsOwnedByAUser();
}

bool NetObject::IsOwnedByUser(Cog* cog) const
{
  NetUserId netUserId = 0;

  // Valid cog?
  if(cog)
  {
    // Get net user ID
    NetUser* netUser = cog->has(NetUser);
    if(!netUser) // Unable?
    {
      DoNotifyWarning("NetObject", "Invalid Cog parameter - Cog must have a NetUser component");
      return false;
    }
    netUserId = netUser->mNetUserId;
  }

  return GetNetUserOwnerUserId() == netUserId;
}
bool NetObject::IsOwnedByUserId(NetUserId netUserId) const
{
  return GetNetUserOwnerUserId() == netUserId;
}

bool NetObject::IsOwnedByPeer(NetPeerId netPeerId) const
{
  return GetNetUserOwnerPeerId() == netPeerId;
}

bool NetObject::IsMine() const
{
  // Get net peer
  NetPeer* netPeer = GetNetPeer();
  if(!netPeer || !netPeer->IsOpen()) // Unable or not open?
    return false;

  return IsOwnedByPeer(netPeer->GetNetPeerId());
}
bool NetObject::IsNotMine() const
{
  return !IsMine();
}

bool NetObject::IsClientAndMine() const
{
  return IsClient() && IsMine();
}
bool NetObject::IsClientButNotMine() const
{
  return IsClient() && !IsMine();
}

bool NetObject::IsServerAndMine() const
{
  return IsServer() && IsMine();
}
bool NetObject::IsServerButNotMine() const
{
  return IsServer() && !IsMine();
}

bool NetObject::IsOfflineAndMine() const
{
  return IsOffline() && IsMine();
}
bool NetObject::IsOfflineButNotMine() const
{
  return IsOffline() && !IsMine();
}

NetPeerId NetObject::GetNetUserOwnerPeerId() const
{
  // Get net user object
  Cog* cog = GetNetUserOwner();
  if(!cog) // Unable?
    return 0;

  // Get net user
  NetUser* netUser = cog->has(NetUser);
  Assert(netUser);

  return netUser->mNetPeerId;
}
NetUserId NetObject::GetNetUserOwnerUserId() const
{
  return mNetUserOwnerUserId;
}
Cog* NetObject::GetNetUserOwner() const
{
  // Get net peer
  NetPeer* netPeer = GetNetPeer();
  if(!netPeer) // Unable?
    return nullptr;

  return netPeer->GetUser(GetNetUserOwnerUserId());
}

void NetObject::SetNetUserOwnerUserId(NetUserId netUserId)
{
  // Get net peer
  NetPeer* netPeer = GetNetPeer();
  if(!netPeer) // Unable?
    return;

  // Not server or offline?
  if(!netPeer->IsServerOrOffline())
  {
    DoNotifyError("Unable to set NetUser owner on NetObject", "NetPeer must be open as a server or offline");
    return;
  }

  // Set net user owner ID
  NetUserId previousNetUserOwnerUserId = mNetUserOwnerUserId;
  mNetUserOwnerUserId = netUserId;

  // Has changed?
  if(GetNetUserOwnerUserId() != previousNetUserOwnerUserId)
  {
    // Replicate net user owner ID changes now (if any)
    if(NetChannel* netObjectChannel = GetNetChannel("NetObject"))
      netObjectChannel->ReplicateNow();
    else
      Assert(false);

    // Handle the net user owner change
    HandleNetUserOwnerChanged(previousNetUserOwnerUserId);
  }
}
void NetObject::SetNetUserOwner(Cog* cog)
{
  NetUserId netUserId = 0;

  // Valid cog?
  if(cog)
  {
    // Get net user ID
    NetUser* netUser = cog->has(NetUser);
    if(!netUser) // Unable?
    {
      DoNotifyWarning("NetObject", "Invalid Cog parameter - Cog must have a NetUser component");
      return;
    }
    netUserId = netUser->mNetUserId;
  }

  // Set net user owner
  SetNetUserOwnerUserId(netUserId);
}

void NetObject::SetNetUserOwnerUpById(NetUserId netUserId)
{
  // Get net peer
  NetPeer* netPeer = GetNetPeer();
  if(!netPeer) // Unable?
    return;

  // Not server or offline?
  if(!netPeer->IsServerOrOffline())
  {
    DoNotifyError("Unable to set NetUser owner on NetObject", "NetPeer must be open as a server or offline");
    return;
  }

  // Set net user owner on this object
  SetNetUserOwnerUserId(netUserId);

  // Has parent net object?
  Cog*       parentCog       = GetOwner()->GetParent();
  NetObject* parentNetObject = parentCog ? parentCog->has(NetObject) : nullptr;
  if(parentNetObject)
  {
    // Set net user owner up the tree on it's parent recursively in pre-order
    parentNetObject->SetNetUserOwnerUpById(netUserId);
  }
}
void NetObject::SetNetUserOwnerUp(Cog* cog)
{
  NetUserId netUserId = 0;

  // Valid cog?
  if(cog)
  {
    // Get net user ID
    NetUser* netUser = cog->has(NetUser);
    if(!netUser) // Unable?
    {
      DoNotifyWarning("NetObject", "Invalid Cog parameter - Cog must have a NetUser component");
      return;
    }
    netUserId = netUser->mNetUserId;
  }

  // Set net user owner
  SetNetUserOwnerUpById(netUserId);
}

void NetObject::SetNetUserOwnerDownById(NetUserId netUserId)
{
  // Get net peer
  NetPeer* netPeer = GetNetPeer();
  if(!netPeer) // Unable?
    return;

  // Not server or offline?
  if(!netPeer->IsServerOrOffline())
  {
    DoNotifyError("Unable to set NetUser owner on NetObject", "NetPeer must be open as a server or offline");
    return;
  }

  // Set net user owner on this object
  SetNetUserOwnerUserId(netUserId);

  // For all child net objects
  Hierarchy* hierarchy = GetOwner()->has(Hierarchy);
  if(hierarchy)
  {
    forRange(HierarchyList::sub_reference child, hierarchy->GetChildren())
    {
      NetObject* childNetObject = child.has(NetObject);
      if(childNetObject)
      {
        // Set net user owner down the tree on all it's children recursively in pre-order
        childNetObject->SetNetUserOwnerDownById(netUserId);
      }
    }
  }
}
void NetObject::SetNetUserOwnerDown(Cog* cog)
{
  NetUserId netUserId = 0;

  // Valid cog?
  if(cog)
  {
    // Get net user ID
    NetUser* netUser = cog->has(NetUser);
    if(!netUser) // Unable?
    {
      DoNotifyWarning("NetObject", "Invalid Cog parameter - Cog must have a NetUser component");
      return;
    }
    netUserId = netUser->mNetUserId;
  }

  // Set net user owner
  SetNetUserOwnerDownById(netUserId);
}

void NetObject::HandleNetUserOwnerChanged(NetUserId previousNetUserOwnerUserId)
{
  Assert(GetNetUserOwnerUserId() != previousNetUserOwnerUserId); // (Should have actually changed)

  // Set change authority client to net user owner's peer
  Replica::SetAuthorityClientReplicatorId(GetNetUserOwnerPeerId());

  // Get owner
  Cog* owner = GetOwner();

  // Get net peer
  NetPeer* netPeer = GetNetPeer();
  if(!netPeer) // Unable?
    return;

  // Get previous net user owner
  Cog* previousNetUserOwner = netPeer->GetUser(previousNetUserOwnerUserId);

  // Get current net user owner
  Cog* currentNetUserOwner = GetNetUserOwner();

  //
  // Update User's Owned Object Lists
  //

  // Has previous owner?
  if(previousNetUserOwner)
  {
    Assert(previousNetUserOwner->has(NetUser));

    // Remove object from previous owner's network owned object list (should succeed)
    previousNetUserOwner->has(NetUser)->mOwnedNetObjects.Erase(owner->mObjectId);
  }

  // Has current owner?
  if(currentNetUserOwner)
  {
    Assert(currentNetUserOwner->has(NetUser));

    // Add object to current owner's network owned object list (must succeed)
    currentNetUserOwner->has(NetUser)->mOwnedNetObjects.InsertOrError(owner->mObjectId);
  }

  //
  // Notify Users and This Object
  //

  // Net object is not online?
  if(!IsOnline())
    return; // Do nothing (these generated events would be unexpected)

  // Has previous owner?
  if(previousNetUserOwner)
  {
    // Notify previous owner
    NetUserLostObjectOwnership event;
    event.mLostObject          = owner;
    event.mCurrentNetUserOwner = currentNetUserOwner;

    // Dispatch event
    previousNetUserOwner->DispatchEvent(Events::NetUserLostObjectOwnership, &event);
  }

  // Has current owner?
  if(currentNetUserOwner)
  {
    // Notify current owner
    NetUserAcquiredObjectOwnership event;
    event.mAcquiredObject       = owner;
    event.mPreviousNetUserOwner = previousNetUserOwner;

    // Dispatch event
    currentNetUserOwner->DispatchEvent(Events::NetUserAcquiredObjectOwnership, &event);
  }

  // Notify this object
  {
    NetUserOwnerChanged event;
    event.mPreviousNetUserOwner = previousNetUserOwner;
    event.mCurrentNetUserOwner  = currentNetUserOwner;

    // Dispatch event
    owner->DispatchEvent(Events::NetUserOwnerChanged, &event);
  }
}

//
// Network Dispatch Interface
//

void NetObject::DispatchLocal(StringParam eventId, Event* event)
{
  // Get owner
  Cog* owner = GetOwner();

  // Get net peer
  NetPeer* netPeer = owner->GetGameSession()->has(NetPeer);
  if(!netPeer) // Unable?
  {
    DoNotifyException("Cog", "Unable to network dispatch event - GameSession must have a NetPeer component");
    return;
  }

  // Not open?
  if(!netPeer->IsOpen())
  {
	  DoNotifyException("Cog", "Unable to network dispatch event - NetPeer must be open");
    return;
  }

  // Network dispatch event on this Cog
  netPeer->DispatchLocalInternal(eventId, event, owner);
}

void NetObject::DispatchRemote(StringParam eventId, Event* event, NetPeerId netPeerId)
{
  // Get owner
  Cog* owner = GetOwner();

  // Get net peer
  NetPeer* netPeer = owner->GetGameSession()->has(NetPeer);
  if(!netPeer) // Unable?
  {
    DoNotifyWarning("Cog", "Unable to network dispatch event - GameSession must have a NetPeer component");
    return;
  }

  // Not open?
  if(!netPeer->IsOpen())
  {
    DoNotifyWarning("Cog", "Unable to network dispatch event - NetPeer must be open");
    return;
  }

  // Is offline?
  if(netPeer->IsOffline())
  {
    // Network dispatch event on this Cog locally
    netPeer->DispatchLocalInternal(eventId, event, owner);
  }
  // Is online?
  else
  {
    // Network dispatch event on this Cog remotely
    netPeer->DispatchRemoteInternal(eventId, event, netPeerId, owner);
  }
}

void NetObject::DispatchBroadcast(StringParam eventId, Event* event)
{
  // Get owner
  Cog* owner = GetOwner();

  // Get net peer
  NetPeer* netPeer = owner->GetGameSession()->has(NetPeer);
  if(!netPeer) // Unable?
  {
    DoNotifyException("Cog", "Unable to network dispatch event - GameSession must have a NetPeer component");
    return;
  }

  // Not open?
  if(!netPeer->IsOpen())
  {
    DoNotifyException("Cog", "Unable to network dispatch event - NetPeer must be open");
    return;
  }

  // Is offline?
  if(netPeer->IsOffline())
  {
    // Network dispatch event on this Cog locally
    netPeer->DispatchLocalInternal(eventId, event, owner);
  }
  // Is online?
  else
  {
    // Network dispatch event on this Cog broadcast
    netPeer->DispatchBroadcastInternal(eventId, event, owner);
  }
}

void NetObject::DispatchLocalAndRemote(StringParam eventId, Event* event, NetPeerId netPeerId)
{
  // Get owner
  Cog* owner = GetOwner();

  // Get net peer
  NetPeer* netPeer = owner->GetGameSession()->has(NetPeer);
  if(!netPeer) // Unable?
  {
    DoNotifyException("Cog", "Unable to network dispatch event - GameSession must have a NetPeer component");
    return;
  }

  // Not open?
  if(!netPeer->IsOpen())
  {
    DoNotifyException("Cog", "Unable to network dispatch event - NetPeer must be open");
    return;
  }

  // Is offline?
  if(netPeer->IsOffline())
  {
    // Network dispatch event on this Cog locally
    netPeer->DispatchLocalInternal(eventId, event, owner);
  }
  // Is online?
  else
  {
    // Network dispatch event on this Cog locally and remotely
    if(netPeer->DispatchLocalInternal(eventId, event, owner))
      netPeer->DispatchRemoteInternal(eventId, event, netPeerId, owner);
  }
}

void NetObject::DispatchLocalAndBroadcast(StringParam eventId, Event* event)
{
  // Get owner
  Cog* owner = GetOwner();

  // Get net peer
  NetPeer* netPeer = owner->GetGameSession()->has(NetPeer);
  if(!netPeer) // Unable?
  {
    DoNotifyException("Cog", "Unable to network dispatch event - GameSession must have a NetPeer component");
    return;
  }

  // Not open?
  if(!netPeer->IsOpen())
  {
    DoNotifyException("Cog", "Unable to network dispatch event - NetPeer must be open");
    return;
  }

  // Is offline?
  if(netPeer->IsOffline())
  {
    // Network dispatch event on this Cog locally
    netPeer->DispatchLocalInternal(eventId, event, owner);
  }
  // Is online?
  else
  {
    // Network dispatch event on this Cog locally and broadcast
    if(netPeer->DispatchLocalInternal(eventId, event, owner))
      netPeer->DispatchBroadcastInternal(eventId, event, owner);
  }
}

//
// Channel Configuration
//

void NetObject::SetAutomaticChannel(NetChannelConfig* netChannelConfig)
{
  if(netChannelConfig)
    mAutomaticChannel = netChannelConfig;
}
NetChannelConfig* NetObject::GetAutomaticChannel()
{
  return mAutomaticChannel;
}

//
// Property Info
//

bool NetObject::HasNetPropertyInfo(BoundType* componentType, StringParam propertyName)
{
  return GetNetPropertyInfo(componentType, propertyName) != nullptr;
}

NetPropertyInfo* NetObject::GetNetPropertyInfo(BoundType* componentType, StringParam propertyName)
{
  return mNetPropertyInfos.FindPointer(Pair<BoundType*, String>(componentType, propertyName));
}

NetPropertyInfo* NetObject::AddNetPropertyInfo(BoundType* componentType, StringParam propertyName)
{
  // Get meta property (specified by component and property name)
  Property* property = componentType->GetProperty(propertyName);
  if(!property) // Unable?
    return nullptr;

  // Not a valid net property?
  if(!IsValidNetProperty(property))
    return nullptr;

  // Already have net property info?
  if(HasNetPropertyInfo(componentType, propertyName))
    return nullptr;

  // Create net property info
  NetPropertyInfo netPropertyInfo(componentType, propertyName);

  // Does this property belong to the Transform or RigidBody component?
  if(componentType == ZilchTypeId(Transform)
  || componentType == ZilchTypeId(RigidBody))
  {
    // Note: We special case Transform and RigidBody to both use the Transform NetChannel by default for performance and convenience reasons
    // In the future this may change, but for now it's reasonable

    // Set initial net channel config to the "Transform" NetChannelConfig resource
    netPropertyInfo.mNetChannelConfig = NetChannelConfigManager::Find("Transform");
  }
  else
  {
    // Set initial net channel config to the automatic net channel config (typically the "DefaultChannel") resource by default
    netPropertyInfo.mNetChannelConfig = mAutomaticChannel;
  }

  // Add net property info to set
  mNetPropertyInfos.PushBack(netPropertyInfo);

  Space* space = GetSpace();
  if(space)
    space->MarkModified();

  // We want the property grid to reflect the changes
  Event e;
  GetOwner()->DispatchEvent(Events::ObjectStructureModified, &e);

  // Success
  return &mNetPropertyInfos.Back();
}

void NetObject::RemoveNetPropertyInfo(BoundType* componentType, StringParam propertyName)
{
  // Find net property info in set
  NetPropertyInfo* netPropertyInfo = GetNetPropertyInfo(componentType, propertyName);
  if(!netPropertyInfo) // Unable?
    return;

  // Remove net property info from set
  mNetPropertyInfos.Erase(netPropertyInfo);

  // The object has been modified
  Space* space = GetSpace();
  if(space)
    space->MarkModified();

  // We want the property grid to reflect the changes
  Event e;
  GetOwner()->DispatchEvent(Events::ObjectStructureModified, &e);
}

} // namespace Zero
