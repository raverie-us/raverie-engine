// MIT Licensed (see LICENSE.md).
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

ZilchDefineType(ContentEditorOptions, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::CallSetDefaults);

  ZilchBindFieldProperty(mShowInEditor);
}

void ContentEditorOptions::Serialize(Serializer& stream)
{
  SerializeNameDefault(mShowInEditor, false);
}

void ContentEditorOptions::Initialize(ContentComposition* item)
{
  mOwner = item;
  item->ShowInEditor = mShowInEditor;
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

void CreateSupportContent(ContentSystem* system)
{
  AddContentComponent<ContentCopyright>(system);
  AddContentComponent<ContentNotes>(system);
  AddContentComponent<ContentEditorOptions>(system);
  AddContentComponent<ResourceTemplate>(system);
}

} // namespace Zero
