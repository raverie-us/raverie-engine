///////////////////////////////////////////////////////////////////////////////
///
/// \file PropertyView.cpp
/// Implementation of PropertyView and supporting classes.
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

const String cLabel = "PropertyLabel";

ZilchDefineType(MetaPropertyEditor, builder, type)
{
}

//-------------------------------------------------------------- Property Widget
ZilchDefineType(PropertyWidget, builder, type)
{
}

//******************************************************************************
PropertyWidget::PropertyWidget(PropertyWidgetInitializer& init, StyleMode::Enum style)
  : Composite(init.Parent)
{
  mDestination = Vec3::cZero;
  mAnimating = false;
  static const String className = "PropertyGrid";
  mDefSet = init.Parent->GetDefinitionSet()->GetDefinitionSet(className);

  mProp = init.CurrentInterface;
  mGrid = init.Grid;

  mCustomIcons = nullptr;

  // Add custom icons if they exist
  PropertyView::CustomPropertyIconArray& customIconCallbacks = mGrid->mCustomIconCallbacks;
  if(!customIconCallbacks.Empty())
  {
    mCustomIcons = new Composite(this);
    mCustomIcons->SetMinSize(Pixels(0, 0));
    mCustomIcons->SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Pixels(5, 0), Thickness(Pixels(-3, 4, 2, 0))));
    
    for(uint i = 0; i < customIconCallbacks.Size(); ++i)
    {
      CustomIconCreatorFunction callback = mGrid->mCustomIconCallbacks[i];
      
      // Always default to the given instance of the Cog
      Handle object = init.Instance;
      
      // If it has a sub-object, use that instance instead
      if(init.ObjectNode && init.ObjectNode->mObject.IsNotNull())
        object = init.ObjectNode->mObject;
      
      void* clientData = mGrid->mCallbackClientData.FindValue(callback, nullptr);
      callback(mCustomIcons, object, init.Property, clientData);
    }

    if(mCustomIcons->mChildren.Empty())
      mCustomIcons->SetActive(false);
    else
      mCustomIcons->SizeToContents();
  }  

  if(style == StyleMode::Regular)
    mLabel = new Label(this, cLabel);
  else
    mLabel = new Label(this, "BoldText");

  mLabel->SetTextClipping(true);

  ConnectThisTo(mLabel, Events::MouseHover, OnMouseHover);

  SetSize(Vec2(0, PropertyViewUi::PropertySize));
  Parent = nullptr;
}

//******************************************************************************
PropertyWidget::~PropertyWidget()
{
  if(Parent != nullptr)
    Parent->ChildWidgets.Erase(this);
}

//******************************************************************************
void PropertyWidget::OnMouseHover(MouseEvent* event)
{
  ToolTipColor::Enum color = ToolTipColor::Default;
  String toolTipText = GetToolTip(&color);
  if(!toolTipText.Empty())
  {
    ToolTip* toolTip = new ToolTip(mLabel);
    toolTip->SetText(toolTipText);
    toolTip->SetColor(color);

    ToolTipPlacement placement;
    placement.SetScreenRect(GetScreenRect());
    placement.SetPriority(IndicatorSide::Right, IndicatorSide::Left, 
                          IndicatorSide::Bottom, IndicatorSide::Top);
    toolTip->SetArrowTipTranslation(placement);
  }
}

//******************************************************************************
void PropertyWidget::UpdateTransform()
{
  Composite::UpdateTransform();
}

//******************************************************************************
LayoutResult PropertyWidget::GetNameLayout()
{
  float namePercent = mGrid->mNamePercent;
  float nameSize = SnapToPixels(mGrid->mNamePercent * mSize.x);
  nameSize = Math::Min(nameSize, 140.0f);

  LayoutResult layout;
  layout.Size = SnapToPixels(Vec2(nameSize, mSize.y));

  // Offset the name by a custom icon if it exists
  float x = 0;

  if(mCustomIcons && mCustomIcons->GetActive())
    x = mCustomIcons->GetSize().x;

  layout.Translation = SnapToPixels(Vec3(x, Pixels(1), 0));
  return layout;
}

//******************************************************************************
LayoutResult PropertyWidget::GetContentLayout(LayoutResult& nameLayout)
{
  float offset = nameLayout.Size.x + nameLayout.Translation.x;
  float editSizeX = mSize.x - offset;

  LayoutResult r;
  r.Size = Vec2(editSizeX, mSize.y - 2.0f);
  r.Translation = Vec3(offset, 1.0f, 0);
  return r;
}

}//namespace Zero
