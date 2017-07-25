///////////////////////////////////////////////////////////////////////////////
///
/// \file Scratchboard.cpp
/// Implementation of the Scratchboard Composite.
///
/// Authors: Joshua Claeys
/// Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//------------------------------------------------------------------ Graph Lines
class ScratchboardDrawer : public Widget
{
public:
  Vec3 mOffset;
  float mHashSize;
  float mLineWidth;
  ByteColor mLineColor;

  //****************************************************************************
  ScratchboardDrawer(Composite* parent, 
                     float lineWidth = 0.75f,
                     ByteColor lineColor = ToByteColor(Vec4(0,0,0,0.1f)),
                     float hashSize = Pixels(15))
    : Widget(parent, AttachType::Direct)
  {
    mOffset = Vec3::cZero;
    mHashSize = hashSize;
    mLineWidth = lineWidth;
    mLineColor = lineColor;
  }

  //****************************************************************************
  void Draw(DisplayRender* render, Mat4Param parentTx, ColorTransform& colorTx, DrawParams& params)
  {
    if(mSize.x < 0 || mSize.y < 0)
      return;

    // Draw vertical lines
    DrawLines(render, mOffset.x, Vec3::cXAxis, mSize);

    // Draw horizontal lines
    DrawLines(render, mOffset.y, Vec3::cYAxis, mSize);
  }

  //****************************************************************************
  void DrawLines(DisplayRender* render, float offset, 
                 Vec3 axis, Vec2 displaySize)
  {
    //Vec3 tangent = Math::Abs(Math::Cross(axis, Vec3::cZAxis));
    //Vec3 finalOffset = Math::FMod(offset, mHashSize) * axis;

    //float hashSize = (axis * Vec3(displaySize)).Length();
    //float hashLength = (tangent * Vec3(displaySize)).Length();

    //render->StartLines(mLineColor, mLineWidth);
    //uint hashCount = uint(hashSize / mHashSize) + 2;
    //for(uint i = 0; i < hashCount; ++i)
    //{
    //  Vec3 start = axis * mHashSize * float(i);
    //  Vec3 end = start + tangent * hashLength;

    //  render->LineSegment(SnapToPixels(start + finalOffset), 
    //                      SnapToPixels(end + finalOffset));
    //}
    //render->EndLines();
  }
};

//------------------------------------------------------- Graph Node Manipulator
//******************************************************************************
class ScratchboardObjectMover : public MouseManipulation
{
public:
  Scratchboard* mScratchboard;
  Widget* mWidget;
  bool mSnapping;
  float mSnapFidelity;
  Vec3 mScrollDirection;
  Vec2 mLastLocalMousePos;

  //****************************************************************************
  ScratchboardObjectMover(Mouse* mouse, Scratchboard* scratchBoard, 
                          Widget* widget, bool snapping, float snapFidelity)
    : MouseManipulation(mouse, scratchBoard)
  {
    mScratchboard = scratchBoard;
    mWidget = widget;
    mSnapping = snapping;
    mSnapFidelity = snapFidelity;
    mScrollDirection = Vec3::cZero;
    mLastLocalMousePos = mScratchboard->ToLocal(mouse->GetClientPosition());
  }

  //****************************************************************************
  void OnMouseMove(MouseEvent* event) override
  {
    // Get the local position on the scratchboard
    Vec2 local = mScratchboard->ToLocal(event->Position);


    // If the mouse is near the edge of the graph (within this distance),
    // we want to start scrolling the graph.  Calculate and store the current
    // scroll direction for frame update
    const float cScrollAreaSize = Pixels(30);

    mScrollDirection = Vec3::cZero;
    if(local.y < cScrollAreaSize)
      mScrollDirection += Vec3::cYAxis * (1.0f - (local.y / cScrollAreaSize));
    else if(mScratchboard->GetSize().y - local.y < cScrollAreaSize)
      mScrollDirection -= Vec3::cYAxis * (1.0f - ((mScratchboard->GetSize().y - local.y) / cScrollAreaSize));
    if(local.x < cScrollAreaSize)
      mScrollDirection += Vec3::cXAxis * (1.0f - (local.x / cScrollAreaSize));
    else if(mScratchboard->GetSize().x - local.x < cScrollAreaSize)
      mScrollDirection -= Vec3::cXAxis * (1.0f - ((mScratchboard->GetSize().x - local.x) / cScrollAreaSize));

    // Store the position for frame update
    mLastLocalMousePos = local;
  }

  //****************************************************************************
  void OnUpdate(UpdateEvent* event) override
  {
    Vec2 local = mLastLocalMousePos;
    local.x = Math::Clamp(local.x, 0.0f, mScratchboard->GetSize().x);
    local.y = Math::Clamp(local.y, 0.0f, mScratchboard->GetSize().y);

    // Convert to world space and set the new translation
    Vec3 world = mScratchboard->ToGraphPosition(Vec3(local));

    // Snap in the world space
    if(mSnapping)
    {
      world.x -= Math::FMod(world.x, mSnapFidelity);
      world.y -= Math::FMod(world.y, mSnapFidelity);
    }

    mWidget->SetTranslation(world);

    // Scroll if we're near the edge of the screen
    const float cScrollSpeed = Pixels(10);
    mScratchboard->Scroll(mScrollDirection * cScrollSpeed);
  }

  //****************************************************************************
  void OnMouseUp(MouseEvent* event) override
  {
    this->Destroy();
  }
};

//--------------------------------------------------------------- Graph Scroller
//******************************************************************************
class ScratchboardScroller : public MouseManipulation
{
public:
  Scratchboard* mScratchboard;

  //****************************************************************************
  ScratchboardScroller(Mouse* mouse, Scratchboard* scratchboard) 
    : MouseManipulation(mouse, scratchboard)
  {
    mScratchboard = scratchboard;
  }

  //****************************************************************************
  void OnMouseMove(MouseEvent* event) override
  {
    // Scroll the graph
    mScratchboard->Scroll(Vec3(event->Movement));
  }

  //****************************************************************************
  void OnMiddleMouseUp(MouseEvent* event) override
  {
    this->Destroy();
    event->GetMouse()->SetCursor(Cursor::Arrow);
  }

  //****************************************************************************
  void OnRightMouseUp(MouseEvent* event) override
  {
    this->Destroy();
    event->GetMouse()->SetCursor(Cursor::Arrow);
  }

};

//----------------------------------------------------------------- Scratchboard
//******************************************************************************
Scratchboard::Scratchboard(Composite* parent) : Composite(parent)
{
  mClientArea = nullptr;

  // Create the line drawers
  mLightGridDrawer = new ScratchboardDrawer(this);
  mBoldGridDrawer = new ScratchboardDrawer(this, 2, ToByteColor(Vec4(0,0,0,0.2f)));

  // Set the default grid size
  SetGridSize(Pixels(20));

  // Create the client area
  mClientArea = new Composite(this, AttachType::Direct);

  ConnectThisTo(mBoldGridDrawer, Events::MiddleMouseDown, OnMiddleMouseDown);
  ConnectThisTo(mBoldGridDrawer, Events::RightMouseDown, OnMiddleMouseDown);

  mDraggingEnabled = true;
  mKeepElementsInView = true;
}

//******************************************************************************
void Scratchboard::Scroll(Vec3Param direction)
{
  mClientArea->mTranslation += direction;

  MarkAsNeedsUpdate();
}

//******************************************************************************
void Scratchboard::CenterToPoint(Vec3Param graphPosition, float panTime)
{
  Vec3 destination = Vec3(mSize) * 0.5f - graphPosition;
  ActionSequence* seq = new ActionSequence(this);
  seq->Add(MoveWidgetAction(mClientArea, destination, panTime));

  MarkAsNeedsUpdate();
}

//******************************************************************************
void Scratchboard::Frame(Aabb& worldAabb, float panTime)
{
  Vec3 center = worldAabb.GetCenter();
  CenterToPoint(center, panTime);
  MarkAsNeedsUpdate();
}

//******************************************************************************
MouseManipulation* Scratchboard::StartObjectDrag(Mouse* mouse, Widget* object, 
                                              bool snapping, float snapFidelity)
{
  return new ScratchboardObjectMover(mouse, this, object, snapping, snapFidelity);
}

//******************************************************************************
bool Scratchboard::WithinView(Vec3Param graphPosition)
{
  Vec3 local = ToPixelPosition(graphPosition);
  bool inBounds = true;
  inBounds &= Math::InBounds(local.x, 0.0f, mSize.x);
  inBounds &= Math::InBounds(local.y, 0.0f, mSize.y);
  return inBounds;
}

//******************************************************************************
Vec3 Scratchboard::ToPixelPosition(Vec3Param graphPosition)
{
  return graphPosition + mClientArea->GetTranslation();
}

//******************************************************************************
Vec3 Scratchboard::ToGraphPosition(Vec3Param pixelPosition)
{
  return pixelPosition - mClientArea->GetTranslation();
}

//******************************************************************************
void Scratchboard::SetGridSize(float pixels)
{
  mLightGridDrawer->mHashSize = pixels;
  mBoldGridDrawer->mHashSize = pixels * 5.0f;
}

//******************************************************************************
float Scratchboard::GetGridSize() const
{
  return mLightGridDrawer->mHashSize;
}

//******************************************************************************
void Scratchboard::SetDragging(bool state)
{
  mDraggingEnabled = state;
}

//******************************************************************************
Vec3 Scratchboard::SnapToGrid(Vec3Param graphPos)
{
  float gridSize = GetGridSize();
  float x = Math::Round(graphPos.x / gridSize) * gridSize;
  float y = Math::Round(graphPos.y / gridSize) * gridSize;
  return Vec3(x, y, 0.0f);
}

//******************************************************************************
Composite* Scratchboard::GetClientArea()
{
  return mClientArea;
}

//******************************************************************************

void Scratchboard::AttachChildWidget(Widget* widget, AttachType::Enum attachType)
{
  // Used to redirect attachments to the client area
  if(attachType == AttachType::Direct)
    Composite::AttachChildWidget(widget);
  else
    mClientArea->AttachChildWidget(widget);
}

//******************************************************************************
void Scratchboard::UpdateTransform()
{
  if(mKeepElementsInView)
    ClampViewToElements();

  mLightGridDrawer->SetSize(mSize);
  mBoldGridDrawer->SetSize(mSize);
  mLightGridDrawer->mOffset = mClientArea->GetTranslation();
  mBoldGridDrawer->mOffset = mClientArea->GetTranslation();

  Composite::UpdateTransform();
}

//******************************************************************************
void Scratchboard::OnMiddleMouseDown(MouseEvent* e)
{
  if(mDraggingEnabled)
  {
    e->GetMouse()->SetCursor(Cursor::SizeAll);
    new ScratchboardScroller(e->GetMouse(), this);
  }
}

//******************************************************************************
void Scratchboard::ClampViewToElements()
{
  Aabb objectsAabb = GetObjectsAabb();

  Vec2 min(objectsAabb.mMin.x - mSize.x, objectsAabb.mMin.y - mSize.y);
  Vec2 max(objectsAabb.mMax.x, objectsAabb.mMax.y);

  Vec3 currPos = mClientArea->mTranslation;
  mClientArea->mTranslation.x = -Math::Clamp(-currPos.x, min.x, max.x);
  mClientArea->mTranslation.y = -Math::Clamp(-currPos.y, min.y, max.y);
}

//******************************************************************************
Aabb Scratchboard::GetObjectsAabb()
{
  if(mClientArea->mChildren.Empty())
    return Aabb(Vec3::cZero, Vec3::cZero);
  Array<Vec3> positions;
  forRange(Widget& widget, mClientArea->GetChildren())
  {
    positions.PushBack(widget.GetTranslation());
  }

  Aabb aabb;
  aabb.Compute(positions);
  return aabb;
}

}//namespace Zero
