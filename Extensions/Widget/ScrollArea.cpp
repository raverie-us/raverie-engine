///////////////////////////////////////////////////////////////////////////////
///
/// \file ScrollArea.cpp
/// Implementation of the ScrollArea.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

const cstr cLocation = "EditorUi/Controls/ScrollBar";
namespace ScrollBarUi
{
Tweakable(Vec4,  SliderColor,   Vec4(1,1,1,1), cLocation);
Tweakable(Vec4,  MouseOverSlider,   Vec4(1,1,1,1), cLocation);
Tweakable(float, Width,   Pixels(6),     cLocation);
Tweakable(float, Spacing, Pixels(2),     cLocation);
}

namespace Events
{
  DefineEvent(ScrollUpdated);
}

//--------------------------------------------------------- File-Scope Functions
void UpdateVisible(Vec2& size, Vec2Param clientSize, Vec2Param sliderSize, 
                   ScrollBar** bar)
{
  for(uint i = 0; i < 2; ++i)
  {
    if(size[i] < sliderSize[i] * 2)
    {
      bar[i]->mVisible = false;
    }
    else if(!bar[i]->mVisible && !bar[i]->mDoNotShow && size[i] < clientSize[i])
    {
      bar[i]->mVisible = true;
      size[!i] -= sliderSize[!i];
    }
  }
}

//---------------------------------------------------------- Scroll Manipulation
struct ScrollManipulation : public MouseManipulation
{
  BaseScrollArea* mScrollArea;
  Vec2 mScrollStart;
  uint mOrientation;

  ScrollManipulation(uint orientation, Mouse* mouse, Composite* owner, 
                     BaseScrollArea* scrollArea)
    : MouseManipulation(mouse, owner)
  {
    mOrientation = orientation;
    mScrollArea = scrollArea;
    mScrollStart = scrollArea->GetScrolledPercentage();
  }

  void OnMouseUp(MouseEvent* event) override
  {
    mScrollArea->TryTakeFocus();
    this->Destroy();
  }

  void OnMouseMove(MouseEvent* event) override
  {
    //Update the scroll bars on the object
    Vec2 mouseDelta = event->Position - mMouseStartPosition;

    //Divide by the size of the of the area can scroll in (the scroll area)
    Vec2 scrollRoom = mScrollArea->SliderRoom();
    scrollRoom.x = Math::Max(0.01f, scrollRoom.x);
    scrollRoom.y = Math::Max(0.01f, scrollRoom.y);

    Vec2 scrollDelta = mouseDelta / scrollRoom;
    Vec2 scrollPos = mScrollStart;
    scrollPos[mOrientation] = mScrollStart[mOrientation] + 
                              scrollDelta[mOrientation];

    mScrollArea->SetScrolledPercentageInternal(scrollPos, ScrollUpdate::ScrollBar, true);
  }
};

//------------------------------------------------------------------------------
const String cScrollClientAreaName = "ScrollClientArea";
const String cScrollClipAreaName = "ScrollClipArea";
const String cScrollUiName = "ScrollUi";

const String cScrollerButton = "ButtonBackground";
const String cScrollerButtonOver = "ButtonBackgroundOver";

const float cScrollSensitivity = 32.0f;

//------------------------------------------------------------------- Scroll Bar
ScrollBar::ScrollBar(BaseScrollArea* scrollparent, uint orientation)
  : Composite(scrollparent, AttachType::Direct)
{
  static const String className = "ScrollBar";
  mDefSet = scrollparent->GetDefinitionSet()->GetDefinitionSet(className);
  mScrollParent = scrollparent;
  mOrientation = orientation;

  mBackground = CreateAttached<Element>(cWhiteSquare);
  mUp = CreateAttached<Element>(cScrollerButton);
  mDown = CreateAttached<Element>(cScrollerButton);
  mSlider = CreateAttached<Element>(cWhiteSquare);
  mBackground->SetActive(false);
  mUp->SetActive(false);
  mDown->SetActive(false);
  mSliderVisible = false;
  mVisible = false;
  mDoNotShow = false;

  mSlider->SetColor((Vec4)ScrollBarUi::SliderColor);
  if (mScrollParent->mModernStyle)
  {
    mSlider->SetColor(Vec4(1, 1, 1, 0.45f));
    mBackground->SetColor(Vec4(0, 0, 0, 0.4f));
    mBackground->SetActive(true);
  }

  ConnectThisTo(mSlider, Events::LeftMouseDown, MouseDownSlider);
  ConnectThisTo(mSlider, Events::MouseEnter, MouseEnterSlider);
  ConnectThisTo(mSlider, Events::MouseExit, MouseExitSlider);

  ConnectThisTo(mUp, Events::LeftMouseDown, MouseDownUp);
  ConnectThisTo(mUp, Events::MouseEnter, MouseEnterUp);
  ConnectThisTo(mUp, Events::MouseExit, MouseExitUp);

  ConnectThisTo(mDown, Events::LeftMouseDown, MouseDownDown);
  ConnectThisTo(mDown, Events::MouseEnter, MouseEnterDown);
  ConnectThisTo(mDown, Events::MouseExit, MouseExitDown);
}


ScrollBar::~ScrollBar()
{
  //
}

void ScrollBar::MouseDownSlider(MouseEvent* event)
{
  new ScrollManipulation(mOrientation, event->GetMouse(), mScrollParent, 
                         mScrollParent);
}

void ScrollBar::MouseEnterSlider(MouseEvent* event)
{
  if (mScrollParent->mModernStyle)
    mSlider->SetColor(Vec4(1, 1, 1, 0.55f));
  else
    mSlider->SetColor(ScrollBarUi::MouseOverSlider);
}

void ScrollBar::MouseExitSlider(MouseEvent* event)
{
  if (mScrollParent->mModernStyle)
    mSlider->SetColor(Vec4(1, 1, 1, 0.45f));
  else
    mSlider->SetColor(ScrollBarUi::SliderColor);
}

void ScrollBar::MouseDownUp(MouseEvent* event)
{
  Vec2 percentage = mScrollParent->GetScrolledPercentage();
  percentage[mOrientation] -= 0.10f;
  mScrollParent->SetScrolledPercentage(percentage);
}

void ScrollBar::MouseEnterUp(MouseEvent* event)
{
  mUp->ChangeDefinition(mDefSet->GetDefinition(cScrollerButtonOver));
}

void ScrollBar::MouseExitUp(MouseEvent* event)
{
  mUp->ChangeDefinition(mDefSet->GetDefinition(cScrollerButton));
}

void ScrollBar::MouseDownDown(MouseEvent* event)
{
  Vec2 percentage = mScrollParent->GetScrolledPercentage();
  percentage[mOrientation] += 0.10f;
  mScrollParent->SetScrolledPercentage(percentage);
}

void ScrollBar::MouseEnterDown(MouseEvent* event)
{
  mDown->ChangeDefinition(mDefSet->GetDefinition(cScrollerButtonOver));
}

void ScrollBar::MouseExitDown(MouseEvent* event)
{
  mDown->ChangeDefinition(mDefSet->GetDefinition(cScrollerButton));
}

//------------------------------------------------------------- Base Scroll Area
ZilchDefineType(BaseScrollArea, builder, type)
{
}

BaseScrollArea::BaseScrollArea(Composite* parent, bool modernStyle)
  : Composite(parent)
  , mModernStyle(modernStyle)
{
  mDefSet = parent->GetDefinitionSet();
  mSize = Vec2(100, 100);
  mVisibleSize = mSize;
  mClientOffset = Vec2(0, 0);
  mAnimatingToClientOffset = mClientOffset;
  mScrollSpeedScalar = 1.0f;

  for(uint i = 0; i < 2; ++i)
    mScrollBar[i] = new ScrollBar(this, i);
}

BaseScrollArea::~BaseScrollArea()
{
  //
}

void BaseScrollArea::DisableScrollBar(uint index)
{
  mScrollBar[index]->mDoNotShow = true;
}

void BaseScrollArea::OnMouseScroll(MouseEvent* event)
{
  if(!event->CtrlPressed)
  {
    // default scroll wheel behavior is to scroll up and down
    if(this->IsScrollBarVisible(1) && !event->Handled && !event->ShiftPressed)
    {
      event->Handled = true;
      Vec2 offset = mClientOffset;
      float offsetValue = event->Scroll.y * cScrollSensitivity * mScrollSpeedScalar;
      offset.y += offsetValue;
      this->SetScrolledOffsetInternal(offset, ScrollUpdate::Auto, true);
    }
    // when holding shift scroll horizontally instead
    if (this->IsScrollBarVisible(0) && !event->Handled && event->ShiftPressed)
    {
      event->Handled = true;
      Vec2 offset = mClientOffset;
      float offsetValue = event->Scroll.y * cScrollSensitivity * mScrollSpeedScalar;
      offset.x += offsetValue;
      this->SetScrolledOffsetInternal(offset, ScrollUpdate::Auto, true);
    }
  }
}

void BaseScrollArea::UpdateScrollBars()
{
  Vec2 newSize = mSize;
  Vec2 newVisibleSize = mSize;
  Vec2 scrollBarMinSize = Pixels(10,10);
  Vec2 scrollButtonSize = Vec2(ScrollBarUi::Width, ScrollBarUi::Width);
  if (mModernStyle)
    scrollButtonSize = Vec2(7.0f);

  mScrollBar[0]->mVisible = false;
  mScrollBar[1]->mVisible = false;
  mScrollBar[0]->mSliderVisible = true;
  mScrollBar[1]->mSliderVisible = true;

  //Get the client size (implemented by derived class)
  Vec2 clientSize = this->GetClientSize();

  //Determining if scroll bars are needed requires more than one pass
  UpdateVisible(newVisibleSize, clientSize, scrollButtonSize, mScrollBar);
  //Intentional second call (not a bug)
  UpdateVisible(newVisibleSize, clientSize, scrollButtonSize, mScrollBar);

  Vec2 totalScrollSize = mSize - scrollButtonSize * 2;

  //The vertical scroll bar shrinks when both are visible
  if(mScrollBar[1]->mVisible && mScrollBar[0]->mVisible)
    totalScrollSize.x = totalScrollSize.x - scrollButtonSize.x;

  clientSize.x = Math::Max(Pixels(1), clientSize.x);
  clientSize.y = Math::Max(Pixels(1), clientSize.y);

  Vec2 percentageVisible = newVisibleSize / clientSize;
  Vec2 scollerSize = percentageVisible * totalScrollSize;

  for(uint i = 0; i < 2; ++i)
  {
    //scroll area is very small clamp to limit
    if(scollerSize[i] < scrollBarMinSize[i])
      scollerSize[i] = scrollBarMinSize[i];

    //If there is not enough room to display the slider, disable it
    if(scollerSize[i] > totalScrollSize[i])
      mScrollBar[i]->mSliderVisible = false;
  }

  //Left over area the scroll moves in.
  Vec2 scrollRoom = totalScrollSize - scollerSize;
  mScrollSlideRoom = totalScrollSize - scollerSize;

  Vec2 scrolledPercentage = GetScrolledPercentage();
  mVisibleSize = newVisibleSize;

  // Size may have changed so clamp the offset
  mClientOffset = ClampOffset(mClientOffset);

  for(uint i = 0; i < 2; ++i)
  {
    ScrollBar& bar = *mScrollBar[i];

    if(mScrollBar[i]->mVisible)
    {
      uint oi = !i;

      Vec3 bgPos = Vec3::cZero;
      bgPos[oi] = newSize[oi] - scrollButtonSize[oi];

      if (mModernStyle)
        bgPos[oi] -= Pixels(6);

      //Position the up button
      bar.mUp->SetVisible(true);
      bar.mUp->SetTranslation(bgPos);
      bar.mUp->SetSize(scrollButtonSize);

      //Position the slider background
      bar.mBackground->SetVisible(true);
      bgPos[i] = scrollButtonSize[i];
      bar.mBackground->SetTranslation(bgPos);

      //Size the slider background
      Vec2 backSize;
      backSize[i] = totalScrollSize[i];
      backSize[oi] = scrollButtonSize[oi];

      bar.mBackground->SetSize(backSize);

      //Position the down button
      bgPos[i] = scrollButtonSize[i] + totalScrollSize[i];
      bar.mDown->SetVisible(true);
      bar.mDown->SetTranslation(bgPos);
      bar.mDown->SetSize(scrollButtonSize);

      if(bar.mSliderVisible)
      {
        //Set up the slider for this orientation
        bar.mSlider->SetVisible(true);

        //Slider position
        Vec3 sliderPos = Vec3::cZero;
        sliderPos[oi] = newSize[oi] - scrollButtonSize[oi];

        if (mModernStyle)
          sliderPos[oi] -= Pixels(6);

        sliderPos[i] = scrollButtonSize[i] + scrolledPercentage[i] * scrollRoom[i];
        bar.mSlider->SetTranslation(sliderPos);

        //Slider size
        Vec2 sliderSize;
        sliderSize[i] = scollerSize[i];
        sliderSize[oi] = scrollButtonSize[oi];
        bar.mSlider->SetSize(sliderSize);
      }
      else
      {
        //disable the slider for this orientation
        bar.mSlider->SetVisible(false);
      }
    }
    else
    {
      bar.mSlider->SetVisible(false);
      bar.mUp->SetVisible(false);
      bar.mDown->SetVisible(false);
      bar.mBackground->SetVisible(false);
    }
  }
}

void BaseScrollArea::UpdateTransform()
{
  Composite::UpdateTransform();
}

Vec2 BaseScrollArea::GetScrolledPercentage()
{
  //Determine the scroll position by dividing the area
  //by dividing offset by the area.
  Vec2 clientSize = this->GetClientSize();
  Vec2 scollingArea = clientSize - mVisibleSize;

  //prevent divide by zero
  if(scollingArea.x == 0.0f)
    scollingArea.x = 1.0f;

  if(scollingArea.y == 0.0f)
    scollingArea.y = 1.0f;

  return -(mClientOffset / scollingArea);
}

void BaseScrollArea::SetScrolledPercentage(Vec2 scrollPercentage)
{
  SetScrolledPercentageInternal(scrollPercentage, ScrollUpdate::External, true);
}

void BaseScrollArea::SetScrolledOffset(Vec2 scrollOffset, bool animate)
{
  SetScrolledOffsetInternal(-scrollOffset, ScrollUpdate::External, true, animate);
}

Vec2 BaseScrollArea::GetScrolledOffset()
{
  return -mClientOffset;
}

void BaseScrollArea::ScrollAreaToView(Vec2 min, Vec2 max, bool animate)
{
  Vec2 scrollTop = GetScrolledOffset();
  Vec2 viewBottom = scrollTop + GetClientVisibleSize();

  for(uint i=0;i<2;++i)
  {
    if(IsScrollBarVisible(i))
    {
      if (max[i] > viewBottom[i])
        scrollTop[i] += max[i] - viewBottom[i];
      else if (min[i] < scrollTop[i])
        scrollTop[i] += min[i] - scrollTop[i];
    }
  }

  SetScrolledOffset(scrollTop, animate);
  MarkAsNeedsUpdate();
}

Vec4 BaseScrollArea::GetClientArea()
{
  Vec2 pos = -mClientOffset;
  Vec2 visibleSize = GetClientVisibleSize();
  return Vec4(pos.x, pos.y, pos.x + visibleSize.x, pos.y + visibleSize.y);
}

void BaseScrollArea::ScrollPercent(Vec2 additivePercentage)
{
  Vec2 pos = GetScrolledPercentage();
  SetScrolledPercentage(pos + additivePercentage);
}

void BaseScrollArea::ScrollPixels(Vec2 additivePixels)
{
  SetScrolledOffsetInternal(mClientOffset + additivePixels,
                            ScrollUpdate::External, true);
}

Vec2 BaseScrollArea::ClampOffset(Vec2Param offset)
{
  Vec2 clientOffset = offset;
  Vec2 clientSize = GetClientSize();
  Vec2 scollingArea = clientSize - mVisibleSize;

  // If scroll area is negative there is already enough room
  scollingArea.x = Math::Max(scollingArea.x, 0.0f);
  scollingArea.y = Math::Max(scollingArea.y, 0.0f);

  // Limit client offset from 0 to negative scroll area 
  clientOffset.x = Math::Clamp(clientOffset.x, -scollingArea.x, 0.0f);
  clientOffset.y = Math::Clamp(clientOffset.y, -scollingArea.y, 0.0f);

  return clientOffset;
}

void BaseScrollArea::SetScrolledPercentageInternal(Vec2 scrollPercentage, 
                                                 ScrollUpdate::Enum updateType,
                                                 bool generateMessages)
{
  // Clamp scroll to valid range.
  scrollPercentage.x = Math::Clamp(scrollPercentage.x, 0.0f, 1.0f);
  scrollPercentage.y = Math::Clamp(scrollPercentage.y, 0.0f, 1.0f);

  // Call only scroll the extra area
  Vec2 clientSize = this->GetClientSize();
  Vec2 scollingArea = clientSize - mVisibleSize;

  // Determine the client offset
  // by scaling by the area by the percentage offset
  Vec2 clientOffset = SnapToPixels(-scrollPercentage * scollingArea);

  SetScrolledOffsetInternal(clientOffset, updateType, generateMessages);
}

void BaseScrollArea::SetScrolledOffsetInternal(Vec2Param clientOffset, ScrollUpdate::Enum updateType,
                                               bool generateMessages, bool animate)
{
  if(generateMessages)
    MarkAsNeedsUpdate();

  Vec2 clampedOffset = ClampOffset(clientOffset);

  mClientOffset = mAnimatingToClientOffset;
  mAnimatingToClientOffset = clampedOffset;

  if(animate)
  {
    GetActions()->Cancel();
    ActionSequence* seq = new ActionSequence();
    seq->Add(AnimatePropertyGetSet(BaseScrollArea, ClientOffset, Ease::Quad::Out, this, 0.2f, clampedOffset));
    GetActions()->Add(seq, ActionExecuteMode::FrameUpdate);
  }
  else
  {
    mClientOffset = clampedOffset;
  }

  if(generateMessages)
  {
    ObjectEvent e(this);
    DispatchEvent(Events::ScrollUpdated, &e);
  }
  // Inform derived classes
  UpdateArea(updateType);
}

//------------------------------------------------------------------ Scroll Area
ZilchDefineType(ScrollArea, builder, type)
{
}

const String BackgroundArea = "ScrollBackground";

ScrollArea::ScrollArea(Composite* composite, bool modernStyle)
  : BaseScrollArea(composite, modernStyle)
{
  // Create the widget the will clip the attached client area widget
  mClipWidget = new Composite(this, AttachType::Direct);
  mClipWidget->SetName(cScrollClipAreaName);
  mClipWidget->SetClipping(true);

  // Move this behind the scroll bars
  mClipWidget->MoveToBack();

  // Create the widget that all child widget will be attached to
  // this will translation within the clip widget for scrolling
  mClientArea = new Composite(mClipWidget, AttachType::Direct);
  mClientArea->SetName(cScrollClientAreaName);

  // Create a background on the client area for mouse events
  // such as mouse wheel and clicking
  mBackground = new Spacer(this);
  mBackground->SetNotInLayout(true);
  mBackground->SetName("ScrollAreaBackground");

  // Any scroll in the scroll area (including the scroll bars)
  Connect(this, Events::MouseScroll, (BaseScrollArea*)this, &BaseScrollArea::OnMouseScroll);
}

Composite* ScrollArea::GetClientWidget()
{
  return mClientArea;
}

Vec2 ScrollArea::GetClientSize()
{
  return mClientArea->GetSize();
}

void ScrollArea::SetClientSize(Vec2 newSize)
{
  mClientArea->SetSize(newSize);
  UpdateScrollBars();
}

void ScrollArea::UpdateTransform()
{
  UpdateScrollBars();

  // Update for clipping
  if (mModernStyle)
    mClipWidget->SetSize(mSize);
  else
    mClipWidget->SetSize(mVisibleSize);

  /// move the client area to the client offset for scrolling
  mClientArea->SetTranslation(Vec3(mClientOffset));

  // Background for clicks
  // It is larger than the client area because the scroll area
  // may be expanded larger than the needed client area
  mBackground->SetSize(Math::Max(mVisibleSize, mClientArea->GetSize()));

  BaseScrollArea::UpdateTransform();
}

void ScrollArea::AttachChildWidget(Widget* widget, AttachType::Enum attachType)
{
  if(attachType == AttachType::Direct)
    Composite::AttachChildWidget(widget);
  else
    mClientArea->AttachChildWidget(widget);
}

bool ScrollArea::TakeFocusOverride()
{
  return mBackground->TryTakeFocus();
}

void ScrollArea::UpdateArea(ScrollUpdate::Enum updateType)
{
  Vec2 clientSize = mClientArea->GetSize();
  // Update the client's transform so it visually moves.
  mClientArea->SetTranslationAndSize(Vec3(mClientOffset), clientSize);
}

float ScrollArea::GetScrollBarSize()
{
  if (mModernStyle)
    return Pixels(7.0f);
  return ScrollBarUi::Width;
}

}//namespace Zero
