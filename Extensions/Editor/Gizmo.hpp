///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Forward Declarations
class Editor;
class MouseCapture;
class ViewportMouseEvent;

//------------------------------------------------------------------------- Tags
namespace Tags
{
  DeclareTag(Gizmo);
}

namespace Events
{
  DeclareEvent(GizmoRayTest);
  DeclareEvent(GizmoModified);
  /// Sent on root Gizmo's when the target has been set. This only happens with
  /// Gizmos that were created by the editor to edit a specific Component.
  /// This event is only sent because in their Initialize, the editing object
  /// won't be accessible (it's set right after creation).
  DeclareEvent(GizmoTargetSet);
  DeclareEvent(MouseEnterGizmo);
  DeclareEvent(MouseEnterGizmoHierarchy);
  DeclareEvent(MouseExitGizmo);
  DeclareEvent(MouseExitGizmoHierarchy);
}

//------------------------------------------------------------------ Gizmo Event
class GizmoEvent : public Event
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  GizmoEvent(Cog* gizmo, ViewportMouseEvent* e);

  /// Getters.
  Cog* GetGizmo();
  ViewportMouseEvent* GetViewportMouseEvent();

  /// The Gizmo corresponding to this event.
  CogId mGizmo;

  /// This can be null if the gizmo was disabled while the mouse was over it.
  ViewportMouseEvent* mMouseEvent;
};

//--------------------------------------------------------- Gizmo Ray Test Event
/// Sent to all Gizmos when the mouse moves on the view-port. All Gizmos should
/// respond to this event and add ray-cast results to it through RegisterResult.
class GizmoRayTestEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  /// Constructor.
  GizmoRayTestEvent();

  /// This result will be 
  void RegisterResult(Cog* gizmo, float distance, int pickingPriority);

  /// Easy access to the world ray on the mouse event.
  Ray GetWorldRay();

  ViewportMouseEvent* mMouseEvent;

  Cog* mGizmoHit;
  float mHitDistance;
  int mPickingPriority;
};

//------------------------------------------------------------------------ Gizmo
/// Registers itself with the GizmoSpace. This allows GizmoSpace to keep track
/// of which Gizmo the mouse is over, as well as send other input events
/// directly to Gizmos.
class Gizmo : public Component
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Constructor.
  Gizmo();

  /// Component Interface.
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  /// The object the gizmo was created for.
  void SetTargetObject(Cog* cog);

  /// Because only the root Gizmo's are stored on the space, whenever we're
  /// attached or detached, we need to respectfully update ourself in that list.
  void AttachTo(AttachmentInfo& info) override;
  void Detached(AttachmentInfo& info) override;

  void ForwardEvent(Event* e);

  Cog* GetEditingObject();

  /// Active setter / getter.
  void SetActive(bool state);
  bool GetActive();

  bool GetMouseOver();

  /// If set to false, it will not receive input events.
  bool mActive;

  /// Whether or not the mouse is currently over the gizmo.
  bool mMouseOver;

  /// You may want to manually forward the input events to specific children.
  bool mForwardEventsToChildren;

  /// Needed?
  CogId mEditingObject;
};

//------------------------------------------------------------------ Gizmo Space
class GizmoSpace : public Component
{
public:
  /// Meta Initialization.
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// Component Interface.
  void Initialize(CogInitializer& initializer) override;
  bool ShouldSerialize() override { return false; }

  /// Registers the given Gizmo with the Space.
  void AddOrUpdateGizmo(Cog* cog);

  /// When the mouse is updated on the view-port, this will update which Gizmo
  /// the mouse is currently over.
  void OnMouseUpdate(ViewportMouseEvent* e);

  /// Forwards the given event to the gizmo that the mouse is currently over.
  void ForwardEvent(Event* e);

  /// Forwards the given event to all root Gizmos.
  void ForwardEventToAllGizmos(Event* e);

  /// The gizmo that the mouse is currently over
  CogId mMouseOverGizmo;

  /// Contains only the root Gizmos.
  HashSet<CogId> mRootGizmos;
};

}//namespace Zero
