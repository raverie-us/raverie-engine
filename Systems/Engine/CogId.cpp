///////////////////////////////////////////////////////////////////////////////
///
/// \file CogId.cpp
/// Implementation of CogId class.
///
/// Authors: Chris Peters
/// Copyright 2010-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
CogId::CogId()
  : Id(cInvalidObjectRawId), Slot(cInvalidObjectRawId)
{
}

CogId::CogId(u32 id, u32 slot)
  : Id(id), Slot(slot)
{
}

CogId::CogId(u64 idAndSlot)
{
  CogId* other = (CogId*)&idAndSlot;
  Id = other->Id;
  Slot = other->Slot;
}

CogId::CogId(Cog* object)
{
  if(object)
  {
    *this = object->GetId();
  }
  else
  {
   Id = cInvalidObjectRawId;
   Slot = cInvalidObjectRawId;
  }
}

Cog* CogId::ToCog() const
{
  return Z::gTracker->GetObjectWithId(*this);
}

u64 CogId::ToUint64() const
{
  return  *(u64*)this;
}

u32 CogId::Hash() const
{
  return Id ^ Slot;
}

bool CogId::IsValid()
{
  return ToCog() != NULL;
}

void CogId::SafeDestroy()
{
  if(Cog* cog = ToCog())
    cog->Destroy();
}

Component* CogId::QueryComponentId(BoundType* typeId)
{
  Cog* cog = ToCog();
  if(cog != nullptr)
    return cog->QueryComponentType(typeId);
  else
    return nullptr;
}

namespace Serialization
{
  bool Policy<CogId>::Serialize(Serializer& stream, cstr fieldName, CogId& cogId)
  {
    CogSavingContext* context = static_cast<CogSavingContext*>(stream.GetSerializationContext());

    if(context != NULL && context->CurrentContextMode == ContextMode::Saving)
    {
      CogSavingContext* savingContext = (CogSavingContext*)context;

      // If the slot is cInvalidObjectRawId this
      // id has not had OnAllObjectsCreated created called
      // on it and is being saved. (Can happen with binary cache)
      // In this case just use the id.
      // Otherwise convert the id to a contextId and save it.
      u32 contextId = cogId.Id;
      if(cogId.Slot != cInvalidObjectRawId)
        contextId = savingContext->ToContextId(contextId);

      // Save as Int
      return stream.FundamentalField(fieldName, contextId);
    }
    else
    {
      // Load as Int into id
      return stream.FundamentalField(fieldName, cogId.Id);
    }
  }
}


Cog* CogId::OnAllObjectsCreated(CogInitializer& initializer)
{
  return RestoreLink(this, initializer.Context);
}

Cog* RestoreLink(CogId* id, CogCreationContext* context, Component* component,
                 StringParam propertyName, bool isError)
{
  // Id is invalid link is intentionally NULL.
  if(*id == cInvalidCogId || id->Id == 0)
    return NULL;

  // The CogId can be set to an actual valid Id
  // in that case the SlotIndex not be cInvalidObjectRawId
  if(id->Slot != cInvalidObjectRawId)
  {
    //Try the CogId without context
    Cog* cog = id->ToCog();
    if(cog)
    {
      *id = cog->GetId();
      return cog;
    }
    else
      return NULL;
  }

  if(context==NULL)
    return NULL;

  //The CogId refers to an object in the local creation context. Look up the id
  //in the stream id amp
  CogCreationContext::IdMapType::range r0 = context->mContextIdMap.Find(id->Id + context->mCurrentSubContextId);
  if(r0.Empty())
  {
    // Look in space if 'LocalModificationOverride' is set on the property
    if(component)
    {
      BoundType* componentType = ZilchVirtualTypeId(component);
      Property* metaProperty = componentType->GetProperty(propertyName);
      ErrorIf(metaProperty == nullptr, String::Format("Property '%s' doesn't exist on component '%s'", propertyName.c_str(), componentType->Name.c_str()).c_str());
      if(metaProperty && metaProperty->HasAttribute(PropertyAttributes::cLocalModificationOverride))
        r0 = context->mContextIdMap.Find(id->Id);
    }
  }
  if(!r0.Empty())
  {
    //ZPrint("Resolve link id %d  context %d.\n", id->Id, context->ContextId);
    *id = r0.Front().second.Object->GetId();
    return r0.Front().second.Object;
  }
  else
  {
    if (isError)
    {
      //The object could not be found. An object referred to an object outside its context within archetype or level.
      String errorMessage = "Failed to restore direct id link";

      if(component)
      {
        String componentName = component->GetDescription();
        errorMessage = String::Format("Failed to restore direct id link '%s for property %s'", componentName.c_str(), propertyName.c_str());
      }
      else if(!propertyName.Empty())
      {
        errorMessage = String::Format("Failed to restore direct id link %s", propertyName.c_str());
      }

      DoNotifyErrorWithContext(errorMessage);
    }

    *id = cInvalidCogId;
    return NULL;
  }
}

}
