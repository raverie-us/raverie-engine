///////////////////////////////////////////////////////////////////////////////
///
/// \file TileView.cpp
/// Implementation of the TileView and supporting classes.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace PreviewWidgetUi
{
const cstr cLocation = "EditorUi/PreviewWidgets";
Tweakable(Vec2, DefaultSize, Pixels(130,130), cLocation);
}

namespace Events
{
  DefineEvent(PreviewObjectChanged);
}

//------------------------------------------------------------------ Tile Widget
ZilchDefineType(PreviewWidget, builder, type)
{
}

//******************************************************************************
PreviewWidget::PreviewWidget(Composite* parent)
  : Composite(parent)
{
  mMinSize = PreviewWidgetUi::DefaultSize;
}

//******************************************************************************
PreviewWidget::PreviewWidget(PreviewWidgetInitializer& initializer)
  : Composite(initializer.Area)
{
  mObject = initializer.Object;
  mName = initializer.Name;
  mMinSize = PreviewWidgetUi::DefaultSize;
}

//******************************************************************************
void PreviewWidget::UpdateTransform()
{
  Composite::UpdateTransform();
}

//******************************************************************************
Vec2 PreviewWidget::GetHalfSize()
{
  return GetMinSize() * 0.5f;
}

//--------------------------------------------------------- Preview Widget Group
//******************************************************************************
PreviewWidgetGroup::PreviewWidgetGroup(Composite* parent)
  : PreviewWidget(parent)
{

}

//******************************************************************************
PreviewWidgetGroup::PreviewWidgetGroup(Composite* parent, StringParam name)
  : PreviewWidget(parent)
{
  mName = name;
}

//******************************************************************************
PreviewWidget* PreviewWidgetGroup::AddPreviewWidget(StringParam name, HandleParam instance,
                                                    PreviewImportance::Enum minImportance)
{
  // Create the preview
  PreviewWidget* preview = ResourcePreview::CreatePreviewWidget(this, name,
                                                       instance, minImportance);

  if(preview == nullptr)
    return nullptr;

  // Add it
  mPreviewWidgets.PushBack(preview);
  MarkAsNeedsUpdate();
  return preview;
}

//******************************************************************************
void PreviewWidgetGroup::AnimatePreview(PreviewAnimate::Enum value)
{
  forRange(PreviewWidget* preview, mPreviewWidgets.All())
  {
    preview->AnimatePreview(value);
  }
}

//******************************************************************************
void PreviewWidgetGroup::UpdateTransform()
{
  if (mSize.LengthSq() < 0.00001f || mPreviewWidgets.Empty())
  {
    Composite::UpdateTransform();
    return;
  }

  const float cPadding = Pixels(2);

  IntVec2 iDimensions = GetDimensions();
  Vec2 dimension = Math::ToVec2(iDimensions);
  Vec2 itemSize = (mSize) / dimension;
  itemSize.x = Math::Floor(itemSize.x);
  itemSize.y = Math::Floor(itemSize.y);

  Vec2 padding = SnapToPixels((dimension - Vec2(1,1)) * Vec2(cPadding, cPadding));
  itemSize -= SnapToPixels(padding / dimension);
  
  for(int x = 0; x < iDimensions.x; ++x)
  {
    for(int y = 0; y < iDimensions.y; ++y)
    {
      uint index = x + (iDimensions.x * y);
      if(index >= mPreviewWidgets.Size())
        break;

      PreviewWidget* preview = mPreviewWidgets[index];

      Vec2 translation = ToVec2(IntVec2(x, y)) * itemSize + ToVec2(IntVec2(x, y)) * Vec2(cPadding, cPadding);
      preview->SetTranslation(ToVector3(translation));
      preview->SetSize(itemSize);
    }
  }

  Composite::UpdateTransform();
}

//******************************************************************************
Vec2 PreviewWidgetGroup::GetMinSize()
{
  Vec2 itemSize = GetMinTileSize();
  IntVec2 intDimensions = GetDimensions();
  Vec2 dimension = Math::ToVec2(intDimensions);
  return itemSize * dimension + Pixels(2, 2) * Math::ToVec2(intDimensions + IntVec2(1,1));
}

//******************************************************************************
IntVec2 PreviewWidgetGroup::GetDimensions()
{
  uint count = mPreviewWidgets.Size();

  // Max is 10x10
  const uint cMax = 10;

  // Start at a 1x1 until we fit into a square
  for(uint i = 1; i < cMax; ++i)
  {
    if(count <= i * i)
    {
      uint x, y;
      x = y = i;

      // If all the objects can be contained with one less row,
      // remove the bottom row
      if(count <= (x * (y-1)))
        y -= 1;

      return IntVec2(x, y);
    }
  }

  Error("Too many objects on preview.");
  return IntVec2(1,1);
}

//******************************************************************************
Vec2 PreviewWidgetGroup::GetMinTileSize()
{
  Vec2 itemSize = Vec2::cZero;
  forRange(PreviewWidget* preview, mPreviewWidgets.All())
  {
    Vec2 minSize = preview->GetHalfSize();
    itemSize = Math::Max(itemSize, minSize);
  }
  return itemSize;
}

//----------------------------------------------------- Tile View Widget Factory
ZilchDefineType(PreviewWidgetFactory, builder, type)
{
}

//******************************************************************************
PreviewWidget* ResourcePreview::CreatePreviewWidget(Composite* parent, StringParam name, HandleParam instance,
                                            PreviewImportance::Enum minImportance)
{
  PreviewWidgetFactory* tileFactory = PreviewWidgetFactory::GetInstance();

  String itemType = instance.StoredType ? instance.StoredType->Name : String();
  PreviewWidgetCreator defaultEntry(PreviewImportance::None, tileFactory->GetCreator(CoreArchetypes::EmptyTile));
  PreviewWidgetCreator createTileWidget = tileFactory->Creators.FindValue(itemType, defaultEntry);
  if(createTileWidget.Creator && createTileWidget.Importance >= minImportance)
  {
    PreviewWidgetInitializer initializer;
    initializer.Area = parent;
    initializer.Name = name;
    initializer.Object = instance;

    PreviewWidget* tileViewWidget = (*createTileWidget.Creator)(initializer);
    return tileViewWidget;
  }

  return nullptr;
}

//******************************************************************************
PreviewImportance::Enum ResourcePreview::GetPreviewImportance(BoundType* resourceType)
{
  PreviewWidgetFactory::CellEditorMapType& creators = PreviewWidgetFactory::GetInstance()->Creators;
  PreviewWidgetCreator* createTileWidget = creators.FindPointer(resourceType->Name, nullptr);

  ReturnIf(createTileWidget == nullptr, PreviewImportance::None, "Could not find creator");

  return createTileWidget->Importance;
}

}//namespace Zero
