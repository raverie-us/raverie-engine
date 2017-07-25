///////////////////////////////////////////////////////////////////////////////
///
/// \file AnimationGraph.Cpp
/// Implementation of the AnimationGraph Composite
///
/// Authors: Joshua Claeys
/// Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace AnimGraphUi
{
const cstr cLocation = "EditorUi/AnimationEditor/AnimGraph";
Tweakable(Vec4, BackgroundColor,   Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, GridLineColor,     Vec4(1,1,1,1), cLocation);
Tweakable(Vec4, GridHalfLineColor, Vec4(1,1,1,1), cLocation);
}

// The amount of room on the left of the graph reserved for the hash mark labels
const float cIndentSize = Pixels(30);

// The amount of padding when focusing on curves
const Vec2 cFocusPadding = Pixels(15, 15);

// The time taken to animate focusing on the curves
const float cFocusTime = 0.3f;

//******************************************************************************
void Expand(Vec2* min, Vec2* max, Vec2Param pos)
{
  *min = Math::Min(*min, pos);
  *max = Math::Max(*max, pos);
}

//------------------------------------------------------- Animation Curve Editor
//******************************************************************************
AnimationCurveEditor::AnimationCurveEditor(AnimationGraphEditor* parent)
  : CurveEditor(parent)
{
  mGraph = parent;
  mClampMouseDrag = false;
}

//******************************************************************************
void AnimationCurveEditor::GetSelectedControlPoints(HashSet<ControlPoint*>& set)
{
  forRange(Draggable* selected, mSelection.All())
  {
    if(ControlPoint* controlPoint = Type::DynamicCast<ControlPoint*>(selected))
    {
      set.Insert(controlPoint);
    }
    else
    {
      Tangent* tangent = (Tangent*)selected;
      set.Insert(tangent->mControlPoint);
    }
  }
}

//******************************************************************************
bool AnimationCurveEditor::GetSelectionAabb(Vec2* min, Vec2* max)
{
  // Need at least 2 control points in the selection
  static HashSet<ControlPoint*> sControlPoints;
  sControlPoints.Clear();

  GetSelectedControlPoints(sControlPoints);

  if(sControlPoints.Size() < 2)
    return false;

  // Expand the Aabb by the control points and their tangents
  forRange(ControlPoint* controlPoint, sControlPoints.All())
  {
    Vec2 graphPos = controlPoint->GetGraphPosition();

    // Expand by the display position of the tangents (accounts for
    // non-weighted tangents)
    Vec2 tanIn = ToGraphPosition(controlPoint->GetPixelPosition() +
                                 controlPoint->mTangentIn->GetPixelDirection());
    Vec2 tanOut = ToGraphPosition(controlPoint->GetPixelPosition() +
                                controlPoint->mTangentOut->GetPixelDirection());
    Expand(min, max, graphPos);

    if(controlPoint->mCurve->mControlPoints.Front() != controlPoint)
      Expand(min, max, tanIn);
    if(controlPoint->mCurve->mControlPoints.Back() != controlPoint)
      Expand(min, max, tanOut);
  }

  return true;
}

//******************************************************************************
Vec2 AnimationCurveEditor::ToPixelPosition(Vec2 graphPos)
{
  return mGraph->ToPixelPosition(graphPos);
}

//******************************************************************************
Vec2 AnimationCurveEditor::ToGraphPosition(Vec2 pixelPos)
{
  return mGraph->ToGraphPosition(pixelPos);
}

//******************************************************************************
Vec2 AnimationCurveEditor::ClampPixelPosition(Vec2 pixelPos)
{
  // First clamp it so that the point cannot be dragged outside of
  // the widgets area
  Vec2 clampedPixelPos;
  clampedPixelPos.x = Math::Clamp(pixelPos.x, 0.0f, mSize.x);
  clampedPixelPos.y = Math::Clamp(pixelPos.y, 0.0f, mSize.y);

  // Now make sure the position in graph space is valid
  Vec2 graphPos = ToGraphPosition(clampedPixelPos);
  graphPos.x = Math::Max(graphPos.x, 0.0f);

  // Have to go back to pixel position
  return ToPixelPosition(graphPos);
}

//******************************************************************************
void AnimationCurveEditor::MouseDragUpdate(MouseEvent* e)
{
  Vec2 local = ToLocal(e->Position);

  const float cDragSize = Pixels(3);
  const float maxSpeed = Pixels(3);
  Vec2 scroll = Vec2::cZero;

  if(local.x < cDragSize)
  {
    float percent = 1.0f - (local.x / cDragSize);
    scroll.x = -percent * maxSpeed;
  }
  else if(local.x > mSize.x - cDragSize)
  {
    float percent = 1.0f - (mSize.x - local.x) / cDragSize;
    scroll.x = percent * maxSpeed;
  }

  if(local.y < cDragSize)
  {
    float percent = 1.0f - (local.y / cDragSize);
    scroll.y = -percent * maxSpeed;
  }
  else if(local.y > mSize.y - cDragSize)
  {
    float percent = 1.0f - (mSize.y - local.y) / cDragSize;
    scroll.y = percent * maxSpeed;
  }

  scroll.x *= -1.0f;
  Math::Clamp(&scroll, -maxSpeed, maxSpeed);
  mGraph->mGraphData->ScrollPixels(scroll);
}

//******************************************************************************
String AnimationCurveEditor::GraphToDisplayTextX(float graphValue)
{
  // We want the edit text to be in frames, not time
  AnimationSettings* settings = mGraph->mEditor->GetSettings();
  return CurveEditor::GraphToDisplayTextX(settings->mEditFps * graphValue);
}

//******************************************************************************
float AnimationCurveEditor::DisplayTextToGraphX(StringParam displayText)
{
  // We want the edit text to be in frames, not time
  AnimationSettings* settings = mGraph->mEditor->GetSettings();

  float position;
  ToValue(displayText, position);
  return position / settings->mEditFps;
}

//------------------------------------------------------- Animation Curve Object
//******************************************************************************
AnimationCurveObject::AnimationCurveObject(AnimationGraphEditor* editor,
                                           TrackNode* track)
  : CurveObject(editor->mCurveEditor)
{
  mGraph = editor;
  mIgnoreAnimationEvents = false;
  mTrack = track;

  SetCurveColor(Color::Green);

  BuildCurve(mTrack);

  ConnectThisTo(this, Events::ControlPointAdded, OnControlPointAdded);
  ConnectThisTo(this, Events::ControlPointModified, OnControlPointModified);
  ConnectThisTo(this, Events::ControlPointDeleted, OnControlPointDeleted);

  ConnectThisTo(mTrack, Events::KeyFrameAdded, OnKeyFrameAdded);
  ConnectThisTo(mTrack, Events::KeyFrameModified, OnKeyFrameModified);
  ConnectThisTo(mTrack, Events::KeyFrameDeleted, OnKeyFrameDeleted);
  ConnectThisTo(mTrack, Events::TrackDeleted, OnTrackDeleted);
}

//******************************************************************************
void AnimationCurveObject::GetCurve(Vec3Array& curve)
{
  float editFps = mGraph->mEditor->GetSettings()->mEditFps;

  // We want to display the final baked curve
  Array<TrackNode::KeyEntry> bakedCurve;
  mTrack->BakeKeyFrames(bakedCurve);

  /// Convert to the format the curve editor wants
  curve.Reserve(bakedCurve.Size());
  forRange(TrackNode::KeyEntry entry, bakedCurve.All())
  {
    float time = entry.first;
    float value = entry.second.Get<float>();

    curve.PushBack(Vec3(time, value, 0));
  }
}

//******************************************************************************
TrackNode* AnimationCurveObject::GetTrack()
{
  return mTrack;
}

//******************************************************************************
void AnimationCurveObject::BuildCurve(TrackNode* track)
{
  forRange(KeyFrame* keyFrame, track->mKeyFrames.AllValues())
  {
    CreateControlPoint(keyFrame);
  }
}

//******************************************************************************
void AnimationCurveObject::CreateControlPoint(KeyFrame* keyFrame)
{
  mIgnoreAnimationEvents = true;
  // Pull data out from the key frame
  Vec2 pos = Vec2(keyFrame->GetTime(), keyFrame->GetValue().Get<float>());
  Vec2 tanIn = keyFrame->GetTangentIn();
  Vec2 tanOut = keyFrame->GetTangentOut();
  CurveEditing::CurveEditorFlags::Type flags = keyFrame->mEditorFlags;

  // Create the control point
  ControlPoint* cp = CurveObject::CreateControlPoint(pos, tanIn, tanOut, flags);
  cp->mClientData = (void*)keyFrame;
  cp->mEditorFlags = flags;

  // Add it to the map so we can look it up later
  mKeysToControlPoints.Insert(keyFrame, cp);

  mEditor->MarkAsNeedsUpdate();
  mIgnoreAnimationEvents = false;
}

//******************************************************************************
void AnimationCurveObject::OnControlPointAdded(CurveEvent* e)
{
  if(mIgnoreAnimationEvents)
    return;

  // We can ignore any changes to the animation because we're making them
  mIgnoreAnimationEvents = true;

  // Pull out data from the control point
  ControlPoint* controlPoint = e->mControlPoint;
  float time = controlPoint->mGraphPosition.x;
  Any value = controlPoint->mGraphPosition.y;

  // Create the key frame
  KeyFrame* keyFrame = mTrack->CreateKeyFrame(time, value);
  ErrorIf(keyFrame == NULL, "Failed to create key frame.");

  // Set up the client data on the control point in the curve editor
  controlPoint->mClientData = keyFrame;

  // Update the tangents of the control point to the default key frame tangents
  controlPoint->SetTangentIn(keyFrame->GetTangentIn());
  controlPoint->SetTangentOut(keyFrame->GetTangentOut());
  controlPoint->mEditorFlags = keyFrame->mEditorFlags;

  // Add it to the map so we can look it up later
  mKeysToControlPoints.Insert(keyFrame, controlPoint);

  mIgnoreAnimationEvents = false;

  mEditor->MarkAsNeedsUpdate();
}

//******************************************************************************
void AnimationCurveObject::OnControlPointModified(CurveEvent* e)
{
  if(mIgnoreAnimationEvents)
    return;

  // We can ignore any changes to the animation because we're making them
  mIgnoreAnimationEvents = true;

  // Pull out data from the control point
  ControlPoint* controlPoint = e->mControlPoint;
  float time = controlPoint->mGraphPosition.x;
  time = Math::Max(0.0f, time);
  controlPoint->mGraphPosition.x = time;
  Any value = controlPoint->mGraphPosition.y;
  Vec2 tanIn = controlPoint->GetTangentIn();
  Vec2 tanOut = controlPoint->GetTangentOut();

  // Get the animation key frame from curves control point
  KeyFrame* keyFrame = (KeyFrame*)controlPoint->mClientData;
  ErrorIf(keyFrame == NULL, "Invalid client data on curve.");
  keyFrame->SetTime(time);
  keyFrame->SetValue(value);
  keyFrame->SetTangents(tanIn, tanOut);
  keyFrame->mEditorFlags = controlPoint->mEditorFlags;

  mIgnoreAnimationEvents = false;

  mEditor->MarkAsNeedsUpdate();
}

//******************************************************************************
void AnimationCurveObject::OnControlPointDeleted(CurveEvent* event)
{
  // Do nothing if we're the ones modifying the curve
  if(mIgnoreAnimationEvents)
    return;

  // We can ignore any changes to the animation because we're making them
  mIgnoreAnimationEvents = true;

  // Get the key frame from the deleted control point
  KeyFrame* keyFrame = (KeyFrame*)event->mControlPoint->mClientData;
  event->mControlPoint->mClientData = NULL;

  // Destroy the key frame
  keyFrame->Destroy();

  mIgnoreAnimationEvents = false;

  mEditor->MarkAsNeedsUpdate();
}

//******************************************************************************
void AnimationCurveObject::OnKeyFrameAdded(KeyFrameEvent* event)
{
  // Do nothing if we're the ones modifying the curve
  if(mIgnoreAnimationEvents)
    return;

  // Create a control point for the key frame
  CreateControlPoint(event->mKeyFrame);
}

//******************************************************************************
void AnimationCurveObject::OnKeyFrameModified(KeyFrameEvent* event)
{
  // Do nothing if we're the ones modifying the curve
  if(mIgnoreAnimationEvents)
    return;

  KeyFrame* keyFrame = event->mKeyFrame;

  // Find the control point associated with the key frame that was modified
  ControlPoint* controlPoint = mKeysToControlPoints.FindValue(keyFrame, NULL);

  ErrorIf(controlPoint == NULL, "Modified key frame does not correspond to a "
                                "control point in the curve editor.");

  // Set the position of the key frame
  float x = keyFrame->GetTime();
  float y = keyFrame->GetValue().Get<float>();
  Vec2 position(x, y);

  mIgnoreAnimationEvents = true;
  controlPoint->SetGraphPosition(position);
  controlPoint->UpdateLinearTangents();
  controlPoint->UpdateNeighborLinearTangents();
  mIgnoreAnimationEvents = false;

  Sort(mControlPoints.All(), SortByX());
  
  // No need to grab tangents as the curve editor is the only way
  // to modify the tangents

  mEditor->MarkAsNeedsUpdate();
}

//******************************************************************************
void AnimationCurveObject::OnKeyFrameDeleted(KeyFrameEvent* event)
{
  // Do nothing if we're the ones modifying the curve
  if(mIgnoreAnimationEvents)
    return;

  KeyFrame* keyFrame = event->mKeyFrame;

  // Find the control point corresponding to the given control point
  ControlPoint* cp = mKeysToControlPoints.FindValue(keyFrame, NULL);

  ErrorIf(cp == NULL, "No control point for the deleted key frame.");

  // Destroy and remove it
  mKeysToControlPoints.Erase(keyFrame);

  // Deleting the control point will send an event saying that the curve
  // was modified, so lets ignore that event.
  mIgnoreAnimationEvents = true;
  cp->Delete();
  mIgnoreAnimationEvents = false;

  mEditor->MarkAsNeedsUpdate();
}

//******************************************************************************
void AnimationCurveObject::OnTrackDeleted(TrackEvent* event)
{
  Destroy();
}

//------------------------------------------------------------------ Graph Lines
class GraphDrawer : public Widget
{
public:
  //****************************************************************************
  GraphDrawer(Composite* parent, AnimationGraphEditor* graph)
    : Widget(parent)
  {
    mScrub = NULL;
    mGraph = graph;
    mGraphData = graph->mGraphData;
    mFont = FontManager::GetInstance()->GetRenderFont(cTextFont, 11, 0);
  }

  void SetScrubber(AnimationScrubber* scrubber)
  {
    mScrub = scrubber;
  }

  //****************************************************************************
  //void Draw(DisplayRender* render, Mat4Param parentTx, ColorTransform& colorTx,
  //          DrawParams& params) override
  //{
  //  DrawVerticalLines(render);
  //  DrawHorizontalLines(render);

  //  if(mGraph->mEnabled)
  //  {
  //    DrawGhostPlayHead(render);
  //    DrawPlayHead(render);
  //  }
  //}

  void RenderUpdate(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, Rect clipRect)
  {
    Widget::RenderUpdate(viewBlock, frameBlock, parentTx, colorTx, clipRect);

    Array<StreamedVertex> lines;

    DrawVerticalLines(viewBlock, frameBlock, clipRect, lines);
    DrawHorizontalLines(viewBlock, frameBlock, clipRect, lines);

    if (mGraph->mEnabled)
    {
      DrawGhostPlayHead(viewBlock, frameBlock, clipRect, lines);
      DrawPlayHead(viewBlock, frameBlock, clipRect, lines);
    }

    CreateRenderData(viewBlock, frameBlock, clipRect, lines, PrimitiveType::Lines);
  }

  //****************************************************************************
  void DrawVerticalLines(ViewBlock& viewBlock, FrameBlock& frameBlock, Rect clipRect, Array<StreamedVertex>& lines)
  {
    Vec4 color = AnimGraphUi::GridLineColor;

    float startPos = mGraphData->ToPixels(Vec2::cZero).x + Pixels(30);
    startPos = Math::Max(startPos, Pixels(30));

    // A vertical line across on the left
    lines.PushBack(StreamedVertex(Vec3(startPos,0,0), Vec2(0, 0), color));
    lines.PushBack(StreamedVertex(Vec3(startPos,mSize.y,0), Vec2(0, 0), color));

    // Draw the full vertical lines
    forRange(auto entry, mGraphData->GetWidthHashes(mSize.x))
    {
      if(entry.Position < 0.0f)
        continue;
      Vec3 start = ToVector3(mGraph->ToPixelPosition(Vec2(entry.Position, 0)));
      if(start.x < cIndentSize)
        continue;
      start.y = 0;
      Vec3 end = start + Vec3(0, mSize.y, 0);

      lines.PushBack(StreamedVertex(SnapToPixels(start), Vec2(0, 0), color));
      lines.PushBack(StreamedVertex(SnapToPixels(end), Vec2(0, 0), color));
    }

    // Draw the half vertical lines
    color = AnimGraphUi::GridHalfLineColor;
    forRange(auto entry, mGraphData->GetWidthHashes(mSize.x, true))
    {
      if(entry.Position < 0.0f)
        continue;
      Vec3 start = ToVector3(mGraph->ToPixelPosition(Vec2(entry.Position, 0)));
      if(start.x < cIndentSize)
        continue;
      start.y = 0;
      Vec3 end = start + Vec3(0, mSize.y, 0);

      lines.PushBack(StreamedVertex(SnapToPixels(start), Vec2(0, 0), color));
      lines.PushBack(StreamedVertex(SnapToPixels(end), Vec2(0, 0), color));
    }
  }

  //****************************************************************************
  void DrawHorizontalLines(ViewBlock& viewBlock, FrameBlock& frameBlock, Rect clipRect, Array<StreamedVertex>& lines)
  {
    Vec4 color = AnimGraphUi::GridLineColor;

    float startPos = mGraphData->ToPixels(Vec2::cZero).x + Pixels(30);
    startPos = Math::Max(startPos, Pixels(30));

    // Draw the full horizontal lines
    forRange(auto entry, mGraphData->GetHeightHashes(mSize.y))
    {
      Vec3 start = ToVector3(mGraph->ToPixelPosition(Vec2(0, entry.Position)));
      start.x = startPos;
      Vec3 end = start + Vec3(mSize.x, 0, 0);

      lines.PushBack(StreamedVertex(SnapToPixels(start), Vec2(0, 0), color));
      lines.PushBack(StreamedVertex(SnapToPixels(end), Vec2(0, 0), color));
    }

    // Draw the half horizontal lines
    color = AnimGraphUi::GridHalfLineColor;
    forRange(auto entry, mGraphData->GetHeightHashes(mSize.y, true))
    {
      Vec3 start = ToVector3(mGraph->ToPixelPosition(Vec2(0, entry.Position)));
      start.x = startPos;
      Vec3 end = start + Vec3(mSize.x, 0, 0);

      lines.PushBack(StreamedVertex(SnapToPixels(start), Vec2(0, 0), color));
      lines.PushBack(StreamedVertex(SnapToPixels(end), Vec2(0, 0), color));
    }

    ViewNode& viewNode = AddRenderNodes(viewBlock, frameBlock, clipRect, mFont->mTexture);
    FontProcessor fontProcessor(frameBlock.mRenderQueues, &viewNode, ToFloatColor(AnimationConstants::cHashColor));

    // Draw labels
    forRange(auto entry, mGraphData->GetHeightHashes(mSize.y))
    {
      Vec2 textPos = mGraph->ToPixelPosition(Vec2(0, entry.Position));

      Vec2 textSize = mFont->MeasureText(entry.Label);

      // Random positioning that looked nice
      textPos.x = startPos - textSize.x - Pixels(6);
      textPos.y += Pixels(-7);

      ProcessTextRange(fontProcessor, mFont, entry.Label, textPos, TextAlign::Left, Vec2(1, 1), textSize);
    }
  }

  //****************************************************************************
  void DrawGhostPlayHead(ViewBlock& viewBlock, FrameBlock& frameBlock, Rect clipRect, Array<StreamedVertex>& lines)
  {
    if(!mScrub->mShowGhostPlayHead)
      return;

    float localX = mScrub->ToPixels(mScrub->mGhostPlayHead);

    Vec4 color = AnimScrubberUi::GhostPlayHeadColor;

    lines.PushBack(StreamedVertex(SnapToPixels(Vec3(localX, Pixels(1), 0)), Vec2(0, 0), color));
    lines.PushBack(StreamedVertex(SnapToPixels(Vec3(localX, mSize.y,   0)), Vec2(0, 0), color));
  }

  //****************************************************************************
  void DrawPlayHead(ViewBlock& viewBlock, FrameBlock& frameBlock, Rect clipRect, Array<StreamedVertex>& lines)
  {
    float localX = mScrub->ToPixels(mScrub->GetPlayHead());

    Vec4 color = AnimScrubberUi::PlayHeadColor;

    lines.PushBack(StreamedVertex(SnapToPixels(Vec3(localX, Pixels(1), 0)), Vec2(0, 0), color));
    lines.PushBack(StreamedVertex(SnapToPixels(Vec3(localX, mSize.y,   0)), Vec2(0, 0), color));
  }

  RenderFont* mFont;
  AnimationGraphEditor* mGraph;
  ScrollingGraph* mGraphData;
  AnimationScrubber* mScrub;
};

//------------------------------------------------------------ Graph Manipulator
//******************************************************************************
class GraphManipulator : public MouseManipulation
{
public:
  AnimationGraphEditor* mGraph;

  //****************************************************************************
  GraphManipulator(Mouse* mouse, AnimationGraphEditor* graph)
    : MouseManipulation(mouse, graph)
  {
    mGraph = graph;
  }

  //****************************************************************************
  void OnMouseMove(MouseEvent* event) override
  {
    mGraph->mGraphData->ScrollPixels(Vec2(event->Movement.x, -event->Movement.y));
    mGraph->MarkAsNeedsUpdate();
  }

  //****************************************************************************
  void OnMiddleMouseUp(MouseEvent* event) override
  {
    mGraph->mEditor->TakeFocus();
    this->Destroy();
  }
};

//-------------------------------------------------------------- Animation Graph
//******************************************************************************
AnimationGraphEditor::AnimationGraphEditor(Composite* parent,
                             AnimationEditor* editor, ScrollingGraph* graphData)
  : Composite(parent)
{
  mScrubber = NULL;
  mEditor = editor;
  mEditorData = NULL;
  mGraphData = graphData;

  mBackground = CreateAttached<Element>(cWhiteSquareWithBorder);
  mGraphDrawer = new GraphDrawer(this, this);

  mCurveEditor = new AnimationCurveEditor(this);

  mNegativeArea = mCurveEditor->CreateAttached<Element>(cWhiteSquare);

  mEnabled = false;
  ConnectThisTo(this, Events::MiddleMouseDown, OnMiddleMouseDown);
  ConnectThisTo(this, Events::MouseScroll, OnMouseScroll);
  ConnectThisTo(mNegativeArea, Events::MouseScroll, OnMouseScrollNegativeArea);
  ConnectThisTo(mCurveEditor, Events::MouseMove, OnMouseMove);
  ConnectThisTo(this, Events::KeyDown, OnKeyDown);
  ConnectThisTo(this, Events::MouseEnter, OnMouseEnter);
}

//******************************************************************************
void AnimationGraphEditor::SetScrubber(AnimationScrubber* scrubber)
{
  mScrubber = scrubber;
  mGraphDrawer->SetScrubber(scrubber);
}

//******************************************************************************
void AnimationGraphEditor::SetToolBox(AnimationToolBox* toolBox)
{
  mCurveEditor->RegisterToolBar(toolBox->mCurveToolBar);
  mCurveEditor->RegisterLinearTangentsButton(toolBox->mLinearTangents);
  mCurveEditor->RegisterSplitTangentsButton(toolBox->mSplitTangents);
  mCurveEditor->RegisterWeightedTangentsButton(toolBox->mWeightedTangents);
}

//******************************************************************************
void AnimationGraphEditor::SetAnimationEditorData(AnimationEditorData* editorData)
{
  mEditorData = editorData;

  DisconnectAll(mEditorData, this);
  ConnectThisTo(mEditorData, Events::TrackSelectionModified, OnSelectionModified);

  RebuildCurves();
}

//******************************************************************************
void AnimationGraphEditor::UpdateTransform()
{
  mBackground->SetSize(mSize);
  mBackground->SetColor(AnimGraphUi::BackgroundColor);
  mGraphDrawer->SetSize(mSize);
  mCurveEditor->SetSize(mSize);
  SetClipping(true);
  Composite::UpdateTransform();
}

//******************************************************************************
void AnimationGraphEditor::Hide()
{
  mEnabled = false;
  mCurveEditor->Hide();
  mCurveEditor->ClearSelection();
}

//******************************************************************************
void AnimationGraphEditor::Show()
{
  mEnabled = true;
  mCurveEditor->Show();
}

//******************************************************************************
Vec2 AnimationGraphEditor::ToPixelPosition(Vec2Param graphPos)
{
  // This is with [0,0] in the upper left corner
  Vec2 pixelPos = mGraphData->ToPixels(graphPos);

  // We want [0,0] to be in the lower left corner
  pixelPos.y = mSize.y - pixelPos.y;

  return pixelPos + Vec2(cIndentSize,0);
}

//******************************************************************************
Vec2 AnimationGraphEditor::ToGraphPosition(Vec2Param pixelPos)
{
  // The given pixel position's origin is in the upper left corner, so we need
  // to convert it to where the origin is in the lower left corner (flip the y)
  Vec2 truePixelPos = pixelPos;
  truePixelPos.y = mSize.y - truePixelPos.y;
  return mGraphData->ToGraphSpace(truePixelPos - Vec2(cIndentSize,0));
}

//******************************************************************************
bool ExpandAabbByTrack(Vec2* min, Vec2* max, TrackNode* track)
{
  ErrorIf(track->mPropertyTypeId != ZilchTypeId(float), "Invalid track type.");

  if(track->mKeyFrames.Empty())
    return false;

  for(uint i = 0; i < track->mKeyFrames.Size(); ++i)
  {
    KeyFrame* keyFrame = track->mKeyFrames[i].second;

    float x = keyFrame->GetTime();
    float y = keyFrame->GetValue().Get<float>();

    Vec2 tanIn = keyFrame->GetTangentIn();
    Vec2 tanOut = keyFrame->GetTangentOut();

    // We want to ignore the tangent in on the first key frame, and the tangent
    // out on the last key frame
    if(i == 0)
      tanIn = Vec2::cZero;
    if(i == track->mKeyFrames.Size() - 1)
      tanOut = Vec2::cZero;

    // If we expand the aabb by all control points and the tangents of the
    // control points, we're guaranteed to contain the entire curve
    Vec2 controlPoint(x, y);
    Expand(min, max, controlPoint);
    Expand(min, max, controlPoint + tanIn);
    Expand(min, max, controlPoint + tanOut);
  }

  return true;
}

//******************************************************************************
void AnimationGraphEditor::FocusOnSelectedCurves(IntVec2Param axes)
{
  // Build an Aabb around all selected tracks
  Vec2 min(Math::cInfinite, Math::cInfinite);
  Vec2 max(-Math::cInfinite, -Math::cInfinite);

  // First attempt to get an aabb from the selected key frames
  bool validAabb = mCurveEditor->GetSelectionAabb(&min, &max);
  
  // If there were no selected key frames, focus on the selected tracks
  if(!validAabb)
  {
    forRange(TrackNode* track, mEditorData->mVisiblePropertyTracks.All())
    {
      // Expand the aabb
      if(track->mPropertyTypeId == ZilchTypeId(float))
        validAabb |= ExpandAabbByTrack(&min, &max, track);
    }
  }

  min.x = Math::Max(0.0f, min.x);

  // If there is an area to zoom in on, zoom
  if(validAabb)
  {
    // If it's a straight horizontal line, expand the aabb a little by a set amount
    if(max.y - min.y == 0.0f)
    {
      max.y += 1.0f;
      min.y -= 1.0f;
    }

    // Same for if it's a single key frame (no width)
    if(max.x - min.x == 0.0f)
    {
      max.x += 1.0f;
      min.x = 0.0f;
    }

    // The graph needs an actions object to animate properties
    mGraphData->mActions = GetActions();

    // We're using 30 pixels for the indent area
    Vec2 size = mSize - Vec2(cIndentSize, 0);

    // Focus on the Aabb
    mGraphData->Frame(min, max, size, axes, cFocusPadding, cFocusTime);
  }
}

//******************************************************************************
/// This function attempts to generate similar, yet contrasting colors.
/// This is accomplished by walking the Hue (HSV) around a circle,
/// alternating between rotating slightly, and jumping across the circle.
/// It will also slightly modify the saturation and value (HSV) of the 
/// color for every other index.
ByteColor GetNextCurveColor(uint index)
{
  // The amount to rotate for every other index
  const float cRotateOffset = Math::RadToDeg(Math::cPi * 0.3f);
  const float cSaturationSwitch = 0;
  const float cValueSwitch = -0;

  // The starting values of HSV. I wanted the first color to be similar
  // to the Zero Engine orange, so these initial values start us there
  float hue = 26;
  float saturation = 51;
  float value = 97;

  // This is completely hard coded to the numbers above. I didn't like the
  // yellow color generated when the index is 3, but I liked the colors all
  // around it, so I'm simply skipping this index. The algorithm may
  // eventually get back to this color, but what's important is that it
  // doesn't show up early on
  if(index >= 3)
    index += 1;

  for(uint i = 0; i < index; ++i)
  {
    if(i % 2 == 0)
    {
      // Jump across the color wheel
      hue += 180.0f;

      // Alternate saturation and value
      saturation += cSaturationSwitch;
      value += cValueSwitch;
    }
    else
    {
      // Rotate slightly
      hue += cRotateOffset;

      // Alternate saturation and value
      saturation -= cSaturationSwitch;
      value -= cValueSwitch;
    }
  }

  // Bring the hue back to [0-360]
  hue = Math::FMod(hue, 360.0f);

  // Convert to HSV 
  Vec4 color = HSVToFloatColor(hue / 360.0f, saturation / 100.0f, value / 100.0f);
  return ToByteColor(color);
}

//******************************************************************************
ByteColor AnimationGraphEditor::GetTrackColor(TrackNode* track,
                              HashSet<ByteColor>& takenColors, uint& colorIndex)
{
  ByteColor color;

  /// Assign set colors for vector type tracks
  if(track->Name == "X" || track->Name == "R")
    color = ByteColorRGBA(240, 73, 80, 255);
  else if(track->Name == "Y" || track->Name == "G")
    color = ByteColorRGBA(112, 246, 79, 255);
  else if(track->Name == "Z" || track->Name == "B")
    color = ByteColorRGBA(50, 118, 181, 255);
  else if(track->Name == "W" || track->Name == "A")
    color = ByteColorRGBA(255, 255, 255, 255);
  else
  {
    // If it was already assigned a color, use that instead
    if(track->mDisplayColor.LengthSq() != 0.0f)
      color = ToByteColor(track->mDisplayColor);
    // Otherwise, Assign it the next color
    else
      color = GetNextCurveColor(colorIndex++);

    // If the color is already taken, continue to check for a new color
    while(takenColors.Count(color) > 0)
      color = GetNextCurveColor(colorIndex++);
  }

  return color;
}

//******************************************************************************
void AnimationGraphEditor::RebuildCurves()
{
  // Pull out all the existing curve editors
  uint curveCount = mCurveEditor->mCurves.Size();

  /// Copy all existing editors in a list. As we walk through all the selected
  /// tracks, if they already have an editor, remove it from this extra list.
  /// Once we're done, any left in this list can be deleted
  Array<AnimationCurveObject*> existingCurveObjects(curveCount);
  for(uint i = 0; i < curveCount; ++i)
  {
    CurveObject* curveObject = mCurveEditor->mCurves[i];
    existingCurveObjects[i] = static_cast<AnimationCurveObject*>(curveObject);
  }

  // Used to keep track of which colors have been taken so that we don't
  // have two curves with the same exact color
  // NOTE* This only checks if the colors are exactly the same, and could
  // be extended to check instead if the colors are within a tolerance
  HashSet<ByteColor> takenColors;

  uint colorIndex = 0;

  forRange(TrackNode* track, mEditorData->mVisiblePropertyTracks.All())
  {
    BoundType* keyType = track->mPropertyTypeId;

    // Only create graphs for tracks whose key values are floats
    if(keyType != ZilchTypeId(float))
      continue;

    AnimationCurveObject* curveObject = NULL;

    // Check to see if this track already existed
    for(uint i = 0; i < existingCurveObjects.Size(); ++i)
    {
      AnimationCurveObject* currentCurve = existingCurveObjects[i];
      if(currentCurve->GetTrack() == track)
      {
        curveObject = currentCurve;

        // Remove it from the list as we're going to remove all curve
        // editors remaining in the list at the end
        existingCurveObjects.EraseAt(i);
        break;
      }
    }

    // If we didn't find a curve editor that already existed for this track,
    // we have to create a new one
    if(curveObject == NULL)
    {
      // Create the curve editor
      curveObject = new AnimationCurveObject(this, track);

      // We want to know when the mouse enters and exits the curve to update
      // the status text to display what curve the mouse is over
      ConnectThisTo(curveObject, Events::MouseEnterCurve, OnMouseEnterCurve);
      ConnectThisTo(curveObject, Events::MouseExitCurve, OnMouseExitCurve);
    }

    // Assign the track a color
    ByteColor color = GetTrackColor(track, takenColors, colorIndex);

    // Mark the chosen color as taken
    takenColors.Insert(color);

    // Store the color on the track
    track->mDisplayColor = ToFloatColor(color);

    // Set the color of the curve editor
    curveObject->SetCurveColor(color);
  }

  // If auto focus is set, focus on the selected curves
  if(mEditor->GetSettings()->mAutoFocus)
    FocusOnSelectedCurves(IntVec2(1,1));

  // Destroy all the curve editors for tracks that are no longer selected
  forRange(AnimationCurveObject* curve, existingCurveObjects.All())
  {
    curve->Destroy();
  }
}

//******************************************************************************
void AnimationGraphEditor::Zoom(MouseEvent* e, Vec2Param axis)
{
  if(e->Handled)
    return;

  Vec2 zoomPos = mCurveEditor->ToLocal(e->Position);
  Vec2 zoom = Vec2(1,1) * e->Scroll.y * 0.02f * mGraphData->GetZoom();

  // Flip the Y
  zoomPos.y = mCurveEditor->GetSize().y - zoomPos.y;
  zoomPos.x -= Pixels(30);

  mGraphData->ZoomAtPosition(zoomPos, zoom * axis);

  MarkAsNeedsUpdate();
}

//******************************************************************************
void AnimationGraphEditor::OnSelectionModified(Event* e)
{
  RebuildCurves();
}

//******************************************************************************
void AnimationGraphEditor::OnAnimationModified(Event* e)
{
  RebuildCurves();
}

//******************************************************************************
void AnimationGraphEditor::OnMiddleMouseDown(MouseEvent* e)
{
  if(e->Handled)
    return;

  new GraphManipulator(e->GetMouse(), this);
}

//******************************************************************************
void AnimationGraphEditor::OnMouseScroll(MouseEvent* e)
{
  if(e->Handled)
    return;

  // Zoom on both axes
  Zoom(e, Vec2(1, 1));
}

//******************************************************************************
void AnimationGraphEditor::OnMouseScrollNegativeArea(MouseEvent* e)
{
  // Zoom on only the y
  Zoom(e, Vec2(0, 1));

  e->Handled = true;
}

//******************************************************************************
void AnimationGraphEditor::OnMouseMove(MouseEvent* e)
{
  mScrubber->mShowGhostPlayHead = (mCurveEditor->GetMouseOverCurve() != NULL);
  mScrubber->mGhostPlayHead = ToGraphPosition(ToLocal(Z::gMouse->GetClientPosition())).x;
}

//******************************************************************************
void AnimationGraphEditor::OnMouseEnterCurve(CurveEvent* e)
{
  // Grab the track who's curve was just entered
  AnimationCurveObject* curve = (AnimationCurveObject*)e->mCurve;
  TrackNode* node = curve->GetTrack();

  // We want to display the path of the track in the
  // color it's being displayed in
  ByteColor color = ToByteColor(node->mDisplayColor);

  mEditor->SetStatusText(node->Path, color);
}

//******************************************************************************
void AnimationGraphEditor::OnMouseExitCurve(CurveEvent* e)
{
  mEditor->SetStatusText(String());
}

//******************************************************************************
void AnimationGraphEditor::OnKeyDown(KeyboardEvent* e)
{
  // Forward the event to the curve editor
  mCurveEditor->DispatchEvent(Events::KeyDown, e);
}

//******************************************************************************
void AnimationGraphEditor::OnMouseEnter(MouseEvent* e)
{
  this->SoftTakeFocus();
}

}//namespace Zero
