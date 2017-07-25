///////////////////////////////////////////////////////////////////////////////
///
/// \file ClothTools.hpp
/// Declaration of the Cloth Tool classes.
/// 
/// Authors: Joshua Claeys
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class ToolUiEvent;
class SpringSystem;
class SpringSystemConnection;
class SpringTools;

DeclareEnum5(SpringSubTools, Anchoring, PointSelector, SpringSelector, SpringCreator, RopeCreator);

//-------------------------------------------------------------------SpringSubTool
/// A sub-tool for spring systems. Is notified of basic mouse events as they happen.
class SpringSubTool : public Object
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  virtual ~SpringSubTool() {};

  // Dragging
  virtual void OnMouseDragStart(ViewportMouseEvent* e) {};
  virtual void OnMouseDragMove(ViewportMouseEvent* e) {};
  virtual void OnMouseEndDrag(Event* e) {};

  virtual void Draw(){};

  // Mouse down/up
  virtual void OnLeftMouseDown(ViewportMouseEvent* e) {};
  virtual void OnLeftMouseUp(ViewportMouseEvent* e) {};
  virtual void OnKeyDown(KeyboardEvent* e) {};


  // Get the current selected spring system (can be null)
  SpringSystem* GetSystem();
  // Helper that finds out what point we would select on a spring system
  // from a ray (if we snap to the closest point on a triangle face)
  bool RayCastSpringSystem(const Ray& ray, SpringSystem* system, Vec3& closestPoint, uint& index);
  // Raycast against a cog and determine what point we hit.
  // If this is an object with a spring system then we'll snap to a
  // vertex position, otherwise we'll use the ray intersection point.
  bool RayCastCog(ViewportMouseEvent* e, CogId& hitCog,
                  Vec3& hitPoint, uint& hitIndex);

  // The current system that is selected (may be an invalid object)
  CogId mSelectedSystem;
};

//-------------------------------------------------------------------DragSelectSubTool
/// A sub-tool for spring systems that handles turning a drag into a frustum. 
/// This also takes care of rendering the visual element for the drag.
class DragSelectSubTool : public SpringSubTool
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  DragSelectSubTool();
  
  // New interface function that tells you a frustum is being dragged with these screen coordinates.
  virtual void MouseDragFrustum(Viewport* viewport, Vec2Param upperLeftScreen, Vec2Param lowerRightScreen) {};

  void OnMouseDragStart(ViewportMouseEvent* e) override;
  void OnMouseDragMove(ViewportMouseEvent* e) override;
  void OnMouseEndDrag(Event* e) override;

  // The visual element to show where we are selecting
  HandleOf<Element> mDragElement;
  Vec2 mMouseStartPosition;
};

//-------------------------------------------------------------------SelectorSpringSubTool
class SelectorSpringSubTool : public DragSelectSubTool
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  SelectorSpringSubTool();

  // New interface function that casts a frustum against something.
  // This expects mCurrentSelection to have points added to it if something hits the frustum.
  virtual void CastFrustum(const Frustum& frustum) {};

  void OnMouseDragStart(ViewportMouseEvent* e) override;
  void OnMouseDragMove(ViewportMouseEvent* e) override;
  void OnLeftMouseUp(ViewportMouseEvent* e) override;

  
  typedef HashSet<uint> PointMap;
  // Simple helper to get what indices are selected on the spring system.
  void GetSelection(PointMap& indices);

  /// The indices we are currently selecting (during this drag).
  /// This list is constantly updating and being cleared as the mouse moves.
  PointMap mCurrentSelection;
  /// The indices that we selected from our previous drag (since we can multi-drag select).
  PointMap mPreviousSelection;
};

//-------------------------------------------------------------------PointMassSelectorSubTool
/// Base class for spring-sub-tools that multi-select point masses.
class PointMassSelectorSubTool : public SelectorSpringSubTool
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  PointMassSelectorSubTool();

  void Draw() override;
  void CastFrustum(const Frustum& frustum) override;
};

//-------------------------------------------------------------------AnchoringSubTool
/// Spring sub-tool used to anchor point-masses to other objects.
/// Uses a frustum to figure out what points to select and then creates a
/// proxy view of the combined values of the point masses.
class AnchoringSubTool : public PointMassSelectorSubTool
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  AnchoringSubTool();

  void Draw() override;
  
  /// Do we draw all points that are anchored (even though they aren't selected).
  bool mDrawAnchoredPoints;
  Vec4 mAnchoredPointMassColor;
};

//-------------------------------------------------------------------PointSelectorSubTool
class PointSelectorSubTool : public PointMassSelectorSubTool
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  PointSelectorSubTool();
};

//-------------------------------------------------------------------SpringSelectorSubTool
class SpringSelectorSubTool : public SelectorSpringSubTool
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  SpringSelectorSubTool();

  void Draw() override;
  void CastFrustum(const Frustum& frustum) override;
};

//-------------------------------------------------------------------SpringCreatorSubTool
class SpringCreatorSubTool : public SpringSubTool
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  SpringCreatorSubTool();

  void Draw() override;

  void OnMouseDragStart(ViewportMouseEvent* e) override;
  void OnMouseDragMove(ViewportMouseEvent* e) override;
  void OnMouseEndDrag(Event* e) override;

  void ConnectDifferentSystems();

  bool mIsDragging;

  CogId mStartCog;
  CogId mEndCog;
  uint mStartIndex;
  uint mEndIndex;
  Vec3 mStartPos;
  Vec3 mEndPos;
};

//-------------------------------------------------------------------RopeCreatorSubTool
/// A spring sub-tool to create a SpringRope between two objects.
class RopeCreatorSubTool : public SpringSubTool
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  RopeCreatorSubTool();

  void Draw() override;

  void OnLeftMouseDown(ViewportMouseEvent* e) override;
  void OnMouseDragMove(ViewportMouseEvent* e) override;
  void OnMouseEndDrag(Event* e) override;

  void OnKeyDown(KeyboardEvent* e) override;

  //Clear the current rope selection (so we don't select anything)
  void Clear();

  uint GetNumberOfLinks();
  void SetNumberOfLinks(uint numberOfLinks);

  bool mIsDragging;

  CogId mStartCog;
  CogId mEndCog;
  Vec3 mStartPos;
  Vec3 mEndPos;
  uint mStartIndex;
  uint mEndIndex;

  uint mNumberOfLinks;
};

//-------------------------------------------------------------------SpringPointProxy
/// A proxy component to represent a point mass for the spring system.
/// Combines the properties of a collection of selected points and allows
/// group changing values of the point masses.
class SpringPointProxy : public EventObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  SpringPointProxy();

  bool GetFixed();
  void SetFixed(bool fixed);
  PropertyState GetFixedState();

  Cog* GetAnchor();
  void SetAnchor(Cog* object);
  PropertyState GetAnchorState();

  real GetMass();
  void SetMass(real mass);
  PropertyState GetMassState();

  AnchoringSubTool* mAnchorTool;
};
//-------------------------------------------------------------------SpringPointProxyProperty
// Special property class to redirect get properties on an object to a function
// that can return a property state (which is 3 states instead of 2: true, false, conflicted)
class SpringPointProxyProperty : public PropertyInterface
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  /// Returns whether or not the value is valid. For example, it could be
  /// invalid if this is a multi-selection and there is a conflict between
  /// the values on multiple objects.
  PropertyState GetValue(HandleParam object, PropertyPathParam property) override;
};

//-------------------------------------------------------------------SpringTools
class SpringTools : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  SpringTools();
  ~SpringTools();

  void Initialize(CogInitializer& initializer);
  
  void OnToolDraw(Event* e);
  void OnLeftMouseDown(ViewportMouseEvent* e);
  void OnLeftMouseUp(ViewportMouseEvent* e);
  void OnMouseMove(ViewportMouseEvent* e);

  // Dragging
  void OnMouseDragStart(ViewportMouseEvent* e);
  void OnMouseDragMove(ViewportMouseEvent* e);
  void OnMouseEndDrag(Event* e);

  void OnKeyDown(KeyboardEvent* e);

  // Helper to get a spring system from the current selection
  SpringSystem* GetSpringSystem();
  void SetEditingSystem(SpringSystem* system);

  /// What sub tool is currently active?
  SpringSubTools::Enum GetCurrentSubToolType();
  void SetCurrentSubToolType(SpringSubTools::Enum toolType);

  void OnGetToolInfo(ToolUiEvent* e);


  Array<SpringSubTool*> mSubTools;
  SpringSubTool* mCurrentSubTool;
  SpringSubTools::Enum mCurrentSubToolType;
  SpringPointProxyProperty mPointProxyPropertyInterface;

  // The position of the mouse in screen space from mouse down.
  // Used to determine if we've moved far enough to start dragging.
  Vec2 mMouseDownScreenPosition;
  // Used to determine if the mouse was down during the mouse move
  bool mMouseIsDown;

  CogId mSelectedSystem;

  SpringPointProxy mProxy;
  PropertyView* mPropertyView;
};

}//namespace Zero
