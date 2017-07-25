///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Ryan Edgemon, Josh Claeys
/// Copyright 2016-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//------------------------------------------------------------------- Events ---
namespace Events
{
DeclareEvent(ToolCreateGizmoEvent);
DeclareEvent(DuplicateFirstChance);
}

class ToolGizmoEvent : public Event
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  ToolGizmoEvent(Cog* gizmo) { mGizmo = gizmo; }

public:
  Cog* mGizmo;
};

//------------------------------------------------------------- GizmoCreator ---
class GizmoCreator : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  GizmoCreator();

  void Serialize(Serializer& stream);
  void Initialize(CogInitializer& initializer);

  void OnDestroy(uint flags = 0) override;
  void DestroyGizmo();

  Archetype* GetGizmoArchetype();
  void SetGizmoArchetype(Archetype* gizmoArchetype);

  Cog* GetGizmo(Space* space);

public:
  HandleOf<Archetype> mGizmoArchetype;
  HandleOf<Cog> mGizmo;
  HandleOf<Space> mSpace;
};


//------------------------------------------------------ ObjectTransformTool ---
class ObjectTransformTool : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ObjectTransformTool();

  void Serialize(Serializer& stream);
  void Initialize(CogInitializer& initializer);
  void DestroyGizmo();

  void RegisterNewGizmo();

  void OnToolActivate(Event* event);
  void OnToolDeactivate(Event* event);
  void OnToolGizmoCreated(ToolGizmoEvent* event);

  virtual void OnFrameUpdate(UpdateEvent* event);

  void OnComponentsChanged(Event* event);
  void OnSelectionChanged(Event* event);
  void OnFinalSelectionChanged(Event* event);
  void OnKeyDown(KeyboardEvent* e);

  void UpdateGrabState(GizmoGrabMode::Enum state);

  GizmoGrabMode::Enum GetGrab();
  void SetGrab(GizmoGrabMode::Enum grab);

  GizmoBasis::Enum GetBasis();
  void SetBasis(GizmoBasis::Enum basis);

  GizmoPivot::Enum GetPivot();
  void SetPivot(GizmoPivot::Enum pivot);

  Space* GetSpaceFromSelection(MetaSelection* selection);

  virtual void GizmoCreated(Cog* gizmo) {};
  virtual void CopyPropertiesToGizmo() {};

public:
  bool mChangeFromUs;
  CogId mGizmo;

  MetaSelection mRegisteredObjects;

  GizmoGrabMode::Enum mGrab;
  GizmoBasis::Enum mBasis;
  GizmoPivot::Enum mPivot;

  bool mSnapping;
  float mSnapDistance;
};

//------------------------------------------------------ ObjectTranslateTool ---
class ObjectTranslateTool : public ObjectTransformTool
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ObjectTranslateTool();

  void Serialize(Serializer& stream);
  void Initialize(CogInitializer& initializer);

  GizmoDragMode::Enum GetDragMode( );

  bool GetSnapping( );
  void SetSnapping(bool snapping);

  float GetSnapDistance();
  void SetSnapDistance(float distance);

  GizmoSnapMode::Enum GetSnapMode( );
  void SetSnapMode(GizmoSnapMode::Enum mode);

  void OnGizmoObjectsDuplicated(Event* event);

  void GizmoCreated(Cog* gizmo) override;
  void CopyPropertiesToGizmo() override;

public:
  GizmoSnapMode::Enum mSnapMode;
};

//---------------------------------------------------------- ObjectScaleTool ---
class ObjectScaleTool : public ObjectTransformTool
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ObjectScaleTool();

  void Serialize(Serializer& stream);
  void Initialize(CogInitializer& initializer);

  void OnFrameUpdate(UpdateEvent* event) override;

  GizmoDragMode::Enum GetDragMode( );

  bool GetSnapping( );
  void SetSnapping(bool snapping);

  void SetSnapDistance(float distance);
  float GetSnapDistance();

  GizmoSnapMode::Enum GetSnapMode( );
  void SetSnapMode(GizmoSnapMode::Enum mode);

  Vec3 GetChangeInScale();

  void CopyPropertiesToGizmo() override;

public:
  GizmoSnapMode::Enum mSnapMode;
  bool mAffectTranslation;
  Vec3 mChangeInScale;
};

//---------------------------------------------------------- ObjectScaleTool ---
class ObjectRotateTool : public ObjectTransformTool
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ObjectRotateTool();

  void Serialize(Serializer& stream);
  void Initialize(CogInitializer& initializer);

  void OnFrameUpdate(UpdateEvent* event) override;

  bool GetSnapping( );
  void SetSnapping(bool snapping);

  float GetSnapAngle();
  void SetSnapAngle(float angle);

  void CopyPropertiesToGizmo() override;
  
public:
  bool mAffectTranslation;
  float mChangeInRotation;
};

}// end namespace Zero
