///////////////////////////////////////////////////////////////////////////////
///
/// \file ToolTip.hpp
///  Declaration of the ToolTip Widget.
///
/// Authors: Joshua Claeys, Chris Peters
/// Copyright 2010-2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace ToolTipUi
{
const cstr cLocation = "EditorUi/Controls/ToolTip";
Tweakable(Vec4, BackgroundColor, Vec4(1, 1, 1, 1), cLocation);
Tweakable(Vec4, BorderColor, Vec4(1, 1, 1, 1), cLocation);
Tweakable(float, ToolTipWrapWidth, 280.0f, cLocation);
}

//----------------------------------------------------------- Tool Tip Placement
//******************************************************************************
ToolTipPlacement::ToolTipPlacement()
{
  mScreenRect = Rect::PointAndSize(Vec2::cZero, Vec2::cZero);

  SetPriority(IndicatorSide::Left, IndicatorSide::Right,
    IndicatorSide::Top, IndicatorSide::Bottom);
}

//******************************************************************************
void ToolTipPlacement::SetScreenRect(const Rect& rect)
{
  mScreenRect = rect;
  mHotSpot = rect.Center();
}

//******************************************************************************
void ToolTipPlacement::SetPriority(IndicatorSide::Type pri0, IndicatorSide::Type pri1,
  IndicatorSide::Type pri2, IndicatorSide::Type pri3)
{
  mPriority[0] = pri0;
  mPriority[1] = pri1;
  mPriority[2] = pri2;
  mPriority[3] = pri3;
}

//--------------------------------------------------------------------- Tool Tip
ZilchDefineType(ToolTip, builder, type)
{
}

//******************************************************************************
ToolTip::ToolTip(Widget* source)
  : Composite(source->GetRootWidget()->GetPopUp())
{
  Initialize(source);
}

//******************************************************************************
ToolTip::ToolTip(Widget* source, StringParam text)
  : Composite(source->GetRootWidget()->GetPopUp())
{
  Initialize(source);
  SetText(text);

  ToolTipPlacement placement;
  placement.SetScreenRect(source->GetScreenRect());

  SetArrowTipTranslation(placement);
}

//******************************************************************************
void ToolTip::Initialize(Widget* source)
{
  SetInteractive(false);

  // Store the source
  mSource = source;

  // The default border color
  SetColor(ToolTipColor::Default);

  mBackground = CreateAttached<Element>(cWhiteSquare);
  mBorder = CreateAttached<Element>(cWhiteSquareBorder);
  mArrow = CreateAttached<Element>("IndicatorArrow");
  mArrowBorder = CreateAttached<Element>("IndicatorArrowBorder");

  // We will be rotating the arrows, so the easiest is to have them centered
  mArrow->mOrigin = DisplayOrigin::Center;
  mArrowBorder->mOrigin = DisplayOrigin::Center;

  // Default margins
  mContentPadding = Thickness(6, 4, 6, 4);
  mContent = nullptr;
  mArrowOffset = Vec2::cZero;
  mSide = IndicatorSide::Right;

  mDestroyOnMouseExitSource = true;

  ConnectThisTo(source->GetRootWidget(), Events::MouseUpdate, OnMouseUpdate);
}

//******************************************************************************
void ToolTip::UpdateTransform()
{
  // Set the color of the border objects
  mBorder->SetColor(mBorderColor);
  mArrowBorder->SetColor(mBorderColor);
  
  mArrow->SetColor(mBackgroundColor);
  mBackground->SetColor(mBackgroundColor);

  // The size of the arrow
  float arrowSize = mArrow->GetSize().x;

  Rect contentArea = Rect::PointAndSize(Vec2::cZero, GetSize());
  Rect arrowArea = Rect::PointAndSize(Vec2::cZero, GetSize());
  float arrowRotation = 0.0f;

  // Used for minor adjustments for pixel perfect results
  Vec2 arrowAdjustments = Vec2::cZero;

  // Used to shrink the content and arrow area's based on which
  // side the arrow needs to be
  Thickness contentMargins = Thickness::cZero;
  Thickness arrowMargins = Thickness::cZero;

  switch (mSide)
  {
  case IndicatorSide::Right:
  {
    contentMargins.Left += arrowSize;
    arrowMargins.Right = GetSize().x - arrowSize;
    arrowRotation = Math::cPi;
    arrowAdjustments.x += Pixels(1);
    break;
  }
  case IndicatorSide::Left:
  {
    contentMargins.Right += arrowSize;
    arrowMargins.Left = GetSize().x - arrowSize;
    arrowRotation = 0;
    arrowAdjustments.x -= Pixels(1);
    break;
  }
  case IndicatorSide::Bottom:
  {
    contentMargins.Top += arrowSize;
    arrowMargins.Bottom = GetSize().y - arrowSize;
    arrowRotation = -Math::cPi * 0.5f;
    arrowAdjustments.y += Pixels(1);
    break;
  }
  case IndicatorSide::Top:
  {
    contentMargins.Bottom += arrowSize;
    arrowMargins.Top = GetSize().y - arrowSize;
    arrowRotation = Math::cPi * 0.5f;
    arrowAdjustments.y -= Pixels(1);
    break;
  }
  }

  // Update the areas
  contentArea.RemoveThickness(contentMargins);
  arrowArea.RemoveThickness(arrowMargins);

  mArrow->SetRotation(arrowRotation);
  mArrowBorder->SetRotation(arrowRotation);

  // Set the background and border
  PlaceWithRect(contentArea, mBackground);
  PlaceWithRect(contentArea, mBorder);

  // Shrink the content area by the margins and place the content
  if (mContent)
  {
    contentArea.RemoveThickness(mContentPadding);
    PlaceWithRect(contentArea, mContent);
  }

  // Center the arrows inside the rect
  PlaceCenterToRect(arrowArea, mArrow, arrowAdjustments + mArrowOffset);
  PlaceCenterToRect(arrowArea, mArrowBorder, arrowAdjustments + mArrowOffset);

  Composite::UpdateTransform();
}

//******************************************************************************
void ToolTip::SizeToContents()
{
  // Calculate the size from the arrows location
  Vec2 arrowSize = Vec2::cZero;
  switch (mSide)
  {
  case IndicatorSide::Right:
  case IndicatorSide::Left:
    arrowSize.x = mArrow->GetSize().x;
    break;
  case IndicatorSide::Bottom:
  case IndicatorSide::Top:
    arrowSize.y = mArrow->GetSize().x;
    break;
  }

  // The size of the content
  Vec2 contentSize = Vec2::cZero;
  if (mContent)
    contentSize = mContent->GetMinSize();

  // The accumulated margins
  Vec2 padding(mContentPadding.Left + mContentPadding.Right,
    mContentPadding.Top + mContentPadding.Bottom);

  mSize = arrowSize + contentSize + padding;

  // Must be large enough for at least the arrow to fit
  float arrowWidth = mArrow->GetSize().y;
  mSize = Math::Max(mSize, Vec2(arrowWidth, arrowWidth));

  // Make sure the height has an even amount of pixels so that the 
  // arrow can be perfectly centered
  if (((int)mSize.y % 2) != 0)
    mSize.y += Pixels(1);
}

//******************************************************************************
Text* ToolTip::SetText(StringParam text)
{
  // Create the text object
  Text* textObject = new Text(this, cText);
  textObject->SetText(text);

  textObject->SetMultiLine(true);
  textObject->FitToWidth(ToolTipUi::ToolTipWrapWidth, Pixels(1000));

  // Set it as the content
  SetContent(textObject);
  return textObject;
}

//******************************************************************************
void ToolTip::SetContent(Widget* content)
{
  // Destroy the old one if it exists
  SafeDestroy(mContent);

  // Attach it to ourself
  AttachChildWidget(content);
  mContent = content;

  // Update the size based on the new content
  SizeToContents();

  UpdateTransform();
  MarkAsNeedsUpdate();
}

//******************************************************************************
bool ToolTip::SetArrowTipTranslation(Vec3Param screenPos)
{
  // Make sure we're properly sized
  SizeToContents();

  Vec3 position = screenPos;

  // Attempt to center the arrow in the center of the tool tip
  if (mSide == IndicatorSide::Right || mSide == IndicatorSide::Left)
    position.y -= (GetSize().y * 0.5f);
  else
    position.x -= (GetSize().x * 0.5f);
  if (mSide == IndicatorSide::Left)
    position.x -= GetSize().x;
  else if (mSide == IndicatorSide::Top)
    position.y -= GetSize().y;

  // In case we are unable to fit the tooltip on the screen (comes next), 
  // place it here as a default case
  SetTranslation(position);

  mArrowOffset = Vec2::cZero;

  // Now we want to check if we're off the screen. If we are, attempt to
  // shift the tooltip on the screen
  Vec2 screenSize = GetParent()->GetSize();
  Vec2 thisSize = GetSize();
  float arrowSize = mArrow->GetSize().y;

  // Left check
  if (position.x < 0)
  {
    // Cannot shift if the arrow is on the left or right
    if (mSide == IndicatorSide::Left || mSide == IndicatorSide::Right)
      return false;

    // Shift back onto the screen
    float distanceOffScreen = Math::Abs(position.x);
    position.x += distanceOffScreen;
    mArrowOffset = Vec2(-distanceOffScreen, 0);
  }
  // Right check
  else if ((position.x + thisSize.x) > screenSize.x)
  {
    // Cannot shift if the arrow is on the left or right
    if (mSide == IndicatorSide::Left || mSide == IndicatorSide::Right)
      return false;

    // Shift back onto the screen
    float distanceOffScreen = (position.x + thisSize.x) - screenSize.x;
    position.x -= distanceOffScreen;
    mArrowOffset = Vec2(distanceOffScreen, 0);
  }
  // Top check
  else if (position.y < 0)
  {
    // Cannot shift if the arrow is on the top or bottom
    if (mSide == IndicatorSide::Bottom || mSide == IndicatorSide::Top)
      return false;

    // Shift back onto the screen
    float distanceOffScreen = Math::Abs(position.y);
    position.y += distanceOffScreen;
    mArrowOffset = Vec2(0, -distanceOffScreen);
  }
  // Bottom check
  else if ((position.y + thisSize.y) > screenSize.y)
  {
    // Cannot shift if the arrow is on the top or bottom
    if (mSide == IndicatorSide::Bottom || mSide == IndicatorSide::Top)
      return false;

    // Shift back onto the screen
    float distanceOffScreen = (position.y + thisSize.y) - screenSize.y;
    position.y -= distanceOffScreen;
    mArrowOffset = Vec2(0, distanceOffScreen);
  }

  // Set the final translation
  SetTranslation(position);

  return true;
}

//******************************************************************************
void ToolTip::SetArrowTipTranslation(ToolTipPlacement& placement)//const Rect& rect, Vec2Param hotSpot,
                        //const Thickness& margins, IndicatorSide::Type priority[4])
{
  Rect& rect = placement.mScreenRect;

  // Attempt to place the tool tip on each side of the rect based on the priority
  for (uint i = 0; i < 4; ++i)
  {
    // The current side
    IndicatorSide::Type arrowSide = placement.mPriority[i];
    mSide = arrowSide;

    // Start the position at the hotspot and then shift it based on the side
    Vec2 position = placement.mHotSpot;
    switch (arrowSide)
    {
    case IndicatorSide::Right:
      position.x = rect.X + rect.SizeX;
      break;
    case IndicatorSide::Left:
      position.x = rect.X;
      break;
    case IndicatorSide::Bottom:
      position.y = rect.Y + rect.SizeY;
      break;
    case IndicatorSide::Top:
      position.y = rect.Y;
      break;
    }

    // Try to place the tooltip at this position
    bool success = SetArrowTipTranslation(ToVector3(position));

    // If it fit there, we're done
    if (success)
      break;
  }
}

//******************************************************************************
void ToolTip::SetDestroyOnMouseExit(bool state)
{
  mDestroyOnMouseExitSource = state;
}

//******************************************************************************
void ToolTip::SetColor(ToolTipColor::Enum color)
{
  if(color == ToolTipColor::Default)
  {
    mBackgroundColor = FloatColorRGBA(42, 90, 120, 255);
    mBorderColor = FloatColorRGBA(63, 135, 180, 255);
  }
  else if (color == ToolTipColor::Gray)
  {
    mBackgroundColor = ToolTipUi::BackgroundColor;
    mBorderColor = ToolTipUi::BorderColor;
  }
  else if (color == ToolTipColor::Red)
  {
    mBackgroundColor = FloatColorRGBA(120, 42, 44, 255);
    mBorderColor = FloatColorRGBA(180, 63, 66, 255);
  }
  else if (color == ToolTipColor::Yellow)
  {
    mBackgroundColor = FloatColorRGBA(135, 110, 43, 255);
    mBorderColor = FloatColorRGBA(195, 165, 65, 255);
  }
  else if (color == ToolTipColor::Green)
  {
    mBackgroundColor = FloatColorRGBA(49, 113, 48, 255);
    mBorderColor = FloatColorRGBA(74, 170, 72, 255);
  }
  else if (color == ToolTipColor::Orange)
  {
    mBackgroundColor = FloatColorRGBA(128, 69, 34, 255);
    mBorderColor = FloatColorRGBA(192, 104, 51, 255);
  }
  MarkAsNeedsUpdate();
}

//******************************************************************************
void ToolTip::OnMouseUpdate(MouseEvent* event)
{
  // The source widget has been destroyed, destroy ourself
  Widget* source = mSource;
  if(source == nullptr)
  {
    this->Destroy();
    return;
  }

  if(mDestroyOnMouseExitSource)
  {
    // If our mouse moves onto an object that's not a child of the
    // source, then destroy ourself
    bool notChildOf = !source->IsAncestorOf(event->Source);
    if(notChildOf)
      this->Destroy();
  }
}

}//namespace Zero
