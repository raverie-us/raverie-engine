///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"


namespace Zero
{
ZilchDefineType(ContentCopyright, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::CallSetDefaults);

  ZilchBindFieldProperty(Owner);
  ZilchBindFieldProperty(Date);
}

void ContentCopyright::Serialize(Serializer& stream)
{
  SerializeName(Owner);
  SerializeName(Date);
}

ZilchDefineType(ContentHistory, builder, type)
{
  //ZilchBindFieldProperty(mRevisions); // METAREFACTOR array

  ZeroBindComponent();
  ZeroBindSetup(SetupMode::CallSetDefaults);
  
  ZilchBindFieldProperty(mRevisions);
}

void ContentHistory::Initialize(ContentComposition* item)
{
  mOwner = item;

  SourceControl* sourceControl = GetSourceControl(mOwner->mLibrary->SourceControlType);

  //get the path to the meta file
  String path = mOwner->GetMetaFilePath();

  Status status;
  sourceControl->GetRevisions(status, path, mRevisions);
}

ZilchDefineType(ContentNotes, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::CallSetDefaults);

  ZilchBindFieldProperty(Notes);
}

void ContentNotes::Serialize(Serializer& stream)
{
  SerializeName(Notes);
}

ZilchDefineType(ResourceTemplate, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::CallSetDefaults);

  ZilchBindFieldProperty(mDisplayName);
  ZilchBindFieldProperty(mDescription);
  ZilchBindFieldProperty(mSortWeight);
  ZilchBindFieldProperty(mCategory);
  ZilchBindFieldProperty(mCategorySortWeight);
}

void ResourceTemplate::Serialize(Serializer& stream)
{
  SerializeNameDefault(mDisplayName, String());
  SerializeNameDefault(mDescription, String());
  SerializeNameDefault(mSortWeight, 100u);
  SerializeNameDefault(mCategory, String());
  SerializeNameDefault(mCategorySortWeight, 100u);
}

//------------------------------------------------------------------------------
void CreateSupportContent(ContentSystem* system)
{
  AddContentComponent<ContentCopyright>(system);
  AddContentComponent<ContentHistory>(system);
  AddContentComponent<ContentNotes>(system);
  AddContentComponent<ResourceTemplate>(system);
}

}// namespace Zero
