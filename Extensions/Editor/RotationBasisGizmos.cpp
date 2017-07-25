///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
DefineEvent(AddRotationBasisGizmoObject);
DefineEvent(RotationBasisGizmoBegin);
DefineEvent(RotationBasisGizmoModified);
DefineEvent(RotationBasisGizmoEnd);
DefineEvent(RotationBasisAabbQuery);
}//namespace Events


//------------------------------------------------------------------- RotationBasisGizmoInitializationEvent
ZilchDefineType(RotationBasisGizmoInitializationEvent, builder, type)
{
  ZilchBindFieldProperty(mIntData);
}

RotationBasisGizmoInitializationEvent::RotationBasisGizmoInitializationEvent()
{
  mIntData = 0;
}

//-------------------------------------------------------------------RotationBasisGizmoAabbQueryEvent
ZilchDefineType(RotationBasisGizmoAabbQueryEvent, builder, type)
{

}

RotationBasisGizmoAabbQueryEvent::RotationBasisGizmoAabbQueryEvent()
{
  mAabb.SetInvalid();
}

//-------------------------------------------------------------------RotationBasisGizmoMetaTransform
ZilchDefineType(RotationBasisGizmoMetaTransform, builder, type)
{

}

MetaTransformInstance RotationBasisGizmoMetaTransform::GetInstance(HandleParam object)
{
  RotationBasisGizmo* gizmo = object.Get<RotationBasisGizmo*>();
  ReturnIf(gizmo == nullptr, MetaTransformInstance(), "Invalid object given");

  Transform* transform = gizmo->mTransform;
  BoundType* transformType = ZilchTypeId(Transform);

  MetaTransformInstance instance(transform);
  instance.mSpace = gizmo->GetSpace();
  instance.mLocalTranslation = transformType->GetProperty("LocalTranslation");
  instance.mLocalRotation = transformType->GetProperty("LocalRotation");
  instance.mLocalScale = transformType->GetProperty("LocalScale");

  instance.mWorldTranslation = transformType->GetProperty("WorldTranslation");
  instance.mWorldRotation = transformType->GetProperty("WorldRotation");
  instance.mWorldScale = transformType->GetProperty("WorldScale");

  instance.mAabb = gizmo->GetSelectionAabb();

  return instance;
}

//-------------------------------------------------------------------RotationBasisGizmo
ZilchDefineType(RotationBasisGizmo, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDependency(Transform);
  type->Add(new RotationBasisGizmoMetaTransform());

  ZeroBindEvent(Events::RotationBasisGizmoBegin, Event);
  ZeroBindEvent(Events::RotationBasisGizmoModified, Event);
  ZeroBindEvent(Events::RotationBasisGizmoEnd, Event);
  ZeroBindEvent(Events::RotationBasisAabbQuery, RotationBasisGizmoAabbQueryEvent);

  ZilchBindMethod(ActivateAsGizmo);
  // Does not need to be serialized. Purely for editing purposes.
  ZilchBindGetterSetterProperty(WorldRotation);

  ZilchBindField(mXAxisName)->ZeroSerialize(String("X-Axis"));
  ZilchBindField(mYAxisName)->ZeroSerialize(String("Y-Axis"));
  ZilchBindField(mZAxisName)->ZeroSerialize(String("Z-Axis"));
}

RotationBasisGizmo::RotationBasisGizmo()
{
  mStartingWorldRotation = Quat::cIdentity;
  mFinalWorldRotation = Quat::cIdentity;
}

void RotationBasisGizmo::Serialize(Serializer& stream)
{
  // Force meta serialization until the switch is made
  MetaSerializeProperties(this, stream);
}

void RotationBasisGizmo::Initialize(CogInitializer& initializer)
{
  mTransform = GetOwner()->has(Transform);

  ConnectThisTo(GetOwner(), Events::MouseDragStart, OnGizmoDragStart);
  ConnectThisTo(GetOwner(), Events::RingGizmoModified, OnRingGizmoModified);
  ConnectThisTo(GetOwner(), Events::MouseDragEnd, OnGizmoDragEnd);
  // For text debug drawing
  ConnectThisTo(GetSpace(), Events::FrameUpdate, OnFrameUpdate);
}

Quat RotationBasisGizmo::GetWorldRotation()
{
  return mFinalWorldRotation;
}

void RotationBasisGizmo::SetWorldRotation(QuatParam rotation)
{
  // Send out a begin event before we do anything
  Event toSend;
  GetOwner()->DispatchEvent(Events::RotationBasisGizmoBegin, &toSend);
  // Update the actual world rotation
  SetWorldRotationInternal(rotation);
  // Now notify everyone that we've finished a rotation
  GetOwner()->DispatchEvent(Events::RotationBasisGizmoEnd, &toSend);
}

void RotationBasisGizmo::SetWorldRotationInternal(QuatParam rotation)
{
  mStartingWorldRotation = Math::Normalized(rotation);
  mFinalWorldRotation = mStartingWorldRotation;
  mTransform->SetWorldRotation(mStartingWorldRotation);
}

void RotationBasisGizmo::SetGizmoWorldRotationInternal(QuatParam rotation)
{
  mFinalWorldRotation = Math::Normalized(rotation);
  mTransform->SetWorldRotation(mFinalWorldRotation);
}

void RotationBasisGizmo::ActivateAsGizmo()
{
  Cog* owner = GetOwner();
  // Use the Transient flag to determine if we've already been activated so we
  // can not double connect to events (maybe add a real flag later...)
  if(owner->GetTransient())
    return;

  owner->SetObjectViewHidden(true);
  owner->SetTransient(true);
  // Listen for the selection changing so we can delete ourself
  ConnectThisTo(Z::gEditor->GetSelection(), Events::SelectionChanged, OnSelectionChanged);
}

void RotationBasisGizmo::SetAsSelection()
{
  MetaSelection* selection = Z::gEditor->GetSelection();
  selection->Clear();
  selection->Add(this);
  selection->FinalSelectionChanged();
}

void RotationBasisGizmo::DrawBasisText(QuatParam axisRotation, StringParam axisText, Vec3Param localOffset)
{
  Debug::Text text;
  text.SetViewScaled(true);
  text.mText = axisText;
  text.mPosition = GetOwner()->has(Transform)->GetWorldTranslation();
  text.mColor = Vec4(1, 1, 1, 1);
  text.mTextHeight = 1;
  text.ViewScaleOffset(localOffset);
  text.OnTop(true);
  text.mRotation = mFinalWorldRotation * axisRotation;
  gDebugDraw->Add(text);
}

Aabb RotationBasisGizmo::GetSelectionAabb()
{
  RotationBasisGizmoAabbQueryEvent toSend;
  GetOwner()->DispatchEvent(Events::RotationBasisAabbQuery, &toSend);
  return toSend.mAabb;
}

Aabb RotationBasisGizmo::GetCogAabb(Cog* cog)
{
  Aabb result;
  result.SetInvalid();
  if(cog == nullptr)
    return result;

  // For now only combine physics and graphics aabbs
  Collider* collider = cog->has(Collider);
  if(collider != nullptr)
    result.Combine(collider->GetWorldAabb());
  Graphical* graphical = cog->has(Graphical);
  if(graphical != nullptr)
    result.Combine(graphical->GetWorldAabb());

  return result;
}

void RotationBasisGizmo::OnGizmoDragStart(Event* e)
{
  Event toSend;
  GetOwner()->DispatchEvent(Events::RotationBasisGizmoBegin, &toSend);
}

void RotationBasisGizmo::OnRingGizmoModified(RingGizmoEvent* e)
{
  // Compute the final world rotation from the ring gizmo's delta rotation (that's what world rotation is here)
  mFinalWorldRotation = e->mWorldRotation * mStartingWorldRotation;
  mFinalWorldRotation.Normalize();
  // Update the gizmo's rotation
  mTransform->SetWorldRotation(mFinalWorldRotation);

  Event toSend;
  GetOwner()->DispatchEvent(Events::RotationBasisGizmoModified, &toSend);
}

void RotationBasisGizmo::OnGizmoDragEnd(Event * e)
{
  mStartingWorldRotation = mFinalWorldRotation;
  
  Event toSend;
  GetOwner()->DispatchEvent(Events::RotationBasisGizmoEnd, &toSend);
}

void RotationBasisGizmo::OnSelectionChanged(Event* e)
{
  GetOwner()->Destroy();
}

void RotationBasisGizmo::OnFrameUpdate(Event* e)
{
  // Some hard-coded offsets and rotations for the text. Needed for now because debug drawing is the
  // only way to have view-scaled text. Otherwise the text should probably just be SpriteText components.
  Quat xRotation = Quat::cIdentity;
  Quat yRotation = Math::ToQuaternion(Vec3(0, 0, 1), Math::cPi * 0.5f);
  Quat zRotation = Math::ToQuaternion(Vec3(0, 1, 0), 3 * Math::cPi * 0.5f);

  DrawBasisText(xRotation, mXAxisName, Vec3(3, 0.6f, 0));
  DrawBasisText(yRotation, mYAxisName, Vec3(3, 0.6f, 0));
  DrawBasisText(zRotation, mZAxisName, Vec3(3, 0.6f, 0));
}

//-------------------------------------------------------------------OrientationBasisGizmo
ZilchDefineType(OrientationBasisGizmo, builder, type)
{
  ZeroBindComponent();
  ZeroBindDependency(RotationBasisGizmo);
  ZeroBindSetup(SetupMode::DefaultSerialization);
}

OrientationBasisGizmo::OrientationBasisGizmo()
{
  mRotationBasisGizmo = nullptr;
}

void OrientationBasisGizmo::Initialize(CogInitializer& initializer)
{
  ConnectThisTo(GetOwner(), Events::AddRotationBasisGizmoObject, OnAddRotationBasisGizmoObject);
  ConnectThisTo(GetOwner(), Events::RotationBasisGizmoBegin, OnRotationBasisGizmoBegin);
  ConnectThisTo(GetOwner(), Events::RotationBasisGizmoModified, OnRotationBasisGizmoModified);
  ConnectThisTo(GetOwner(), Events::RotationBasisGizmoEnd, OnRotationBasisGizmoEnd);
  ConnectThisTo(GetSpace(), Events::FrameUpdate, OnFrameUpdate);
  ConnectThisTo(GetOwner(), Events::RotationBasisAabbQuery, OnRotationBasisAabbQuery);
  
  // Cache the rotation basis gizmo
  mRotationBasisGizmo = GetOwner()->has(RotationBasisGizmo);
  // Set each basis' display name
  mRotationBasisGizmo->mXAxisName = "Right";
  mRotationBasisGizmo->mYAxisName = "Forward";
  mRotationBasisGizmo->mZAxisName = "Up";
}

void OrientationBasisGizmo::OnAddRotationBasisGizmoObject(RotationBasisGizmoInitializationEvent* e)
{
  // Create the property that we store for this object (will contain a cached rotation later)
  OrientationBasisProperty prop;
  prop.mCogId = (Cog*)e->GetSource();

  // If this is the first cog added then use it's rotation as the gizmo's initial rotation
  if(mCogs.Empty())
  {
    Quat worldBasis = GetWorldRotation(prop);
    mRotationBasisGizmo->SetWorldRotationInternal(worldBasis);
    mRotationBasisGizmo->SetAsSelection();
  }

  // Add the cog and recompute the world translation of this gizmo
  mCogs.PushBack(prop);
  UpdateTranslation();

  // Activate this gizmo (delete on selection change, hidden in editor, etc...).
  // We have to do this so the archetype can still be modified in the editor.
  // Do this last so that the gizmo doesn't get a selection changed event until
  // we've finished setting it as the selection.
  mRotationBasisGizmo->ActivateAsGizmo();
}

void OrientationBasisGizmo::OnRotationBasisGizmoBegin(Event* e)
{
  // For undo, cache where each cog's rotation started at
  for(size_t i = 0; i < mCogs.Size(); ++i)
    CacheRotation(mCogs[i]);
}

void OrientationBasisGizmo::OnRotationBasisGizmoModified(Event* e)
{
  // Update each cog's rotation to the current gizmo's rotation
  for(size_t i = 0; i < mCogs.Size(); ++i)
    UpdateRotation(mCogs[i], mRotationBasisGizmo->mFinalWorldRotation);
}

void OrientationBasisGizmo::OnRotationBasisGizmoEnd(Event* e)
{
  if(mCogs.Empty())
    return;

  // Revert all cogs back to their initial rotation so we can properly queue undos
  for(size_t i = 0; i < mCogs.Size(); ++i)
    RevertRotation(mCogs[i]);

  // For each cog, queue a batch undo that takes them from their
  // initial rotation to the gizmo's final rotation
  OperationQueue* queue = Z::gEditor->GetOperationQueue();
  queue->BeginBatch();
  queue->SetActiveBatchName("UndoRotations batch");

  for(size_t i = 0; i < mCogs.Size(); ++i)
    QueueRotationsWithUndo(queue, mCogs[i], mRotationBasisGizmo->mStartingWorldRotation);

  queue->EndBatch();
}

void OrientationBasisGizmo::OnFrameUpdate(Event* e)
{
  if(mCogs.Empty())
    return;

  // Update the gizmo's bases. Mostly to catch undo operations
  OrientationBasisProperty prop = mCogs[0];
  Quat worldBasis = GetWorldRotation(prop);
  mRotationBasisGizmo->SetGizmoWorldRotationInternal(worldBasis);

  for(size_t i = 0; i < mCogs.Size(); ++i)
  {
    Orientation* orientation = mCogs[i].mCogId.has(Orientation);
    if(orientation != nullptr)
      orientation->DebugDraw();
  }
}

void OrientationBasisGizmo::OnRotationBasisAabbQuery(RotationBasisGizmoAabbQueryEvent* e)
{
  for(size_t i = 0; i < mCogs.Size(); ++i)
    e->mAabb.Combine(mRotationBasisGizmo->GetCogAabb(mCogs[i].mCogId));
}

void OrientationBasisGizmo::UpdateTranslation()
{
  if(mCogs.Empty())
    return;

  // Average the cog's translation values
  Vec3 worldPos = Vec3::cZero;
  for(size_t i = 0; i < mCogs.Size(); ++i)
    worldPos += GetWorldTranslation(mCogs[i]);
  worldPos /= (float)mCogs.Size();
  mRotationBasisGizmo->mTransform->SetWorldTranslation(worldPos);
}

void OrientationBasisGizmo::CacheRotation(OrientationBasisProperty& prop)
{
  Orientation* orientation = prop.mCogId.has(Orientation);
  if(orientation == nullptr)
    return;

  prop.mOriginalBasis = GetWorldRotation(prop);
  prop.mBasisType = orientation->GetDefaultOrientationBases();
}

void OrientationBasisGizmo::UpdateRotation(OrientationBasisProperty& prop, QuatParam rotation)
{
  SetWorldRotation(prop, rotation);
}

void OrientationBasisGizmo::RevertRotation(OrientationBasisProperty& prop)
{
  Orientation* orientation = prop.mCogId.has(Orientation);
  if(orientation == nullptr)
    return;

  SetWorldRotation(prop, prop.mOriginalBasis);
  orientation->SetDefaultOrientationBases(prop.mBasisType);
}

void OrientationBasisGizmo::QueueRotationsWithUndo(OperationQueue* queue, OrientationBasisProperty& prop, QuatParam rotation)
{
  Orientation* orientation = prop.mCogId.has(Orientation);
  if(orientation != nullptr)
  {
    Transform* transform = prop.mCogId.has(Transform);
    // Take the gizmo's world-space basis into local space
    Quat worldRotation = transform->GetWorldRotation();
    Quat localBasis = worldRotation.Inverted() * rotation;
    // Also queue up that we're changing to a custom basis
    ChangeAndQueueProperty(queue, orientation, "DefaultOrientationBases", OrientationBases::Custom);
    // Queue up a property change to set the local basis
    ChangeAndQueueProperty(queue, orientation, "LocalOrientationBasis", ToRightHanded(localBasis));
  }
}

Vec3 OrientationBasisGizmo::GetWorldTranslation(OrientationBasisProperty& prop)
{
  Transform* transform = prop.mCogId.has(Transform);
  if(transform != nullptr)
    return transform->GetWorldTranslation();
  return Vec3::cZero;
}

Quat OrientationBasisGizmo::GetWorldRotation(OrientationBasisProperty& prop)
{
  Orientation* orientation = prop.mCogId.has(Orientation);
  if(orientation == nullptr)
    return Quat::cIdentity;

  Transform* transform = prop.mCogId.has(Transform);
  // Transform the local basis into world space
  Quat worldRotation = transform->GetWorldRotation();
  Quat localBasis = ToLeftHanded(orientation->GetLocalOrientationBasis());
  Quat worldBasis = worldRotation * localBasis;
  return worldBasis;
}

void OrientationBasisGizmo::SetWorldRotation(OrientationBasisProperty& prop, QuatParam rotation)
{
  Orientation* orientation = prop.mCogId.has(Orientation);
  if(orientation == nullptr)
    return;

  // Transform the gizmo's world-space basis into local space
  Transform* transform = prop.mCogId.has(Transform);
  Quat worldRotation = transform->GetWorldRotation();
  Quat localBasis = worldRotation.Inverted() * rotation;

  orientation->SetLocalOrientationBasis(ToRightHanded(localBasis));
}

Quat OrientationBasisGizmo::ToLeftHanded(QuatParam basis)
{
  Mat3 basisMat = Math::ToMatrix3(basis);
  Vec3 y = basisMat.GetBasis(1);
  Vec3 z = basisMat.GetBasis(2);
  basisMat.SetBasis(1, -z);
  basisMat.SetBasis(2, y);
  return Math::ToQuaternion(basisMat);
}

Quat OrientationBasisGizmo::ToRightHanded(QuatParam basis)
{
  Mat3 basisMat = Math::ToMatrix3(basis);
  Vec3 y = basisMat.GetBasis(1);
  Vec3 z = basisMat.GetBasis(2);
  basisMat.SetBasis(1, z);
  basisMat.SetBasis(2, -y);
  return Math::ToQuaternion(basisMat);
}

//-------------------------------------------------------------------PhysicsCarWheelBasisGizmo
ZilchDefineType(PhysicsCarWheelBasisGizmo, builder, type)
{
  ZeroBindComponent();
  ZeroBindDependency(RotationBasisGizmo);
  ZeroBindSetup(SetupMode::DefaultSerialization);
}

PhysicsCarWheelBasisGizmo::PhysicsCarWheelBasisGizmo()
{
  mRotationBasisGizmo = nullptr;
}

void PhysicsCarWheelBasisGizmo::Initialize(CogInitializer& initializer)
{
  ConnectThisTo(GetOwner(), Events::AddRotationBasisGizmoObject, OnAddRotationBasisGizmoObject);
  ConnectThisTo(GetOwner(), Events::RotationBasisGizmoBegin, OnRotationBasisGizmoBegin);
  ConnectThisTo(GetOwner(), Events::RotationBasisGizmoModified, OnRotationBasisGizmoModified);
  ConnectThisTo(GetOwner(), Events::RotationBasisGizmoEnd, OnRotationBasisGizmoEnd);
  ConnectThisTo(GetSpace(), Events::FrameUpdate, OnFrameUpdate);
  ConnectThisTo(GetOwner(), Events::RotationBasisAabbQuery, OnRotationBasisAabbQuery);

  // Cache the rotation basis gizmo
  mRotationBasisGizmo = GetOwner()->has(RotationBasisGizmo);
  // Set each basis' display name
  mRotationBasisGizmo->mXAxisName = "Spring";
  mRotationBasisGizmo->mYAxisName = "Forward";
  mRotationBasisGizmo->mZAxisName = "Axle";
}

void PhysicsCarWheelBasisGizmo::OnAddRotationBasisGizmoObject(RotationBasisGizmoInitializationEvent* e)
{
  // Create the property that we store for this object (will contain a cached rotation later)
  SimpleBasisProperty prop;
  prop.mCogId = (Cog*)e->GetSource();

  // If this is the first cog added then use it's rotation as the gizmo's initial rotation
  if(mCogs.Empty())
  {
    Quat worldBasis = GetWorldRotation(prop);
    mRotationBasisGizmo->SetWorldRotationInternal(worldBasis);
    mRotationBasisGizmo->SetAsSelection();
  }

  // Add the cog and recompute the world translation of this gizmo
  mCogs.PushBack(prop);
  UpdateTranslation();

  // Activate this gizmo (delete on selection change, hidden in editor, etc...).
  // We have to do this so the archetype can still be modified in the editor.
  // Do this last so that the gizmo doesn't get a selection changed event until
  // we've finished setting it as the selection.
  mRotationBasisGizmo->ActivateAsGizmo();
}

void PhysicsCarWheelBasisGizmo::OnRotationBasisGizmoBegin(Event* e)
{
  // For undo, cache where each cog's rotation started at
  for(size_t i = 0; i < mCogs.Size(); ++i)
    CacheRotation(mCogs[i]);
}

void PhysicsCarWheelBasisGizmo::OnRotationBasisGizmoModified(Event* e)
{
  // Update each cog's rotation to the current gizmo's rotation
  for(size_t i = 0; i < mCogs.Size(); ++i)
    UpdateRotation(mCogs[i], mRotationBasisGizmo->mFinalWorldRotation);
}

void PhysicsCarWheelBasisGizmo::OnRotationBasisGizmoEnd(Event* e)
{
  if(mCogs.Empty())
    return;

  // Revert all cogs back to their initial rotation so we can properly queue undos
  for(size_t i = 0; i < mCogs.Size(); ++i)
    RevertRotation(mCogs[i]);

  // For each cog, queue a batch undo that takes them from their
  // initial rotation to the gizmo's final rotation
  OperationQueue* queue = Z::gEditor->GetOperationQueue();
  queue->BeginBatch();
  queue->SetActiveBatchName("Undo CarWheel rotations batch");

  for(size_t i = 0; i < mCogs.Size(); ++i)
    QueueRotationsWithUndo(queue, mCogs[i], mRotationBasisGizmo->mStartingWorldRotation);

  queue->EndBatch();
}

void PhysicsCarWheelBasisGizmo::OnFrameUpdate(Event* e)
{
  if(mCogs.Empty())
    return;

  // Update the gizmo's bases. Mostly to catch undo operations
  SimpleBasisProperty prop = mCogs[0];
  Quat worldBasis = GetWorldRotation(prop);
  mRotationBasisGizmo->SetGizmoWorldRotationInternal(worldBasis);

  for(size_t i = 0; i < mCogs.Size(); ++i)
  {
    PhysicsCarWheel* wheel = mCogs[i].mCogId.has(PhysicsCarWheel);
    if(wheel != nullptr)
      wheel->DebugDraw();
  }
}

void PhysicsCarWheelBasisGizmo::OnRotationBasisAabbQuery(RotationBasisGizmoAabbQueryEvent* e)
{
  for(size_t i = 0; i < mCogs.Size(); ++i)
    e->mAabb.Combine(mRotationBasisGizmo->GetCogAabb(mCogs[i].mCogId));
}

void PhysicsCarWheelBasisGizmo::UpdateTranslation()
{
  if(mCogs.Empty())
    return;

  // Average the cog's translation values
  Vec3 worldPos = Vec3::cZero;
  for(size_t i = 0; i < mCogs.Size(); ++i)
    worldPos += GetWorldTranslation(mCogs[i]);
  worldPos /= (float)mCogs.Size();
  mRotationBasisGizmo->mTransform->SetWorldTranslation(worldPos);
}

void PhysicsCarWheelBasisGizmo::CacheRotation(SimpleBasisProperty& prop)
{
  prop.mOriginalBasis = GetWorldRotation(prop);
}

void PhysicsCarWheelBasisGizmo::UpdateRotation(SimpleBasisProperty& prop, QuatParam rotation)
{
  SetWorldRotation(prop, rotation);
}

void PhysicsCarWheelBasisGizmo::RevertRotation(SimpleBasisProperty& prop)
{
  SetWorldRotation(prop, prop.mOriginalBasis);
}

void PhysicsCarWheelBasisGizmo::QueueRotationsWithUndo(OperationQueue* queue, SimpleBasisProperty& prop, QuatParam rotation)
{
  PhysicsCarWheel* wheel = prop.mCogId.has(PhysicsCarWheel);
  if(wheel == nullptr)
    return;

  ChangeAndQueueProperty(queue, wheel, "WorldWheelBasis", rotation);
}

Vec3 PhysicsCarWheelBasisGizmo::GetWorldTranslation(SimpleBasisProperty& prop)
{
  Transform* transform = prop.mCogId.has(Transform);
  if(transform != nullptr)
    return transform->GetWorldTranslation();
  return Vec3::cZero;
}

Quat PhysicsCarWheelBasisGizmo::GetWorldRotation(SimpleBasisProperty& prop)
{
  PhysicsCarWheel* wheel = prop.mCogId.has(PhysicsCarWheel);
  if(wheel == nullptr)
    return Quat::cIdentity;

  return wheel->GetWorldWheelBasis();
}

void PhysicsCarWheelBasisGizmo::SetWorldRotation(SimpleBasisProperty& prop, QuatParam rotation)
{
  PhysicsCarWheel* wheel = prop.mCogId.has(PhysicsCarWheel);
  if(wheel == nullptr)
    return;

  return wheel->SetWorldWheelBasis(rotation);
}

//-------------------------------------------------------------------RevoluteBasisGizmo
ZilchDefineType(RevoluteBasisGizmo, builder, type)
{
  ZeroBindComponent();
  ZeroBindDependency(RotationBasisGizmo);
  ZeroBindSetup(SetupMode::DefaultSerialization);
}

RevoluteBasisGizmo::RevoluteBasisGizmo()
{
  mRotationBasisGizmo = nullptr;
  mBasisType = 0b01;
}

void RevoluteBasisGizmo::Initialize(CogInitializer& initializer)
{
  ConnectThisTo(GetOwner(), Events::AddRotationBasisGizmoObject, OnAddRotationBasisGizmoObject);
  ConnectThisTo(GetOwner(), Events::RotationBasisGizmoBegin, OnRotationBasisGizmoBegin);
  ConnectThisTo(GetOwner(), Events::RotationBasisGizmoModified, OnRotationBasisGizmoModified);
  ConnectThisTo(GetOwner(), Events::RotationBasisGizmoEnd, OnRotationBasisGizmoEnd);
  ConnectThisTo(GetSpace(), Events::FrameUpdate, OnFrameUpdate);
  ConnectThisTo(GetOwner(), Events::RotationBasisAabbQuery, OnRotationBasisAabbQuery);

  // Cache the rotation basis gizmo
  mRotationBasisGizmo = GetOwner()->has(RotationBasisGizmo);
  // Set each basis' display name
  mRotationBasisGizmo->mXAxisName = "Primary";
  mRotationBasisGizmo->mYAxisName = "Secondary";
  mRotationBasisGizmo->mZAxisName = "Hinge";
}

void RevoluteBasisGizmo::OnAddRotationBasisGizmoObject(RotationBasisGizmoInitializationEvent* e)
{
  // Create the property that we store for this object (will contain a cached rotation later)
  RevoluteJointBasisProperty prop;
  prop.mCogId = (Cog*)e->GetSource();

  // If this is the first cog added then use it's rotation as the gizmo's initial rotation
  if(mCogs.Empty())
  {
    mBasisType = e->mIntData;
    Quat worldBasis = GetWorldBasis(prop);
    mRotationBasisGizmo->SetWorldRotationInternal(worldBasis);
    mRotationBasisGizmo->SetAsSelection();
  }

  // Add the cog and recompute the world translation of this gizmo
  mCogs.PushBack(prop);
  UpdateTranslation();

  // Activate this gizmo (delete on selection change, hidden in editor, etc...).
  // We have to do this so the archetype can still be modified in the editor.
  // Do this last so that the gizmo doesn't get a selection changed event until
  // we've finished setting it as the selection.
  mRotationBasisGizmo->ActivateAsGizmo();
}

void RevoluteBasisGizmo::OnRotationBasisGizmoBegin(Event* e)
{
  // For undo, cache where each cog's rotation started at
  for(size_t i = 0; i < mCogs.Size(); ++i)
    CacheRotation(mCogs[i]);
}

void RevoluteBasisGizmo::OnRotationBasisGizmoModified(Event* e)
{
  // Update each cog's rotation to the current gizmo's rotation
  for(size_t i = 0; i < mCogs.Size(); ++i)
    UpdateRotation(mCogs[i], mRotationBasisGizmo->mFinalWorldRotation);
}

void RevoluteBasisGizmo::OnRotationBasisGizmoEnd(Event* e)
{
  if(mCogs.Empty())
    return;

  // Revert all cogs back to their initial rotation so we can properly queue undos
  for(size_t i = 0; i < mCogs.Size(); ++i)
    RevertRotation(mCogs[i]);

  // For each cog, queue a batch undo that takes them from their
  // initial rotation to the gizmo's final rotation
  OperationQueue* queue = Z::gEditor->GetOperationQueue();
  queue->BeginBatch();
  queue->SetActiveBatchName("Undo RevoluteBasis batch");

  for(size_t i = 0; i < mCogs.Size(); ++i)
    QueueRotationsWithUndo(queue, mCogs[i], mRotationBasisGizmo->mStartingWorldRotation);

  queue->EndBatch();
}

void RevoluteBasisGizmo::OnFrameUpdate(Event* e)
{
  if(mCogs.Empty())
    return;

  // Update the gizmo's bases. Mostly to catch undo operations
  RevoluteJointBasisProperty prop = mCogs[0];
  Quat worldBasis = GetWorldBasis(prop);
  mRotationBasisGizmo->SetGizmoWorldRotationInternal(worldBasis);

  for(size_t i = 0; i < mCogs.Size(); ++i)
  {
    RevoluteJoint* joint = mCogs[i].mCogId.has(RevoluteJoint);
    if(joint != nullptr)
      joint->DebugDraw();
  }
}

void RevoluteBasisGizmo::OnRotationBasisAabbQuery(RotationBasisGizmoAabbQueryEvent* e)
{
  for(size_t i = 0; i < mCogs.Size(); ++i)
  {
    RevoluteJoint* joint = mCogs[i].mCogId.has(RevoluteJoint);
    if(joint != nullptr)
    {
      e->mAabb.Combine(mRotationBasisGizmo->GetCogAabb(joint->GetCog(0)));
      e->mAabb.Combine(mRotationBasisGizmo->GetCogAabb(joint->GetCog(1)));
    }
  }
}

void RevoluteBasisGizmo::UpdateTranslation()
{
  if(mCogs.Empty())
    return;

  // Average the cog's translation values
  Vec3 worldPos = Vec3::cZero;
  for(size_t i = 0; i < mCogs.Size(); ++i)
    worldPos += GetWorldTranslation(mCogs[i]);
  worldPos /= (float)mCogs.Size();
  mRotationBasisGizmo->mTransform->SetWorldTranslation(worldPos);
}

void RevoluteBasisGizmo::CacheRotation(RevoluteJointBasisProperty& prop)
{
  RevoluteJoint* joint = prop.mCogId.has(RevoluteJoint);
  if(joint == nullptr)
    return;

  // Cache both world rotations
  prop.mOriginalBasisA = GetWorldBasis(joint, 0);
  prop.mOriginalBasisB = GetWorldBasis(joint, 1);
}

void RevoluteBasisGizmo::UpdateRotation(RevoluteJointBasisProperty& prop, QuatParam rotation)
{
  // Set both world bases to the same rotation
  SetWorldBasis(prop, rotation);
}

void RevoluteBasisGizmo::RevertRotation(RevoluteJointBasisProperty& prop)
{
  RevoluteJoint* joint = prop.mCogId.has(RevoluteJoint);
  if(joint == nullptr)
    return;

  // Revert each basis if we care about it
  if(mBasisType & 0b01)
    SetWorldBasis(joint, 0, prop.mOriginalBasisA);
  if(mBasisType & 0b10)
    SetWorldBasis(joint, 1, prop.mOriginalBasisB);
}

void RevoluteBasisGizmo::QueueRotationsWithUndo(OperationQueue* queue, RevoluteJointBasisProperty& prop, QuatParam rotation)
{
  RevoluteJoint* joint = prop.mCogId.has(RevoluteJoint);
  if(joint == nullptr)
    return;

  // Compute the local basis for each basis we care about and then queue up the property change.
  // Note: these have to be set independently otherwise the they won't revert to their individual values
  if(mBasisType & 0b01)
  {
    Quat worldRotation = GetColliderWorldRotation(joint, 0);
    Quat localBasisA = worldRotation.Inverted() * rotation;
    ChangeAndQueueProperty(queue, joint, "LocalBasisA", localBasisA);
  }
  if(mBasisType & 0b10)
  {
    Quat worldRotation = GetColliderWorldRotation(joint, 1);
    Quat localBasisB = worldRotation.Inverted() * rotation;
    ChangeAndQueueProperty(queue, joint, "LocalBasisB", localBasisB);
  }
}

Vec3 RevoluteBasisGizmo::GetWorldTranslation(RevoluteJointBasisProperty& prop)
{
  RevoluteJoint* joint = prop.mCogId.has(RevoluteJoint);
  if(joint == nullptr)
    return Vec3::cZero;

  // Check if we care about just basis A, or just B, or both. Note: this doesn't perform a bit masking operation
  if(mBasisType == 0b01)
    return joint->GetWorldPointA();
  if(mBasisType == 0b10)
    return joint->GetWorldPointB();
  return (joint->GetWorldPointA() + joint->GetWorldPointB()) * 0.5;
}

Quat RevoluteBasisGizmo::GetWorldBasis(RevoluteJointBasisProperty& prop)
{
  RevoluteJoint* joint = prop.mCogId.has(RevoluteJoint);
  if(joint == nullptr)
    return Quat::cIdentity;

  // This is used to figure out an initial rotation. If we are set exclusively
  // to basis A or we are set to modify both but basis A is the primary basis on
  // the joint then use that rotation. Otherwise use B.
  if(mBasisType == 0b01)
    return GetWorldBasis(joint, 0);
  if(mBasisType == 0b10)
    return GetWorldBasis(joint, 1);
  return joint->GetWorldBasis();
}

void RevoluteBasisGizmo::SetWorldBasis(RevoluteJointBasisProperty& prop, QuatParam worldBasis)
{
  RevoluteJoint* joint = prop.mCogId.has(RevoluteJoint);
  if(joint == nullptr)
    return;

  // Set each basis depending on our basis type
  if(mBasisType & 0b01)
    SetWorldBasis(joint, 0, worldBasis);
  if(mBasisType & 0b10)
    SetWorldBasis(joint, 1, worldBasis);
}

Quat RevoluteBasisGizmo::GetColliderWorldRotation(RevoluteJoint* joint, uint colliderIndex)
{
  // Safely get the world rotation from the provided collider index,
  // taking into account all possible null components
  Cog* cog = joint->GetCog(colliderIndex);
  if(cog == nullptr)
    return Quat::cIdentity;

  Transform* transform = cog->has(Transform);
  if(transform == nullptr)
    return Quat::cIdentity;

  return transform->GetWorldRotation();
}

Quat RevoluteBasisGizmo::GetWorldBasis(RevoluteJoint* joint, uint colliderIndex)
{
  // The joint has a local basis for each collider. Get that collider's local-to-world
  // transformation to bring the basis to world space. We do this because visually the
  // user wants to see a world-space basis.
  Quat worldRotation = GetColliderWorldRotation(joint, colliderIndex);
  Quat localBasis;
  if(colliderIndex == 0)
    return worldRotation * joint->GetLocalBasisA();
  return worldRotation * joint->GetLocalBasisB();
}

void RevoluteBasisGizmo::SetWorldBasis(RevoluteJoint* joint, uint colliderIndex, QuatParam worldBasis)
{
  // Bring the provided world-space basis into the correct collider's local space and to set the joint's field.
  if(colliderIndex == 0)
  {
    Quat worldRotation = GetColliderWorldRotation(joint, colliderIndex);
    Quat localBasisA = worldRotation.Inverted() * worldBasis;
    joint->SetLocalBasisA(localBasisA);
  }
  if(colliderIndex == 1)
  {
    Quat worldRotation = GetColliderWorldRotation(joint, colliderIndex);
    Quat localBasisB = worldRotation.Inverted() * worldBasis;
    joint->SetLocalBasisB(localBasisB);
  }
}

}//namespace Zero
