///////////////////////////////////////////////////////////////////////////////
///
/// \file MultiConvexMeshEditor.cpp
/// 
/// Authors: Joshua Davis
/// Copyright 2014, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace ConvexMeshEditorUi
{
const cstr cLocation = "EditorUi/ConvexMeshEditor";
Tweakable(Vec4,  ControlPointColor, Vec4(0.995f, 0.995f, 0.995f, 1), cLocation);
Tweakable(Vec4,  ControlPointHighlightColor, Vec4(0.75f, 0.75f, 0.75f, 1), cLocation);
Tweakable(Vec4,  ControlPointSelectionColor, Vec4(0, 0.760784f, 1, 1), cLocation);
Tweakable(Vec4,  OuterContourColor, Vec4(0.85f, 0.85f, 0.85f, 1), cLocation);
Tweakable(Vec4,  OuterContourHighlightColor, Vec4(0.685f, 0.67815f, 0.67815f, 1), cLocation);
Tweakable(Vec4,  OuterContourInvalidColor, Vec4(1, 0, 0, 1), cLocation);
Tweakable(float, OuterContourWidth, 0.1f, cLocation);
Tweakable(Vec4,  ConvexMeshColor, Vec4(0, 0, 1, 0.5f), cLocation);
Tweakable(Vec4,  ConvexMeshEdgeColor, ToFloatColor(Color::Lime), cLocation);
Tweakable(float, ConvexMeshWidth, 0.1f, cLocation);
Tweakable(float, ControlPointSize, Pixels(8),  cLocation);
Tweakable(float, ControlPointSnappingSize, Pixels(10),  cLocation);
Tweakable(bool,  DebuggingMode, false,  cLocation);
}

//-------------------------------------------------------------------MultiConvexMeshDrawer
MultiConvexMeshDrawer::MultiConvexMeshDrawer(Composite* parent, MultiConvexMeshEditor* editor)
  : Widget(parent)
{
  mEditor = editor;
}

void MultiConvexMeshDrawer::RenderUpdate(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, Rect clipRect)
{
  Widget::RenderUpdate(viewBlock, frameBlock, parentTx, colorTx, clipRect);

  mEditor->DrawMesh();

  for (uint i = 0; i < mEditor->mPoints.Size(); ++i)
  {
    MultiConvexMeshPoint* point = mEditor->mPoints[i];
    //make sure the viewport position is up-to-date (this is easier than just
    //caching it only when we move it as this is affected by the camera
    //position, size and even the viewport's size)
    point->UpdateViewportPosition();
  }

  DrawOuterContour(viewBlock, frameBlock, clipRect);
  DrawPoints(viewBlock, frameBlock, clipRect);
  DrawClosestPointOnEdge(viewBlock, frameBlock, clipRect);
  DrawAutoComputedContours(viewBlock, frameBlock, clipRect);
}

void MultiConvexMeshDrawer::DrawOuterContour(ViewBlock& viewBlock, FrameBlock& frameBlock, Rect clipRect)
{
  Array<StreamedVertex> lines;

  uint size = mEditor->mPoints.Size();

  //get the color of the edges (if the mesh is invalid use the invalid color instead)
  Vec4 contourColor = mEditor->mPropertyViewInfo.GetOuterContourColor();
  Vec4 highlightColor = ConvexMeshEditorUi::OuterContourHighlightColor;
  if(mEditor->mIsValid == false)
  {
    contourColor = ConvexMeshEditorUi::OuterContourInvalidColor;
    highlightColor = ConvexMeshEditorUi::OuterContourInvalidColor;
  }

  //render all the normal lines
  for(uint i = 0; i < size; ++i)
  {
    Vec4 color;
    if(i != mEditor->mSelectedEdge)
      color = contourColor;
    else
      color = highlightColor;

    MultiConvexMeshPoint* currPoint = mEditor->mPoints[i];
    MultiConvexMeshPoint* nextPoint = mEditor->mPoints[(i + 1) % size];

    lines.PushBack(StreamedVertex(currPoint->mViewportPoint, Vec2(0, 0), color));
    lines.PushBack(StreamedVertex(nextPoint->mViewportPoint, Vec2(0, 0), color));
  }

  CreateRenderData(viewBlock, frameBlock, clipRect, lines, PrimitiveType::Lines);

  //Debug drawing
  bool debugging = ConvexMeshEditorUi::DebuggingMode;
  if(debugging == false)
    return;

  static Texture* white = TextureManager::FindOrNull("White");
  static RenderFont* font = FontManager::Instance->GetRenderFont("NotoSans-Regular", 11, 0);
  ViewNode& viewNodeText = AddRenderNodes(viewBlock, frameBlock, clipRect, white);
  FontProcessor fontProcessor(frameBlock.mRenderQueues, &viewNodeText, ToFloatColor(Color::White));

  //draw text in the center of each line for which line number it is
  for(uint i = 0; i < size; ++i)
  {
    MultiConvexMeshPoint* currPoint = mEditor->mPoints[i];
    MultiConvexMeshPoint* nextPoint = mEditor->mPoints[(i + 1) % size];

    Vec3 pos = (currPoint->mViewportPoint + nextPoint->mViewportPoint) * 0.5f;
    ProcessTextRange(fontProcessor, font, String::Format("%d", i), ToVector2(pos), TextAlign::Left, Vec2(1, 1), Vec2(1, 1));
  }
}

void MultiConvexMeshDrawer::DrawPoints(ViewBlock& viewBlock, FrameBlock& frameBlock, Rect clipRect)
{
  static Texture* white = TextureManager::FindOrNull("White");
  ViewNode& viewNode = AddRenderNodes(viewBlock, frameBlock, clipRect, white);

  for(uint i = 0; i < mEditor->mPoints.Size(); ++i)
  {
    MultiConvexMeshPoint* point = mEditor->mPoints[i];

    Vec3 viewportPoint = point->mViewportPoint;

    //determine which color to use (based upon if the point is highlighted/selected)
    Vec4 color = ConvexMeshEditorUi::ControlPointColor;
    
    if(mEditor->mSelection.Contains(point))
      color = ConvexMeshEditorUi::ControlPointSelectionColor;
    //if the point is highlighted over then combine it with the highlight color
    if(point->mMouseOver)
      color *= ConvexMeshEditorUi::ControlPointHighlightColor;

    Vec3 halfWidth = Vec3(0.5f, 0.5f, 0.0f) * ConvexMeshEditorUi::ControlPointSize;
    Vec3 p0 = viewportPoint - halfWidth;
    Vec3 p1 = viewportPoint + halfWidth;

    frameBlock.mRenderQueues->AddStreamedQuad(viewNode, p0, p1, Vec2(0, 0), Vec2(1, 1), color);
  }
}

void MultiConvexMeshDrawer::DrawClosestPointOnEdge(ViewBlock& viewBlock, FrameBlock& frameBlock, Rect clipRect)
{
  //debug! shows the closest point on the closest edge (where just adding a new point will go to)
  Keyboard* keyboard = Keyboard::GetInstance();
  if(mEditor->mClosestEdgeInfo.mClosestEdge <= mEditor->mPoints.Size() && keyboard->KeyIsDown(Keys::Control))
  {
    Vec3 viewportPoint = mEditor->mClosestEdgeInfo.mClosestViewportPoint;
    Vec4 color = ConvexMeshEditorUi::ControlPointColor;

    Vec3 halfWidth = Vec3(0.5f, 0.5f, 0.0f) * ConvexMeshEditorUi::ControlPointSize;
    Vec3 p0 = viewportPoint - halfWidth;
    Vec3 p1 = viewportPoint + halfWidth;

    static Texture* white = TextureManager::FindOrNull("White");
    ViewNode& viewNode = AddRenderNodes(viewBlock, frameBlock, clipRect, white);
    frameBlock.mRenderQueues->AddStreamedQuad(viewNode, p0, p1, Vec2(0, 0), Vec2(1, 1), color);
  }
}

void MultiConvexMeshDrawer::DrawAutoComputedContours(ViewBlock& viewBlock, FrameBlock& frameBlock, Rect clipRect)
{
  bool debugging = ConvexMeshEditorUi::DebuggingMode;
  if(debugging == false)
    return;

  static Texture* white = TextureManager::FindOrNull("White");
  static RenderFont* font = FontManager::Instance->GetRenderFont("NotoSans-Regular", 11, 0);
  ViewNode& viewNodeText = AddRenderNodes(viewBlock, frameBlock, clipRect, white);
  FontProcessor fontProcessor(frameBlock.mRenderQueues, &viewNodeText, ToFloatColor(Color::White));

  Array<StreamedVertex> lines;

  for(uint index = 0; index < mEditor->mMarchingSquares.mContours.Size(); ++index)
  {
    MarchingSquares::Contour& contour = mEditor->mMarchingSquares.mContours[index];

    //render the index number at the vertex
    for(uint i = 0; i < contour.Size(); ++i)
    {
      Vec2Param start = contour[i];
      //Vec2Param end = contour[(i + 1) % contour.Size()];

      Vec2 screenPoint = mEditor->mViewport->WorldToScreen(Vec3(start.x, start.y, 0));
      Vec2 p0 = mEditor->mViewport->ScreenToViewport(screenPoint);

      ProcessTextRange(fontProcessor, font, String::Format("%d", i), p0, TextAlign::Left, Vec2(1, 1), Vec2(1, 1));
    }

    //render all of the lines of the auto-computed contours
    Vec4 color = ToFloatColor(Color::SaddleBrown);
    for(uint i = 0; i < contour.Size(); ++i)
    {
      Vec2Param start = contour[i];
      Vec2Param end = contour[(i + 1) % contour.Size()];

      Vec2 screenPoint = mEditor->mViewport->WorldToScreen(Vec3(start.x, start.y, 0));
      Vec3 p0 = Vec3(mEditor->mViewport->ScreenToViewport(screenPoint));
      screenPoint = mEditor->mViewport->WorldToScreen(Vec3(end.x, end.y, 0));
      Vec3 p1 = Vec3(mEditor->mViewport->ScreenToViewport(screenPoint));

      lines.PushBack(StreamedVertex(p0, Vec2(0, 0), color));
      lines.PushBack(StreamedVertex(p1, Vec2(0, 0), color));
    }
  }

  CreateRenderData(viewBlock, frameBlock, clipRect, lines, PrimitiveType::Lines);
}

//-------------------------------------------------------------------MultiConvexMeshDragManipulator
class MultiConvexMeshDragManipulator : public MouseManipulation
{
public:

  /// Stores the initial relative conditions of all the selected points from the drag position.
  struct PointData
  {
    PointData() {}
    PointData(MultiConvexMeshPoint* point, Vec3Param anchor)
    {
      mPoint = point;
      mOffset = point->mWorldPoint - anchor;
    }

    MultiConvexMeshPoint* mPoint;
    /// The offset that this point had initially from the anchor point.
    Vec3 mOffset;
  };

  MultiConvexMeshDragManipulator(Mouse* mouse, Composite* parent, MultiConvexMeshEditor* editor, MultiConvexMeshPoint* point)
    : MouseManipulation(mouse, parent)
  {
    mEditor = editor;
    mStartPosition = point->mWorldPoint;

    //grab the current selection and the offset of every point in the
    //selection from the primary point (the one that was dragged from)
    MultiConvexMeshEditor::SelectionSet::range range = mEditor->mSelection.All();
    for(; !range.Empty(); range.PopFront())
      mOffsets.PushBack(PointData(range.Front(), mStartPosition));
  }

  void OnMouseMove(MouseEvent* e) override
  {
    e->Handled = true;
    UpdatePosition(e->Position);
  }

  void OnMouseUpdate(MouseEvent* e) override
  {
    e->Handled = true;
    UpdatePosition(e->Position);
  }

  void UpdatePosition(Vec2Param screenPosition)
  {
    //mark that no edge is currently selected (because the order of events might
    //mean that a capture happens before our parents gets the exit)
    mEditor->ClearSelectedEdge();

    //snap the world point
    Vec3 worldPoint = mEditor->ScreenPointToSnappedWorldPoint(screenPosition);

    //update each point's world position by the new world position and the
    //cached offset (the only point that is snapped is the one we're dragging)
    for(uint i = 0; i < mOffsets.Size(); ++i)
    {
      PointData& data = mOffsets[i];
      data.mPoint->mWorldPoint = worldPoint + data.mOffset;
      data.mPoint->UpdateViewportPosition();
    }

    mEditor->MarkMeshModified();
    mEditor->UpdateSelectedPointText();
    mEditor->BuildConvexMeshes();
  }

  void OnMouseUp(MouseEvent* e) override
  {
    //Queue undos for all the points moving
    mEditor->mQueue.BeginBatch();
    mEditor->mQueue.SetActiveBatchName("MultiConvexMeshEditor_OnMouseUp");
    for(uint i = 0; i < mOffsets.Size(); ++i)
    {
      PointData& data = mOffsets[i];

      Vec3 startPosition = mStartPosition + data.mOffset;
      PointMovementOp* op = new PointMovementOp(data.mPoint, startPosition);
      mEditor->mQueue.Queue(op);
    }
    mEditor->mQueue.EndBatch();

    //see if this was a valid mesh
    mEditor->TestConvexMeshes();

    this->Destroy();
  }

  Array<PointData> mOffsets;
  MultiConvexMeshEditor* mEditor;
  Vec3 mStartPosition;
};

//-------------------------------------------------------------------MultiConvexMeshPoint
ZilchDefineType(MultiConvexMeshPoint, builder, type)
{
}

MultiConvexMeshPoint::MultiConvexMeshPoint(Composite* parent, MultiConvexMeshEditor* editor)
  : Widget(parent)
{
  Setup(editor, Vec3::cZero);
}

MultiConvexMeshPoint::MultiConvexMeshPoint(Composite* parent, MultiConvexMeshEditor* editor, Vec3Param worldPoint)
  : Widget(parent)
{
  Setup(editor, worldPoint);
}

void MultiConvexMeshPoint::Setup(MultiConvexMeshEditor* editor, Vec3Param worldPoint)
{
  mWorldPoint = worldPoint;
  mMouseOver = false;
  mEditor = editor;

  //needs to be interactive so that we get all the mouse events
  SetInteractive(true);
  //we're manually positioned
  SetNotInLayout(true);
  //recompute our desired viewport position from the set world point
  UpdateViewportPosition();

  //so that the editor doesn't clear the selection by thinking we clicked empty space
  ConnectThisTo(this, Events::LeftMouseDown, OnHandleMouseEvent);
  ConnectThisTo(this, Events::LeftClick, OnLeftClick);
  //so another point isn't created by us double clicking on a point
  ConnectThisTo(this, Events::DoubleClick, OnHandleMouseEvent);
  ConnectThisTo(this, Events::LeftMouseDrag, OnLeftMouseDrag);
  ConnectThisTo(this, Events::RightMouseDown, OnRightMouseDown);
  //so the logic for highlighting edges doesn't run
  ConnectThisTo(this, Events::MouseUpdate, OnHandleMouseEvent);
  ConnectThisTo(this, Events::MouseEnter, OnMouseEnter);
  ConnectThisTo(this, Events::MouseExit, OnMouseExit);
}

void MultiConvexMeshPoint::OnHandleMouseEvent(MouseEvent* e)
{
  e->Handled = true;
}

void MultiConvexMeshPoint::OnLeftClick(MouseEvent* e)
{
  e->Handled = true;

  //a click happened that didn't result in a drag operation and shift wasn't pressed,
  //the user is trying to select one point only so clear the old selection
  if(!e->ShiftPressed)
    mEditor->ClearSelection();

  //tell the editor we were selected
  mEditor->AddToSelection(this);
}

void MultiConvexMeshPoint::OnLeftMouseDrag(MouseEvent* e)
{
  //if drag was handled for some reason then don't do anything
  if(e->Handled)
    return;

  e->Handled = true;

  //if we are not currently in the selection then the user is either trying
  //to drag a new point (and none of the current selection) or they're
  //holding shift and so this point will be added to the selection for the drag
  if(!mEditor->mSelection.Contains(this))
  {
    if(!e->ShiftPressed)
      mEditor->ClearSelection();
    mEditor->AddToSelection(this);
  }
  StartDrag(e->GetMouse());
}

void MultiConvexMeshPoint::OnRightMouseDown(MouseEvent* e)
{
  e->Handled = true;

  //create a context menu below the mouse
  ContextMenu* menu = new ContextMenu(this);
  Mouse* mouse = Z::gMouse;
  menu->SetBelowMouse(mouse, Pixels(0,0));

  //and add an option to delete ourself
  ConnectMenu(menu, "Delete Point", OnDeletePoint);
}

void MultiConvexMeshPoint::OnMouseEnter(MouseEvent* e)
{
  //keep track of the mouse being over this point (for highlight)
  mMouseOver = true;
}

void MultiConvexMeshPoint::OnMouseExit(MouseEvent* e)
{
  mMouseOver = false;
}

void MultiConvexMeshPoint::OnDeletePoint(Event* e)
{
  mEditor->RemovePoint(this);
  mEditor->RemoveFromSelection(this);
  mEditor->TestConvexMeshes();
  mEditor->HardTakeFocus();

  //make sure this is last!
  this->Destroy();
}

void MultiConvexMeshPoint::UpdateViewportPosition()
{
  //bring the world point to a viewport point
  Vec2 screenPoint = mEditor->mViewport->WorldToScreen(mWorldPoint);
  mViewportPoint = Vec3(mEditor->mViewport->ScreenToViewport(screenPoint));

  //update our size
  Vec2 size(ConvexMeshEditorUi::ControlPointSize);
  SetSize(size);

  //figure out our correct position (since the viewport point is our center but we're
  //upper left positioned) and make sure to snap that to pixel boundaries
  Vec3 actualPos = mViewportPoint - Vec3(size) * 0.5f;
  SnapToPixels(actualPos);
  SetTranslation(actualPos);
}

void MultiConvexMeshPoint::StartDrag(Mouse* mouse)
{
  //we started a drag, so start our special drag manipulator for this points
  new MultiConvexMeshDragManipulator(mouse, mEditor->mViewport, mEditor, this);
}

//-------------------------------------------------------------------PointMovementOp
PointMovementOp::PointMovementOp(MultiConvexMeshPoint* point, Vec3Param startPosition)
{
  mName = "MultiConvexMeshPoint Movement";
  //String display(netObject->GetMeta( )->Display(netObject->GetMeta( ), netObject, true));
  //mName = BuildString("Set net property \"", propertyName, "\" on \"", display, "\"");

  mEditor = point->mEditor;
  mIndex = mEditor->mPoints.FindIndex(point);
  mStartPosition = startPosition;
  mEndPosition = point->mWorldPoint;
}

void PointMovementOp::Undo()
{
  SetPosition(mStartPosition);
}

void PointMovementOp::Redo()
{
  SetPosition(mEndPosition);
}

void PointMovementOp::SetPosition(Vec3Param newPosition)
{
  MultiConvexMeshPoint* point = mEditor->mPoints[mIndex];
  point->mWorldPoint = newPosition;
  point->UpdateViewportPosition();
  mEditor->UpdateSelectedPointText();
  mEditor->BuildConvexMeshes();
}

//-------------------------------------------------------------------PointAddRemoveOp
PointAddRemoveOp::PointAddRemoveOp(MultiConvexMeshPoint* point, bool add)
{
  if(add)
    mName = "MultiConvexMeshPoint Add";
  else
    mName = "MultiConvexMeshPoint Remove";

  mEditor = point->mEditor;
  mIndex = mEditor->mPoints.FindIndex(point);
  mWorldPosition = point->mWorldPoint;
  mAdd = add;
}

void PointAddRemoveOp::Undo()
{
  //if we're an add op, then undoing is to remove the point
  PerformOp(!mAdd);
}

void PointAddRemoveOp::Redo()
{
  PerformOp(mAdd);
}

void PointAddRemoveOp::PerformOp(bool add)
{
  if(add)
    mEditor->AddPointAt(mIndex, mWorldPosition, false, false);
  else
    mEditor->RemovePoint(mEditor->mPoints[mIndex], false, false);
  mEditor->BuildConvexMeshes();
  mEditor->UpdateSelectedPointText();
}

//-------------------------------------------------------------------MultiSelectManipulator
class MultiSelectionManipulator : public MouseManipulation
{
public:

  MultiSelectionManipulator(MouseDragEvent* dragEvent, Composite* parent, MultiConvexMeshEditor* editor)
    : MouseManipulation(dragEvent->GetMouse(), parent)
  {
    mEditor = editor;
    
    mViewportStartPosition = mEditor->mViewport->ScreenToViewport(dragEvent->Position);

    mSelectBox = mEditor->CreateAttached<Element>(cDragBox);
    mSelectBox->SetVisible(true);
  }

  void OnDestroy() override
  {
    mSelectBox->Destroy();
  }

  void SelectPoints(Vec2Param p0, Vec2Param p1)
  {
    Aabb aabb;
    aabb.SetInvalid();
    //make the aabb fat on the z axis to guarantee we'll get a point if we should
    aabb.Expand(Vec3(p0,-0.5));
    aabb.Expand(Vec3(p1, 0.5));

    //clear the old selection since we're going to create a brand new one
    mEditor->ClearSelection();
    //see what points are in the current selection
    for(uint i = 0; i < mEditor->mPoints.Size(); ++i)
    {
      MultiConvexMeshPoint* point = mEditor->mPoints[i];
      Vec3 pos = point->mViewportPoint;
      if(aabb.ContainsPoint(pos))
        mEditor->AddToSelection(point);
    }
  }

  Rect GetSelectionRect(Vec2Param screenEndPosition)
  {
    Viewport* viewport = mEditor->mViewport;
    Vec2 startPosition = mEditor->ToLocal(viewport->ViewportToScreen(mViewportStartPosition));
    Vec2 currPosition = mEditor->ToLocal(screenEndPosition);

    //the start point might be the max or the min, same for the current position
    //so construct the proper rect by getting the min and max corners
    Vec2 min = Math::Min(startPosition, currPosition);
    Vec2 max = Math::Max(startPosition, currPosition);
    return Rect::MinAndMax(min, max);
  }

  void OnMouseUpdate(MouseEvent* e) override
  {
    e->Handled = true;

    //get the mouse position in viewport space but clamp it to the viewport
    //(so we can't select things outside the current viewport window)
    Viewport* viewport = mEditor->mViewport;
    Vec2 viewportEndPosition = viewport->ScreenToViewport(e->Position);
    viewportEndPosition = Math::Clamp(viewportEndPosition, Vec2::cZero, viewport->GetSize());

    //get the selection rect to raw
    Vec2 screenEndPosition = viewport->ViewportToScreen(viewportEndPosition);
    Rect selection = GetSelectionRect(screenEndPosition);
    mSelectBox->SetSize(selection.GetSize());
    mSelectBox->SetTranslation(ToVector3(selection.TopLeft()));
    
    SelectPoints(mViewportStartPosition, viewportEndPosition);
  }

  void OnMouseUp(MouseEvent* event) override
  {
    this->Destroy();
  }

  void OnKeyDown(KeyboardEvent* e) override
  {
    //if the user hit escape then they want to cancel the drag
    if(e->Key == Keys::Escape)
      this->Destroy();
  }

  Element* mSelectBox;
  MultiConvexMeshEditor* mEditor;
  Vec2 mViewportStartPosition;
};

//-------------------------------------------------------------------MultiConvexMeshPropertyViewInfo
ZilchDefineType(MultiConvexMeshPropertyViewInfo, builder, type)
{
  ZilchBindDefaultConstructor();
  ZeroBindDocumented();
  type->HandleManager = ZilchManagerId(PointerManager);

  ZilchBindGetterSetterProperty(MeshThickness);
  ZilchBindGetterSetterProperty(SpriteSource);
  ZilchBindGetterSetterProperty(ClearColor);
  ZilchBindGetterSetterProperty(OuterContourColor);
  ZilchBindFieldProperty(mDrawMode);
  ZilchBindMethodProperty(AutoCompute);
  ZilchBindFieldProperty(mAutoComputeMode);
  ZilchBindFieldProperty(mSurfaceLevelThreshold)->Add(new EditorRange(0, 1, 0.05f));
  ZilchBindFieldProperty(mAutoComputeMethod);
  ZilchBindFieldProperty(mSimplificationThreshold);
}

MultiConvexMeshPropertyViewInfo::MultiConvexMeshPropertyViewInfo()
{
  mMeshThickness = 1.0f;
  mSpriteSource = SpriteSourceManager::GetDefault();
  mDrawMode = MultiConvexMeshDrawMode::Edges;
  mClearColor = Vec4(0.155f, 0.155f, 0.155f, 1);
  mOuterContourColor = ConvexMeshEditorUi::OuterContourColor;
  mSurfaceLevelThreshold = 0.5f;
  mAutoComputeMode = MultiConvexMeshAutoComputeMode::Alpha;
  mSimplificationThreshold = 0.5f;
  mAutoComputeMethod = MultiConvexMeshAutoComputeMethod::Pixels;
}

float MultiConvexMeshPropertyViewInfo::GetMeshThickness()
{
  return mMeshThickness;
}

void MultiConvexMeshPropertyViewInfo::SetMeshThickness(float thickness)
{
  mMeshThickness = thickness;
  mEditor->MarkMeshModified();
  mEditor->BuildConvexMeshes();
}

Vec4 MultiConvexMeshPropertyViewInfo::GetClearColor()
{
  return mClearColor;
}

void MultiConvexMeshPropertyViewInfo::SetClearColor(Vec4Param color)
{
  mClearColor = color;
  Any clearColor(mClearColor);
  mEditor->mRenderer->SetProperty("ClearColor", clearColor);
}

Vec4 MultiConvexMeshPropertyViewInfo::GetOuterContourColor()
{
  return mOuterContourColor;
}

void MultiConvexMeshPropertyViewInfo::SetOuterContourColor(Vec4Param color)
{
  mOuterContourColor = color;
}

SpriteSource* MultiConvexMeshPropertyViewInfo::GetSpriteSource()
{
  return mSpriteSource;
}

void MultiConvexMeshPropertyViewInfo::SetSpriteSource(SpriteSource* source)
{
  mSpriteSource = source;
  mEditor->UpdatePreview(source);
  mEditor->SetGridSizeToPixels();
}

void MultiConvexMeshPropertyViewInfo::AutoCompute()
{
  mEditor->AutoCompute();
}

//-------------------------------------------------------------------MultiConvexMeshEditor
ZilchDefineType(MultiConvexMeshEditor, builder, type)
{
}

MultiConvexMeshEditor::MultiConvexMeshEditor(Composite* parent, MultiConvexMesh* mesh)
  : Composite(parent), mMesh(mesh)
{
  SetLayout(CreateRowLayout());
  mPropertyViewInfo.mEditor = this;
  mGridDraw = NULL;
  mPreviewCog = NULL;
  mIsValid = true;
  mGridCellSize = real(0.5);
  mSnappingMode = MultiConvexMeshSnappingMode::IfClose;
  
  //create a dummy panel to contain property view and the selected point text
  Composite* leftPanelDummy = new Composite(this);
  leftPanelDummy->SetSizing(SizeAxis::X, SizePolicy::Fixed, 325);
  leftPanelDummy->SetLayout(CreateStackLayout());
  //create the property view and point it and our info object
  mPropertyView = new PropertyView(leftPanelDummy);
  mPropertyView->SetSizing(SizeAxis::Y, SizePolicy::Flex, 20);
  mPropertyView->SetObject(&mPropertyViewInfo);
  
  //create a splitter in-between the property view and the viewport
  Splitter* splitter = new Splitter(this);
  splitter->SetSizing(SizeAxis::X, SizePolicy::Fixed, 4);

  Composite* rightPanelDummy = new Composite(this);
  rightPanelDummy->SetSizing(SizeAxis::X, SizePolicy::Flex, 100);
  rightPanelDummy->SetLayout(CreateStackLayout());

  CreateToolbar(rightPanelDummy);

  //create a splitter in-between the property view and the viewport
  Spacer* spacer = new Spacer(rightPanelDummy);
  spacer->SetSizing(SizeAxis::Y, SizePolicy::Fixed, 4);

  //make a GameWidget for viewport to attach to
  mGameWidget = new GameWidget(rightPanelDummy);
  mGameWidget->SetSizing(SizeAxis::Y, SizePolicy::Flex, 100);
  mGameWidget->SetClipping(true);

  //set-up the preview space for the viewport
  SetupPreviewSpace();
  //update the preview cog to be the default sprite
  UpdatePreview(NULL);

  //most events should be connected on the viewport because we care about when mouse
  //events, etc, happen on the viewport only, not on the property view
  ConnectThisTo(mViewport, Events::LeftMouseDown, OnLeftMouseDown);
  ConnectThisTo(mViewport, Events::DoubleClick, OnDoubleClick);
  ConnectThisTo(mViewport, Events::RightMouseDown, OnRightMouseDown);
  ConnectThisTo(mViewport, Events::MiddleMouseDown, OnMiddleMouseDown);
  ConnectThisTo(mViewport, Events::LeftMouseDrag, OnLeftMouseDrag);
  ConnectThisTo(mViewport, Events::MouseUpdate, OnMouseUpdate);
  ConnectThisTo(mViewport, Events::MouseScroll, OnMouseScroll);
  ConnectThisTo(mViewport, Events::MouseExit, OnMouseExit);
  ConnectThisTo(this, Events::KeyDown, OnKeyDown);
  ConnectThisTo(GetRootWidget(), Events::WidgetUpdate, OnWidgetUpdate);

  //for drag and drop of sprites/archetypes/meshes
  ConnectThisTo(mViewport, Events::MetaDrop, OnMetaDrop);
  ConnectThisTo(mViewport, Events::MetaDropTest, OnMetaDrop);
  ConnectThisTo(Z::gResources, Events::ResourceRemoved, OnResourceRemoved);

  //now try to focus on the preview cog (so it's fully framed)
  FocusOnPreviewCog();
  //also create a custom drawer for
  new MultiConvexMeshDrawer(mViewport, this);
}

void MultiConvexMeshEditor::OnDestroy()
{
  mPreviewSpace->Destroy();
  Composite::OnDestroy();
}

void MultiConvexMeshEditor::SetupPreviewSpace()
{
  //create the preview space
  mPreviewSpace = Z::gFactory->CreateSpace(CoreArchetypes::DefaultSpace, CreationFlags::Editing, Z::gEditor->GetEditGameSession());
  mPreviewSpace->mGameWidgetOverride = mGameWidget;
  mPreviewSpace->SetName("MultiConvexMeshEditorPreview");
  //pause the time space (so that physics doesn't run, etc...)
  TimeSpace* time = mPreviewSpace->has(TimeSpace);
  if(time != NULL)
    time->SetPaused(true);

  Level* editorLevel = LevelManager::GetInstance()->Find("MultiConvexMeshEditorLevel");
  mPreviewSpace->LoadLevel(editorLevel);

  //now find the editor camera and set it to be orthographic
  Cog* cameraCog = mPreviewSpace->FindObjectByName("EditorCamera");

  CameraViewport* cameraViewport = cameraCog->has(CameraViewport);
  mViewport = cameraViewport->mViewport;

  //grab the camera controller and also set it up for 2d mode
  mCameraController = cameraCog->has(EditorCameraController);
  if(mCameraController != NULL)
  {
    mCameraController->SetControlMode(ControlMode::ZPlane);
    mCameraController->SetEnabled(true);
    mCameraController->mMinCameraSize = 0.01f;
    mCameraController->mMaxCameraSize = 50.0f;

    //have the camera controller listed for keyboard events on the viewport
    Zero::Connect(mViewport, Events::KeyDown, mCameraController, &EditorCameraController::ProcessKeyboardEvent);
    Zero::Connect(mViewport, Events::KeyUp, mCameraController, &EditorCameraController::ProcessKeyboardEvent);
  }

  //Set the renderer's clear color
  Cog* rendererCog = mPreviewSpace->FindObjectByName("Renderer");
  mRenderer = rendererCog->GetComponentByName("ForwardRenderer");
  Any clearColor(mPropertyViewInfo.mClearColor);
  mRenderer->SetProperty("ClearColor", clearColor);

  //create the preview mesh cog, this is the one that we 
  //put the multi-convex mesh collider on to render the mesh
  Cog* previewMeshCog = mPreviewSpace->CreateAt(CoreArchetypes::Transform, Vec3::cZero);
  previewMeshCog->AddComponentByName(ZilchTypeId(MultiConvexMeshCollider)->Name);
  mPreviewMesh = previewMeshCog->has(MultiConvexMeshCollider);
  SetMultiConvexMesh(mMesh);

  //we also need a grid drawing component, for now just add it
  //to the mesh cog (since we aren't moving it or destroying it)
  Cog* levelSettingsCog = mPreviewSpace->FindObjectByName("LevelSettings");
  mGridDraw = levelSettingsCog->has(GridDraw);
  UpdateGridDrawing();
}

void MultiConvexMeshEditor::CreateToolbar(Composite* toolbarParent)
{
  Composite* topToolbar = new Composite(toolbarParent);
  topToolbar->SetSizing(SizeAxis::Y, SizePolicy::Fixed, 20);
  topToolbar->SetLayout(CreateRowLayout());

  Spacer* spacer = new Spacer(topToolbar);
  spacer->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(4));

  //create the label for the x
  Composite* labelX = new Label(topToolbar, DefaultTextStyle, "X:");
  labelX->SetSizing(SizeAxis::X, SizePolicy::Fixed, 18);
  //create the editable text box for the x
  mSelectedPointX = new TextBox(topToolbar);
  mSelectedPointX->SetSizing(SizeAxis::X, SizePolicy::Fixed, 80);
  mSelectedPointX->SetEditable(false);
  ConnectThisTo(mSelectedPointX, Events::TextBoxChanged, OnSelectedPointXChanged);

  spacer = new Spacer(topToolbar);
  spacer->SetSizing(SizeAxis::X, SizePolicy::Fixed, Pixels(4));

  //create the label for the y
  Composite* labelY = new Label(topToolbar, DefaultTextStyle, "Y:");
  labelY->SetSizing(SizeAxis::X, SizePolicy::Fixed, 18);
  //create the editable text box for the y
  mSelectedPointY = new TextBox(topToolbar);
  mSelectedPointY->SetSizing(SizeAxis::X, SizePolicy::Fixed, 80);
  mSelectedPointY->SetEditable(false);
  ConnectThisTo(mSelectedPointY, Events::TextBoxChanged, OnSelectedPointYChanged);

  spacer = new Spacer(topToolbar);
  spacer->SetSizing(SizeAxis::X, SizePolicy::Fixed, 10);

  //create a button to toggle grid drawing
  mShowGridButton = new ToggleIconButton(topToolbar);
  mShowGridButton->SetEnabledIcon("ViewportGridIconOn");
  mShowGridButton->SetDisabledIcon("ViewportGridIconOff");
  mShowGridButton->SetSize(Pixels(20, 20));
  mShowGridButton->SetToolTip("Toggle Grid");
  ConnectThisTo(mShowGridButton, Events::ButtonPressed, OnToggleShowGrid);

  spacer = new Spacer(topToolbar);
  spacer->SetSizing(SizeAxis::X, SizePolicy::Fixed, 5);

  //create the text box to manually type in the grid cell size
  mGridCellSizeTextBox = new TextBox(topToolbar);
  mGridCellSizeTextBox->SetSizing(SizeAxis::X, SizePolicy::Fixed, 80);
  mGridCellSizeTextBox->SetText(String::Format("%f", mGridCellSize));
  mGridCellSizeTextBox->SetEditable(true);
  ConnectThisTo(mGridCellSizeTextBox, Events::TextSubmit, OnGridCellSizeChanged);

  spacer = new Spacer(topToolbar);
  spacer->SetSizing(SizeAxis::X, SizePolicy::Fixed, 5);

  //create a button to auto-compute the grid cell size from the pixels per unit of the sprite source
  TextButton* setGridSizeToPixelsButton = new TextButton(topToolbar);
  setGridSizeToPixelsButton->SetSizing(SizeAxis::X, SizePolicy::Fixed, 180);
  setGridSizeToPixelsButton->SetText("SetGridSizeToPixels");
  setGridSizeToPixelsButton->SetToolTip("Sets the size of the grid to be the same as the pixels of the current sprite");
  ConnectThisTo(setGridSizeToPixelsButton, Events::ButtonPressed, OnSetGridSizeToPixels);

  spacer = new Spacer(topToolbar);
  spacer->SetSizing(SizeAxis::X, SizePolicy::Fixed, 14);
  
  //there's no good way to create a centered icon element so it requires a bit of composite trickery...
  Composite* snappingDummy = new Composite(topToolbar);
  snappingDummy->SetLayout(CreateStackLayout());
  snappingDummy->SetSizing(SizeAxis::X, SizePolicy::Fixed, 16);
  //create some padding at the top to help center the icon
  Composite* padding = new Composite(snappingDummy);
  padding->SetSizing(SizeAxis::Y, SizePolicy::Fixed, 2);
  //to make the icon line up nicely (and not stretch) create another composite to attach the element to
  Composite* iconComposite = new Composite(snappingDummy);
  Element* element = iconComposite->CreateAttached<Element>("AnimatorSnappingDisabled");

  //create a combo box for the user to choose snapping options
  mSnappingModeComboBox = new ComboBox(topToolbar);
  mSnappingModeSource.Strings.PushBack(MultiConvexMeshSnappingMode::Names[0]);
  mSnappingModeSource.Strings.PushBack(MultiConvexMeshSnappingMode::Names[1]);
  mSnappingModeSource.Strings.PushBack(MultiConvexMeshSnappingMode::Names[2]);
  mSnappingModeComboBox->SetListSource(&mSnappingModeSource);
  mSnappingModeComboBox->SetSizing(SizeAxis::X, SizePolicy::Fixed, 70);
  mSnappingModeComboBox->SetText("IfClose");
  ConnectThisTo(mSnappingModeComboBox, Events::ItemSelected, OnChangeSnappingMode);
}

MultiConvexMeshPoint* MultiConvexMeshEditor::FindPointAtScreenPosition(Vec2Param screenPosition)
{
  Vec3 worldPoint = ScreenPointToSnappedWorldPoint(screenPosition);
  for(uint i = 0; i < mPoints.Size(); ++i)
  {
    if(mPoints[i]->mWorldPoint == worldPoint)
      return mPoints[i];
  }

  return NULL;
}

void MultiConvexMeshEditor::AddPointAtScreenPosition(Vec2Param screenPosition)
{
  MarkMeshModified();

  ClearSelection();
  //get our world point (which is snapped if necessary)
  Vec3 worldPoint = ScreenPointToSnappedWorldPoint(screenPosition);

  //if there's 0 or 1 points then just add the point where it is (there's no edges to check)
  if(mPoints.Size() <= 1)
  {
    MultiConvexMeshPoint* newPoint = AddPointAt(mPoints.Size(), worldPoint);
    AddToSelection(newPoint);
    return;
  }

  //find the edge closest to the screen position
  ClosestEdgeInfo closestEdgeInfo;
  FindClosestEdge(screenPosition, closestEdgeInfo);
  //if there is a closest edge (should pretty much always happen)
  if(closestEdgeInfo.mClosestEdge <= mPoints.Size())
  {
    //if we were close to an edge then choose the point on the edge instead of where we clicked
    if(closestEdgeInfo.mClosestDistance < ConvexMeshEditorUi::OuterContourWidth)
    {
      Vec2 screenPoint = mViewport->ViewportToScreen(Math::ToVector2(closestEdgeInfo.mClosestViewportPoint));
      worldPoint = mViewport->ScreenToWorldZPlane(screenPoint, 0);
    }
  }
  //as a safeguard just select the last edge
  else
    closestEdgeInfo.mClosestEdge = mPoints.Size() - 1;

  //have to Insert at edge index + 1 (since edge 0 means we want to be after the current point 0)
  MultiConvexMeshPoint* newPoint = AddPointAt(closestEdgeInfo.mClosestEdge + 1, worldPoint);
  AddToSelection(newPoint);

  //see if this was a valid mesh
  TestConvexMeshes();
}

MultiConvexMeshPoint* MultiConvexMeshEditor::AddPointAt(uint index, Vec3Param worldPosition, bool queueUndo, bool testMesh)
{
  MultiConvexMeshPoint* point = new MultiConvexMeshPoint(mViewport, this, worldPosition);
  mPoints.InsertAt(index, point);

  if(queueUndo)
    mQueue.Queue(new PointAddRemoveOp(point, true));

  MarkMeshModified();
  //see if this was a valid mesh
  if(testMesh)
    TestConvexMeshes();
  else
    BuildConvexMeshes();

  return point;
}

void MultiConvexMeshEditor::RemovePoint(MultiConvexMeshPoint* point, bool queueUndo, bool testMesh)
{
  if(queueUndo)
    mQueue.Queue(new PointAddRemoveOp(point, false));

  MarkMeshModified();
  mPoints.EraseValueError(point);
  mSelection.Erase(point);

  //see if this was a valid mesh
  if(testMesh)
    TestConvexMeshes();
  else
    BuildConvexMeshes();
}

void MultiConvexMeshEditor::ClearPoints(bool queueUndo)
{
  //if we queue undos then queue each point removal
  if(queueUndo)
  {
    mQueue.BeginBatch();
    mQueue.SetActiveBatchName("MultiConvexMeshEditor_ClearPoints");
    for(int i = mPoints.Size() - 1; i >= 0; --i)
    {
      MultiConvexMeshPoint* point = mPoints[i];
      mQueue.Queue(new PointAddRemoveOp(point, false));
      point->Destroy();
    }
    mQueue.EndBatch();
  }
  else
  {
    for(int i = mPoints.Size() - 1; i >= 0; --i)
      delete mPoints[i];
  }
  
  mPoints.Clear();

  //since all of the points are now gone the selection
  //needs to be invalidated and the selection text updated
  ClearSelection();
  UpdateSelectedPointText();
}

void MultiConvexMeshEditor::AutoCompute()
{
  Sprite* sprite = mPreviewCog->has(Sprite);
  //if the preview cog doesn't have a sprite (archetype mode) then don't auto compute 
  //(could technically combine the sprites of all children but that's more work than it's worth)
  if(sprite == NULL)
    return;

  SpriteSource* spriteSource = sprite->mSpriteSource;

  mMarchingSquares = MarchingSquares();
  mMarchingSquares.mDensitySurfaceLevel = mPropertyViewInfo.mSurfaceLevelThreshold;
  //setup the density sampler function (by default sample alpha)
  mMarchingSquares.mDensitySampler = &MultiConvexMeshEditor::SamplePixelAlpha;
  if(mPropertyViewInfo.mAutoComputeMode == MultiConvexMeshAutoComputeMode::Intensity)
    mMarchingSquares.mDensitySampler = &MultiConvexMeshEditor::SamplePixelIntensity;

  
  //the threshold of simplification is 1 pixel's triangle area
  //(the half area of the pixel) at 0 and a 5x5 triangle area at 1
  real threshold = (1 / spriteSource->PixelsPerUnit);
  threshold = threshold * threshold * 0.5f;
  threshold += threshold * 25 * mPropertyViewInfo.mSimplificationThreshold;

  Vec2 size = Vec2((real)spriteSource->FrameSizeX, (real)spriteSource->FrameSizeY);
  Vec2 sampleFrequency = Vec2(1, 1);

  //sample based upon our computation method
  if(mPropertyViewInfo.mAutoComputeMethod == MultiConvexMeshAutoComputeMethod::Pixels)
  {
    //sample with the pixel boundary positions and the pixel sampling method
    mMarchingSquares.mPositionSampler = &MultiConvexMeshEditor::SamplePixelWorldPosition;
    mMarchingSquares.SamplePixels(Vec2(-1, -1), size + Vec2(2, 2), sampleFrequency, spriteSource);
  }
  else
  {
    //sample with the center of a pixel's position plus the regular marching cubes method
    mMarchingSquares.mPositionSampler = &MultiConvexMeshEditor::SamplePixelWorldPositionAtCenter;
    mMarchingSquares.Sample(Vec2(-1, -1), size + Vec2(2, 2), sampleFrequency, spriteSource);
  }
  

  mMarchingSquares.BuildAndSimplifyContours(threshold);

  Array<Vec2>& points = mMarchingSquares.mContours[0];
  
  mQueue.BeginBatch();
  mQueue.SetActiveBatchName("MultiConvexMeshEditor_AddPoints");
  //clear all of the points and add all of the points from the first
  //contour (deal with multiple contours later)
  ClearPoints();
  for(uint i = 0; i < points.Size(); ++i)
    AddPointAt(mPoints.Size(), Vec3(points[i]), true, false);
  mQueue.EndBatch();

  //now that all of the points have been added we can attempt to
  //build the entire mesh and test it for validity
  TestConvexMeshes();
}

Vec3 MultiConvexMeshEditor::ScreenPointToSnappedWorldPoint(Vec2Param screenPosition)
{
  Vec3 worldPoint = mViewport->ScreenToWorldZPlane(screenPosition, 0);

  //if we don't snap then just return the world point
  if(mSnappingMode == MultiConvexMeshSnappingMode::None)
    return worldPoint;

  //snap the world point to the grid
  float cellSize = GetGridCellSize();
  Vec3 gridPoint = Vec3(worldPoint / cellSize);
  gridPoint.Round();
  gridPoint *= cellSize;

  //if we return a point that is always snapped then just return the grid point
  if(mSnappingMode == MultiConvexMeshSnappingMode::Always)
    return gridPoint;

  //now we are in a snap-if-close mode, we want to do this based upon a screen size
  Vec2 viewportPoint = mViewport->ScreenToViewport(screenPosition);
  //compute the corner of a control point with our snapping size and bring that to world space
  Vec2 selectionCorner = viewportPoint + Vec2(ConvexMeshEditorUi::ControlPointSnappingSize * 0.5f);
  Vec2 cornerScreen = mViewport->ViewportToScreen(selectionCorner);
  Vec3 cornerWorld = mViewport->ScreenToWorldZPlane(cornerScreen, 0.0f);

  //compute the distance of our original world point to the grid
  float distance = Math::Length(gridPoint - worldPoint);
  //also radius of our snapping point
  float cornerDistance = Math::Length(cornerWorld - worldPoint);

  //if the distance to the grid point is less than our radius then snap the grid point
  if(distance < cornerDistance)
  {
    worldPoint = gridPoint;
  }
  return worldPoint;
}

void MultiConvexMeshEditor::OnLeftMouseDown(MouseEvent* e)
{
  if(e->Handled)
    return;

  //the user clicked in empty space so clear the selection
  ClearSelection();
  if(mCameraController->IsActive())
  {
    new EditorCameraMouseDrag(e->GetMouse(), mViewport, mCameraController);
    e->Handled = true;
  }
}

void MultiConvexMeshEditor::OnRightMouseDown(MouseEvent* e)
{
  if(e->Handled)
    return;

  //save where the mouse was so we can create the point at this position
  //(could also use the context menu's position but then we'd
  //just have to save that instead of the mouse position)
  mCachedMousePosition = e->Position;

  ContextMenu* menu = new ContextMenu(this);
  Mouse* mouse = Z::gMouse;
  menu->SetBelowMouse(mouse, Pixels(0,0));

  if(FindPointAtScreenPosition(mCachedMousePosition) == NULL)
  {
    ConnectMenu(menu, "Add Point", OnAddPoint);
  }
  else
  {
    ConnectMenu(menu, "Delete Point", OnRemovePoint);
  }
}

void MultiConvexMeshEditor::OnAddPoint(ObjectEvent* e)
{
  AddPointAtScreenPosition(mCachedMousePosition);

  //the right click menu takes focus away from the editor which is bad because
  //undo and redo will fall back to the main viewport so force focus back to use
  HardTakeFocus();
}

void MultiConvexMeshEditor::OnRemovePoint(ObjectEvent* e)
{
  MultiConvexMeshPoint* point = FindPointAtScreenPosition(mCachedMousePosition);
  if(point != NULL)
  {
    RemovePoint(point, true);
  }

  //the right click menu takes focus away from the editor which is bad because
  //undo and redo will fall back to the main viewport so force focus back to use
  HardTakeFocus();
}

void MultiConvexMeshEditor::OnMiddleMouseDown(MouseEvent* e)
{
  if(e->Handled)
    return;

  mCameraController->MouseDrag(CameraDragMode::Pan);
  new EditorCameraMouseDrag(e->GetMouse(), mViewport, mCameraController);
}

void MultiConvexMeshEditor::OnDoubleClick(MouseEvent* e)
{
  if(e->Handled)
    return;

  e->Handled = true;

  if(FindPointAtScreenPosition(e->Position) != NULL)
    return;

  AddPointAtScreenPosition(e->Position);
}

void MultiConvexMeshEditor::OnLeftMouseDrag(MouseDragEvent* e)
{
  if(e->Handled)
    return;

  e->Handled = true;
  new MultiSelectionManipulator(e, mViewport, this);
}

void MultiConvexMeshEditor::OnMouseUpdate(MouseEvent* e)
{
  if(e->Handled)
    return;

  //we're checking for the closest edge, so if there aren't any edges then don't do anything
  uint size = mPoints.Size();
  if(size <= 1)
    return;

  FindClosestEdge(e->Position, mClosestEdgeInfo);
  //if we're close enough to the edge then mark it as the
  //selected edge, otherwise clear our current selected edge
  if(mClosestEdgeInfo.mClosestDistance < ConvexMeshEditorUi::OuterContourWidth)
    mSelectedEdge = mClosestEdgeInfo.mClosestEdge;
  else
    ClearSelectedEdge();
}

void MultiConvexMeshEditor::OnMouseScroll(MouseEvent* e)
{
  if(e->Handled)
    return;

  mCameraController->MouseScroll(e->Scroll);
}

void MultiConvexMeshEditor::OnMouseExit(MouseEvent* e)
{
  ClearSelectedEdge();
}

void MultiConvexMeshEditor::OnKeyDown(KeyboardEvent* e)
{
  if(e->Handled)
    return;

  if(e->Key == Keys::F)
  {
    FocusOnPreviewCog();
    e->Handled = true;
  }
  if(e->Key == Keys::Z && e->CtrlPressed)
  {
    mQueue.Undo();
    e->Handled = true;
  }
  if(e->Key == Keys::Y && e->CtrlPressed)
  {
    mQueue.Redo();
    e->Handled = true;
  }
  //the user wants to delete the entire selection
  if(e->Key == Keys::Delete)
  {
    DeleteSelection();
  }
}

void MultiConvexMeshEditor::OnWidgetUpdate(Event* e)
{
  //this isn't the best time to debug draw the mesh because there's a
  //frame lag, figure out a better method later!
  DrawMesh();
  UpdateGridDrawing();
}

void MultiConvexMeshEditor::OnResourceRemoved(ResourceEvent* e)
{
  MultiConvexMesh* mesh = mMesh;
  //if the resource being removed is the one we're editing, close our window
  if(e->EventResource == mesh)
  {
    CloseTabContaining(this);
    return;
  }
}

void MultiConvexMeshEditor::OnMetaDrop(MetaDropEvent* e)
{
  SpriteSource* spriteSource = e->Instance.Get<SpriteSource*>();
  if(spriteSource != NULL)
  {
    if(e->Testing)
    {
      e->Result = String::Format("Display Sprite: %s", spriteSource->Name.c_str());
      return;
    }

    UpdatePreview(spriteSource);
    return;
  }

  //MultiConvexMesh* multiConvexMesh = e->Instance.As<MultiConvexMesh>(NULL);
  //if(multiConvexMesh != NULL)
  //{
  //  if(e->Testing)
  //  {
  //    e->Result = String::Format("Edit MultiConvexMesh: %s", multiConvexMesh->Name.c_str());
  //    return;
  //  }
  //
  //  SetMultiConvexMesh(multiConvexMesh);
  //  return;
  //}

  Archetype* archetype = e->Instance.Get<Archetype*>();
  if(archetype != NULL)
  {
    if(e->Testing)
    {
      e->Result = String::Format("Display Archetype: %s", archetype->Name.c_str());
      return;
    }

    UpdatePreview(NULL, archetype);
    return;
  }
}

void MultiConvexMeshEditor::OnSelectedPointXChanged(Event* e)
{
  OnSelectedPointChangedGeneric(mSelectedPointX, 0);
}

void MultiConvexMeshEditor::OnSelectedPointYChanged(Event* e)
{
  OnSelectedPointChangedGeneric(mSelectedPointY, 1);
}

void MultiConvexMeshEditor::OnSelectedPointChangedGeneric(TextBox* changedTextBox, uint axisChanged)
{
  //There was no selection so how did the text even change?
  if(mSelection.Empty())
    return;

  MarkMeshModified();

  //get the value that just changed
  real newValue;
  ToValue(changedTextBox->GetText(), newValue);

  mQueue.BeginBatch();
  mQueue.SetActiveBatchName("MultiConvexMeshEditor_PointChangedGeneric");
  //update the entire selection
  SelectionSet::range range = mSelection.All();
  for(; !range.Empty(); range.PopFront())
  {
    MultiConvexMeshPoint* point = range.Front();
    Vec3 oldPos = point->mWorldPoint;

    //only change the axis value that got changed
    point->mWorldPoint[axisChanged] = newValue;
    point->UpdateViewportPosition();

    //queue the undo for the old position to the new position
    mQueue.Queue(new PointMovementOp(point, oldPos));
  }
  mQueue.EndBatch(); 

  //see if this was a valid mesh
  TestConvexMeshes();
}

void MultiConvexMeshEditor::FindClosestEdge(Vec2Param testScreenPoint, ClosestEdgeInfo& info)
{
  info.mClosestDistance = Math::PositiveMax();
  info.mClosestEdge = (uint)-1;
  Vec3 testPoint = Vec3(mViewport->ScreenToViewport(testScreenPoint));

  //loop over each edge and find the which one is closest to the screen point
  uint size = mPoints.Size();
  for(uint i = 0; i < size; ++i)
  {
    MultiConvexMeshPoint* currPoint = mPoints[i];
    MultiConvexMeshPoint* nextPoint = mPoints[(i + 1) % size];

    Vec3 p0 = currPoint->mViewportPoint;
    Vec3 p1 = nextPoint->mViewportPoint;

    Vec3 result = testPoint;

    //find the point on the segment closest to our screen point
    Intersection::ClosestPointOnSegmentToPoint(p0, p1, &result);
    //if that point is closer than our best result then select it as the closest
    real distance = Math::Length(result - testPoint);
    if(distance < info.mClosestDistance)
    {
      info.mClosestDistance = distance;
      info.mClosestEdge = i;
      info.mClosestViewportPoint = result;
    }
  }
}

void MultiConvexMeshEditor::ClearSelectedEdge()
{
  mSelectedEdge = (uint)-1;
}

void MultiConvexMeshEditor::AddToSelection(MultiConvexMeshPoint* selectedPoint)
{
  mSelection.Insert(selectedPoint);
  UpdateSelectedPointText();
}

void MultiConvexMeshEditor::RemoveFromSelection(MultiConvexMeshPoint* selectedPoint)
{
  mSelection.Erase(selectedPoint);
  UpdateSelectedPointText();
}

void MultiConvexMeshEditor::DeleteSelection()
{
  MarkMeshModified();
  
  mQueue.BeginBatch();
  mQueue.SetActiveBatchName("MultiConvexMeshEditor_DeleteSelection");
  //delete every point in the selection
  SelectionSet::range range = mSelection.All();
  for(; !range.Empty(); range.PopFront())
  {
    MultiConvexMeshPoint* point = range.Front();

    //queue the undo for removing this point
    mQueue.Queue(new PointAddRemoveOp(point, false));
    mPoints.EraseValueError(point);
    point->Destroy();
  }
  mSelection.Clear();
  mQueue.EndBatch();

  //the selection is empty, update the text boxes
  UpdateSelectedPointText();
  //see if this was a valid mesh
  TestConvexMeshes();
}

void MultiConvexMeshEditor::ClearSelection()
{
  mSelection.Clear();
  UpdateSelectedPointText();
}

void MultiConvexMeshEditor::UpdateSelectedPointText()
{
  //the text boxes are active if the point is valid
  bool activeState = !mSelection.Empty();
  
  mSelectedPointX->SetEditable(activeState);
  mSelectedPointY->SetEditable(activeState);

  if(!mSelection.Empty())
  {
    //we have to determine if any points are in a conflicted state
    bool xIsSame = true;
    bool yIsSame = true;

    HashSet<MultiConvexMeshPoint*>::range range = mSelection.All();
    //grab the first point (if any other point is different then there's a conflict)
    MultiConvexMeshPoint* firstPoint = range.Front();
    range.PopFront();
    //check all other points
    for(; !range.Empty(); range.PopFront())
    {
      MultiConvexMeshPoint* point = range.Front();
      //if either the x or y is different from the first point then mark a conflict on that axis
      if(point->mViewportPoint.x != firstPoint->mViewportPoint.x)
        xIsSame = false;
      if(point->mViewportPoint.y != firstPoint->mViewportPoint.y)
        yIsSame = false;
    }

    //update the text accordingly (setting invalid sets the text to "-")
    if(xIsSame)
      mSelectedPointX->SetText(String::Format("%g", firstPoint->mWorldPoint.x));
    else
      mSelectedPointX->SetInvalid();
    if(yIsSame)
      mSelectedPointY->SetText(String::Format("%g", firstPoint->mWorldPoint.y));
    else
      mSelectedPointY->SetInvalid();
  }
  //there is no selected point, clear the text
  else
  {
    mSelectedPointX->SetText(String());
    mSelectedPointY->SetText(String());
  }
}

real MultiConvexMeshEditor::SamplePixelAlpha(Vec2Param pixelCoord, void* userData)
{
  uint x = (uint)pixelCoord.x;
  uint y = (uint)pixelCoord.y;
  SpriteSource* spriteSource = (SpriteSource*)userData;

  //bounds checking
  if(x < 0 || x >= (int)spriteSource->FrameSizeX)
    return real(0);
  if(y < 0 || y >= (int)spriteSource->FrameSizeY)
    return real(0);

  return ToFloatColor(spriteSource->SourceImage.GetPixel(x, y)).w;
}

real MultiConvexMeshEditor::SamplePixelIntensity(Vec2Param pixelCoord, void* userData)
{
  uint x = (uint)pixelCoord.x;
  uint y = (uint)pixelCoord.y;
  SpriteSource* spriteSource = (SpriteSource*)userData;

  //bounds checking
  if(x < 0 || x >= (int)spriteSource->FrameSizeX)
    return real(0);
  if(y < 0 || y >= (int)spriteSource->FrameSizeY)
    return real(0);

  Vec4 color = ToFloatColor(spriteSource->SourceImage.GetPixel(x, y));
  return (color.x + color.y + color.z) / real(3.0) * color.w;
}

Vec2 MultiConvexMeshEditor::SamplePixelWorldPosition(Vec2Param pixelCoord, void* userData)
{
  SpriteSource* spriteSource = (SpriteSource*)userData;

  Vec2 size = Vec2((real)spriteSource->FrameSizeX, (real)spriteSource->FrameSizeY);
  Vec2 flippedPixels = Vec2(pixelCoord.x, size.y - pixelCoord.y);
  Vec2 flippedOrigin = Vec2(spriteSource->OriginX, size.y - spriteSource->OriginY);
  Vec2 temp = flippedPixels - flippedOrigin;
  temp /= size;

  Vec2 extents = spriteSource->PixelSize / spriteSource->PixelsPerUnit;
  temp *= extents;

  return temp;
}

Vec2 MultiConvexMeshEditor::SamplePixelWorldPositionAtCenter(Vec2Param pixelCoord, void* userData)
{
  SpriteSource* spriteSource = (SpriteSource*)userData;
  return SamplePixelWorldPosition(pixelCoord + Vec2(0.5f, 0.5f), spriteSource);
}

void MultiConvexMeshEditor::SetMultiConvexMesh(MultiConvexMesh* multiConvexMesh)
{
  //update the name of the editor composite
  this->SetName(multiConvexMesh->Name);

  mMesh = multiConvexMesh;
  mPreviewMesh->mMesh = multiConvexMesh;

  ClearPoints(false);

  //the actual mesh has a thickness so it has twice as many points,
  //for the 2d editing only pull half of the points out
  uint halfVertexCount = multiConvexMesh->mVertices.Size() / 2;
  for(uint i = 0; i < halfVertexCount; ++i)
  {
    Vec3 pos = multiConvexMesh->mVertices[i];
    pos.z = 0;

    MultiConvexMeshPoint* newPoint = new MultiConvexMeshPoint(mViewport, this, pos);
    mPoints.PushBack(newPoint);
  }

  //recompute the thickness of the mesh
  if(multiConvexMesh->mVertices.Size() > 1)
  {
    real thickness = Math::Length(multiConvexMesh->mVertices[0] - multiConvexMesh->mVertices[halfVertexCount]);
    mPropertyViewInfo.mMeshThickness = thickness;
  }

  BuildConvexMeshes();
}

void MultiConvexMeshEditor::UpdatePreview(SpriteSource* spriteSource, Archetype* archetype)
{
  //since we don't know if the old preview was a sprite or an archetype it's
  //easier to just destroy and create it every time we set it to something new
  if(mPreviewCog != NULL)
    mPreviewCog->Destroy();

  //create the appropriate type of cog
  if(archetype != NULL)
    mPreviewCog = mPreviewSpace->CreateAtPosition(archetype, Vec3::cZero);
  else
    mPreviewCog = mPreviewSpace->CreateAt(CoreArchetypes::Sprite, Vec3::cZero);

  //if we were given a sprite source then try to set it on the root
  if(spriteSource != NULL)
  {
    Sprite* sprite = mPreviewCog->has(Sprite);
    if(sprite != NULL)
    {
      sprite->mSpriteSource = spriteSource;
    }
  }

  //make sure the property view is updated to display the
  //new sprite source (or none if we used an archetype)
  mPropertyViewInfo.mSpriteSource = spriteSource;
  mPropertyView->Refresh();
}

void MultiConvexMeshEditor::FocusOnPreviewCog()
{
  //get the combined aabb of the preview cog and the mesh
  Aabb aabb = GetAabb(mPreviewMesh->GetOwner());
  if(mPreviewCog != NULL)
  {
    Aabb previewCogAabb = GetAabb(mPreviewCog);
    aabb.Combine(previewCogAabb);
  }

  CameraFocusSpace(mPreviewSpace, mCameraController->GetOwner(), aabb, EditFocusMode::AutoTime);
}

bool MultiConvexMeshEditor::GetDrawGrid()
{
  return mShowGridButton->GetEnabled();
}

real MultiConvexMeshEditor::GetGridCellSize()
{
  return mGridCellSize;
}

void MultiConvexMeshEditor::SetGridCellSize(real cellSize)
{
  mGridCellSize = cellSize;
  mGridCellSizeTextBox->SetText(String::Format("%f", mGridCellSize));
}

void MultiConvexMeshEditor::SetGridSizeToPixels()
{
  SpriteSource* source = mPropertyViewInfo.mSpriteSource;
  if(source != NULL)
  {
    SetGridCellSize(1.0f / source->PixelsPerUnit);
  }
}

void MultiConvexMeshEditor::OnGridCellSizeChanged(Event* e)
{
  ToValue(mGridCellSizeTextBox->GetText(), mGridCellSize);
}

void MultiConvexMeshEditor::OnSetGridSizeToPixels(Event* e)
{
  SetGridSizeToPixels();
}

void MultiConvexMeshEditor::OnToggleShowGrid(Event* e)
{
  UpdateGridDrawing();
}

void MultiConvexMeshEditor::OnChangeSnappingMode(Event* e)
{
  mSnappingMode = mSnappingModeComboBox->GetSelectedItem();
}

void MultiConvexMeshEditor::UpdateGridDrawing()
{
  if(mGridDraw != NULL)
  {
    Vec3 viewCenterInWorld = mViewport->ScreenToWorldZPlane(mViewport->ViewportToScreen(mViewport->GetSize() / 2), 0);
    Vec3 viewCornerInWorld = mViewport->ScreenToWorldZPlane(mViewport->ViewportToScreen(Vec2::cZero), 0);
    Vec3 worldSize = viewCornerInWorld - viewCenterInWorld;

    float xCells = Math::Ceil(Math::Abs(worldSize.x) / GetGridCellSize());
    float yCells = Math::Ceil(Math::Abs(worldSize.y) / GetGridCellSize());
    mGridDraw->mLines = (uint)Math::Max(xCells, yCells) * 2 + 2;
    //and just double the number of lines to prevent frame behind issues when the camera moves/resizes
    mGridDraw->mLines *= 2;

    mGridDraw->mAxis = AxisDirection::Z;
    mGridDraw->mAlwaysDrawInEditor = GetDrawGrid();
    mGridDraw->mCellSize = GetGridCellSize();
    mGridDraw->mFollowEditorCamera = true;
  }
}

void MultiConvexMeshEditor::MarkMeshModified()
{
  if(Z::gEditor != NULL)
    MetaOperations::NotifyObjectModified(mMesh);
}

void MultiConvexMeshEditor::TestConvexMeshes()
{
  if(BuildConvexMeshes() == false)
  {
    mQueue.Undo();
    //remove the last action from the redo list (so the user can't redo to a bad state)
    mQueue.RedoCommands.PopBack();

    DoNotifyWarning("Invalid Mesh", "The mesh resulting from the last operation is invalid. The last operation will be undone.");
  }
}

bool MultiConvexMeshEditor::BuildConvexMeshes()
{
  MultiConvexMesh* mesh = mMesh;
  mesh->ClearSubMeshes();

  //convert the world point data to a format for decomposition
  Array<Vec2> vertices;
  for(uint i = 0; i < mPoints.Size(); ++i)
    vertices.PushBack(Math::ToVector2(mPoints[i]->mWorldPoint));

  //decompose into convex meshes (for now use the triangulator)
  ConvexDecomposition::SubShapeArray meshes;
  mIsValid = Create2dMeshes(vertices, ConvexDecomposition::ConvexMeshDecompositionMode::Triangulator, meshes);
  //if we failed to do convex decomposition then don't do anything
  if(mIsValid == false)
    return false;

  float halfThickness = mPropertyViewInfo.mMeshThickness * 0.5f;

  uint vertexCount = vertices.Size();
  //we need double the vertices since the mesh is extruded by a thickness
  mesh->mVertices.Resize(vertexCount * 2);
  for(uint i = 0; i < vertexCount; ++i)
  {
    Vec3 point = mPoints[i]->mWorldPoint;
    //add the front point then the back point
    //(making sure the back points are after all the front points)
    mesh->mVertices[i] = Vec3(point.x, point.y, halfThickness);
    mesh->mVertices[i + vertexCount] = Vec3(point.x, point.y, -halfThickness);
  }

  //now actually build each individual convex mesh from indices
  for(uint i = 0; i < meshes.Size(); ++i)
  {
    SubConvexMesh* newMesh = mesh->AddSubMesh();
    SubConvexMesh& subMesh = *newMesh;
    ConvexDecomposition::SubShape::IndexArray& subIndices = meshes[i].mIndices;

    uint indexCount = subIndices.Size();
    //like with the vertices, there's twice as many indices because of the back face
    subMesh.mIndices.Resize(indexCount * 2);
    for(uint j = 0; j < indexCount; ++j)
    {
      uint index = subIndices[j];

      subMesh.mIndices[j] = index;
      subMesh.mIndices[j + indexCount] = index + vertexCount;
    }

    //now add the triangle indices (only for debug drawing)

    //the front and back faces
    for(uint i = 1; i < indexCount - 1; ++i)
    {
      //front face
      subMesh.mTriangleIndices.PushBack(subIndices[0]);
      subMesh.mTriangleIndices.PushBack(subIndices[i]);
      subMesh.mTriangleIndices.PushBack(subIndices[i + 1]);
      //back face (make sure to flip the order of two vertices to have the correct winding order)
      subMesh.mTriangleIndices.PushBack(subIndices[0] + vertexCount);
      subMesh.mTriangleIndices.PushBack(subIndices[i + 1] + vertexCount);
      subMesh.mTriangleIndices.PushBack(subIndices[i] + vertexCount);
    }
    //the side faces
    for(uint i = 0; i < indexCount; ++i)
    {
      uint frontCurr = subIndices[i];
      uint frontNext = subIndices[(i + 1) % indexCount];
      uint backCurr = subIndices[i] + vertexCount;
      uint backNext = subIndices[(i + 1) % indexCount] + vertexCount;
    
      //face1
      subMesh.mTriangleIndices.PushBack(frontCurr);
      subMesh.mTriangleIndices.PushBack(backNext);
      subMesh.mTriangleIndices.PushBack(frontNext);
      //face2
      subMesh.mTriangleIndices.PushBack(frontCurr);
      subMesh.mTriangleIndices.PushBack(backCurr);
      subMesh.mTriangleIndices.PushBack(backNext);
    }
  }

  mesh->UpdateAndNotifyIfModified();

  //update the aabb of mesh for the new points
  if(mPreviewMesh != NULL)
    mPreviewMesh->ComputeWorldBoundingVolumes();

  return true;
}

void MultiConvexMeshEditor::DrawMesh()
{
  MultiConvexMesh* mesh = mMesh;

  //push on the space id of our preview space so we only debug draw in that space
  Debug::ActiveDrawSpace drawSpace(mPreviewSpace->GetRuntimeId());

  if(mPreviewMesh != NULL)
  {
    Transform* transform = mPreviewMesh->GetOwner()->has(Transform);
    Mat4 worldMat = transform->GetWorldMatrix();

    //depending on our draw mode draw the mesh filled, edges only or just don't draw it
    if(mPropertyViewInfo.mDrawMode == MultiConvexMeshDrawMode::Edges)
      mesh->Draw(worldMat, true, false);
    else if(mPropertyViewInfo.mDrawMode == MultiConvexMeshDrawMode::Filled)
      mesh->Draw(worldMat, true, true);
  }
}

}//namespace Zero
