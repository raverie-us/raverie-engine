///////////////////////////////////////////////////////////////////////////////
///
/// \file Space.cpp
/// Implementation of the Space component class.
/// 
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
  DefineEvent(SpaceLevelLoaded);
  DefineEvent(SpaceModified);
  DefineEvent(SpaceObjectsChanged);
  DefineEvent(SpaceDestroyed);
}//namespace Events

Memory::Pool* Space::sCogLists = new Memory::Pool("CogLists", Memory::GetNamedHeap("CogList"), sizeof(NameCogList), 64);

ZilchDefineType(Space, builder, type)
{
  ZeroBindDocumented();
  ZilchBindGetterProperty(ObjectCount);
  ZilchBindDefaultConstructor();
  ZilchBindDestructor();

  ZilchBindMethod(Create);
  ZilchBindMethod(CreateAtPosition);
  ZilchBindMethod(CreateLink);

  ZilchBindMethod(LoadLevel);
  ZilchBindMethod(ReloadLevel);
  ZilchBindGetter(CurrentLevel);
  ZilchBindMethod(AddObjectsFromLevel);

  ZilchBindMethod(FindObjectByName);
  ZilchBindMethod(FindFirstObjectByName);
  ZilchBindMethod(FindLastObjectByName);
  ZilchBindMethod(FindFirstRootObjectByName);
  ZilchBindMethod(FindLastRootObjectByName);
  ZilchBindMethod(FindAllObjectsByName);

  ZilchBindMethod(DestroyAll);
  ZilchBindMethod(DestroyAllFromLevel);

  ZilchBindCustomGetter(IsEditorMode);
  ZilchBindMethod(MarkModified);
  ZilchBindMethod(GetModified);
  ZilchBindMethod(MarkNotModified);

  ZilchBindCustomGetter(AllObjects);
  ZilchBindCustomGetter(AllRootObjects);
}

Space::Space()
{
  mGameSession = nullptr;
  mCogsInSpace = 0;
  mRootCount = 0;
  mCreationFlags.SetFlag(CreationFlags::Editing);
  mSubSpace = false;
  mModified = false;
  mObjectsChanged = false;
  mIsLoadingLevel = false;
  mInvalidObjectPositionOccurred = false;
  mMaxObjectPosition = real(1e+10);
}

Space::~Space()
{
  ErrorIf(!mCogList.Empty(), "Not all objects in space destroyed.");
  Z::gEngine->mSpaceList.Erase(this);
}

bool Space::IsEditorMode()
{
  return mCreationFlags.IsSet(CreationFlags::Editing);
}

bool Space::IsPreviewMode()
{
  return mCreationFlags.IsSet(CreationFlags::Preview);
}

bool Space::IsEditorOrPreviewMode()
{
  return mCreationFlags.IsSet(CreationFlags::Editing)
      || mCreationFlags.IsSet(CreationFlags::Preview);
}

Cog* Space::Clone()
{
  DoNotifyException("Failed to Clone", "Spaces cannot be cloned");
  return nullptr;
}

Space* Space::GetSpace()
{
  return this;
}

GameSession* Space::GetGameSession()
{
  return mGameSession;
}

void Space::Initialize(CogInitializer& initializer)
{
  mGameSession = initializer.mGameSession;

  mCreationFlags = initializer.Flags;

  Z::gEngine->mSpaceList.PushBack(this);
  Cog::Initialize(initializer);
}

uint Space::GetCreationFlags()
{ 
  return mCreationFlags.U32Field; 
}

void Space::AddObject(Cog* cog)
{
  ++mCogsInSpace;
  mCogList.PushBack(cog);

  if(cog->mHierarchyParent == nullptr)
  {
    mRoots.PushBack(cog);
    ++mRootCount;
  }

  this->ChangedObjects();
}

void Space::RemoveObject(Cog* cog)
{
  --mCogsInSpace;
  mCogList.Erase(cog);
  this->ChangedObjects();
}

void Space::AddToNameMap(Cog* cog, StringParam name)
{
  NameCogList* nameList = mNameMap.FindValue(name, nullptr);
  if (!nameList)
  {
    nameList = Space::sCogLists->AllocateType<NameCogList>();
    mNameMap.Insert(name, nameList);
  }

  nameList->PushBack(cog);
}

void Space::RemoveFromNameMap(Cog* cog, StringParam name)
{
  NameCogList::Unlink(cog);

  NameCogList* nameList = mNameMap.FindValue(name, nullptr);
  if (nameList && nameList->Empty())
  {
    mNameMap.Erase(name);
    Space::sCogLists->DeallocateType(nameList);
  }
}

//------------------------------------------------------------ Creation

Cog* Space::CreateAt(StringParam source, Transform* transform)
{
  Mat4 world = transform->GetWorldMatrix();
  Vec3 translation;
  Mat3 rotation;
  Vec3 scale;
  world.Decompose(&scale, &rotation, &translation);

  return CreateAt(source, translation, Math::ToQuaternion(rotation));
}

Cog* Space::CreateAt(StringParam source, Vec3Param position, QuatParam rotation,
                     Vec3Param scale)
{
  CogCreationContext context(this, source);

  CogInitializer initializer(this);
  initializer.Context = &context;
  Cog* cog = Z::gFactory->BuildAndSerialize(ZilchTypeId(Cog), source, &context);

  if(cog == nullptr)
    return nullptr;

  Transform* transform = cog->has(Transform);
  if(transform)
  {
    transform->SetTranslation(position);
    transform->SetRotation(Normalized(rotation));
    transform->SetScale(scale);
  }

  cog->Initialize(initializer);
  initializer.AllCreated();

  return cog;
}

Cog* Space::CreateAt(StringParam source, Vec3Param position, QuatParam rotation)
{
  CogCreationContext context(this, source);

  CogInitializer initializer(this);
  initializer.Context = &context;
  Cog* cog = Z::gFactory->BuildAndSerialize(ZilchTypeId(Cog), source, &context);

  if(cog == nullptr)
    return nullptr;

  Transform* transform = cog->has(Transform);
  if(transform)
  {
    transform->SetTranslation(position);
    transform->SetRotation(Normalized(rotation));
  }

  cog->Initialize(initializer);
  initializer.AllCreated();

  return cog;
}

Cog* Space::CreateAt(StringParam source, Vec3Param position, Vec3Param scale)
{
  CogCreationContext context(this, source);

  CogInitializer initializer(this);
  initializer.Context = &context;
  Cog* cog = Z::gFactory->BuildAndSerialize(ZilchTypeId(Cog), source, &context);

  if(cog == nullptr)
    return nullptr;

  Transform* transform = cog->has(Transform);
  if(transform)
  {
    transform->SetTranslation(position);
    Quat rotation = transform->GetRotation();
    Normalize(rotation);
    transform->SetRotation(rotation);
    transform->SetScale(scale);
  }
  cog->Initialize(initializer);

  initializer.AllCreated();

  return cog;
}

Cog* Space::CreateAt(StringParam source, Vec3Param position)
{
  CogCreationContext context(this, source);

  CogInitializer initializer(this);
  initializer.Context = &context;
  Cog* cog = Z::gFactory->BuildAndSerialize(ZilchTypeId(Cog), source, &context);
  if(cog == nullptr)
    return nullptr;

  Transform* transform = cog->has(Transform);
  if(transform)
  {
    transform->SetTranslation(position);
    Quat rotation = transform->GetRotation();
    Normalize(rotation);
    transform->SetRotation(rotation);
  }

  cog->Initialize(initializer); 
  initializer.AllCreated();

  return cog;
}

Cog* Space::Create(Archetype* archetype)
{
  if(archetype == nullptr)
    return nullptr;

  return CreateNamed(archetype->ResourceIdName);
}

Cog* Space::CreateAtPosition(Archetype* archetype, Vec3Param position)
{
  if(archetype == nullptr)
    return nullptr;

  return CreateAt(archetype->ResourceIdName, position);
}

Cog* Space::CreateNamed(StringParam source, StringParam name)
{
  // Space is being destroyed?
  if(this->GetMarkedForDestruction())
  {
    // Don't allow objects to be created
    DoNotifyException("Space", "Cannot create a Cog in a Space that is being destroyed. Check the IsBeingDestroyed property on the Space.");
    return nullptr;
  }

  CogCreationContext context(this, source);

  CogInitializer initializer(this);
  initializer.Context = &context;
  Cog* cog = Z::gFactory->BuildAndSerialize(ZilchTypeId(Cog), source, &context);
  if(cog == nullptr)
    return nullptr;

  if(!name.Empty())
    cog->SetName(name);

  cog->Initialize(initializer);
  initializer.AllCreated();

  return cog;
}


Cog* Space::CreateLink(Archetype* archetype, Cog* objectA, Cog* objectB)
{
  if(archetype == nullptr)
    return nullptr;

  return CreateNamedLink(archetype->ResourceIdName, objectA, objectB);
}

Cog* Space::CreateNamedLink(StringParam archetypeName, Cog* objectA, Cog* objectB)
{
  if(objectA == nullptr || objectB == nullptr)
    return nullptr;

  CogCreationContext context(this, archetypeName);

  CogInitializer initializer(this);
  initializer.Context = &context;
  Cog* cog = Z::gFactory->BuildAndSerialize(ZilchTypeId(Cog), archetypeName, &context);

  if(cog == nullptr)
    return nullptr;

  // Set up the object link
  ObjectLink* link = cog->has(ObjectLink);
  if(link)
  {
    link->SetCogAInternal(objectA);
    link->SetCogBInternal(objectB);
  }

  cog->Initialize(initializer);
  initializer.AllCreated();

  return cog;
}

//------------------------------------------------------------ Destroying 

void Space::DestroyAll()
{
  range r = mCogList.All();
  while(!r.Empty())
  {
    if(!r.Front().mFlags.IsSet(CogFlags::Persistent))
      r.Front().ForceDestroy();
    r.PopFront();
  }
}

void Space::DestroyAllFromLevel()
{
  range r = mCogList.All();
  while(!r.Empty())
  {
    if(r.Front().mFlags.IsSet(CogFlags::CreatedFromLevel))
      r.Front().Destroy();
    r.PopFront();
  }
}

void Space::Destroy()
{
  if(!mFlags.IsSet(CogFlags::Protected))
  {
    ForceDestroy();
  }
}

void Space::ForceDestroy()
{
  //let people know this space was destroyed
  if (!GetMarkedForDestruction())
  {
    ObjectEvent e(this);
    GetDispatcher()->Dispatch(Events::SpaceDestroyed, &e);
  }
  range r = mCogList.All();
  while(!r.Empty())
  {
    r.Front().ForceDestroy();
    r.PopFront();
  }
  Cog::ForceDestroy();

  // Remove ourself from the game session list
  if(GameSession* gameSession = GetGameSession())
    gameSession->InternalRemove(this);
}

void Space::SaveLevelFile(StringParam filename)
{
  if(Level* pendingLoad = mPendingLevel)
  {
    ErrorIf(pendingLoad != nullptr, "Can not save while level is loading.");
    return;
  }
  
  // Open the file for write
  Status status;
  ObjectSaver saver;
  saver.Open(status, filename.c_str());

  if(status.Succeeded())
  {
    CogSavingContext context;
    saver.SetSerializationContext(&context);

    
    saver.StartPolymorphic(ZilchTypeId(Level));
    CogSerialization::SaveSpaceToStream(saver, this);
    saver.EndPolymorphic();
  }
}

void Space::SerializeObjectsToSpace(CogInitializer& initializer, 
                                    CogCreationContext& context, 
                                    Serializer& loader)
{
  bool hadCogs = loader.Start("Cogs", "cogs", StructureType::Object);

  uint numberOfObjects = 0;
  loader.ArraySize(numberOfObjects);
  for(uint i = 0; i < numberOfObjects; ++i)
  {
    Cog* cog = Z::gFactory->BuildFromStream(&context, loader);
    if(cog == nullptr)
    {
      Error("Cog failed to be created!");
      //abort serialization.
      return;
    }

    cog->Initialize(initializer);
    //On all objects created called after all objects
    //are initialized.
  }


  if(hadCogs)
    loader.End("Cogs", StructureType::Object);
}

/// Find an object in the space with a given name.
CogNameRange Space::FindAllObjectsByName(StringParam name)
{
  NameCogList* nameList = mNameMap.FindValue(name, nullptr);
  if (nameList)
    return nameList->All();
  else
    return CogNameRange();
}

Cog* Space::FindObjectByName(StringParam name)
{
  return FindLastObjectByName(name);
}

Cog* Space::FindFirstObjectByName(StringParam name)
{
  NameCogList* nameList = mNameMap.FindValue(name, nullptr);
  if (nameList)
    return &nameList->Front();
  else
    return nullptr;
}

Cog* Space::FindLastObjectByName(StringParam name)
{
  NameCogList* nameList = mNameMap.FindValue(name, nullptr);
  if (nameList)
    return &nameList->Back();
  else
    return nullptr;
}

Cog* Space::FindFirstRootObjectByName(StringParam name)
{
  NameCogList* nameList = mNameMap.FindValue(name, nullptr);
  if(nameList)
  {
    forRange(Cog& cog, nameList->All())
    {
      if(cog.GetParent() == nullptr)
        return &cog;
    }
  }
  return nullptr;
}

Cog* Space::FindLastRootObjectByName(StringParam name)
{
  NameCogList* nameList = mNameMap.FindValue(name, nullptr);
  if(nameList)
  {
    forRange(Cog& cog, nameList->ReverseAll())
    {
      if(cog.GetParent() == nullptr)
        return &cog;
    }
  }
  return nullptr;
}

void Space::LoadLevelAdditive(Level* level)
{
  mLevelLoaded = level;
  mLevelLoaded = AddObjectsFromLevel(level);
}

void Space::LoadLevelFile(StringParam filePath)
{
  //Enable in debug for level loading (object initialization, etc)
  FpuExceptionsEnablerDebug();
  TimerBlock block("Loaded level");

  // Load from Level file
  Status status;
  UniquePointer<Serializer> stream(GetLoaderStreamFile(status, filePath));
  if(stream)
  {
    //Read Level Node
    PolymorphicNode node;
    stream->GetPolymorphic(node);

    AddObjectsFromStream(filePath, *stream);

    String fileName = FilePath::GetFileName(filePath);

    MarkNotModified();
    ZPrintFilter(Filter::DefaultFilter, "Level file '%s' was loaded.\n", fileName.c_str());
    stream->EndPolymorphic();
  }
  else
  {
    String message = String::Format("Failed to load level file '%s'", filePath.c_str());
    DoNotifyError("Load Failed", message);
  }

  ObjectEvent e(this);
  this->GetDispatcher()->Dispatch(Events::SpaceLevelLoaded, &e);

}

Level* Space::AddObjectsFromLevel(Level* level)
{
  // Begin Loading Level
  mIsLoadingLevel = true;

  //Enable in debug for level loading (object initialization, etc)
  FpuExceptionsEnablerDebug();

  PushErrorContextObject("Loading level", level);

  TimerBlock block("Loaded level");

  if(level)
  {
    String levelPath = level->GetLoadPath();
    String levelMessage = String::Format("Loading Level From File %s", levelPath.c_str());
    PushErrorContext(levelMessage.c_str());

    // Load from Level resource
    Status status;
    ObjectLoader stream;
    stream.OpenFile(status, levelPath);
    if(status.Succeeded())
    {
      //Read Level Node
      PolymorphicNode node;
      stream.GetPolymorphic(node);

      AddObjectsFromStream(level->Name, stream);

      MarkNotModified();
      ZPrint("Level '%s' was loaded.\n", level->Name.c_str());
    }
    else
    {
      String message = String::Format("Failed to load level '%s' %s", level->Name.c_str(), status.Message.c_str());
      DoNotifyErrorWithContext(message);
    }
  }

  ObjectEvent event(this);
  this->GetDispatcher()->Dispatch(Events::SpaceLevelLoaded, &event);

  // Finished Loading Level
  mIsLoadingLevel = false;
  return level;
}

Space::range Space::AddObjectsFromStream(StringParam source, Serializer& loader)
{
  CogCreationContext context;
  context.mSpace = this;
  context.Source = source;
  context.mGameSession = mGameSession;

  loader.SetSerializationContext(static_cast<void*>(&context));

  //Create the CogInitializer and set the owner.
  CogInitializer initializer(this);
  initializer.Context = &context;
  initializer.mLevel = source;

  SerializeObjectsToSpace(initializer, context, loader);

  return initializer.AllCreated();
}

void Space::LoadLevel(Level* level)
{
  // Space is being destroyed?
  if(this->GetMarkedForDestruction())
  {
    // Don't allow levels to be loaded
    DoNotifyException("Space", "Cannot load a Level in a Space that is being destroyed. Check the IsBeingDestroyed property on the Space.");
    return;
  }

  if(Level* pendingLoad = mPendingLevel)
  {
    ZPrintFilter(Filter::DefaultFilter, "Already loading level '%s'\n", pendingLoad->ResourceIdName.c_str());
    return;
  }

  if(level == nullptr)
  {
    ZPrintFilter(Filter::DefaultFilter, "Failed to find level.\n");
    return;
  }

  if(!mLevelLoaded)
  {
    ZPrintFilter(Filter::DefaultFilter, "Loading level '%s' directly.\n", level->Name.c_str());

    // No level has ever been loaded into this space. This is when a space has
    // just been created and no important objects are loaded.
    // Load level directly
    LoadLevelAdditive(level);
  }
  else
  {
    ZPrintFilter(Filter::DefaultFilter, "Loading level '%s' delayed.\n", level->Name.c_str());

    // Add all objects to the destroy list
    DestroyAll();

    // There is a issue when a level is loaded where all objects from the current space
    // will exist with and interfere with the new objects due to delayed destruction.
    // Wait one frame for the previous objects to be destroyed then load the level.
    mPendingLevel = level;
  }
}

void Space::LoadPendingLevel()
{
  if(Level* level = mPendingLevel)
  {
    LoadLevelAdditive(level);
    mPendingLevel = nullptr;
  }
}

void Space::ReloadLevel()
{
  Level* level = mLevelLoaded;
  if(level)
    LoadLevel(level);
}

void Space::MarkModified()
{
  if(!mModified)
  {
    mModified = true;

    ObjectEvent e(this);
    this->GetDispatcher()->Dispatch(Events::SpaceModified, &e);
  }
}

void Space::MarkNotModified()
{
  if(mModified)
  {
    mModified = false;

    ObjectEvent e(this);
    this->GetDispatcher()->Dispatch(Events::SpaceModified, &e);
  }
}

void Space::CheckForChangedObjects()
{
  if(mObjectsChanged)
  {
    mObjectsChanged = false;

    ObjectEvent e(this);
    this->GetDispatcher()->Dispatch(Events::SpaceObjectsChanged, &e);
  }
}

void Space::ChangedObjects()
{
  mObjectsChanged = true;
}

bool Space::GetModified()
{
  return mModified;
}

void Space::SetName(StringParam newName)
{
  Cog::SetName(newName);
  if(GameSession* gameSession = GetGameSession())
    gameSession->InternalRenamed(this);
}

Level* Space::GetCurrentLevel()
{
  Level* level = mLevelLoaded;
  if(level)
    return level;
  else
    return nullptr;
}

}//namespace Zero
