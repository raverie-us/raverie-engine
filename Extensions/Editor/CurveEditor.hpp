///////////////////////////////////////////////////////////////////////////////
///
/// \file CurveEditor.hpp
/// Declaration of the CurveEditor Widget and CurveEditor Composite.
/// 
/// Authors: Joshua Claeys
/// Copyright 2012-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Typedefs
typedef u64 CurveId;

/// Forward Declarations
class CurveEditor;
class GraphWidget;
class CurveDrawer;
class Window;
class GraphWidget;
class TextBox;
class CurveDrawer;
class CurveObject;
class ButtonBase;

/// Events
namespace Events
{
  /// Debug samples should be pushed when this is sent on the Curve editor
  DeclareEvent(PushDebugSamples);
  /// Sent whenever anything on the curve has been modified
  DeclareEvent(CurveModified);
  DeclareEvent(ControlPointAdded);
  DeclareEvent(ControlPointModified);
  /// Sent when a given control point is deleted.
  DeclareEvent(ControlPointDeleted);
  /// Sent when the mouse enters the curve
  DeclareEvent(MouseEnterCurve);
  /// Sent when the mouse leaves the curve
  DeclareEvent(MouseExitCurve);
  /// Sent when the curve is double clicked
  DeclareEvent(CurveDoubleClicked);
}

namespace CurveEditing
{
DeclareBitField4(CurveEditorFlags, TangentsSplit, LinearIn, LinearOut,
                                   NonWeighted);
class CurveEditorToolbar;
class Draggable;
class ControlPoint;
class Tangent;
}//namespace CurveEditing

//------------------------------------------------------------------ Curve Event
class CurveEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  CurveEvent() : mControlPoint(NULL){};

  /// The curve that was modified.
  CurveObject* mCurve;

  /// The control point that was modified.
  CurveEditing::ControlPoint* mControlPoint;
};

//----------------------------------------------------------------- Curve Object
/// Represents a single curve edited by the curve editor.
class CurveObject : public EventObject
{
public:
  typedef CurveEditing::ControlPoint ControlPoint;
  typedef Array<ControlPoint*> ControlPointArray;

  CurveObject(CurveEditor* editor);

  /// Clears all control points in the curve.
  void Clear();

  /// Destroys the curve and removes it from the curve editor.
  void Destroy();

  /// Samples the curve at the given time.
  virtual float Sample(float t);

  /// Bakes the curve to the given array.
  virtual void GetCurve(Vec3Array& curve);

  /// Creates a control point and adds it to the curve.
  ControlPoint* CreateControlPoint(Vec2Param pos, Vec2Param tanIn,
                                   Vec2Param tanOut, 
                                   CurveEditing::CurveEditorFlags::Type flags);

  /// Returns whether or not the mouse is over the curve.
  bool IsMouseOver(Vec2Param pixelPos);

  /// Adds a control point to the current curve.  Returns the index of 
  /// its position.
  uint AddControlPoint(ControlPoint* controlPoint);

  /// Creates a new control point on the given position.
  ControlPoint* AddNewControlPoint(Vec2Param pos);

  /// Finds and removes the given control point.
  virtual void RemoveControlPoint(ControlPoint* controlPoint);

  /// Sets the display color of the curve and control points.
  void SetCurveColor(ByteColor color);

  /// Returns the current line thickness (it changes when the mouse is over
  /// the curve).
  float GetCurrentLineThickness();

  /// Dispatches an event of type CurveEvent.
  void DispatchCurveEvent(StringParam eventName,
                          ControlPoint* controlPoint = NULL);

  void Modified(ControlPoint* controlPoint);

  /// Returns all the control points.
  ControlPointArray::range GetControlPoints(){return mControlPoints.All();}

  /// Used to sort the control points by the x-position.
  struct SortByX
  {
    bool operator()(const ControlPoint* left, const ControlPoint* right);
  };

  /// The color of the curve.
  ByteColor mCurveColor;

  /// The control points of this curve.
  ControlPointArray mControlPoints;

  /// Whether or not the mouse is currently over the curve.
  bool mMouseOverCurve;

  /// The allowed distance of a point on the curve from the baked approximation.
  float mError;

  /// Pointer back to the editor that owns us.
  CurveEditor* mEditor;
};

//----------------------------------------------------------------- Curve Editor
class CurveEditor : public Composite
{
public:
  /// Typedefs
  typedef CurveEditor ZilchSelf;
  typedef CurveEditing::CurveEditorToolbar CurveEditorToolbar;
  typedef CurveEditing::Draggable Draggable;
  typedef CurveEditing::ControlPoint ControlPoint;
  typedef CurveEditing::Tangent Tangent;
  typedef CurveObject::ControlPointArray ControlPointArray;
  typedef Array<CurveObject*>::range CurveObjectRange;
  typedef HashSet<Draggable*>::range SelectionRange;

  /// Constructor.
  CurveEditor(Composite* parent);
  ~CurveEditor();

  /// Composite Interface.
  void UpdateTransform() override;

  CurveObjectRange GetCurveObjects();

  /// Conversion functions.
  virtual Vec2 ToPixelPosition(Vec2 graphPos);
  virtual Vec2 ToGraphPosition(Vec2 pixelPos);
  virtual Vec2 ClampPixelPosition(Vec2 pixelPos);

  /// Used to override what the x and y edit text boxes display.
  virtual String GraphToDisplayTextX(float graphValue);
  virtual String GraphToDisplayTextY(float graphValue);
  virtual float DisplayTextToGraphX(StringParam displayText);
  virtual float DisplayTextToGraphY(StringParam displayText);

  /// Called when the mouse is close to the edges while dragging.
  virtual void MouseDragUpdate(MouseEvent* e){}

  /// Returns the first curve that the mouse is over.
  CurveObject* GetMouseOverCurve();

  /// Adds the given draggable to the selection. Clears the selection if specified
  void SelectDraggable(Draggable* draggable, bool singleSelect = true);
  bool IsSelected(Draggable* draggable);
  void DeSelect(Draggable* draggable);
  SelectionRange GetSelection();
  
  /// Clears all selections.
  void ClearSelection();

  /// Deletes all selected control points.
  void DeleteSelected();

  /// Hides all widgets used to display the curve.
  void Hide();

  /// Re-displays all hidden widgets.
  void Show();

  /// Allows controls to be placed elsewhere that control this editor.
  void RegisterToolBar(CurveEditing::CurveEditorToolbar* toolBar);
  void RegisterLinearTangentsButton(ButtonBase* button);
  void RegisterSplitTangentsButton(ButtonBase* button);
  void RegisterWeightedTangentsButton(ButtonBase* button);

protected:
  friend class MultiCurveEditor;
  friend class CurveEditing::ControlPoint;
  friend class CurveEditing::Tangent;
  friend class CurveObject;

  void AddCurveObject(CurveObject* curve);
  void RemoveCurveObject(CurveObject* curve);

  /// Event response.
  void OnMouseMove(MouseEvent* event);
  void OnLeftClick(MouseEvent* event);
  void OnDoubleClick(MouseEvent* event);
  void OnLeftDrag(MouseDragEvent* event);
  void OnKeyDown(KeyboardEvent* event);

  /// Tangent button response.
  void OnLinearTangentsPressed(ObjectEvent* e);
  void OnSplitTangentsPressed(ObjectEvent* e);
  void OnWeightedTangentsPressed(ObjectEvent* e);

  /// A handle to the right click context menu.
  HandleOf<ContextMenu> mContextMenuHandle;

  CurveEditorToolbar* mToolbar;

  HashSet<Draggable*> mSelection;

public:
  /// Whether or not the curve editor is enabled.
  bool mEnabled;

  /// Draws the curve on top of the graph.
  CurveDrawer* mCurveDrawer;

  /// The line thickness of the curve.
  float mDefaultLineThickness; 
  
  /// The line thickness of the curve when the mouse is over it.
  float mHighlightThickness;

  /// Whether or not to highlight the curve when the mouse is over it.
  bool mHighlightOnMouseEnter;

  /// If enabled, clamps the dragging of control points to the graph.
  bool mClampMouseDrag;

  /// All displayed curves.
  Array<CurveObject*> mCurves;

  /// Used to keep track of what the mouse is over.
  HandleOf<Draggable> mMouseOver;

  /// Debugging.
  struct DebugSample
  {
    float Time;
    String Id;
    Vec4 Color;
  };
  Array<DebugSample> mDebugSamples;
};

namespace CurveEditing
{

class Draggable;

//--------------------------------------------------------- Curve Editor Toolbar
class CurveEditorToolbar : public Composite
{
public:
  typedef CurveEditorToolbar ZilchSelf;

  CurveEditorToolbar(Composite* parent, float textBoxWidth = Pixels(50));

  /// Widget Interface.
  void UpdateTransform() override;

  /// Pulls the selection and sets up the text boxes.
  void Update();

  /// Event response from the text boxes changing.
  void OnXChanged(Event* e);
  void OnYChanged(Event* e);

private:
  friend class CurveEditor;

  //---------------------------------------------------------------------- range
  /// Filters out tangents in the selection.
  struct range
  {
    range(CurveEditor::SelectionRange r);

    Draggable* Front();
    bool Empty();
    void PopFront();
    void FindNextControlPoint();

    CurveEditor::SelectionRange mRange;
  };

  /// A range that only Contains control points.
  range allControlPoints();

  /// Will be called by the curve editor when this is registered with the editor
  void SetCurveEditor(CurveEditor* editor);

  /// When the selection has changed, we need to update the text boxes.
  void OnSelectionChanged(Event* e);

  CurveEditor* mEditor;
  TextBox* mPositionX, *mPositionY;
};

//-------------------------------------------------------------------- Draggable
class Draggable : public Widget
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  Draggable(CurveObject* curveObject);
  ~Draggable();

  bool IsSelected();

  /// Event response.
  void OnLeftClick(MouseEvent* e);
  void OnMouseDown(MouseEvent* e);
  void OnMouseDrag(MouseEvent* e);
  void OnMouseEnter(MouseEvent* e);
  void OnMouseExit(MouseEvent* e);

  void StartDrag(Mouse* mouse);

  virtual void MoveTo(Vec2Param pixelPos, bool clampToScreen = true){}
  virtual void FinishedDrag(){}
  virtual Vec2 GetDisplayPosition(){return Vec2::cZero;}

  virtual void Select();
  virtual void DeSelect();

  bool mMouseOver;

//protected:
  CurveObject* mCurve;
  CurveEditor* mEditor;
};

//---------------------------------------------------------------- Control Point
class Tangent;

class ControlPoint : public Draggable
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor / destructor.
  ControlPoint(CurveObject* parent, Vec2Param pos, 
               Vec2Param tanIn, Vec2Param tanOut, uint editorFlags = 0);
  ~ControlPoint();

  /// Widget Interface.
  void UpdateTransform() override;

  /// Returns whether or not the tangents should be visible.
  bool TangentsVisible();

  /// Position setting.
  void SetGraphPosition(Vec2Param graphPosition);
  Vec2 GetGraphPosition();
  void SetPixelPosition(Vec2Param pixelPosition, bool clampToScreen);
  Vec2 GetPixelPosition();
  
  /// Draggable Interface.
  void MoveTo(Vec2Param pixelPos, bool clampToScreen) override;
  void FinishedDrag() override;
  Vec2 GetDisplayPosition() override {return mGraphPosition;}

  void UpdateNeighborLinearTangents();
  void UpdateLinearTangents();

  void Delete();

  /// Event response.
  void OnRightMouseUp(MouseEvent* e);
  void OnDelete(ObjectEvent* e);
  void OnSplitTangents(ObjectEvent* e);
  void OnLinearTangents(ObjectEvent* e);
  void OnWeightedTangents(ObjectEvent* e);
  void OnNormalizeTangents(ObjectEvent* e);
  Tangent* GetOther(Tangent* tangent);

  /// Tangent in setter/getter.
  Vec2 GetTangentIn();
  void SetTangentIn(Vec2Param direction);

  /// Tangent out setter/getter.
  Vec2 GetTangentOut();
  void SetTangentOut(Vec2Param direction);

  bool TangentsSplit();
  bool IsLinear();
  bool InIsLinear();
  bool OutIsLinear();
  bool IsWeighted();
  
  ControlPoint* GetNeighborControlPoint(int direction);

  Vec2 mGraphPosition;
  Tangent* mTangentIn;
  Tangent* mTangentOut;
  CurveEditorFlags::Type mEditorFlags;

  void* mClientData;
};

DeclareEnum2(TangentSide, In, Out);

//---------------------------------------------------------------------- Tangent
class Tangent : public Draggable
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  Tangent(CurveObject* curve, ControlPoint* controlPoint, Vec2Param direction,
          TangentSide::Type side);

  /// Position setting. Setting either pixel or graph position will properly
  /// Update both the graph and pixel directions based on the current mode.
  void SetGraphDirection(Vec2Param graphDirection);
  Vec2 GetGraphDirection();
  void SetPixelDirection(Vec2Param pixelDirection, bool clampToScreen = true);
  Vec2 GetPixelDirection();

  /// Gets the tangents position in pixels
  Vec2 GetPixelPosition();

  /// Draggable Interface.
  void MoveTo(Vec2Param pixelPos, bool clampToScreen) override;
  void FinishedDrag() override;
  Vec2 GetDisplayPosition() override {return mGraphDirection;}

  /// Event response
  void OnRightMouseUp(MouseEvent* e);
  void OnNormalize(ObjectEvent* e);
  void OnLinearPressed(ObjectEvent* e);
  void OnWeightedPressed(ObjectEvent* e);

  void Normalize();
  bool IsLinear();
  bool IsWeighted();

  /// Which side the 
  TangentSide::Type mSide;
  ControlPoint* mControlPoint;

  /// The true, weighted direction that is used when building the curve.
  Vec2 mGraphDirection;
};

}//namespace CurveEditing

//----------------------------------------------------------------- Curve Drawer
class CurveDrawer : public Widget
{
public:
  CurveDrawer(CurveEditor* curveEditor);

  /// Draw the widget.
  void RenderUpdate(ViewBlock& viewBlock, FrameBlock& frameBlock, Mat4Param parentTx, ColorTransform colorTx, Rect clipRect) override;

  void AddCurve(ViewBlock& viewBlock, FrameBlock& frameBlock, Rect clipRect, CurveObject* curveObject);
  void AddControlPoints(ViewBlock& viewBlock, FrameBlock& frameBlock, Rect clipRect, CurveObject* curveObject);
  void AddPoint(Array<StreamedVertex>& lines, Array<StreamedVertex>& triangles, Vec3Param pos, float size, Vec4 color, bool empty = false);

  CurveEditor* mCurveEditor;
};

}//namespace Zero
