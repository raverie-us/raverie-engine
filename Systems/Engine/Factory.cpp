///////////////////////////////////////////////////////////////////////////////
///
/// \file Factory.cpp
/// Implementation of game engine Factory class.
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
  Factory* gFactory = nullptr;
}

ZilchDefineType(Factory, builder, type)
{
}

Factory* Factory::StaticInitialize(Engine* engine, Tracker* tracker)
{
  ErrorIf(Z::gFactory != nullptr, "Factory already created.");
  Z::gFactory = new Factory(engine, tracker);
  return Z::gFactory;
}

Factory::Factory(Engine* engine, Tracker* tracker)
{
  mEngine = engine;
  mTracker = tracker;
}

Factory::~Factory()
{
}

BoundType* GetMetaFromTypeNode(const PolymorphicNode& node)
{
  if(node.RuntimeType != nullptr)
    return node.RuntimeType;
  else
    return MetaDatabase::GetInstance()->FindType(node.TypeName);
}

template<typename type>
bool TestIdent(const PolymorphicNode& node)
{
  BoundType* boundType = ZilchTypeId(type);
  if(node.RuntimeType != nullptr)
  {
    return node.RuntimeType == boundType;
  }
  else
  {
    return node.TypeName == boundType->Name;
  }
}

void ComponentPropertyPatched(cstr fieldName, void* clientData)
{
  Component* component = (Component*)clientData;
  if(*fieldName == 'm')
    ++fieldName;
  Property* property = ZilchVirtualTypeId(component)->GetProperty(fieldName);
  if(property == nullptr)
    return;
  LocalModifications* modifications = LocalModifications::GetInstance();
  PropertyPath path(component, property);
  modifications->SetPropertyModified(component->GetOwner(), path, true);
}

//******************************************************************************
Cog* Factory::BuildFromStream(CogCreationContext* context, Serializer& stream)
{
  bool previousPatching = stream.mPatching;
  //Instantiation of script components can require the game session (to add some sort of script state component to the game)
  GameSession* gameSession = context->mGameSession;
  if(!gameSession && context->mSpace)
    gameSession = context->mSpace->GetGameSession();

  ErrorIf(context == NULL, "Need context");

  stream.SetSerializationContext(context);
  stream.mPatchCallback = ComponentPropertyPatched;

  DataNode* cogDataNode = nullptr;
  ObjectLoader* objectLoader = nullptr;
  if(stream.GetType() == SerializerType::Text)
  {
    objectLoader = (ObjectLoader*)(&stream);
    cogDataNode = objectLoader->GetNext();
  }

  PolymorphicNode cogNode;
  //Make sure the stream is valid
  if(stream.GetPolymorphic(cogNode))
  {
    // Create a new game object to hold the components
    Cog* gameObject = nullptr;
    uint localContextId = 0;

    PolymorphicNode componentNode;
    componentNode.RuntimeType = nullptr;
    bool archetypedInstance = false;

    // When we enter a new sub context (when entering an Archetype), it needs to
    // be restored to the previous sub context when we finish loading the
    // Archetype
    uint prevSubContextId = uint(-1);

    // Replace legacy type with Cog
    if (cogNode.TypeName == "LevelSettings")
      cogNode.TypeName = "Cog";
    
    BoundType* compositionMeta = GetMetaFromTypeNode(cogNode);

    if(TestIdent<ArchetypeInstance>(cogNode))
    {
      String Name;
      SerializeName(Name);

      // Create sub context id and store old id
      prevSubContextId = context->EnterSubContext();

      // Default the Archetype if it could not be found
      Archetype* archetype = ArchetypeManager::Find(Name);
      if(archetype == nullptr)
        archetype = ArchetypeManager::GetDefault();

      // Build the object
      gameObject = BuildFromArchetype(archetype->mStoredType, archetype, context);

      if(gameObject == nullptr)
        return nullptr;

      // Set the sub context id on the object so it can look up its children
      context->AssignSubContextId(gameObject);

      archetypedInstance = true;
      stream.mPatching = true;
    }
    else if(compositionMeta)
    {
      gameObject = ZilchAllocate(Cog, compositionMeta, HeapFlags::NonReferenceCounted);

      gameObject->mChildId = cogNode.UniqueNodeId;

      // We entered an Archetype, so we need to start a sub context so that
      // the link id's we read in are offset to not overlap with id's in the
      // current context
      if(cogNode.Flags.IsSet(PolymorphicFlags::Inherited))
        prevSubContextId = context->EnterSubContext();
    }
    else
    {
      ErrorIf(gameObject == nullptr, "Can not build object from stream");
      return nullptr;
    }
    
    // Pull the Archetype name if it exists
    if(Archetype* archetype = ArchetypeManager::FindOrNull(cogNode.mInheritId))
      gameObject->SetArchetype(archetype);

    // The game session might be what we're creating, in that case the script context is itself
    if(!gameSession)
      gameSession = Type::DynamicCast<GameSession*>(gameObject);

    // Read Local Component
    PushErrorContextObject("Loading Cog", gameObject);

    // Serialize the objects name
    stream.SerializeFieldDefault("Name", gameObject->mName, String(""));

    // Serialize the link id
    stream.SerializeFieldDefault("LinkId", localContextId, localContextId);

    while(stream.GetPolymorphic(componentNode))
    {
      if(TestIdent<LinkId>(componentNode))
      {
        LinkId id;
        id.Serialize(stream);
        localContextId = id.Id; 
      }
      else if(TestIdent<Named>(componentNode))
      {
        Named named;
        named.Serialize(stream);
        gameObject->mName = named.Name;
      
        // Because 'Named' isn't a property, we have to store that it was
        // patched in a custom way. This should probably be changed.
        if(componentNode.Flags.IsSet(PolymorphicFlags::Patched))
        {
          LocalModifications* modifications = LocalModifications::GetInstance();
          PropertyPath path("Name");
          modifications->SetPropertyModified(gameObject, path, true);
        }
      }
      else if(TestIdent<Archetyped>(componentNode))
      {
        Archetyped archetyped;
        archetyped.Serialize(stream);

        Archetype* archetype = ArchetypeManager::Find(archetyped.Name);
        gameObject->SetArchetype(archetype);
      }
      else if(TestIdent<EditorFlags>(componentNode))
      {
        EditorFlags flags;
        flags.Serialize(stream);
      
        // Only set them if we're in the editor
        if(context->mSpace->IsEditorMode())
        {
          gameObject->mFlags.SetState(CogFlags::EditorViewportHidden, flags.mHidden);
          gameObject->mFlags.SetState(CogFlags::Locked, flags.mLocked);
        }
      }
      else if(TestIdent<SpaceObjects>(componentNode))
      {
        Space* newSpace = (Space*)gameObject;
        CogInitializer initializer(nullptr);
        initializer.mGameSession = gameSession;
        newSpace->Initialize(initializer);
        initializer.AllCreated();
        newSpace->AddObjectsFromStream("SubSpace", stream);
      }
      else
      {
        BoundType* componentMeta = MetaDatabase::GetInstance()->FindType(componentNode.TypeName);
        if(componentMeta != nullptr && !componentMeta->IsA(ZilchTypeId(Component)))
        {
          String message = String::Format("We tried to create a Component named %s while loading "
                                          "'%s' but it is not a Component type",
                                          componentMeta->Name.c_str(), context->Source.c_str());
          DoNotifyError("Could not create Component", message);
          componentMeta = nullptr;
        }

        bool subtractiveNode = componentNode.Flags.IsSet(PolymorphicFlags::Subtractive);

        // Handle missing components
        if(componentMeta == nullptr && !subtractiveNode)
        {
          String message = String::Format("Could not find component '%s'. "\
            "Creating proxy.\n", componentNode.TypeName.Data());

          bool proxyComponentsExpected = context->Flags & CreationFlags::ProxyComponentsExpected;
          if(!proxyComponentsExpected)
          {
            // In the editor throw an error in
            // the exported version just log and continue so
            // missing component do not create errors in export
            // This should not be an exception in script because it causes too many export bugs!
            if(Z::EditorDebugFeatures)
              DoNotifyErrorWithContext(message, NotifyException::None);
            else
              ZPrint("Suppressed: %s", message.c_str());
          }

          // Create a Proxy to be used for this component
          componentMeta = ProxyObject<Component>::CreateProxyMetaFromFile(componentNode.TypeName, ProxyReason::TypeDidntExist);
          if(componentMeta == nullptr)
          {
            Error("Could not create proxy meta");
            //Move to next component
            continue;
          }

          // We have a proxy, but lets search all scripts for where it could have possibly come from
          // This is helpful when scripts failed to compile on startup
          EngineLibraryExtensions::FindProxiedTypeOrigin(componentMeta);
        }

        Component* component = nullptr;

        // All components in the Archetype instance should all already be created from the archetype,
        // re-serialize the components in place
        if(archetypedInstance)
        {
          component = gameObject->QueryComponentType(componentMeta);
        }

        // Create and add the component
        if(component == nullptr && !subtractiveNode)
        {
          // METAREFACTOR - calling this destructs the components on the Cog
          bool canAdd = gameObject->CheckForAddition(componentMeta);
          if(canAdd)
          {
            uint flags = 0;
            if(context)
            {
              flags = context->Flags;
              //Get the spaces flags
              if(context->mSpace)
                flags = context->mSpace->GetCreationFlags();
            }

            // Create the component by using the ComponentMeta
            component = ZilchAllocate(Component, componentMeta, HeapFlags::NonReferenceCounted);

            // If we failed to create the object (should only happen on Script Components where
            // an exception was thrown in the constructor), proxy the object and re-create it
            // under the proxy
            if(component == nullptr && componentMeta->HasAttribute(ObjectAttributes::cProxy) == false)
            {
              componentMeta = ProxyObject<Component>::CreateProxyMetaFromFile(componentNode.TypeName, ProxyReason::AllocationException);
              component = ZilchAllocate(Component, componentMeta, HeapFlags::NonReferenceCounted);
            }

            //Be tolerant of the meta create failing!
            //Add the new component to the composition
            if(component)
              gameObject->AddComponentInternal(componentMeta, component);
          }
        }

        if(component && subtractiveNode)
        {
          gameObject->RemoveComponentInternal(component);
          component = nullptr;
        }

        PushErrorContextObject("Serializing Component", component);

        // Serialize the component from the data stream.
        if(component)
        {
          stream.mPatchClientData = component;
          component->Serialize(stream);
        }
      }

      // End the component
      stream.EndPolymorphic();
    }

    // Leave the sub context if we entered one above
    if(prevSubContextId != uint(-1))
    {
      // Archetyped objects have two context ids:
      // One is archetype relative id so child objects of that archetype can
      // find it and one is relative to the outer context so  it can also be
      // linked to other objects in the outer context (Level)
      // Because of this, we want to register the cog before leaving the sub
      // context. The 'localContextId' in this case has been overwritten with
      // the context id from the level. We can always assume the local context
      // id of the Archetype root is 1 as it's the first object saved out

      // If it was an archetyped instance, it would have already been registered
      // when it was created
      if(!archetypedInstance)
        context->RegisterCog(gameObject, 1);

      context->LeaveSubContext(prevSubContextId);
    }

    // Add to the context id map so Cog references can be relinked
    if(context && localContextId != 0)
      context->RegisterCog(gameObject, localContextId);

    //End the composition
    stream.EndPolymorphic();

    // Record all patched nodes on the object
    if(objectLoader)
    {
      CachedModifications modifications;
      modifications.Cache(cogDataNode);
      modifications.ApplyModificationsToObject(gameObject);
    }

    // Move the Hierarchy to the end before initialization
    if(Hierarchy* hierarchy = gameObject->has(Hierarchy))
    {
      gameObject->mComponents.EraseValue(hierarchy);
      gameObject->mComponents.PushBack(hierarchy);
    }
    stream.mPatching = previousPatching;
    return gameObject;
  }
  stream.mPatching = previousPatching;
  //Failed to create composition.
  return nullptr;
}

Cog* TypeCheckFail(cstr sourceType, cstr sourceContainedType, cstr sourceName, BoundType* expectedMetaType)
{
    String message = String::Format("Attempted to create an object from %s that Contains a different type. "
      "Expected '%s'. Source type was '%s' in %s", sourceType, expectedMetaType->Name.c_str(), sourceContainedType, sourceName);
    
    DoNotifyErrorWithContext(message);

    return nullptr;
}

Cog* Factory::BuildFromFile(BoundType* expectedMetaType, StringParam fileName, CogCreationContext* context)
{
  String message = String::Format("Loading Cog File '%s'", fileName.c_str());
  PushErrorContext(message.c_str());

  //Now loading from file.
  if(!FileExists(fileName))
  {
    String errorMessage = String::Format("No loaded archetype or file with name '%s'", fileName.c_str());
    DoNotifyErrorNoAssert("Missing", errorMessage);
    return nullptr;
  }

  Cog* cog = nullptr;

  //Load the file from data tree.
  ObjectLoader loader;
  Status status;
  loader.OpenFile(status, fileName);
  if(!status)
  {
    DoNotifyStatus(status);
    DoNotifyErrorWithContext("Failed to parse data file");
    return nullptr;
  }

  //Validation - make sure the file contains the expected type
  DataNode* root = loader.GetCurrent();
  if(!(root->mTypeName == expectedMetaType->Name))
    return TypeCheckFail("a file", root->mTypeName.c_str(), fileName.c_str(), expectedMetaType);

  return BuildFromStream(context, loader);
}

Cog* Factory::BuildFromArchetype(BoundType* expectedMetaType, Archetype* archetype, CogCreationContext* context)
{

  PushErrorContextObject("Loading Archetype", archetype);

  //Validation - make sure the archetype Contains the expected type
  if(archetype->mStoredType != expectedMetaType)
    return TypeCheckFail("an Archetype", archetype->mStoredType->Name.c_str(), archetype->Name.c_str(), expectedMetaType);

  const bool CacheBinaryArchetypes = true;
  if(archetype->mBinaryCache && CacheBinaryArchetypes)
  {
    BinaryBufferLoader stream;
    stream.SetBlock(archetype->mBinaryCache);
    Cog* cog = BuildFromStream(context, stream);

    if(cog)
      cog->SetArchetype(archetype);

    return cog;
  }
  else if(DataNode* cachedTree = archetype->GetCachedDataTree())
  {
    DataTreeLoader loader;
    loader.SetRoot(cachedTree);

    Cog* cog = BuildFromStream(context, loader);

    if(cog)
      cog->SetArchetype(archetype);

    // The Archetype owns the data tree, so pull it back from the loader so that it doesn't free it
    loader.TakeOwnershipOfRoot();

    if(CacheBinaryArchetypes)
      archetype->BinaryCache(cog, context);

    return cog;
  }
  else
  {
    Cog* cog = BuildFromFile(expectedMetaType,  archetype->mLoadPath, context);

    if(cog)
    {
      cog->SetArchetype(archetype);

      // If the Archetype inherits from a base Archetype, or has child Archetypes with 
      // modifications, we want to clear them because they're modifications of a different context
      cog->MarkNotModified();

      if(CacheBinaryArchetypes)
        archetype->BinaryCache(cog, context);
    }

    return cog;
  }
}

Cog* Factory::BuildAndSerialize(BoundType* expectedMetaType, StringParam source)
{
  CogCreationContext context;
  return BuildAndSerialize(expectedMetaType, source, &context);
}

Cog* Factory::BuildAndSerialize(BoundType* expectedMetaType, StringParam source, CogCreationContext* context)
{
  // Create and setup object

  ErrorIf(context == nullptr, "Context should not be NULL");
  Archetype* archetype = ArchetypeManager::FindOrNull(source);

  if(archetype == nullptr)
    return BuildFromFile(expectedMetaType, source, context);
  else
    return BuildFromArchetype(expectedMetaType, archetype, context);

}

Space* Factory::CreateSpaceFromStream(Serializer& stream, uint flags, GameSession* gameSession)
{
  return (Space*)CreateFromStream(nullptr, stream, flags, gameSession);
}

Cog* Factory::CreateCheckedType(BoundType* expectedType, Space* space, StringParam filename, uint flags, GameSession* gameSession)
{
  CogInitializer initializer(space, gameSession);
  initializer.Flags = flags;

  CogCreationContext context;
  context.Flags = flags;
  context.mGameSession = gameSession;

  Cog* cog = BuildAndSerialize(expectedType, filename, &context);
  if(cog != nullptr)
  {
    // If we're creating a game sessions, the game session in this function would be null (can't
    // create a game session in another game session). So we should set ourself as the game
    // session before calling initialize
    if(expectedType == ZilchTypeId(GameSession))
    {
      ErrorIf(gameSession != nullptr, "Cannot create a game session in another game session");
      initializer.mGameSession = (GameSession*)cog;
    }

    cog->Initialize(initializer);
  }

  initializer.AllCreated();
  return cog;
}

Space* Factory::CreateSpace(StringParam filename, uint flags, GameSession* gameSession)
{
  return (Space*)CreateCheckedType(ZilchTypeId(Space), nullptr, filename, flags, gameSession);
}

Cog* Factory::CreateFromStream(Space* space, Serializer& stream, uint flags, GameSession* gameSession)
{
  CogInitializer initializer(space, gameSession);
  initializer.Flags = flags;

  CogCreationContext context;
  context.Flags = flags;
  context.mGameSession = gameSession;

  Cog* cog = BuildFromStream(&context, stream);
  if(cog != nullptr)
  {
    cog->Initialize(initializer);
  }

  initializer.AllCreated();
  return cog;
}

Cog* Factory::Create(Space* space, StringParam filename, uint flags, GameSession* gameSession)
{
  CogInitializer initializer(space, gameSession);
  initializer.Flags = flags;

  CogCreationContext context;
  context.Flags = flags;
  context.mGameSession = gameSession;

  Cog* cog = BuildAndSerialize(ZilchTypeId(Cog), filename, &context);
  if(cog != nullptr)
  {
    cog->Initialize(initializer);
  }

  initializer.AllCreated();
  return cog;
}

Cog* Factory::CreateRequired(Space* space, StringParam filename, uint flags, GameSession* gameSession)
{
  Cog* object = Create(space, filename, flags, gameSession);
  if(object == nullptr)
  {
    FatalEngineError("Failed to create a required archetype '%s'.", filename.c_str());
  }
  return object;
}

void Factory::Destroy(Cog* gameObject)
{
  mTracker->Destroy(gameObject);
}

void Factory::Update()
{
}

}//namespace Zero
