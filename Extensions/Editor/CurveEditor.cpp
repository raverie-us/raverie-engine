///////////////////////////////////////////////////////////////////////////////
///
/// \file CurveEditor.cpp
/// Implementation of the CurveEditor Widget and CurveEditor Composite.
/// 
/// Authors: Joshua Claeys
/// Copyright 2012-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace CurveEditorUi
{
const cstr cLocation = "EditorUi/CurveEditor";
Tweakable(float, ControlPointSize, Pixels(8),  cLocation);
Tweakable(float, TangentSize,      Pixels(4),  cLocation);
Tweakable(float, TangentLineSize,  1.5f,       cLocation);
Tweakable(float, SelectionScale,   3.5f,       cLocation);
Tweakable(Vec4,  SelectedColor, Vec4(1,1,1,1), cLocation);
}

namespace Events
{
  DefineEvent(PushDebugSamples);
  DefineEvent(CurveModified);
  DefineEvent(ControlPointAdded);
  DefineEvent(ControlPointModified);
  DefineEvent(ControlPointDeleted);
  DefineEvent(MouseEnterCurve);
  DefineEvent(MouseExitCurve);
  DefineEvent(CurveDoubleClicked);
}

ZilchDefineType(CurveEvent, builder, type)
{
}

//----------------------------------------------------------------- Multi Select
class MultiSelectManipulation : public MouseManipulation
{
public:
  CurveEditor* mEditor;

  /// Displays the drag selection box.
  Element* mSelectBox;

  /// Where the drag was started in pixels.
  Vec2 mDragStartGraph;
  
  //****************************************************************************
  MultiSelectManipulation(MouseDragEvent* dragEvent, CurveEditor* editor)
    : MouseManipulation(dragEvent->GetMouse(), editor->GetRootWidget()), mEditor(editor)
  {
    Vec2 localPixels = mEditor->ToLocal(dragEvent->StartPosition);
    mDragStartGraph = mEditor->ToGraphPosition(localPixels);

    // Create a selection box
    mSelectBox = mEditor->CreateAttached<Element>(cDragBox);
    mSelectBox->SetVisible(false);
  }

  //****************************************************************************
  void OnMouseMove(MouseEvent* e) override
  {
    mSelectBox->SetVisible(true);
  }

  //****************************************************************************
  void OnMouseUpdate(MouseEvent* e) override
  {
    Rect selection = GetSelectionRect(e);
    PlaceWithRect(selection, mSelectBox);
  }

  //****************************************************************************
  Rect GetSelectionRect(MouseEvent* e)
  {
    Vec2 currPixels = mEditor->ToLocal(e->Position);
    Vec2 startPixels = mEditor->ToPixelPosition(mDragStartGraph);
    Vec2 min = Math::Min(currPixels, startPixels);
    Vec2 max = Math::Max(currPixels, startPixels);
    min = Math::Clamp(min, Vec2::cZero, mEditor->mSize);
    max = Math::Clamp(max, Vec2::cZero, mEditor->mSize);
    return Rect::MinAndMax(min, max);
  }

  //****************************************************************************
  void OnKeyDown(KeyboardEvent* e) override
  {
    if(e->Key == Keys::Escape)
    {
      this->Destroy();
      mSelectBox->Destroy();
    }
  }

  //****************************************************************************
  bool Contains(CurveEditing::Draggable* target, Rect& selectionRect)
  {
    Vec2 center = ToVector2(target->GetTranslation()) + target->GetSize() * 0.5f;
    return selectionRect.Contains(center);
  }

  //****************************************************************************
  void OnMouseUp(MouseEvent* e) override
  {
    Rect selection = GetSelectionRect(e);

    static Array<CurveEditing::Draggable*> toSelect;
    toSelect.Clear();

    forRange(CurveObject* curve, mEditor->mCurves.All())
    {
      forRange(CurveObject::ControlPoint* target, curve->mControlPoints.All())
      {
        if(target->TangentsVisible())
        {
          if(Contains(target->mTangentIn, selection))
            toSelect.PushBack(target->mTangentIn);
          if(Contains(target->mTangentOut, selection))
            toSelect.PushBack(target->mTangentOut);
        }

        if(Contains(target, selection))
          toSelect.PushBack(target);
      }
    }

    // We're not adding to the selection if ctrl is pressed, so clear it
    if(!e->CtrlPressed)
      mEditor->ClearSelection();

    forRange(CurveEditing::Draggable* selected, toSelect.All())
    {
      selected->Select();
    }

    mSelectBox->Destroy();
    this->Destroy();
  }
};

//----------------------------------------------------------------- Curve Drawer
//******************************************************************************
CurveDrawer::CurveDrawer(CurveEditor* curveEditor) 
  : Widget(curveEditor)
{
  mCurveEditor = curveEditor;
}

void CurveDrawer::RenderUpdate(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, Rect clipRect)
{
  Widget::RenderUpdate(viewBlock, frameBlock, parentTx, colorTx, clipRect);

  if (!mCurveEditor->mEnabled)
    return;

  forRange (CurveObject* curve, mCurveEditor->mCurves.All())
  {
    AddCurve(viewBlock, frameBlock, clipRect, curve);
    AddControlPoints(viewBlock, frameBlock, clipRect, curve);
    // Not implemented
    //AddDebugSamples(viewBlock, frameBlock, clipRect, curve);
  }
}

void CurveDrawer::AddCurve(ViewBlock& viewBlock, FrameBlock& frameBlock, Rect clipRect, CurveObject* curveObject)
{
  Array<StreamedVertex> lines;
  Array<StreamedVertex> triangles;

  // The color of the curve
  Vec4 color = ToFloatColor(curveObject->mCurveColor);

  // Create the curve
  Vec3Array curve;
  curveObject->GetCurve(curve);

  // If there are no segments, do nothing
  if (curve.Size() < 2)
    return;

  // Draw dotted lines for vertical segments
  for (uint i = 0; i < curve.Size() - 1; ++i)
  {
    Vec2 p1(curve[i].x, curve[i].y);
    Vec2 p2(curve[i + 1].x, curve[i + 1].y);

    p1 = mCurveEditor->ToPixelPosition(p1);
    p2 = mCurveEditor->ToPixelPosition(p2);

    if (p1.x != p2.x)
      continue;

    AddPoint(lines, triangles, Vec3(p1), CurveEditorUi::ControlPointSize, ToFloatColor(Color::Black), true);
    lines.PushBack(StreamedVertex(SnapToPixels(Vec3(p1)), Vec2::cZero, ToFloatColor(Color::Black)));
    lines.PushBack(StreamedVertex(SnapToPixels(Vec3(p2)), Vec2::cZero, ToFloatColor(Color::Black)));
  }

  // Curve line segments
  for (uint i = 0; i < curve.Size() - 1; ++i)
  {
    Vec2 p1(curve[i].x, curve[i].y);
    Vec2 p2(curve[i + 1].x, curve[i + 1].y);

    p1 = mCurveEditor->ToPixelPosition(p1);
    p2 = mCurveEditor->ToPixelPosition(p2);

    if (p1.x == p2.x)
      continue;

    lines.PushBack(StreamedVertex(SnapToPixels(Vec3(p1)), Vec2::cZero, color));
    lines.PushBack(StreamedVertex(SnapToPixels(Vec3(p2)), Vec2::cZero, color));
  }

  CreateRenderData(viewBlock, frameBlock, clipRect, lines, PrimitiveType::Lines);
}

void CurveDrawer::AddControlPoints(ViewBlock& viewBlock, FrameBlock& frameBlock, Rect clipRect, CurveObject* curveObject)
{
  Array<StreamedVertex> tangentLines;
  Array<StreamedVertex> pointLines;
  Array<StreamedVertex> pointTriangles;

  // Draw all the control points
  CurveObject::ControlPointArray& controlPoints = curveObject->mControlPoints;

  for (uint i = 0; i < controlPoints.Size(); ++i)
  {
    // Get the current control point
    CurveEditing::ControlPoint& cp = *controlPoints[i];

    Vec2 pos = mCurveEditor->ToPixelPosition(cp.mGraphPosition);

    // If it's the selected control point
    if (cp.TangentsVisible())
    {
      // Draw TangentIn if it's not the first control point
      if (i != 0)
      {
        Vec3 tanPos = Vec3(cp.GetPixelPosition() + cp.mTangentIn->GetPixelDirection());

        tangentLines.PushBack(StreamedVertex(SnapToPixels(Vec3(pos)), Vec2::cZero, ToFloatColor(Color::Black)));
        tangentLines.PushBack(StreamedVertex(SnapToPixels(Vec3(tanPos)), Vec2::cZero, ToFloatColor(Color::Black)));

        Vec4 color = ToFloatColor(curveObject->mCurveColor);
        if (cp.mTangentIn->IsSelected())
          color = CurveEditorUi::SelectedColor;
        if (cp.mTangentIn->mMouseOver)
          color *= Vec4(0.7f, 0.7f, 0.7f, 1);

        AddPoint(pointLines, pointTriangles, tanPos, CurveEditorUi::TangentSize, color);
      }

      // Draw TangentOut if it's not the last control point
      if (i != controlPoints.Size() - 1)
      {
        Vec3 tanPos = Vec3(cp.GetPixelPosition() + cp.mTangentOut->GetPixelDirection());

        tangentLines.PushBack(StreamedVertex(SnapToPixels(Vec3(pos)), Vec2::cZero, ToFloatColor(Color::Black)));
        tangentLines.PushBack(StreamedVertex(SnapToPixels(Vec3(tanPos)), Vec2::cZero, ToFloatColor(Color::Black)));

        Vec4 color = ToFloatColor(curveObject->mCurveColor);
        if (cp.mTangentOut->IsSelected())
          color = CurveEditorUi::SelectedColor;
        if (cp.mTangentOut->mMouseOver)
          color *= Vec4(0.7f, 0.7f, 0.7f, 1);

        AddPoint(pointLines, pointTriangles, tanPos, CurveEditorUi::TangentSize, color);
      }
    }

    Vec4 color = ToFloatColor(curveObject->mCurveColor);
    if (cp.IsSelected())
      color = CurveEditorUi::SelectedColor;
    if (cp.mMouseOver)
      color *= Vec4(0.7f, 0.7f, 0.7f, 1);

    // Draw the point
    AddPoint(pointLines, pointTriangles, Vec3(pos), CurveEditorUi::ControlPointSize, color);
  }

  CreateRenderData(viewBlock, frameBlock, clipRect, tangentLines, PrimitiveType::Lines);
  CreateRenderData(viewBlock, frameBlock, clipRect, pointTriangles, PrimitiveType::Triangles);
  CreateRenderData(viewBlock, frameBlock, clipRect, pointLines, PrimitiveType::Lines);
}

void CurveDrawer::AddPoint(Array<StreamedVertex>& lines, Array<StreamedVertex>& triangles, Vec3Param pos, float size, Vec4 color, bool empty)
{
  float halfSize = size * 0.5f;
  StreamedVertex v0(SnapToPixels(pos + Vec3(-halfSize,-halfSize, 0)), Vec2(0, 0), color);
  StreamedVertex v1(SnapToPixels(pos + Vec3(-halfSize, halfSize, 0)), Vec2(0, 1), color);
  StreamedVertex v2(SnapToPixels(pos + Vec3( halfSize, halfSize, 0)), Vec2(1, 1), color);
  StreamedVertex v3(SnapToPixels(pos + Vec3( halfSize,-halfSize, 0)), Vec2(1, 0), color);

  if (empty == false)
  {
    triangles.PushBack(v0);
    triangles.PushBack(v1);
    triangles.PushBack(v2);
    triangles.PushBack(v2);
    triangles.PushBack(v3);
    triangles.PushBack(v0);
  }

  v0.mColor = ToFloatColor(Color::Black);
  v1.mColor = ToFloatColor(Color::Black);
  v2.mColor = ToFloatColor(Color::Black);
  v3.mColor = ToFloatColor(Color::Black);
  lines.PushBack(v0);
  lines.PushBack(v1);
  lines.PushBack(v1);
  lines.PushBack(v2);
  lines.PushBack(v2);
  lines.PushBack(v3);
  lines.PushBack(v3);
  lines.PushBack(v0);
}

//----------------------------------------------------------------- Curve Object
//******************************************************************************
CurveObject::CurveObject(CurveEditor* editor)
  : mEditor(editor)
{
  mCurveColor = Color::Orange;
  mError = 0.001f;
  mMouseOverCurve = false;
  editor->AddCurveObject(this);
}

static Math::PiecewiseFunction sFunction;

//******************************************************************************
void BuildPiecewiseFunction(Math::PiecewiseFunction& function, CurveObject* curve)
{
  function.mError = curve->mError;
  function.Clear();
  function.mControlPoints.Reserve(curve->mControlPoints.Size());

  forRange(CurveEditor::ControlPoint* cp, curve->mControlPoints.All())
  {
    Vec2 position = cp->mGraphPosition;
    Vec2 tanIn = cp->mTangentIn->GetGraphDirection();
    Vec2 tanOut = cp->mTangentOut->GetGraphDirection();
    function.AddControlPoint(position, tanIn, tanOut);
  }

  function.Bake();
}

//******************************************************************************
void CurveObject::Clear()
{
  // Delete each control point
  for(uint i = 0; i < mControlPoints.Size(); ++i)
    delete mControlPoints[i];

  // Clear the array
  mControlPoints.Clear();
}

//******************************************************************************
void CurveObject::Destroy()
{
  // Remove all selected control points and tangents
  forRange(ControlPoint* cp, mControlPoints.All())
  {
    mEditor->mSelection.Erase(cp);
    mEditor->mSelection.Erase(cp->mTangentIn);
    mEditor->mSelection.Erase(cp->mTangentOut);
    cp->Destroy();
  }

  mEditor->mCurves.EraseValueError(this);
  delete this;
}

//******************************************************************************
float CurveObject::Sample(float t)
{
  BuildPiecewiseFunction(sFunction, this);
  return sFunction.Sample(t);
}

//******************************************************************************
void CurveObject::GetCurve(Vec3Array& curve)
{
  BuildPiecewiseFunction(sFunction, this);
  curve.Assign(sFunction.GetBakedCurve());
}

//******************************************************************************
CurveEditing::ControlPoint* CurveObject::CreateControlPoint(Vec2Param pos,
                                     Vec2Param tanIn, Vec2Param tanOut,
                                     CurveEditing::CurveEditorFlags::Type flags)
{
  ControlPoint* internalPoint = new ControlPoint(this, pos, tanIn, tanOut, 
                                                 flags);

  // Add the point
  AddControlPoint(internalPoint);

  DispatchCurveEvent(Events::ControlPointAdded, internalPoint);
  DispatchCurveEvent(Events::CurveModified, internalPoint);

  return internalPoint;
}

//******************************************************************************
bool CurveObject::IsMouseOver(Vec2Param pixelPos)
{
  // If the cursor is within this many pixels, the mouse is considered over it
  const float cSelectionDistance = Pixels(5);

  Vec2 graphPos = mEditor->ToGraphPosition(pixelPos);
  float sampleY = Sample(graphPos.x);

  // We want to do the distance check in pixel space, so convert the sampled
  // position back to pixels (we don't care about the x value)
  Vec2 samplePixelPos = mEditor->ToPixelPosition(Vec2(0, sampleY));

  // Distance in pixels
  float yDistance = Math::Abs(samplePixelPos.y - pixelPos.y);
  if(yDistance < cSelectionDistance)
  {
    // If there's only one control point, we want to make sure the x is within
    // distance as well
    if(mControlPoints.Size() == 1)
    {
      // Bring the control point into pixels
      Vec2 cpPixelPos = mEditor->ToPixelPosition(mControlPoints.Front()->mGraphPosition);

      // Return whether or not it's within distance
      float xDistance = Math::Abs(cpPixelPos.x - pixelPos.x);
      return xDistance < cSelectionDistance;
    }

    return true;
  }

  return false;
}

//******************************************************************************
uint CurveObject::AddControlPoint(ControlPoint* controlPoint)
{
  mControlPoints.PushBack(controlPoint);
  Sort(mControlPoints.All(), SortByX());
  mEditor->MarkAsNeedsUpdate();
  return mControlPoints.FindIndex(controlPoint);
}

//******************************************************************************
CurveEditing::ControlPoint* CurveObject::AddNewControlPoint(Vec2Param pos)
{
  // Create a new internal control point
  CurveEditing::ControlPoint* point = new ControlPoint(this, pos, Vec2::cZero,
                                                       Vec2::cZero);
  uint index = AddControlPoint(point);

  // Set the tangents
  Vec2 prev = pos;
  Vec2 next = pos;

  // Check for edge cases
  if(index < mControlPoints.Size() - 1)
    next = mControlPoints[index + 1]->mGraphPosition;
  if(index > 0)
    prev = mControlPoints[index - 1]->mGraphPosition;

  // Set the tangents
  point->mTangentIn->mGraphDirection = ((prev - next) * 0.25f);
  point->mTangentOut->mGraphDirection = ((next - prev) * 0.25f);

  DispatchCurveEvent(Events::ControlPointAdded, point);
  DispatchCurveEvent(Events::CurveModified, point);

  return point;
}

//******************************************************************************
void CurveObject::RemoveControlPoint(ControlPoint* controlPoint)
{
  mEditor->DeSelect(controlPoint);
  mEditor->DeSelect(controlPoint->mTangentIn);
  mEditor->DeSelect(controlPoint->mTangentOut);

  mControlPoints.EraseValueError(controlPoint);

  DispatchCurveEvent(Events::ControlPointDeleted, controlPoint);
  DispatchCurveEvent(Events::CurveModified, controlPoint);

  controlPoint->Destroy();
}

//******************************************************************************
void CurveObject::SetCurveColor(ByteColor color)
{
  mCurveColor = color;
}

//******************************************************************************
float CurveObject::GetCurrentLineThickness()
{
  // If the mouse is over it, use the highlight thickness
  if(mMouseOverCurve && mEditor->mHighlightOnMouseEnter)
    return mEditor->mHighlightThickness;

  // Otherwise, use the default thickness
  return mEditor->mDefaultLineThickness;
}

//******************************************************************************
void CurveObject::DispatchCurveEvent(StringParam eventName,
                                     ControlPoint* controlPoint)
{
  CurveEvent eventToSend;
  eventToSend.mCurve = this;
  eventToSend.mControlPoint = controlPoint;
  GetDispatcher()->Dispatch(eventName, &eventToSend);
}

//******************************************************************************
void CurveObject::Modified(ControlPoint* controlPoint)
{
  ErrorIf(controlPoint == nullptr, "Invalid control point.");

  DispatchCurveEvent(Events::ControlPointModified, controlPoint);
  DispatchCurveEvent(Events::CurveModified, controlPoint);

  if(mEditor->mToolbar)
    mEditor->mToolbar->Update();
}

//----------------------------------------------------------------- Curve Editor
//******************************************************************************
CurveEditor::CurveEditor(Composite* parent) : Composite(parent)
{
  // Create the curve drawer
  mEnabled = true;
  mCurveDrawer = new CurveDrawer(this);

  mCurveDrawer->SetSize(GetSize());
  ConnectThisTo(this, Events::MouseMove, OnMouseMove);
  ConnectThisTo(this, Events::LeftClick, OnLeftClick);
  ConnectThisTo(this, Events::DoubleClick, OnDoubleClick);
  ConnectThisTo(this, Events::LeftMouseDrag, OnLeftDrag);
  ConnectThisTo(this, Events::KeyDown, OnKeyDown);

  mDefaultLineThickness = 2.0f;
  mHighlightThickness = 3.0f;
  mHighlightOnMouseEnter = true;

  mClampMouseDrag = true;

  mToolbar = nullptr;

  this->SetLayout(CreateFillLayout());
}

//******************************************************************************
CurveEditor::~CurveEditor()
{
  while(!mCurves.Empty())
    mCurves.Front()->Destroy();
}

//******************************************************************************
void CurveEditor::UpdateTransform()
{
  forRange(CurveObject* curve, mCurves.All())
  {
    forRange(ControlPoint* cp, curve->mControlPoints.All())
    {
      Vec2 halfSize = cp->GetSize() * 0.5f;
      Vec2 pixelPosition = cp->GetPixelPosition() - halfSize;
      cp->SetTranslation(ToVector3(pixelPosition));
    }
  }

  Composite::UpdateTransform();
}

//******************************************************************************
CurveEditor::CurveObjectRange CurveEditor::GetCurveObjects()
{
  return mCurves.All();
}

//******************************************************************************
Vec2 CurveEditor::ToPixelPosition(Vec2 graphPos)
{
  Vec2 pixelPos;
  pixelPos.x = graphPos.x * mSize.x;
  pixelPos.y = (1.0f - graphPos.y) * mSize.y;
  return pixelPos;
}

//******************************************************************************
Vec2 CurveEditor::ToGraphPosition(Vec2 pixelPos)
{
  Vec2 graphPos;
  graphPos.x = pixelPos.x / mSize.x;
  graphPos.y = (mSize.y - pixelPos.y) / mSize.y;
  return graphPos;
}

//******************************************************************************
Vec2 CurveEditor::ClampPixelPosition(Vec2 pixelPos)
{
  Vec2 clampedPos;
  clampedPos.x = Math::Clamp(pixelPos.x, 0.0f, mSize.x);
  clampedPos.y = Math::Clamp(pixelPos.y, 0.0f, mSize.y);
  return clampedPos;
}

//******************************************************************************
String CurveEditor::GraphToDisplayTextX(float graphValue)
{
  return String::Format("%0.4g", graphValue);
}

//******************************************************************************
String CurveEditor::GraphToDisplayTextY(float graphValue)
{
  return String::Format("%0.4g", graphValue);
}

//******************************************************************************
float CurveEditor::DisplayTextToGraphX(StringParam displayText)
{
  float position;
  ToValue(displayText, position);
  return position;
}

//******************************************************************************
float CurveEditor::DisplayTextToGraphY(StringParam displayText)
{
  float position;
  ToValue(displayText, position);
  return position;
}

//******************************************************************************
CurveObject* CurveEditor::GetMouseOverCurve()
{
  forRange(CurveObject* curve, mCurves.All())
  {
    if(curve->mMouseOverCurve)
      return curve;
  }

  return nullptr;
}

//******************************************************************************
void CurveEditor::SelectDraggable(Draggable* draggable, bool singleSelect)
{
  // Clear the selection if it's single select
  if(singleSelect)
    ClearSelection();

  // Add it to the selection
  mSelection.Insert(draggable);

  // The selection has changed
  CurveEvent e;
  e.mCurve = draggable->mCurve;
  GetDispatcher()->Dispatch(Events::SelectionChanged, &e);
}

//******************************************************************************
bool CurveEditor::IsSelected(Draggable* draggable)
{
  return mSelection.Contains(draggable);
}

//******************************************************************************
void CurveEditor::DeSelect(Draggable* draggable)
{
  mSelection.Erase(draggable);

  // The selection has changed
  CurveEvent e;
  GetDispatcher()->Dispatch(Events::SelectionChanged, &e);
}

//******************************************************************************
CurveEditor::SelectionRange CurveEditor::GetSelection()
{
  return mSelection.All();
}

//******************************************************************************
void CurveEditor::ClearSelection()
{
  mSelection.Clear();

  // The selection has changed
  CurveEvent e;
  GetDispatcher()->Dispatch(Events::SelectionChanged, &e);
}

//******************************************************************************
void CurveEditor::DeleteSelected()
{
  Array<ControlPoint*> selectedControlPoints;

  forRange(Draggable* draggable, mSelection.All())
  {
    if(ControlPoint* controlPoint = Type::DynamicCast<ControlPoint*>(draggable))
      selectedControlPoints.PushBack(controlPoint);
  }

  forRange(ControlPoint* controlPoint, selectedControlPoints.All())
  {
    controlPoint->Delete();
  }
}

//******************************************************************************
void CurveEditor::Hide()
{
  mEnabled = false;
}

//******************************************************************************
void CurveEditor::Show()
{
  mEnabled = true;
}

//******************************************************************************
void CurveEditor::RegisterToolBar(CurveEditing::CurveEditorToolbar* toolBar)
{
  mToolbar = toolBar;
  mToolbar->SetCurveEditor(this);
}

//******************************************************************************
void CurveEditor::RegisterLinearTangentsButton(ButtonBase* button)
{
  ConnectThisTo(button, Events::ButtonPressed, OnLinearTangentsPressed);
}

//******************************************************************************
void CurveEditor::RegisterSplitTangentsButton(ButtonBase* button)
{
  ConnectThisTo(button, Events::ButtonPressed, OnSplitTangentsPressed);
}

//******************************************************************************
void CurveEditor::RegisterWeightedTangentsButton(ButtonBase* button)
{
  ConnectThisTo(button, Events::ButtonPressed, OnWeightedTangentsPressed);
}

//******************************************************************************
void CurveEditor::OnLeftClick(MouseEvent* event)
{
  if(event->Handled)
    return;

  if(ContextMenu* contextMenu = mContextMenuHandle)
  {
    contextMenu->FadeOut();
  }

  ClearSelection();
}

//******************************************************************************
void CurveEditor::AddCurveObject(CurveObject* curve)
{
  mCurves.PushBack(curve);
}

//******************************************************************************
void CurveEditor::RemoveCurveObject(CurveObject* curve)
{
  mCurves.EraseValueError(curve);
}

//******************************************************************************
void CurveEditor::OnMouseMove(MouseEvent* event)
{
  // Walk the curves backwards to hit the top-most one first
  for(int i = mCurves.Size() - 1; i >= 0; --i)
  {
    CurveObject* curve = mCurves[(uint)i];

    // If the event was already handled, the mouse is likely over another curve
    if(event->Handled)
    {
      // Send an event if the mouse was last over the curve
      if(curve->mMouseOverCurve)
        curve->DispatchCurveEvent(Events::MouseExitCurve);
      curve->mMouseOverCurve = false;
      return;
    }

    // We need the position in local space in pixels
    Vec2 pixelPos = ToLocal(event->Position);

    // If the mouse is over a control point or tangent, it has priority
    if(curve->IsMouseOver(pixelPos) && mMouseOver.IsNull())
    {
      // Send an event if the mouse was not over the curve last frame
      if(curve->mMouseOverCurve == false)
        curve->DispatchCurveEvent(Events::MouseEnterCurve);

      // The mouse is now over us
      curve->mMouseOverCurve = true;
      event->Handled = true;
    }
    else
    {
      // Send an event if the mouse was last over the curve
      if(curve->mMouseOverCurve)
        curve->DispatchCurveEvent(Events::MouseExitCurve);

      curve->mMouseOverCurve = false;
    }
  }
}

//******************************************************************************
void CurveEditor::OnDoubleClick(MouseEvent* event)
{
  if(event->Handled)
    return;

  forRange(CurveObject* curve, mCurves.All())
  {
    // Get the position in graph coordinates
    Vec2 pixelPos = ToLocal(event->Position);
    if(curve->mMouseOverCurve)
    {
      Vec2 graphPos = ToGraphPosition(pixelPos);

      // Create the control point
      CurveEditing::ControlPoint* point = curve->AddNewControlPoint(graphPos);

      // Select it
      SelectDraggable(point);

      event->Handled = true;
      return;
    }
  }
}

//******************************************************************************
void CurveEditor::OnLeftDrag(MouseDragEvent* event)
{
  if(!event->Handled)
    new MultiSelectManipulation(event, this);
}

//******************************************************************************
void CurveEditor::OnKeyDown(KeyboardEvent* event)
{
  if(event->Key == Keys::Delete)
  {
    DeleteSelected();
  }
}

//******************************************************************************
void CurveEditor::OnLinearTangentsPressed(ObjectEvent* e)
{
  forRange(Draggable* draggable, mSelection.All())
  {
    // Control points always take priority here. If a control point is selected,
    // both tangents are modified regardless of whether or not they're selected
    if(ControlPoint* controlPoint = Type::DynamicCast<ControlPoint*>(draggable))
    {
      controlPoint->OnLinearTangents(e);
    }
    else
    {
      // It must be a tangent if not a control point
      Tangent* tangent = (Tangent*)draggable;

      // Only do it if the control point isn't selected (control point takes priority)
      if(!mSelection.Contains(tangent->mControlPoint))
        tangent->OnLinearPressed(e);
    }
  }
}

//******************************************************************************
void CurveEditor::OnSplitTangentsPressed(ObjectEvent* e)
{
  forRange(Draggable* draggable, mSelection.All())
  {
    // Control points always take priority here. If a control point is selected,
    // both tangents are modified regardless of whether or not they're selected
    if(ControlPoint* controlPoint = Type::DynamicCast<ControlPoint*>(draggable))
    {
      controlPoint->OnSplitTangents(e);
    }
    else
    {
      // It must be a tangent if not a control point
      Tangent* tangent = (Tangent*)draggable;

      // Only do it if the control point isn't selected (control point takes priority)
      if(!mSelection.Contains(tangent->mControlPoint))
        tangent->mControlPoint->OnSplitTangents(e);
    }
  }
}

//******************************************************************************
void CurveEditor::OnWeightedTangentsPressed(ObjectEvent* e)
{
  forRange(Draggable* draggable, mSelection.All())
  {
    // Control points always take priority here. If a control point is selected,
    // both tangents are modified regardless of whether or not they're selected
    if(ControlPoint* controlPoint = Type::DynamicCast<ControlPoint*>(draggable))
    {
      controlPoint->OnWeightedTangents(e);
    }
    else
    {
      // It must be a tangent if not a control point
      Tangent* tangent = (Tangent*)draggable;

      // Only do it if the control point isn't selected (control point takes priority)
      if(!mSelection.Contains(tangent->mControlPoint))
        tangent->OnWeightedPressed(e);
    }
  }
}

//******************************************************************************
bool CurveObject::SortByX::operator()(const ControlPoint* left, const ControlPoint* right)
{
  return left->mGraphPosition.x < right->mGraphPosition.x;
}

namespace CurveEditing
{

//--------------------------------------------------------- Curve Editor Toolbar
//******************************************************************************
CurveEditorToolbar::CurveEditorToolbar(Composite* parent, float textBoxWidth)
  : Composite(parent)
{
  SetLayout(CreateStackLayout(LayoutDirection::LeftToRight, Pixels(0, 0), Thickness(1,1,1,1)));

  Label* labelX = new Label(this);
  labelX->SetText("X:");
  mPositionX = new TextBox(this);
  mPositionX->SetSizing(SizeAxis::X, SizePolicy::Fixed, textBoxWidth);
  ConnectThisTo(mPositionX, Events::TextBoxChanged, OnXChanged);
  
  Label* labelY = new Label(this);
  labelY->SetText("Y:");
  mPositionY = new TextBox(this);
  mPositionY->SetSizing(SizeAxis::X, SizePolicy::Fixed, textBoxWidth);
  ConnectThisTo(mPositionY, Events::TextBoxChanged, OnYChanged);

  mSize.y = Pixels(20);

  mEditor = nullptr;
}

//******************************************************************************
void CurveEditorToolbar::UpdateTransform()
{
  Composite::UpdateTransform();
}

//******************************************************************************
void CurveEditorToolbar::Update()
{
  range selection = allControlPoints();
  bool empty = selection.Empty();

  // They should be editable if we have anything in the selection
  mPositionX->SetEditable(!empty);
  mPositionY->SetEditable(!empty);

  if(empty)
  {
    mPositionX->SetText(String());
    mPositionY->SetText(String());
    return;
  }

  Vec2 initial = selection.Front()->GetDisplayPosition();
  Math::BoolVec2 valid(true, true);
  selection.PopFront();
  forRange(Draggable* draggable, selection)
  {
    Vec2 current = draggable->GetDisplayPosition();

    if(valid.x && current.x != initial.x)
    {
      mPositionX->SetInvalid();
      valid.x = false;
    }

    if(valid.y && current.y != initial.y)
    {
      mPositionY->SetInvalid();
      valid.y = false;
    }
  }

  if(valid.x)
    mPositionX->SetText(mEditor->GraphToDisplayTextX(initial.x));

  if(valid.y)
    mPositionY->SetText(mEditor->GraphToDisplayTextY(initial.y));
}

//******************************************************************************
void CurveEditorToolbar::OnXChanged(Event* e)
{
  String text = mPositionX->GetText();
  forRange(Draggable* draggable, allControlPoints())
  {
    Vec2 pos = draggable->GetDisplayPosition();
    pos.x = mEditor->DisplayTextToGraphX(text);

    // Bring the position into pixel space
    pos = mEditor->ToPixelPosition(pos);
    draggable->MoveTo(pos, false);
  }
}

//******************************************************************************
void CurveEditorToolbar::OnYChanged(Event* e)
{
  String text = mPositionY->GetText();
  forRange(Draggable* draggable, allControlPoints())
  {
    Vec2 pos = draggable->GetDisplayPosition();
    pos.y = mEditor->DisplayTextToGraphY(text);

    // Bring the position into pixel space
    pos = mEditor->ToPixelPosition(pos);
    draggable->MoveTo(pos, false);
  }
}

//******************************************************************************
void CurveEditorToolbar::SetCurveEditor(CurveEditor* editor)
{
  ErrorIf(mEditor != nullptr, "Toolbar was registered twice.");
  mEditor = editor;
  ConnectThisTo(mEditor, Events::SelectionChanged, OnSelectionChanged);
}

//******************************************************************************
void CurveEditorToolbar::OnSelectionChanged(Event* e)
{
  Update();
}

//******************************************************************************
CurveEditorToolbar::range CurveEditorToolbar::allControlPoints()
{
  return range(mEditor->GetSelection());
}

//******************************************************************************
CurveEditorToolbar::range::range(CurveEditor::SelectionRange r)
  : mRange(r)
{
  FindNextControlPoint();
}

//******************************************************************************
Draggable* CurveEditorToolbar::range::Front()
{
  return mRange.Front();
}

//******************************************************************************
bool CurveEditorToolbar::range::Empty()
{
  return mRange.Empty();
}

//******************************************************************************
void CurveEditorToolbar::range::PopFront()
{
  mRange.PopFront();
  FindNextControlPoint();
}

//******************************************************************************
void CurveEditorToolbar::range::FindNextControlPoint()
{
  //forRange(Draggable* draggable, mRange)
  while(!mRange.Empty())
  {
    Draggable* draggable = mRange.Front();

    // Stop once we've hit a control point
    if(ZilchVirtualTypeId(draggable)->IsA(ZilchTypeId(ControlPoint)))
      return;

    mRange.PopFront();
  }
}

//------------------------------------------------------------- Drag Manipulator
class DragManipulator : public MouseManipulation
{
public:
  CurveEditor* mEditor;
  HashMap<Draggable*, Vec2> mOffsets;

  //****************************************************************************
  DragManipulator(Mouse* mouse, CurveEditor* editor)
    : MouseManipulation(mouse, editor->GetRootWidget())
  {
    mEditor = editor;

    Vec2 graphPixels = mEditor->ToLocal(mouse->GetClientPosition());

    forRange(Draggable* target, mEditor->GetSelection())
    {
      Vec2 center = ToVector2(target->GetTranslation()) + target->GetSize() * 0.5f;
      Vec2 offset = center - graphPixels;
      mOffsets.Insert(target, offset);
    }
  }

  //****************************************************************************
  void OnMouseMove(MouseEvent* e) override
  {
    // We want the position in our parents local space (the curve editor)
    Vec2 graphPixels = mEditor->ToLocal(e->Position);
    forRange(Draggable* target, mEditor->GetSelection())
    {
      Vec2 offset = mOffsets.FindValue(target, Vec2::cZero);
      target->MoveTo(graphPixels + offset, mEditor->mClampMouseDrag);
    }
  }

  //****************************************************************************
  void OnMouseUpdate(MouseEvent* e) override
  {
    mEditor->MouseDragUpdate(e);

    // We want the position in our parents local space (the curve editor)
    Vec2 graphPixels = mEditor->ToLocal(e->Position);
    forRange(Draggable* target, mEditor->GetSelection())
    {
      Vec2 offset = mOffsets.FindValue(target, Vec2::cZero);
      target->MoveTo(graphPixels + offset, mEditor->mClampMouseDrag);
    }
  }

  //****************************************************************************
  void OnMouseUp(MouseEvent* e) override
  {
    forRange(Draggable* target, mEditor->GetSelection())
    {
      target->FinishedDrag();
    }
    this->Destroy();
  }
};

//-------------------------------------------------------------------- Draggable
ZilchDefineType(Draggable, builder, type)
{
}

//******************************************************************************
Draggable::Draggable(CurveObject* curveObject) 
  : Widget(curveObject->mEditor)
{
  ConnectThisTo(this, Events::LeftClick, OnLeftClick);
  ConnectThisTo(this, Events::LeftMouseDown, OnMouseDown);
  ConnectThisTo(this, Events::LeftMouseDrag, OnMouseDrag);
  ConnectThisTo(this, Events::MouseEnter, OnMouseEnter);
  ConnectThisTo(this, Events::MouseExit, OnMouseExit);

  mCurve = curveObject;
  mEditor = curveObject->mEditor;
  mMouseOver = false;
}

//******************************************************************************
Draggable::~Draggable()
{
}

//******************************************************************************
bool Draggable::IsSelected()
{
  return mEditor->IsSelected(this);
}

//******************************************************************************
void Draggable::OnLeftClick(MouseEvent* e)
{  
  if(e->CtrlPressed && mEditor->IsSelected(this))
  {
    mEditor->DeSelect(this);
  }
  else
  {
    bool singleSelect = !e->CtrlPressed;
    mEditor->SelectDraggable(this, singleSelect);
  }

  e->Handled = true;
}

//******************************************************************************
void Draggable::OnMouseDown(MouseEvent* e)
{
  e->Handled = true;
}

//******************************************************************************
void Draggable::OnMouseDrag(MouseEvent* e)
{
  if(e->Handled)
    return;

  if(!IsSelected())
  {
    if(!e->CtrlPressed)
      mEditor->ClearSelection();
  }
  mEditor->SelectDraggable(this, false);
  StartDrag(e->GetMouse());
  e->Handled = true;
}

//******************************************************************************
void Draggable::OnMouseEnter(MouseEvent* e)
{
  mMouseOver = true;
  mEditor->mMouseOver = this;
}

//******************************************************************************
void Draggable::OnMouseExit(MouseEvent* e)
{
  mMouseOver = false;
  mEditor->mMouseOver = nullptr;
}

//******************************************************************************
void Draggable::StartDrag(Mouse* mouse)
{
  new DragManipulator(mouse, mEditor);
  mMouseOver = true;
}

//******************************************************************************
void Draggable::Select()
{
  mEditor->SelectDraggable(this, false);
}

//******************************************************************************
void Draggable::DeSelect()
{
  mEditor->DeSelect(this);
}

//---------------------------------------------------------------- Control Point
ZilchDefineType(ControlPoint, builder, type)
{

}

//******************************************************************************
ControlPoint::ControlPoint(CurveObject* curve, Vec2Param pos, 
                           Vec2Param tanIn, Vec2Param tanOut, uint editorFlags) 
  : Draggable(curve), mEditorFlags(editorFlags)
{
  mClientData = nullptr;

  SetNotInLayout(true);
  ConnectThisTo(this, Events::RightMouseUp, OnRightMouseUp);
  SetSize(Vec2(1,1) * CurveEditorUi::ControlPointSize * CurveEditorUi::SelectionScale);

  mGraphPosition = pos;
  mTangentIn = new Tangent(curve, this, tanIn, TangentSide::In);
  mTangentOut = new Tangent(curve, this, tanOut, TangentSide::Out);
}

//******************************************************************************
ControlPoint::~ControlPoint()
{
  mTangentIn->Destroy();
  mTangentOut->Destroy();
}

//******************************************************************************
void ControlPoint::UpdateTransform()
{
  bool tangentsVisible = TangentsVisible();
  mTangentIn->SetActive(tangentsVisible);
  mTangentOut->SetActive(tangentsVisible);

  SetSize(Vec2(1,1) * CurveEditorUi::ControlPointSize * CurveEditorUi::SelectionScale);

  Vec2 pixelPos = GetPixelPosition();
  Vec2 halfSize = Vec2(1,1) * CurveEditorUi::TangentSize * 0.5f * CurveEditorUi::SelectionScale;

  Vec2 inPos = pixelPos + mTangentIn->GetPixelDirection() - halfSize;
  mTangentIn->SetTranslation(ToVector3(inPos));
  Vec2 outPos = pixelPos + mTangentOut->GetPixelDirection() - halfSize;
  mTangentOut->SetTranslation(ToVector3(outPos));
  
  Draggable::UpdateTransform();
}

//******************************************************************************
bool ControlPoint::TangentsVisible()
{
  // Always visible if we're selected
  if(IsSelected())
    return true;

  return mTangentIn->IsSelected() || mTangentOut->IsSelected();
}

//******************************************************************************
void ControlPoint::SetGraphPosition(Vec2Param graphPosition)
{
  mGraphPosition = graphPosition;

  UpdateLinearTangents();
  UpdateNeighborLinearTangents();
  MarkAsNeedsUpdate();

  mCurve->Modified(this);
}

//******************************************************************************
Vec2 ControlPoint::GetGraphPosition()
{
  return mGraphPosition;
}

//******************************************************************************
void ControlPoint::SetPixelPosition(Vec2Param pixelPosition, bool clampToScreen)
{
  // Clamp the position to be inside the graph
  Vec2 pixelClamped = pixelPosition;
  if(clampToScreen)
    pixelClamped = mEditor->ClampPixelPosition(pixelPosition);

  Vec2 graphPos = mEditor->ToGraphPosition(pixelClamped);
  SetGraphPosition(graphPos);
}

//******************************************************************************
Vec2 ControlPoint::GetPixelPosition()
{
  return mEditor->ToPixelPosition(mGraphPosition);
}

//******************************************************************************
void ControlPoint::MoveTo(Vec2Param pixelPos, bool clampToScreen)
{
  SetPixelPosition(pixelPos, clampToScreen);
}

//******************************************************************************
void ControlPoint::FinishedDrag()
{
  mCurve->Modified(this);
}

//******************************************************************************
void ControlPoint::UpdateNeighborLinearTangents()
{
  // Update neighboring control point linear tangents
  if(ControlPoint* cp = GetNeighborControlPoint(1))
    cp->UpdateLinearTangents();
  if(ControlPoint* cp = GetNeighborControlPoint(-1))
    cp->UpdateLinearTangents();

  MarkAsNeedsUpdate();
}

//******************************************************************************
void ControlPoint::UpdateLinearTangents()
{
  // Do nothing if neither are linear
  if(!InIsLinear() && !OutIsLinear())
    return;

  // Get our index
  uint index = mCurve->mControlPoints.FindIndex(this);

  // Set the tangents
  Vec2 prev = mGraphPosition;
  Vec2 next = mGraphPosition;

  // Check for edge cases
  if(ControlPoint* cp = GetNeighborControlPoint(1))
    next = cp->GetGraphPosition();
  if(ControlPoint* cp = GetNeighborControlPoint(-1))
    prev = cp->GetGraphPosition();

  float inLength = mTangentIn->GetGraphDirection().Length();
  float outLength = mTangentOut->GetGraphDirection().Length();

  if(!TangentsSplit() && IsLinear())
  {
    mTangentIn->mGraphDirection = (prev - next).Normalized() * inLength;
    mTangentOut->mGraphDirection = (next - prev).Normalized() * outLength;
  }
  else
  {
    if(index != 0 && InIsLinear())
      mTangentIn->mGraphDirection = (prev - mGraphPosition).Normalized() * inLength;
    if(index != mCurve->mControlPoints.Size() - 1 && OutIsLinear())
      mTangentOut->mGraphDirection = (next - mGraphPosition).Normalized() * outLength;
  }

  MarkAsNeedsUpdate();
}

//******************************************************************************
void ControlPoint::Delete()
{
  mCurve->RemoveControlPoint(this);
}

//******************************************************************************
void ControlPoint::OnRightMouseUp(MouseEvent* event)
{
  if(event->Handled)
    return;

  if(!mEditor->mSelection.Contains(this))
  {
    mEditor->mSelection.Clear();
    mEditor->mSelection.Insert(this);
  }

  ContextMenu* menu = new ContextMenu(this);
  Mouse* mouse = Z::gMouse;
  menu->SetBelowMouse(mouse, Pixels(0,0));

  // Create the delete button
  ConnectMenu(menu, "Delete", OnDelete);

  // Create the split button
  cstr tangentName = "Split Tangents";
  if(TangentsSplit())
    tangentName = "Join Tangents";
  ConnectMenu(menu, tangentName, OnSplitTangents);

  // Create the linear button
  cstr linearTangentName = "Linear Tangents";
  if(InIsLinear() || OutIsLinear())
    linearTangentName = "Manual Tangents";
  ConnectMenu(menu, linearTangentName, OnLinearTangents);

  // Create the weighted button
  cstr itemWeightedName = "Weighted Tangents";
  if(IsWeighted())
    itemWeightedName = "Non Weighted Tangents";
  ConnectMenu(menu, itemWeightedName, OnWeightedTangents);

  // Only weighted tangents can be normalized
  if(IsWeighted())
    ConnectMenu(menu, "Normalize", OnNormalizeTangents);
  
  event->Handled = true;
}

//******************************************************************************
void ControlPoint::OnDelete(ObjectEvent* event)
{
  mEditor->DeleteSelected();
}

//******************************************************************************
void ControlPoint::OnSplitTangents(ObjectEvent* event)
{
  // If the tangents were split, make them collinear
  if(TangentsSplit())
  {
    mTangentIn->SetGraphDirection(-mTangentOut->GetGraphDirection());

    // They are being joined, so both should be on linear 
    // if either is already on linear
    if(InIsLinear() || OutIsLinear())
      mEditorFlags |= (CurveEditorFlags::LinearIn | CurveEditorFlags::LinearOut);
  }

  mEditorFlags ^= CurveEditorFlags::TangentsSplit;

  UpdateLinearTangents();
  mCurve->Modified(this);
  MarkAsNeedsUpdate();
}

//******************************************************************************
void ControlPoint::OnLinearTangents(ObjectEvent* event)
{
  if(InIsLinear() || OutIsLinear())
  {
    // Disable the linear flags
    mEditorFlags &= ~(CurveEditorFlags::LinearIn | CurveEditorFlags::LinearOut);
  }
  else
  {
    // Enable the linear flags
    mEditorFlags |= (CurveEditorFlags::LinearIn | CurveEditorFlags::LinearOut);
  }

  UpdateLinearTangents();
  mCurve->Modified(this);
  MarkAsNeedsUpdate();
}

//******************************************************************************
void ControlPoint::OnWeightedTangents(ObjectEvent* e)
{
  if(IsWeighted())
    mEditorFlags &= ~CurveEditorFlags::NonWeighted;
  else
    mEditorFlags |= CurveEditorFlags::NonWeighted;

  mCurve->Modified(this);
}

//******************************************************************************
void ControlPoint::OnNormalizeTangents(ObjectEvent* event)
{
  mTangentIn->Normalize();
  mTangentOut->Normalize();
  mCurve->Modified(this);
  MarkAsNeedsUpdate();
}

//******************************************************************************
Tangent* ControlPoint::GetOther(Tangent* tangent)
{
  if(tangent == mTangentIn)
    return mTangentOut;
  return mTangentIn;
}

//******************************************************************************
Vec2 ControlPoint::GetTangentIn()
{
  return mTangentIn->GetGraphDirection();
}

//******************************************************************************
void ControlPoint::SetTangentIn(Vec2Param direction)
{
  mTangentIn->SetGraphDirection(direction);
}

//******************************************************************************
Vec2 ControlPoint::GetTangentOut()
{
  return mTangentOut->GetGraphDirection();
}

//******************************************************************************
void ControlPoint::SetTangentOut(Vec2Param direction)
{
  mTangentOut->SetGraphDirection(direction);
}

//******************************************************************************

bool ControlPoint::TangentsSplit()
{
  return (mEditorFlags & CurveEditorFlags::TangentsSplit) > 0;
}

//******************************************************************************
bool ControlPoint::IsLinear()
{
  return InIsLinear() && OutIsLinear();
}

//******************************************************************************
bool ControlPoint::InIsLinear()
{
  return (mEditorFlags & CurveEditorFlags::LinearIn) > 0;
}

//******************************************************************************
bool ControlPoint::OutIsLinear()
{
  return (mEditorFlags & CurveEditorFlags::LinearOut) > 0;
}

//******************************************************************************
bool ControlPoint::IsWeighted()
{
  return (mEditorFlags & CurveEditorFlags::NonWeighted) > 0;
}

//******************************************************************************
ControlPoint* ControlPoint::GetNeighborControlPoint(int direction)
{
  uint index = mCurve->mControlPoints.FindIndex(this);
  index = uint((int)index + direction);

  if(index < mCurve->mControlPoints.Size())
    return mCurve->mControlPoints[index];
  return nullptr;
}

//---------------------------------------------------------------------- Tangent
ZilchDefineType(Tangent, builder, type)
{

}

//******************************************************************************
Tangent::Tangent(CurveObject* curve, ControlPoint* controlPoint, 
                 Vec2Param direction, TangentSide::Type side)
  : Draggable(curve)
{
  ConnectThisTo(this, Events::RightMouseUp, OnRightMouseUp);

  SetNotInLayout(true);
  SetSize(Vec2(1,1) * CurveEditorUi::TangentSize * CurveEditorUi::SelectionScale);
  mControlPoint = controlPoint;
  mSide = side;

  mGraphDirection = direction;
}

//******************************************************************************
void Tangent::SetGraphDirection(Vec2Param graphDirection)
{
  mGraphDirection = graphDirection;

  // Clamp the x position to the position of the control point
  if(mSide == TangentSide::In)
    mGraphDirection.x = Math::Min(mGraphDirection.x, 0.0f);
  else
    mGraphDirection.x = Math::Max(0.0f, mGraphDirection.x);

  // If the tangents aren't split, update the other tangent
  if(!mControlPoint->TangentsSplit())
  {
    Tangent* other = mControlPoint->GetOther(this);
    other->mGraphDirection = -mGraphDirection;

    // Clear both linear flags if it's not split
    u32 flagsToClear = (CurveEditorFlags::LinearIn | CurveEditorFlags::LinearOut);
    mControlPoint->mEditorFlags &= ~(flagsToClear);
  }
  else
  {
    // This tangent has moved, so disable the linear flag

    // Clear the linear in flag
    if(mSide == TangentSide::In && mControlPoint->InIsLinear())
      mControlPoint->mEditorFlags &= ~(CurveEditorFlags::LinearIn);
    // Clear the linear out flag
    else if(mSide == TangentSide::Out && mControlPoint->OutIsLinear())
      mControlPoint->mEditorFlags &= ~(CurveEditorFlags::LinearOut);
  }

  // Update the linear tangents on the control point
  mControlPoint->UpdateLinearTangents();

  // Mark us as modified
  mCurve->Modified(mControlPoint);
  MarkAsNeedsUpdate();
}

//******************************************************************************
Vec2 Tangent::GetGraphDirection()
{
  return mGraphDirection;
}

//******************************************************************************
void Tangent::SetPixelDirection(Vec2Param pixelDirection, bool clampToScreen)
{
  // Find the position of the tangent in pixels
  Vec2 cpPixelPos = mControlPoint->GetPixelPosition();
  Vec2 tangentPixelPos = cpPixelPos + pixelDirection;
  
  if(!IsWeighted())
  {
    const float cScalePercent = 1.0f / 3.0f;

    Vec2 normalizedPixelDirection = (tangentPixelPos - cpPixelPos);
    normalizedPixelDirection.AttemptNormalize();

    if(mSide == TangentSide::In)
    {
      if(pixelDirection.x >= 0.0f)
      {
        Vec2 newDirection = Vec2(0, -pixelDirection.y + 0.0001f).Normalized();
        SetGraphDirection(newDirection);
        return;
      }
    }
    else
    {
      if(pixelDirection.x <= 0.0f)
      {
        Vec2 newDirection = Vec2(0, -pixelDirection.y + 0.0001f).Normalized();
        SetGraphDirection(newDirection);
        return;
      }
    }

    int dir = (mSide == TangentSide::In) ? -1 : 1;
    if(ControlPoint* cp = mControlPoint->GetNeighborControlPoint(dir))
    {
      Vec2 otherCpPos = cp->GetPixelPosition();
      float extentPos = (otherCpPos.x * cScalePercent) +
                        (cpPixelPos.x * (1.0f - cScalePercent));
      float directionRatio = pixelDirection.y / pixelDirection.x;
      Vec2 newDirection = Vec2(extentPos - cpPixelPos.x, (extentPos - cpPixelPos.x) * directionRatio);
      tangentPixelPos = cpPixelPos + newDirection;
    }
  }
  
  // Convert from pixels to graph space
  Vec2 tangentGraphPos = mEditor->ToGraphPosition(tangentPixelPos);
  Vec2 cpGraphPos = mControlPoint->GetGraphPosition();

  // Set the final direction
  Vec2 graphDirection = (tangentGraphPos - cpGraphPos);
  SetGraphDirection(graphDirection);
}

//******************************************************************************
Vec2 Tangent::GetPixelDirection()
{
  // Get the position of the tangent in graph space
  Vec2 cpGraphPos = mControlPoint->GetGraphPosition();
  Vec2 tangentGraphPos = cpGraphPos + mGraphDirection;

  // Convert to pixel space to get the pixel direction
  Vec2 cpPixelPos = mControlPoint->GetPixelPosition();
  Vec2 tangentPixelPos = mEditor->ToPixelPosition(tangentGraphPos);

  Vec2 pixelDirection = (tangentPixelPos - cpPixelPos);

  if(!IsWeighted())
  {
    pixelDirection.AttemptNormalize();
    pixelDirection *= Pixels(70);
  }

  return pixelDirection;
}

//******************************************************************************
Vec2 Tangent::GetPixelPosition()
{
  Vec2 controlPointPixelPos = mControlPoint->GetPixelPosition();
  Vec2 pixelDirection = GetPixelDirection();
  return controlPointPixelPos + pixelDirection;
}

//******************************************************************************
void Tangent::MoveTo(Vec2Param pixelPos, bool clampToScreen)
{
  Vec2 cpPixelPos = mControlPoint->GetPixelPosition();
  Vec2 tangentPixelDirection = (pixelPos - cpPixelPos);

  SetPixelDirection(tangentPixelDirection);
}

//******************************************************************************
void Tangent::FinishedDrag()
{
  mCurve->Modified(mControlPoint);
}

//******************************************************************************
void Tangent::OnRightMouseUp(MouseEvent* e)
{
  ContextMenu* menu = new ContextMenu(this);
  Mouse* mouse = Z::gMouse;
  menu->SetBelowMouse(mouse, Pixels(0,0));

  // Create the split button
  ContextMenuItem* itemSplit = new ContextMenuItem(menu);
  if(mControlPoint->TangentsSplit())
    itemSplit->SetName("Join");
  else
    itemSplit->SetName("Split");
  Zero::Connect(itemSplit, Events::MenuItemSelected, mControlPoint, 
                &ControlPoint::OnSplitTangents);

  // Create the linear button
  cstr linearTangentName = "Linear Tangents";
  if(IsLinear())
    linearTangentName = "Manual Tangents";
  ConnectMenu(menu, linearTangentName, OnLinearPressed);

  // Create the weighted button
  cstr itemWeightedName = "Weighted Tangents";
  if(IsWeighted())
    itemWeightedName = "Non Weighted Tangents";
  ConnectMenu(menu, itemWeightedName, OnWeightedPressed);

  // Only weighted tangents can be normalized
  if(IsWeighted())
    ConnectMenu(menu, "Normalize", OnNormalize);
}

//******************************************************************************
void Tangent::OnNormalize(ObjectEvent* e)
{
  // Normalize ourself
  Normalize();

  // Normalize the other tangent if they aren't split
  if(!mControlPoint->TangentsSplit())
    mControlPoint->GetOther(this)->Normalize();

  mCurve->Modified(mControlPoint);
}

//******************************************************************************
void Tangent::OnLinearPressed(ObjectEvent* e)
{
  if(IsLinear())
  {
    if(mSide == TangentSide::In)
      mControlPoint->mEditorFlags &= ~CurveEditorFlags::LinearIn;
    else
      mControlPoint->mEditorFlags &= ~CurveEditorFlags::LinearOut;
  }
  else
  {
    if(mSide == TangentSide::In)
      mControlPoint->mEditorFlags |= CurveEditorFlags::LinearIn;
    else
      mControlPoint->mEditorFlags |= CurveEditorFlags::LinearOut;
  }

  mControlPoint->UpdateLinearTangents();
  mCurve->Modified(mControlPoint);
}

//******************************************************************************
void Tangent::OnWeightedPressed(ObjectEvent* e)
{
  mControlPoint->OnWeightedTangents(e);
  MarkAsNeedsUpdate();
}

//******************************************************************************
void Tangent::Normalize()
{
  mGraphDirection.Normalize();
  mGraphDirection *= 0.1f;
  MarkAsNeedsUpdate();
}

//******************************************************************************
bool Tangent::IsLinear()
{
  if(mSide == TangentSide::In)
    return mControlPoint->InIsLinear();
  return mControlPoint->OutIsLinear();
}

//******************************************************************************
bool Tangent::IsWeighted()
{
  return mControlPoint->IsWeighted();
}

}//namespace CurveEditing

}//namespace Zero
