////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2017, DigiPen Institute of Technology
///
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Tags
{
  DefineTag(Component);
}

//--------------------------------------------------------------------------------- Component Memory
Memory::Heap* Component::sHeap = new Memory::Heap("Components", Memory::GetNamedHeap("Objects"));

void* Component::operator new(size_t size){return sHeap->Allocate(size);}
void Component::operator delete(void* pMem, size_t size){return sHeap->Deallocate(pMem, size);}

//**************************************************************************************************
Handle ComponentGetOwner(HandleParam object)
{
  Component* component = object.Get<Component*>();
  return component->GetOwner();
}

//**************************************************************************************************
ZilchDefineType(ProxyObject<Component>, builder, type)
{
}

//---------------------------------------------------------------------------------------- Component
//**************************************************************************************************
ZilchDefineType(Component, builder, type)
{
  ZilchBindDefaultConstructor();
  type->HandleManager = ZilchManagerId(ComponentHandleManager);

  //METAREFACTOR Take care of this stuff (meta components)
  //type->ShortDescription = GetShortDescriptionComponent;
  
  type->Add(new MetaSerialization());
  type->Add(new MetaOwner(ComponentGetOwner));
  type->Add(new ComponentMetaDataInheritance());
  type->Add(new ComponentMetaOperations());
  type->AddAttribute(ObjectAttributes::cStoreLocalModifications);

  ZeroBindDocumented();
  ZilchBindGetterAs(OwnerScript, "Owner");
  ZilchBindGetter(Space);
  ZilchBindGetter(LevelSettings);
  ZilchBindGetter(GameSession);

  ZilchBindMethod(DebugDraw);
  ZeroBindTag(Tags::Component);
}

//**************************************************************************************************
Component::Component() : mOwner(NULL)
{
}

//**************************************************************************************************
void Component::Delete()
{
  delete this;
}

//**************************************************************************************************
Cog* Component::GetOwner() const
{
  ErrorIf(mOwner == NULL, "Component is not yet initialized.");
  return mOwner;
}

//**************************************************************************************************
Cog* Component::GetOwnerScript() const
{
  if (mOwner && mOwner->IsInitialized())
    return mOwner;
  return nullptr;
}

//**************************************************************************************************
Space* Component::GetSpace()
{
  if (mOwner)
    return mOwner->GetSpace();
  return NULL;
}

//**************************************************************************************************
Cog* Component::GetLevelSettings()
{
  if (mOwner)
    return mOwner->GetLevelSettings();
  return NULL;
}

//**************************************************************************************************
GameSession* Component::GetGameSession()
{
  if (mOwner)
    return mOwner->GetGameSession();
  return NULL;
}

//**************************************************************************************************
EventReceiver* Component::GetReceiver()
{
  if (mOwner)
    return mOwner->GetReceiver();
  return NULL;
}

//**************************************************************************************************
EventDispatcher* Component::GetDispatcher()
{
  if (mOwner)
    return mOwner->GetDispatcher();
  return NULL;
}

//**************************************************************************************************
bool Component::IsInitialized()
{
  if (mOwner)
    return mOwner->IsInitialized();
  return false;
}

//**************************************************************************************************
void Component::WriteDescription(StringBuilder& builder)
{
  BoundType* type = ZilchVirtualTypeId(this);
  builder << "Component " << type->Name << " of ";

  if (mOwner != nullptr)
    mOwner->WriteDescription(builder);
  else
    builder << "Null Cog";
}

//**************************************************************************************************
String Component::GetDescription()
{
  StringBuilder builder;
  builder << "<";
  WriteDescription(builder);
  builder << ">";
  return builder.ToString();
}

//**************************************************************************************************
void Component::DispatchEvent(StringParam eventId, Event* event)
{
  ReturnIf(!mOwner,, "The Owner was null (is this being called from a constructor or destructor?)");
  mOwner->GetDispatcher()->Dispatch(eventId, event);
}


//------------------------------------------------------------------------- Component Handle Manager
//**************************************************************************************************
void ComponentHandleManager::Allocate(BoundType* type, Handle& handleToInitialize, size_t customFlags)
{
  handleToInitialize.Flags |= HandleFlags::NoReferenceCounting;

  ComponentHandleData& data = *(ComponentHandleData*)(handleToInitialize.Data);
  data.mCogId = CogId();
  data.mRawObject = zAllocate(type->Size);
  memset(data.mRawObject, 0, type->Size);
  data.mComponentType = type;
}

//**************************************************************************************************
void ComponentHandleManager::ObjectToHandle(const byte* object, BoundType* type, Handle& handleToInitialize)
{
  if (object == nullptr)
    return;

  Component* component = (Component*)object;

  ComponentHandleData& data = *(ComponentHandleData*)(handleToInitialize.Data);
  data.mComponentType = ZilchVirtualTypeId(component);

  Cog* cog = component->mOwner;

  // METAREFACTOR This is temporary. We need to rethink Component handles.
  if (cog == nullptr || !cog->mComponentMap.Contains(component))
  {
    data.mCogId = CogId();
    data.mRawObject = component;
  }
  else
  {
    data.mCogId = cog;
    data.mRawObject = nullptr;
  }
}

//**************************************************************************************************
//Handle to a component is a handle to the cog for the
//primary id and a sub object if of the component type
//this allow the Cog to control lifetime and the component
//to be removed and have the handle still work.
byte* ComponentHandleManager::HandleToObject(const Handle& handle)
{
  const ComponentHandleData& data = *(const ComponentHandleData*)(handle.Data);

  if (data.mRawObject)
  {
    Component* component = (Component*)data.mRawObject;
    return (byte*)component;
  }

  // Check the cog handle
  if (Cog* cog = data.mCogId)
  {
    // Check for the component
    if (Component* component = cog->QueryComponentType(data.mComponentType))
    {
      return (byte*)component;
    }
  }
  return nullptr;
}

//**************************************************************************************************
bool ComponentHandleManager::CanDelete(const Handle& handle)
{
  return true;
}

//**************************************************************************************************
void ComponentHandleManager::Delete(const Handle& handle)
{
  const ComponentHandleData& data = *(const ComponentHandleData*)(handle.Data);

  // METAREFACTOR This is what was previously happening with delete, except this doesn't seem correct
  // since mRawObject is not set in all cases...
  zDeallocate(data.mRawObject);
}

}//namespace Zero
