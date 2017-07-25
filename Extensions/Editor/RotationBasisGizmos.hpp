///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Events
{
DeclareEvent(AddRotationBasisGizmoObject);
DeclareEvent(RotationBasisGizmoBegin);
DeclareEvent(RotationBasisGizmoModified);
DeclareEvent(RotationBasisGizmoEnd);
DeclareEvent(RotationBasisAabbQuery);
}//namespace Events

 //-------------------------------------------------------------------RotationBasisGizmoInitializationEvent
 /// Event sent to a rotation basis gizmo to initialize.
class RotationBasisGizmoInitializationEvent : public ObjectEvent
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  RotationBasisGizmoInitializationEvent();

  int mIntData;
};

//-------------------------------------------------------------------RotationBasisGizmoAabbQueryEvent
/// Event sent by a rotation basis gizmo to find out what the selection aabb is.
class RotationBasisGizmoAabbQueryEvent: public ObjectEvent
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  RotationBasisGizmoAabbQueryEvent();

  Aabb mAabb;
};

//-------------------------------------------------------------------RotationBasisGizmoMetaTransform
/// Allows a rotation basis gizmo to override the transform data (for selection).
class RotationBasisGizmoMetaTransform : public MetaTransform
{
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  MetaTransformInstance GetInstance(HandleParam object) override;
};

//-------------------------------------------------------------------RotationBasisGizmo
/// Gizmo for modifying an object's basis. Basically a rotation gizmo with arrows and text that label what each basis means.
class RotationBasisGizmo : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  RotationBasisGizmo();

  //-------------------------------------------------------------------Component Interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  /// Exposed property so the user can manually set a basis' world rotation
  Quat GetWorldRotation();
  void SetWorldRotation(QuatParam rotation);
  void SetWorldRotationInternal(QuatParam rotation);
  /// Set everything necessary for the gizmo to display a new world rotation without affecting
  /// cached dragging. Mostly for properly updating each frame to catch undo operations.
  void SetGizmoWorldRotationInternal(QuatParam rotation);
  
  void ActivateAsGizmo();
  // Set ourself as the active selection. Used so that the world rotation can be modified as a property.
  void SetAsSelection();
  void DrawBasisText(QuatParam axisRotation, StringParam axisText, Vec3Param localOffset);

  /// Get the aabb of everything currently selected by this gizmo
  Aabb GetSelectionAabb();
  /// Helper to get a default aabb for cogs
  Aabb GetCogAabb(Cog* cog);

  void OnGizmoDragStart(Event* e);
  void OnRingGizmoModified(RingGizmoEvent* e);
  void OnGizmoDragEnd(Event * e);
  void OnSelectionChanged(Event* e);
  void OnFrameUpdate(Event* e);
  
  /// The world rotation of this gizmo when the drag event started.
  /// Needed to build the final world rotation.
  Quat mStartingWorldRotation;
  /// The world rotation of this gizmo after all current rotations are applied.
  Quat mFinalWorldRotation;

  // Display names for each axis. It's up to some other component to set these to more intuitive names.
  String mXAxisName;
  String mYAxisName;
  String mZAxisName;
  Transform* mTransform;
};

//-------------------------------------------------------------------SimpleRotationProperty
/// Simple "property" for a rotation basis gizmo. Simply stores a cog and the original value of that cog's property for undo.
class SimpleBasisProperty
{
public:
  CogId mCogId;
  Quat mOriginalBasis;
};

//-------------------------------------------------------------------OrientationBasisGizmo
/// Gizmo class responsible for setting the basis on the Orientation component from a RotationBasisGizmo.
class OrientationBasisGizmo : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  OrientationBasisGizmo();
  void Initialize(CogInitializer& initializer) override;

  class OrientationBasisProperty
  {
  public:
    CogId mCogId;
    Quat mOriginalBasis;
    OrientationBases::Enum mBasisType;
  };

  void OnAddRotationBasisGizmoObject(RotationBasisGizmoInitializationEvent* e);
  void OnRotationBasisGizmoBegin(Event* e);
  void OnRotationBasisGizmoModified(Event* e);
  void OnRotationBasisGizmoEnd(Event* e);
  void OnFrameUpdate(Event* e);
  void OnRotationBasisAabbQuery(RotationBasisGizmoAabbQueryEvent* e);

  void UpdateTranslation();

  // The majority of the code below here should ideally be in a shared class, but due to time
  // constraints this is just set-up to be easily copy-pasted to a new gizmo's class.
  void CacheRotation(OrientationBasisProperty& prop);
  void UpdateRotation(OrientationBasisProperty& prop, QuatParam rotation);
  void RevertRotation(OrientationBasisProperty& prop);
  void QueueRotationsWithUndo(OperationQueue* queue, OrientationBasisProperty& prop, QuatParam rotation);
  Vec3 GetWorldTranslation(OrientationBasisProperty& prop);
  Quat GetLocalBasis(Orientation* orientation);
  Quat GetWorldRotation(OrientationBasisProperty& prop);
  void SetWorldRotation(OrientationBasisProperty& prop, QuatParam rotation);

  // The orientation needs to display a left-handed basis even though the built rotation is right-handed.
  // To do that these helper functions are used that simply rotate about the x-axis and flip the labeling
  // of the y and z axes. To become left-handed, the basis is rotated by -90 about the x-axis.
  // To become right handed the rotation is 90 degrees.
  Quat ToLeftHanded(QuatParam basis);
  Quat ToRightHanded(QuatParam basis);

  RotationBasisGizmo* mRotationBasisGizmo;
  Array<OrientationBasisProperty> mCogs;
};

//-------------------------------------------------------------------PhysicsCarWheelBasisGizmo
/// Gizmo class responsible for setting the basis on a PhysicsCarWheel from a RotationBasisGizmo.
class PhysicsCarWheelBasisGizmo : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  PhysicsCarWheelBasisGizmo();
  void Initialize(CogInitializer& initializer) override;

  void OnAddRotationBasisGizmoObject(RotationBasisGizmoInitializationEvent* e);
  void OnRotationBasisGizmoBegin(Event* e);
  void OnRotationBasisGizmoModified(Event* e);
  void OnRotationBasisGizmoEnd(Event* e);
  void OnFrameUpdate(Event* e);
  void OnRotationBasisAabbQuery(RotationBasisGizmoAabbQueryEvent* e);

  void UpdateTranslation();

  // The majority of the code below here should ideally be in a shared class, but due to time
  // constraints this is just set-up to be easily copy-pasted to a new gizmo's class.
  void CacheRotation(SimpleBasisProperty& prop);
  void UpdateRotation(SimpleBasisProperty& prop, QuatParam rotation);
  void RevertRotation(SimpleBasisProperty& prop);
  void QueueRotationsWithUndo(OperationQueue* queue, SimpleBasisProperty& prop, QuatParam rotation);
  Vec3 GetWorldTranslation(SimpleBasisProperty& prop);
  Quat GetWorldRotation(SimpleBasisProperty& prop);
  void SetWorldRotation(SimpleBasisProperty& prop, QuatParam rotation);

  RotationBasisGizmo* mRotationBasisGizmo;
  Array<SimpleBasisProperty> mCogs;
};

//-------------------------------------------------------------------RevoluteBasisGizmo
/// Gizmo class responsible for setting the basis on the Orientation component from a RotationBasisGizmo.
class RevoluteBasisGizmo : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  /// A revolute needs to store 2 bases that can be independently
  /// reverted, hence we need a special class.
  class RevoluteJointBasisProperty
  {
  public:
    Quat mOriginalBasisA;
    Quat mOriginalBasisB;
    CogId mCogId;
  };

  RevoluteBasisGizmo();
  void Initialize(CogInitializer& initializer) override;

  void OnAddRotationBasisGizmoObject(RotationBasisGizmoInitializationEvent* e);
  void OnRotationBasisGizmoBegin(Event* e);
  void OnRotationBasisGizmoModified(Event* e);
  void OnRotationBasisGizmoEnd(Event* e);
  void OnFrameUpdate(Event* e);
  void OnRotationBasisAabbQuery(RotationBasisGizmoAabbQueryEvent* e);

  void UpdateTranslation();

  // The majority of the code below here should ideally be in a shared class, but due to time
  // constraints this is just set-up to be easily copy-pasted to a new gizmo's class.
  void CacheRotation(RevoluteJointBasisProperty& prop);
  void UpdateRotation(RevoluteJointBasisProperty& prop, QuatParam rotation);
  void RevertRotation(RevoluteJointBasisProperty& prop);
  void QueueRotationsWithUndo(OperationQueue* queue, RevoluteJointBasisProperty& prop, QuatParam rotation);
  Vec3 GetWorldTranslation(RevoluteJointBasisProperty& prop);
  Quat GetWorldBasis(RevoluteJointBasisProperty& prop);
  void SetWorldBasis(RevoluteJointBasisProperty& prop, QuatParam worldBasis);

  Quat GetColliderWorldRotation(RevoluteJoint* joint, uint colliderIndex);
  Quat GetWorldBasis(RevoluteJoint* joint, uint colliderIndex);
  void SetWorldBasis(RevoluteJoint* joint, uint colliderIndex, QuatParam worldBasis);

  /// Bit flags for which bases we care about. 0b01 is base A. 0b10 is basis B. 0b11 is both.
  uint mBasisType;

  RotationBasisGizmo* mRotationBasisGizmo;
  Array<RevoluteJointBasisProperty> mCogs;
};

}//namespace Zero
