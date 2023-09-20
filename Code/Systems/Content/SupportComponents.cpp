// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{
RaverieDefineType(ContentCopyright, builder, type)
{
  RaverieBindComponent();
  RaverieBindSetup(SetupMode::CallSetDefaults);

  RaverieBindFieldProperty(Owner);
  RaverieBindFieldProperty(Date);
}

void ContentCopyright::Serialize(Serializer& stream)
{
  SerializeName(Owner);
  SerializeName(Date);
}

RaverieDefineType(ContentNotes, builder, type)
{
  RaverieBindComponent();
  RaverieBindSetup(SetupMode::CallSetDefaults);

  RaverieBindFieldProperty(Notes);
}

void ContentNotes::Serialize(Serializer& stream)
{
  SerializeName(Notes);
}

RaverieDefineType(ContentEditorOptions, builder, type)
{
  RaverieBindComponent();
  RaverieBindSetup(SetupMode::CallSetDefaults);

  RaverieBindFieldProperty(mShowInEditor);
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

RaverieDefineType(ResourceTemplate, builder, type)
{
  RaverieBindComponent();
  RaverieBindSetup(SetupMode::CallSetDefaults);

  RaverieBindFieldProperty(mDisplayName);
  RaverieBindFieldProperty(mDescription);
  RaverieBindFieldProperty(mSortWeight);
  RaverieBindFieldProperty(mCategory);
  RaverieBindFieldProperty(mCategorySortWeight);
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

} // namespace Raverie
