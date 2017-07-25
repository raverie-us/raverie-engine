///////////////////////////////////////////////////////////////////////////////
///
/// \file ContentItem.cpp
///
/// 
/// Authors: Chris Peters
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{
ZilchDefineType(ContentItem, builder, type)
{
  type->HandleManager = ZilchManagerId(ContentItemHandleManager);
  ZilchBindGetterProperty(Name);
}

void ContentItemHandleManager::ObjectToHandle(const byte* object, BoundType* type, Handle& handleToInitialize)
{
  if (object == nullptr)
    return;

  ContentItem* item = (ContentItem*)object;
  handleToInitialize.HandleU32 = item->Id;
}

byte* ContentItemHandleManager::HandleToObject(const Handle& handle)
{
  return (byte*)Z::gContentSystem->LoadedItems.FindValue(handle.HandleU32, nullptr);
}

ContentItem::ContentItem()
{
  mLibrary = nullptr;
  mRuntimeResource = 0;
  ShowInEditor = true;
  EditMode = ContentEditMode::NoEdit;
  mResourceIsContentItem = false;
  mIgnoreMultipleResourcesWarning = false;

  // Generate an Id for this Content Item
  ++Z::gContentSystem->mIdCount;
  Id = Z::gContentSystem->mIdCount;

  // Store the handle
  Z::gContentSystem->LoadedItems[Id] = this;
}

ContentItem::~ContentItem()
{
  Z::gContentSystem->LoadedItems.Erase(Id);
}

String ContentItem::GetName()
{
  return FilePath::GetFileNameWithoutExtension(Filename);
}

void ContentItem::SaveContent()
{
  ErrorIf(EditMode == ContentEditMode::NoEdit, "Can not save. No edit type.");

  if(EditMode == ContentEditMode::ResourceObject)
  {
    //Ask the resource to save itself
    Resource* resource = Z::gResources->GetResource(mRuntimeResource);
    ErrorIf(resource == nullptr, "Resource has been removed but this content item still refers to it.");

    if(resource)
    {
      if(Z::gContentSystem->mHistoryEnabled)
      {
        String backUpPath = Z::gContentSystem->GetHistoryPath(mLibrary);
        BackUpFile(backUpPath, GetFullPath());
      }
      resource->Save(this->GetFullPath());
    }
  }

  //Save the meta file
  SaveMetaFile();
}


void ContentItem::SaveMetaFile()
{
  String metaFilePath = GetMetaFilePath();
  SaveToDataFile(*this, metaFilePath);
}


String ContentItem::GetMetaFilePath()
{
  return BuildString(GetFullPath(), ".meta");
}

String ContentItem::GetFullPath()
{
  return FilePath::Combine(mLibrary->SourcePath, Filename);
}

void ContentItem::AddTags(Array<String>& tags)
{
  ContentTags* contentTags = this->has(ContentTags);
  if(contentTags == nullptr)
    return;

  forRange(String currTag, contentTags->mTags.All())
  {
    tags.PushBack(currTag);
  }
}

void ContentItem::AddTags(HashSet<String>& tags)
{
  ContentTags* contentTags = this->has(ContentTags);
  if(contentTags == nullptr)
    return;

  forRange(String currTag, contentTags->mTags.All())
  {
    tags.Insert(currTag);
  }
}

void ContentItem::SetTag(StringParam tag)
{
  ContentTags* contentTags = this->has(ContentTags);
  if(contentTags == nullptr)
  {
    contentTags = new ContentTags();
    AddComponent(contentTags);
  }

  // Insert tag
  contentTags->mTags.Insert(tag);
}

void ContentItem::SetTags(HashSet<String>& tags)
{
  if(tags.Empty())
    return;

  ContentTags* contentTags = this->has(ContentTags);
  if(contentTags == nullptr)
  {
    contentTags = new ContentTags();
    AddComponent(contentTags);
  }

  // Insert each tag
  forRange(String tag, tags.All())
  {
    contentTags->mTags.Insert(tag);
  }
}

bool ContentItem::HasTag(StringParam tag)
{
  ContentTags* contentTags = this->has(ContentTags);
  if(contentTags)
    return contentTags->mTags.Contains(tag);
  return false;
}

Object* ContentItem::GetEditingObject(Resource* resource)
{
  ErrorIf(EditMode == ContentEditMode::NoEdit, "Can not edit. No edit type.");
  if(EditMode == ContentEditMode::ResourceObject)
  {
    if (resource == nullptr)
      return Z::gResources->GetResource(mRuntimeResource);

    BoundType* resourceType = ZilchVirtualTypeId(resource);
    
    forRange(Property* prop, resourceType->GetProperties())
    {
      if (prop->HasAttribute(Zilch::PropertyAttribute))
        return resource;
    }

    return this;
  }
  else
  {
    return this;
  }
}

void ContentItem::BuildListing(ResourceListing& listing)
{

}

void ContentItem::Serialize(Serializer& stream)
{

}

void ContentItem::Initialize(ContentLibrary* library)
{
  mLibrary = library;
  if(library)
    library->AddContentItem(this);
  OnInitialize();
}

ContentComponent* ContentItem::QueryComponentId(BoundType* typeId)
{
  return nullptr;
}

void ContentItem::OnInitialize()
{

}

}//namespace Zero
