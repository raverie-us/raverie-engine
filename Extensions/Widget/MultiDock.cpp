///////////////////////////////////////////////////////////////////////////////
///
/// \file MultiDock.cpp
/// Implementation of the Multi Dock classes.
///
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//----------------------------------------------------------------- Docking Area
DockingArea::DockingArea()
{
  DockSize = Vec2(0,0);
  FlexRatio = 0.0f;
  DockShell = 0;
  Area = DockArea::Center;
}

typedef Array<Widget*> WidgetArray;
typedef WidgetArray::range WidgetRange;

DockArea::Enum ToDockAreaDirection(DockMode::Enum dockArea)
{
  switch(dockArea)
  {
  case DockMode::DockLeft: return DockArea::Left;
  case DockMode::DockRight: return DockArea::Right;
  case DockMode::DockTop: return DockArea::Top;
  case DockMode::DockBottom: return DockArea::Bottom;
  case DockMode::DockFill: return DockArea::Center;
  default: return DockArea::Center;
  }
}

// Get the next docking area from the current docking area in the direction provided
// NULL if no docking area in that direction
DockingArea* NextInDirection(MultiDock* multiDock, DockingArea* current, DockArea::Enum direction)
{
  DockArea::Enum currentArea = current->Area;
  DockArea::Enum testArea = currentArea;
  int currentShell = current->DockShell;
  for(;;)
  {
    int shellDirection = GetSign(currentArea) * GetSign(direction);

    // Shell 0 is where the directions can flip and the 
    // center needs to be checked
    if(currentShell == 0)
    {
      // If on center go to the side area
      // in the direction of movement but stay in shell 0
      if(currentArea == DockArea::Center)
      {
        shellDirection = 0;
        testArea = direction;
      }
      else
      {
        // If the signs are different move from side to center
        if(GetSign(currentArea) != GetSign(direction))
        {
          shellDirection = 0;
          testArea = DockArea::Center;
        }
        else
        {
          // From side area out of center
          // Normal movement
        }
      }
    }

    // Get the next shell
    int testShell = currentShell + shellDirection;

    // Ran out of shells no areas in direction
    if( testShell >= int(multiDock->DockingShells.Size()))
      return nullptr;

    // If the area is not empty return else keep searching
    if(!multiDock->DockingShells[testShell].DockAreas[testArea].Widgets.Empty())
      return &multiDock->DockingShells[testShell].DockAreas[testArea];

    // Move to next shell / area
    currentShell = testShell;
    currentArea = testArea;
  }
}


//----------------------------------------------------- Moving Dock Manipulation

// Hint area for docking
class HintRegion : public Composite
{
public:
  Element* mArea;

  HintRegion(Composite* parent)
    :Composite(parent)
  {
    static const String className = "ListBox";
    mDefSet = mDefSet->GetDefinitionSet(className);
    mArea = CreateAttached<Element>(cSelectionHighlight);
    mArea->SetInteractive(false);
    mArea->SetColor(Vec4(1,1,1,0.5f));
  }

  void UpdateTransform() override
  {
    mArea->SetSize(mSize);
    Composite::UpdateTransform();
  }
};


DeclareEnum4(DockHover, None, DockArea, Overlay, Tab);

/// MouseManipulation for moving docked windows around
/// docking with dock areas or edges.
class DockMovingManipulation : public MouseManipulation
{
public:
  DockMovingManipulation(Mouse* mouse, MultiDock* owner, Widget* target);
  ~DockMovingManipulation();

  MultiDock* mMultiDock;

  // Widget being moved
  HandleOf<Widget> mDockMoveTarget;
  
  HandleOf<HintRegion> mDockHint;
  HandleOf<TabArea> mHoverTabs;
  HandleOf<Widget> mHoverWidget;
  DockHover::Enum mDockHover;
  DockArea::Enum mDockDirection;

  Vec3 mTargetStartPosition;
  Vec2 mTargetStartSize;
  Vec2 mGrabOffset;

  // MouseManipulation Interface
  void OnMouseUpdate(MouseEvent* event) override;
  void OnMouseUp(MouseEvent* event) override;
  void OnMouseDown(MouseEvent* event) override {};
  void OnMouseMove(MouseEvent* event) override;
  void ChangeHover(DockHover::Enum newHover);
  void CleanUp();
  HintRegion* GetHint();
};

DockMovingManipulation::DockMovingManipulation(Mouse* mouse, MultiDock* dockOwner, Widget* target)
  : MouseManipulation(mouse, dockOwner)
{
  mTargetStartPosition = target->GetTranslation();
  mTargetStartSize = target->GetSize();
  mGrabOffset = mouse->GetClientPosition() - ToVector2(mTargetStartPosition);
  
  MultiDocker* docker = (MultiDocker*)target->GetDocker();
  target->SetDockArea(DockArea::Floating);

  mDockMoveTarget = target;
  mMultiDock = dockOwner;

  mDockHover = DockHover::None;
  mDockDirection = DockArea::Floating;
}

DockMovingManipulation::~DockMovingManipulation()
{
  ChangeHover(DockHover::None);
  mMouse->SetCursor(Cursor::Arrow);
}

HintRegion* DockMovingManipulation::GetHint()
{
  HintRegion* region = mDockHint;
  if(region == nullptr)
  {
    // Create the hint region
    region = new HintRegion(mMultiDock);
    region->SetColor(Vec4(1,1,1,0.0f));
    region->SetInteractive(false);

    ActionSequence* seq = new ActionSequence(region);
    seq->Add(Fade(region, Vec4(1,1,1,0.8f), 0.2f));

    mDockHint = region;
  }

  return region;
}

void DockMovingManipulation::CleanUp()
{
  // Stop processing mouse moves
  mDockMoveTarget = NULL;
  ChangeHover(DockHover::None);
  mMouse->SetCursor(Cursor::Arrow);
  this->Destroy();
}

void TransferTabs(TabArea* tabArea, Window* window)
{
  //Transfer all tabs
  while(window->mTabArea->mTabs.Size())
  {
    tabArea->TransferTab(window->mTabArea->mTabs.Front(), tabArea->mPreviewTab, true);
    tabArea->mPreviewTab = tabArea->mPreviewTab + 1;
  }
  tabArea->mPreviewTab = -1;
}

void DockMovingManipulation::OnMouseUp(MouseEvent* event)
{
  Widget* dockTarget = mDockMoveTarget;
  if(dockTarget == nullptr)
    return CleanUp();

  Window* window = (Window*)dockTarget;
  MultiDocker* windowDocker = (MultiDocker*)window->GetDocker();
  MultiDock* multiDock = windowDocker->DockOwner;

  if(mDockHover == DockHover::Tab)
  {
    // Drop onto tab area
    TabArea* tabArea = mHoverTabs;
    if(tabArea == nullptr)
      return CleanUp();

    TransferTabs(tabArea, window);
  }
  else if(mDockHover == DockHover::Overlay)
  {
    if(mDockDirection == DockArea::Center)
    {
      Window* centerWindow = multiDock->CenterWindow;
      if(centerWindow)
        TransferTabs(centerWindow->mTabArea, window);
      else
        window->SetDockArea(DockArea::Center);
    }
    else
    {
      // Drop on screen edge move to outermost shell
      window->SetDockArea(mDockDirection);
      windowDocker->DockShell = multiDock->DockingShells.Size();
    }
  }
  else if(mDockHover == DockHover::DockArea)
  {
    Widget* hoverTarget = mHoverWidget;
    if(hoverTarget == nullptr)
      return CleanUp();

    // Dock onto a window
    MultiDocker* hoverDocker = (MultiDocker*)hoverTarget->GetDocker();
    DockingArea* hoverArea = &mMultiDock->DockingShells[hoverDocker->DockShell].DockAreas[hoverDocker->Area];
    DockArea::Enum hoverAreaEnum = hoverDocker->Area;

    if(hoverAreaEnum == DockArea::Center)
    {
      // The center window docks control in shell 0    
      // Move out the widgets in the current shell 0 in 
      // that direction
      for(uint i=0;i<DockArea::Count;++i)
      {
        forRange(Widget* widget, mMultiDock->DockingShells[0].DockAreas[i].Widgets.All())
        {
          MultiDocker* docker = (MultiDocker*)widget->GetDocker();
          if(docker->Area == mDockDirection)
            ++docker->DockShell;
        }
      }

      // Take the shell 0 area
      window->SetDockArea(mDockDirection);
      windowDocker->DockIndex = 0;
      windowDocker->DockShell = 0;
    }
    else 
    {
      //Double all dock indexes so Insert can just +1 or -1
      for(uint i=0;i<hoverArea->Widgets.Size();++i)
      {
        Widget* widget = hoverArea->Widgets[i];
        MultiDocker* docker = (MultiDocker*)widget->GetDocker();
        docker->DockIndex = docker->DockIndex * 2;
      }

      // Place it in the dock area
      int sign = GetSign(mDockDirection);
      window->SetDockArea(hoverAreaEnum);
      windowDocker->DockIndex = hoverDocker->DockIndex + sign;
      windowDocker->DockShell = hoverDocker->DockShell;
    } 
  }

  return CleanUp();
}

void DockMovingManipulation::ChangeHover(DockHover::Enum newHover)
{
  //If the hover mode has changed
  if(newHover != mDockHover)
  {
    //Clean up old hint
    mDockHint.SafeDestroy();
    mDockDirection = DockArea::Floating;

    if(TabArea* tabArea = mHoverTabs)
    {
      tabArea->mPreviewTab = -1;
      tabArea->MarkAsNeedsUpdate();
    }
  }

  mDockHover = newHover;
}

bool NearEdge(uint axis, Vec2 testPosition, Vec2 areaSize, float distance, DockArea::Enum& direction)
{
  if(testPosition[axis] < distance)
  {
    direction = axis ? DockArea::Top : DockArea::Left;
    return true;
  }
  else if(testPosition[axis] > (areaSize[axis]-distance) )
  {
    direction = axis ? DockArea::Bottom : DockArea::Right;
    return true;
  }
  return false;
}

void DockMovingManipulation::OnMouseMove(MouseEvent* event)
{
  OnMouseUpdate(event);
}

void DockMovingManipulation::OnMouseUpdate(MouseEvent* event)
{
  Widget* dockTarget = mDockMoveTarget;

  // Lost window during manipulation
  if(dockTarget == nullptr)
  {
    this->Destroy();
    return;
  }

  MultiDocker* docker = (MultiDocker*)dockTarget->GetDocker();
  Composite* parent = dockTarget->GetParent();

  Vec2 newMousePosition = event->Position;
  Vec2 delta = newMousePosition - mMouseStartPosition;
  Vec2 parentSize = parent->GetSize();
  Vec2 dockAreaSize = mMultiDock->GetSize();

  // Translate the window being dragged
  Vec2 targetSize = dockTarget->GetSize();

  // Clamp to the keep the mouse in the tab area of the window
  const float EdgetLimitR = Pixels(10);
  const float EdgetLimitL = Pixels(10);
  float edgeSize = targetSize.x - EdgetLimitR;
  if(mGrabOffset.x > edgeSize)
    mGrabOffset.x = edgeSize;
  if(mGrabOffset.x < EdgetLimitL)
    mGrabOffset.x  = EdgetLimitL;
  
  // Clamp movement to screen
  Vec3 newObjectPos = ToVector3(newMousePosition - mGrabOffset);

  if(newObjectPos.x < 0)
    newObjectPos.x = 0;

  if(newObjectPos.y < 0)
    newObjectPos.y = 0;

  dockTarget->SetTranslation(SnapToPixels(newObjectPos));

  const Vec2 cCenterRegionSize = Pixels(400, 400);

  Rect centerRect = Rect::CenterAndSize(dockAreaSize * 0.5f, cCenterRegionSize);
  if(centerRect.Contains(newMousePosition))
  {
    ChangeHover(DockHover::Overlay);
    mDockDirection = DockArea::Center;

    // Create the edge hint if necessary
    if(!mDockHint)
    {
      // Create the hint region
      HintRegion* hintRegion = GetHint();
      hintRegion->SetSize(cCenterRegionSize);
      CenterToWindow(parent, hintRegion, false);
    }
    return;
  }

  // Detect Edges for outer shell docking
  DockArea::Enum newEdgeDirection = DockArea::Floating;
  const float edgeLimit = Pixels(5);

  NearEdge(Math::cX, newMousePosition, dockAreaSize, edgeLimit, newEdgeDirection);
  NearEdge(Math::cY, newMousePosition, dockAreaSize, edgeLimit, newEdgeDirection);

  if(newEdgeDirection != DockArea::Floating)
  {
    ChangeHover(DockHover::Overlay);
    mDockDirection = newEdgeDirection;

    // Create the edge hint if necessary
    if(!mDockHint)
    {
      // Create the hint region
      HintRegion* hintRegion = GetHint();

      mDockHint = hintRegion;
      mHoverWidget = mMultiDock;

      const float edgeHintSize = Pixels(100);

      // Size on the correct axis
      Vec2 totalSize = mMultiDock->GetSize();
      Vec2 size = totalSize;
      size[GetAxis(newEdgeDirection)] = edgeHintSize;
      hintRegion->SetSize(size);

      // For positive directions flip to other side of screen
      Vec3 position = Vec3(0,0,0);
      if( GetSign(newEdgeDirection) > 0)
        position[GetAxis(newEdgeDirection)] = totalSize[GetAxis(newEdgeDirection)] - edgeHintSize;
      hintRegion->SetTranslation(position);
    }
    return;
  }


  // Check for possible tab drops. Use DispatchAt
  // to check tabs areas or windows under the
  // mouse
  WindowTabEvent tabEvent;

  DispatchAtParams dispatchAtParams;
  dispatchAtParams.EventId = Events::TabDropTest;
  dispatchAtParams.EventObject = &tabEvent;
  dispatchAtParams.Position = mMouse->GetClientPosition();
  dispatchAtParams.Ignore = dockTarget;

  this->GetRootWidget()->DispatchAt(dispatchAtParams);

  // Above a tab area?
  if(tabEvent.TabAreaFound)
  {
    ChangeHover(DockHover::Tab);

    TabArea* tabArea = tabEvent.TabAreaFound;
    mHoverTabs = tabArea;

    // What index should this tab be inserted at?
    int index = tabArea->TabIndexAt(event);
    if(tabArea->mPreviewTab != index)
    {
      tabArea->mPreviewTab = index;
      tabArea->MarkAsNeedsUpdate();
    }

    // Update the hint Region
    HintRegion* hintRegion = GetHint();
    Vec2 tabSize = tabArea->GetTabSize();
    hintRegion->SetTranslation( tabArea->GetTabLocation(index, tabSize) + tabArea->mParentWindow->GetTranslation() );
    hintRegion->SetSize(tabSize);
    return;
  }


  //Is this tab above a window?
  if(tabEvent.WindowFound && tabEvent.WindowFound->GetDocker())
  {

    // Get the window the mouse is hovering over
    Window* hoverWindow = tabEvent.WindowFound;
    MultiDocker* hoverDocker = (MultiDocker*)hoverWindow->GetDocker();
    Vec2 hoverSize = hoverWindow->GetSize();
    Vec3 hoverTrans = hoverWindow->GetTranslation();

    DockArea::Enum newHoverDirection = DockArea::Floating;
    DockArea::Enum hoverDockArea = hoverDocker->Area;

    uint axis = !GetAxis(hoverDockArea);

    Vec2 hoverMouse = newMousePosition - ToVector2(hoverTrans);

    if(hoverDockArea == DockArea::Center)
    {
      // If it is a center dock window check the both the edges
      float limit = Math::Min(Pixels(60), hoverSize[Math::cX] * 0.2f);
      if(NearEdge(Math::cX, hoverMouse, hoverSize, limit, newHoverDirection))
        axis = Math::cX;
      else
      {
        limit = Math::Min(Pixels(60), hoverSize[Math::cY] * 0.2f);
        if(NearEdge(Math::cY, hoverMouse, hoverSize, limit, newHoverDirection))
          axis = Math::cY;
      }
    }

    // Only edge dock if is Top,Left,Right or Down
    if(hoverDockArea <= DockArea::Right)
    {
      //Other areas only dock near the edges on the same axis as the area
      NearEdge(axis, hoverMouse, hoverSize, hoverSize[axis] * 0.2f, newHoverDirection);
    }

    // Is this a valid direction to dock?
    if(newHoverDirection !=  DockArea::Floating)
    {
      ChangeHover(DockHover::DockArea);

      HintRegion* region = GetHint();

      if(region == nullptr)
        return;

      DockingArea* targetArea = &mMultiDock->DockingShells[hoverDocker->DockShell].DockAreas[hoverDocker->Area];
      int sign = GetSign(newHoverDirection);

      //Is this the last widget in dock are in this dock direction?
      bool innerArea = ( uint(hoverDocker->DockIndex + sign) < targetArea->Widgets.Size() );

      float size = Pixels(10);

      if(targetArea->FlexRatio > 0.0f)
        size = dockTarget->GetSize()[axis] * (1.0f / targetArea->FlexRatio);

      size = Math::Max(Pixels(200), size);

      Vec2 regionSize = hoverSize;
      regionSize[axis] = size;

      if(sign > 0)
        hoverTrans[axis] += hoverSize[axis];

      if(innerArea)
        hoverTrans[axis] -= 0.5f * size;
      else if(sign > 0)
        hoverTrans[axis] -= size;

      region->SetSize(SnapToPixels(regionSize));
      region->SetTranslation(SnapToPixels(hoverTrans));

      mDockDirection = newHoverDirection;
      mHoverWidget = hoverWindow;
      return;
    }
  }

  ChangeHover(DockHover::None);
}

class DockedSizingManipuation : public MouseManipulation
{
public:
  DockArea::Enum mDirection;
  DockMode::Enum mDockMode;
  MultiDock* mMultiDock;
  bool mAreaSizing;

  Vec2 StartSize0;
  Vec2 StartSize1;
  HandleOf<Widget> mSized0;
  HandleOf<Widget> mSized1;
  DockingArea* mArea0;
  DockingArea* mArea1;

  DockedSizingManipuation(Mouse* mouse, MultiDock* multiDock, Widget* widgetToSize, DockMode::Enum direction)
    :MouseManipulation(mouse, multiDock)
  {
    mSized0 = widgetToSize;
    mDockMode = direction;
    mMultiDock = multiDock;
    mDirection = ToDockAreaDirection(direction);
    MultiDocker* docker0 = (MultiDocker*)widgetToSize->GetDocker();
    DockingArea* dockArea = &mMultiDock->DockingShells[docker0->DockShell].DockAreas[docker0->Area];

    // It is a area sizing (Size change between two areas) if the direction
    // of change and the dock area are in the same direction.
    // Otherwise this is a size change between two widgets in the 
    // same dock area.
    mAreaSizing = GetAxis(mDirection) == GetAxis(dockArea->Area);
    if(dockArea->Area == DockArea::Center)
      mAreaSizing = true;

    if(mAreaSizing)
    {
      mArea0 = dockArea;
      mArea1 = NextInDirection(multiDock, dockArea, mDirection);

      // No area in that direction
      if(mArea1 == nullptr || mArea1->Widgets.Empty())
      {
        this->Destroy();
        return;
      }

      mSized1 = dockArea->Widgets.Front();
      StartSize0 = mArea0->DockSize;
      StartSize1 = mArea1->DockSize;
    }
    else
    {
      int direction = GetSign(mDirection);
      uint nextIndex = docker0->DockIndex + direction;

      //No widget in that direction
      if(nextIndex >= dockArea->Widgets.Size())
      {
        this->Destroy();
        return;
      }

      Widget* sized1 = dockArea->Widgets[nextIndex];
      mSized1 = sized1;

      MultiDocker* docker1 = (MultiDocker*)sized1->GetDocker();
      StartSize0 = docker0->FlexSize;
      StartSize1 = docker1->FlexSize;
    }
  }

  void OnMouseMove(MouseEvent* mouseEvent) override
  {
    Vec2 newPos = mouseEvent->Position;
    Vec2 delta = newPos - mMouseStartPosition;

    Widget* sized0 = mSized0;
    Widget* sized1 = mSized1;
    if(sized0 == nullptr || sized1 == nullptr)
    {
      this->Destroy();
      return;
    }

    float minSize = Pixels(100);

    if(mAreaSizing)
    {
      // Changing size of two dock areas
      int sign = GetSign(mDirection);
      int axis = GetAxis(mDirection);

      // Clamp size change to prevent areas
      // from getting too small
      float minChange =  minSize - StartSize0[axis];
      float maxChange =  StartSize1[axis] - minSize;
      float change = delta[axis] * sign;
      change = Math::Clamp(change, minChange, maxChange);

      // Apply area change by adding to one area and removing
      // from the other
      mArea0->DockSize[axis] = StartSize0[axis] + change;
      mArea1->DockSize[axis] = StartSize1[axis] - change;
    }
    else
    {
      MultiDocker* docker0 = (MultiDocker*)sized0->GetDocker();
      DockingArea* dockArea = &mMultiDock->DockingShells[docker0->DockShell].DockAreas[docker0->Area];

      int sign = GetSign(mDirection);
      int axis = GetAxis(mDirection);

      MultiDocker* docker1 = (MultiDocker*)sized1->GetDocker();

      // When sizing controls in a area the pixel change of the mouse
      // needs to be translated into the amount of flex the represents
      // FlexRatio will convert flex values into pixel and back

      float minChange =  minSize - StartSize0[axis] * dockArea->FlexRatio;
      float maxChange =  StartSize1[axis] * dockArea->FlexRatio - minSize;

      // Clamp to prevent small windows
      float change = delta[axis] * sign;
      change = Math::Clamp(change, minChange, maxChange);

      // Change back into flex
      Vec2 changeVector = Vec2(0, 0);
      changeVector[axis] = change * (1.0f / dockArea->FlexRatio);

      // Apply changes
      docker0->FlexSize = StartSize0 + changeVector;
      docker1->FlexSize = StartSize1 - changeVector;
    }

    mMultiDock->MarkAsNeedsUpdate();
    mMultiDock->ComputeAndApplyLayout(false);
  }

  void OnMouseUp(MouseEvent* event) override
  {
    this->Destroy();
  }
};

//----------------------------------------------------------------- Multi-Docker

MultiDocker::MultiDocker(Widget* widget, MultiDock* owner)
{
  DockIndex = 0;
  DockShell = 0;
  DockOwner = owner;
  Zoomed = false;
  PreDockSize = widget->GetLocalRect();
  FlexSize = widget->GetSize();
  Area = DockArea::Floating;
}

void MultiDocker::Dock(Widget* widget, DockArea::Enum newDockArea)
{
  DockArea::Enum oldDockArea = this->Area;
  if(oldDockArea == DockArea::Floating)
  {
    // Store size before docking
    PreDockSize = widget->GetLocalRect();
    FlexSize = widget->GetSize();
  }

  if(oldDockArea == DockArea::Center)
    DockOwner->CenterWindow = nullptr;

  if(newDockArea == DockArea::Center)
    DockOwner->CenterWindow = Type::DynamicCast<Window*>(widget);

  Area = newDockArea;
  DockOwner->MarkAsNeedsUpdate();
}

void MultiDocker::Zoom(Widget* widget)
{
  Zoomed = !Zoomed;

  // Restore size post zoom
  if(!Zoomed && Area == DockArea::Floating)
    AnimateTo(widget, ToVector3(PreDockSize.TopLeft()), PreDockSize.GetSize());
  else
    PreDockSize = widget->GetRectInParent();

  widget->MarkAsNeedsUpdate();
  DockOwner->MarkAsNeedsUpdate();
}

void MultiDocker::WidgetDestroyed(Widget* widget)
{

}

void MultiDocker::Show(Widget* widget)
{
  // Move floating widgets to the front
  if(Area == DockArea::Floating)
    widget->MoveToFront();
}

bool MultiDocker::StartManipulation(Widget* widget, DockMode::Enum direction)
{
  // Move the widget to the front so MultiDock with display it last
  widget->MoveToFront();

  if(direction == DockMode::DockFill)
  {
    new DockMovingManipulation(Z::gMouse, DockOwner, widget);
    return true;
  }

  // Floating windows use standard size manipulation
  if(Area == DockArea::Floating)
  {
    new SizingManipulation(Z::gMouse, widget, direction);
    return true;
  }

  new DockedSizingManipuation(Z::gMouse, DockOwner, widget, direction);
  return true;
}

//------------------------------------------------------------------- Multi-Dock
ZilchDefineType(MultiDock, builder, type)
{
}

MultiDock::MultiDock(Composite* parent)
  : Composite(parent)
{
  mDefSet = parent->GetDefinitionSet();
  mExploded = false;
  mAnimateSize = Vec2(0, 0);
}

MultiDock::~MultiDock()
{

}

// Move the window out of the visible area in the direction
// of docking
void MoveToSide(MultiDock* dock, LayoutResult& result)
{
  Widget* widget = result.PlacedWidget;
  MultiDocker* docker = (MultiDocker*)widget->GetDocker();
  DockArea::Enum area = docker->Area;

  uint axis = GetAxis(area);
  int sign = GetSign(area);

  // Skip floating areas
  if(area == DockArea::Floating)
    return;

  // Center has no direction
  // just go up
  if(area == DockArea::Center)
  {
    axis = 1;
    sign = -1;
  }
  
  // Offset in the direction of docking
  Vec3 offscreen = result.Translation;

  if(sign < 0)
    offscreen[axis] -= result.Translation[axis] + result.Size[axis];
  else
    offscreen[axis] += dock->GetSize()[axis] - result.Translation[axis] + result.Size[axis];

  result.Translation = offscreen;
}

void MultiDock::CancelZoom()
{
  if(Widget* zoomed = mZoomed)
  {
    zoomed->GetDocker()->Zoom(zoomed);
    mZoomed = nullptr;
  }
}

void MultiDock::SetExploded(bool exploded, bool animate)
{
  CancelZoom();

  mExploded = exploded;
  ComputeAndApplyLayout(animate);
  this->MarkAsNeedsUpdate();
}

Window* MultiDock::AddWidget(Widget* widget, LayoutInfo& info)
{
  CancelZoom();

  Vec2 size = info.Size;
  Vec2 minSize = widget->GetMinSize();
  DockArea::Enum dockArea = info.Area;

  if(dockArea == DockArea::Center)
  {
    Window* centerWindow = GetCenterWindow();
    centerWindow->AttachAsTab(widget, true);
    return centerWindow;
  }

  // Create new tabbed window for this widget
  Window* window = new Window(this);
  window->AttachChildWidget(widget, AttachType::Normal);
  window->AttachAsTab(widget);
  minSize = ExpandSizeByThickness(GetTotalWindowPadding(), minSize);

  if(size.x < minSize.x)
    size.x = minSize.x;
  if(size.y < minSize.y)
    size.y = minSize.y;

  window->SetSize(size);
  window->SetDockArea(dockArea);

  // If floating animate the window from the top
  if(dockArea == DockArea::Floating)
  {
    Vec3 center = GetCenterPosition(this, window);
    window->SetTranslation(Vec3(center.x, -size.y, 0));
    AnimateTo(window, center, size);
  }
  else
  {
    // Compute new layout so the final
    // location of new window is know
    Array<LayoutResult> results;
    ComputeLayout(results);

    forRange(LayoutResult& result, results.All())
    {
      if(result.PlacedWidget == window)
      {
        // Move out it will be animated in by the normal update
        MoveToSide(this, result);
        result.PlacedWidget->SetTranslationAndSize(result.Translation, result.Size);
      }
    }
  }

  return window;
}

Window* MultiDock::GetCenterWindow()
{
  Window* centerWindow = CenterWindow;

  // No center window, create one 
  if(centerWindow == nullptr)
  {
    centerWindow = new Window(this);
    centerWindow->SetSize(Pixels(400, 400));
    // Dock to center
    centerWindow->SetDockArea(DockArea::Center);
    centerWindow->mTabArea->LockTabs();
    CenterWindow = centerWindow;
  }

  return centerWindow;
}

void MultiDock::AttachChildWidget(Widget* widget, AttachType::Enum attachType)
{
  Composite::AttachChildWidget(widget);
  // Set up the windows docker
  widget->SetDocker(new MultiDocker(widget, this));
  // Move to manager
  mManager->ManageWidget(widget);

  this->MarkAsNeedsUpdate();
}

void MultiDock::UpdateTransform()
{
  bool sizeChange = mAnimateSize != mSize;
  ComputeAndApplyLayout(!sizeChange);
}

struct SortByDockIndex
{
  bool operator()(Widget* left, Widget* right)
  {
    MultiDocker* dockerL = (MultiDocker*)left->GetDocker();
    MultiDocker* dockerR = (MultiDocker*)right->GetDocker();
    return dockerL->DockIndex < dockerR->DockIndex;
  }
};

// Compute the layout for all widget in the docking area. Area used is removed from
// area parameter
void LayoutDockingArea(MultiDock* dock, DockingArea& dockArea, DockArea::Enum areaIndex,
  Array<LayoutResult>& results, LayoutArea& layoutArea)
{
  // If there is no widgets set the dock size to zero
  // and return
  if(dockArea.Widgets.Empty())
  {
    //dockArea.DockSize = Vec2(0.0f, 0.0f);
    return;
  }

  // What axis to change depend on if this area has a vertical or horizontal layout
  DockArea::Enum areaDirection = areaIndex;
  uint axis0 = GetAxis(areaDirection);
  uint axis1 = !axis0;
  int sign = GetSign(areaDirection);

  // Dock area is zero when no widgets have been added
  if(dockArea.DockSize.LengthSq() == 0.0f)
  {
    // Need to limit
    dockArea.DockSize = dockArea.Widgets.Front()->GetSize();
    dockArea.DockSize = Math::Min(dockArea.DockSize, Vec2(400, 400));
  }

  float size0 = dockArea.DockSize[axis0];

  Vec3 offset = layoutArea.Offset;
  Vec2 areaSize = layoutArea.Size;
  areaSize[axis0] = size0;

  if(sign > 0)
    offset[axis0] += (layoutArea.Size[axis0] - size0);

  // Remove and offset LayoutArea by area used for
  // next dock area
  layoutArea.Size[axis0] -= size0;

  //For top and left move LayoutArea offset down
  //by size used. 
  if(sign < 0)
    layoutArea.Offset[axis0] += size0;

  // Sort by dock index
  Sort(dockArea.Widgets.All(), SortByDockIndex());

  // Do a pass getting all the sizes
  float totalFlexSize = 0.0f;
  float fixedSize = 0.0f;

  // measure all widget in dock area
  LayoutArea subLayoutArea;
  subLayoutArea.Size = areaSize;
  subLayoutArea.Offset = offset;
  subLayoutArea.HorizLimit = LimitMode::Limited;
  subLayoutArea.VerticalLimit = LimitMode::Limited;

  WidgetRange widgets = dockArea.Widgets.All();
  for(;!widgets.Empty();widgets.PopFront())
  {
    Widget& widget = *widgets.Front();
    MultiDocker* docker = (MultiDocker*)widget.GetDocker();

    if(widget.GetSizePolicy().Policy[axis1] == SizePolicy::Fixed)
    {
      Vec2 size = widget.Measure(subLayoutArea);
      fixedSize += size[axis1];
    }
    else
    {
      Vec2 size = docker->FlexSize;
      totalFlexSize += size[axis1];
    }
  }

  float targetSize = areaSize[axis1];
  targetSize -= fixedSize;

  // Determinate the how much size is allocated
  // relative to flex size
  float flexRatio = 1.0f;
  if(totalFlexSize)
    flexRatio = targetSize / totalFlexSize;

  // Store the ratio for size manipulations
  dockArea.FlexRatio = flexRatio;

  uint dockIndex = 0;
  widgets = dockArea.Widgets.All();
  for(;!widgets.Empty();widgets.PopFront())
  {
    Widget& widget = *widgets.Front();
    MultiDocker* docker = (MultiDocker*)widget.GetDocker();
    docker->DockIndex = dockIndex;

    Vec2 size;
    if(widget.GetSizePolicy().Policy[axis1] == SizePolicy::Fixed)
    {
      // Fixed size
      size = widget.Measure(layoutArea);
    }
    else
    {
      // Flex sized
      size = docker->FlexSize;
      size[axis1] *= flexRatio;
    }

    size[axis0] = size0;
    size = SnapToPixels(size);

    // Output the layout
    LayoutResult& layoutResult = results.PushBack();
    layoutResult.PlacedWidget = &widget;
    layoutResult.Translation = offset;
    layoutResult.Size = size;

    // Move down
    offset[axis1] += layoutResult.Size[axis1];
    ++dockIndex;
  }
}

int SpecialIndex(DockArea::Enum dockArea)
{
  return dockArea - DockArea::Count - 1;
}

void MultiDock::ComputeLayout(Array<LayoutResult>& results)
{
  mZoomed = nullptr;

  // Rebuild all docking areas

  // Clear out all docking areas
  uint shellIndex = 0;
  forRange(DockingShell& shell, DockingShells.All())
  {
    for(uint i=0;i<DockArea::Count;++i)
    {
      shell.DockAreas[i].Widgets.Clear();
      shell.DockAreas[i].DockShell = shellIndex;
    }
    ++shellIndex;
  }

  for(uint i=0;i<3;++i)
    SpecialAreas[i].Widgets.Clear();

  // Walk children and place them in the correct docking area
  uint childCount = 0;
  WidgetListRange children = GetChildren();
  while(!children.Empty())
  {
    Widget& child = children.Front();
    MultiDocker* docker = (MultiDocker*)child.mDocker;
    uint shellIndex = docker->DockShell;

    if(shellIndex >= DockingShells.Size())
      DockingShells.Resize(shellIndex+1);

    DockingShell& shell = DockingShells[shellIndex];

    for(uint i=0;i<DockArea::Count;++i)
    {
      shell.DockAreas[i].Area = (DockArea::Enum)i;
      shell.DockAreas[i].DockShell = shellIndex;
    }

    if(docker->Zoomed)
    {
      // Zoomed control
      child.MoveToFront();
      AnimateTo(&child, Vec3::cZero, mSize);
      Composite::UpdateTransform();
      mZoomed = &child;
    }
    else if(child.GetActive())
    {
      DockArea::Enum dockArea = docker->Area;

      if(dockArea > DockArea::Count)
      {
        SpecialAreas[SpecialIndex(dockArea)].Widgets.PushBack(&child);
      }
      else
      {
        // Center window is always in shell 0
        if(dockArea == DockArea::Center)
        {
          shellIndex = 0;
          docker->DockShell = 0;
        }

        // Add to docking area
        DockingShells[shellIndex].DockAreas[dockArea].Widgets.PushBack(&child);
      }
    }

    ++childCount;
    children.PopFront();
  }

  results.Reserve(childCount);

  // Now that docking areas are ready Compute layout
  // for all child widgets
  LayoutArea layoutArea;
  layoutArea.Size = mSize;
  layoutArea.Offset  = Vec3(0, 0, 0);
  layoutArea.HorizLimit = LimitMode::Limited;
  layoutArea.VerticalLimit = LimitMode::Limited;

  // Compute layout for all special docking areas

  // Move all floating windows to the front.
  {
    Array<Widget*>::range children = SpecialAreas[SpecialIndex(DockArea::Floating)].Widgets.All();
    while(!children.Empty())
    {
      Widget* child = children.Front();

      LayoutResult& layoutResult = results.PushBack();
      layoutResult.Translation = child->GetTranslation();
      layoutResult.Size = child->GetSize();
      layoutResult.PlacedWidget = child;

      child->MoveToFront();
      child->UpdateTransformExternal();
      children.PopFront();
    }

  }

  LayoutDockingArea(this, SpecialAreas[SpecialIndex(DockArea::TopTool)], DockArea::Top, results, layoutArea);
  LayoutDockingArea(this, SpecialAreas[SpecialIndex(DockArea::BotTool)], DockArea::Bottom, results, layoutArea);


  // Compute layout for all shell areas
  for(uint shell = 0; shell< DockingShells.Size();++shell)
  {
    DockingShell& dockingShell = DockingShells[DockingShells.Size() - shell -1];

    for(uint i = 0; i < DockArea::Count; ++i)
    {
      DockingArea& dockArea = dockingShell.DockAreas[i];
      switch(i)
      {
        case DockArea::Center:
        {
          // Center area uses remaining area
          if(dockArea.Widgets.Size() != 0)
          {
            LayoutResult& layoutResult = results.PushBack();
            layoutResult.Translation = layoutArea.Offset;
            layoutResult.Size = layoutArea.Size;
            layoutResult.PlacedWidget = dockArea.Widgets[0];
            dockArea.DockSize = layoutArea.Size;
          }
        }
        break;
        default:
        {
          LayoutDockingArea(this, dockArea, (DockArea::Enum)i, results, layoutArea);
          break;
        }
      }
    }
  }
}

void MultiDock::ComputeAndApplyLayout(bool animate)
{
  if(GetTransformUpdateState() == TransformUpdateState::ChildUpdate)
  {
    Composite::UpdateTransform();
    return;
  }
  
  Array<LayoutResult> results;
  ComputeLayout(results);

  mAnimateSize = mSize;

  // Apply layout

  forRange(LayoutResult& result, results.All())
  {
    if(mExploded || mZoomed)
      MoveToSide(this, result);

    const float minChange = Pixels(1);
    float distanceMoved = (result.PlacedWidget->GetTranslation() - result.Translation).Length();
    float sizeChanged = (result.PlacedWidget->GetSize() - result.Size).Length();
    if(animate &&  (distanceMoved > minChange || sizeChanged > minChange))
    {
      AnimateTo(result.PlacedWidget, result.Translation, result.Size, 0.3f);
    }
    else
    {
      result.PlacedWidget->SetTranslationAndSize(result.Translation, result.Size);
      result.PlacedWidget->UpdateTransformExternal();
    }
  }

  Composite::UpdateTransform();
}

}//namespace Zero
