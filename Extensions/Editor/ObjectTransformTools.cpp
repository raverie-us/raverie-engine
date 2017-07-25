///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Ryan Edgemon, Josh Claeys
/// Copyright 2016-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//------------------------------------------------------------------- Events ---
namespace Events
{
DefineEvent(ToolCreateGizmoEvent);
DefineEvent(DuplicateFirstChance);
}

ZilchDefineType(ToolGizmoEvent, builder, type)
{
}

//------------------------------------------------------------- GizmoCreator ---
ZilchDefineType(GizmoCreator, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);

  ZilchBindGetterSetterProperty(GizmoArchetype);
}

//******************************************************************************
GizmoCreator::GizmoCreator()
{
  mGizmoArchetype = nullptr;
  mGizmo = nullptr;
  mSpace = nullptr;
}

/******************************************************************************/
void GizmoCreator::Serialize(Serializer& stream)
{
  SerializeResourceName(mGizmoArchetype, ArchetypeManager);
}

/******************************************************************************/
void GizmoCreator::Initialize(CogInitializer& initializer)
{
}

//******************************************************************************
void GizmoCreator::OnDestroy(uint flags)
{
  if(mGizmo.IsNotNull())
    mGizmo.SafeDestroy( );

  mGizmo = nullptr;
  mSpace = nullptr;
}

//******************************************************************************
void GizmoCreator::DestroyGizmo()
{
  OnDestroy();
}

//******************************************************************************
Archetype* GizmoCreator::GetGizmoArchetype()
{
  return mGizmoArchetype;
}

//******************************************************************************
void GizmoCreator::SetGizmoArchetype(Archetype* gizmoArchetype)
{
  mGizmoArchetype = gizmoArchetype;
}

//******************************************************************************
Cog* GizmoCreator::GetGizmo(Space* space)
{
    // Create the gizmo if it doesn't exist
  if(mGizmo == nullptr)
  {
    mSpace = space;

      // @RYAN: temp until archetypes can be created as transient
    bool restoreModifiedState = space->mModified;
    space->mModified = true;
    mGizmo = space->Create(mGizmoArchetype);
    space->mModified = restoreModifiedState;

    Cog* gizmo = mGizmo;

    gizmo->SetTransient(true);
    gizmo->SetObjectViewHidden(true);

    if(!space->GetModified())
      space->MarkModified();

    Transform *t = gizmo->has(Transform);

    ToolGizmoEvent e(mGizmo);
    GetOwner()->DispatchEvent(Events::ToolCreateGizmoEvent, &e);
  }

  return mGizmo;
}

//------------------------------------------------------------ TransformTool ---
ZilchDefineType(ObjectTransformTool, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);

  ZeroBindDependency(GizmoCreator);

  ZilchBindGetterSetterProperty(Grab);
  ZilchBindGetterSetterProperty(Basis);
  ZilchBindGetterSetterProperty(Pivot);

  ZeroBindTag(Tags::Tool);
}

/******************************************************************************/
ObjectTransformTool::ObjectTransformTool( ) : mRegisteredObjects()
{
  mChangeFromUs = false;
  mGizmo = nullptr;
}

/******************************************************************************/
void ObjectTransformTool::Serialize(Serializer& stream)
{
  SerializeEnumName(GizmoGrabMode, mGrab);
  SerializeEnumName(GizmoBasis, mBasis);
  SerializeEnumName(GizmoPivot, mPivot);

  SerializeName(mSnapping);
  SerializeName(mSnapDistance);
}

/******************************************************************************/
void ObjectTransformTool::Initialize(CogInitializer& initializer)
{
  ConnectThisTo(GetOwner( ), Events::ToolActivate, OnToolActivate);
  ConnectThisTo(GetOwner( ), Events::ToolDeactivate, OnToolDeactivate);

  ConnectThisTo(GetOwner( ), Events::ToolCreateGizmoEvent, OnToolGizmoCreated);

  ConnectThisTo(GetOwner( ), Events::KeyDown, OnKeyDown);
}

//******************************************************************************
void ObjectTransformTool::DestroyGizmo( )
{
  Cog* gizmo = mGizmo.ToCog( );
  if(gizmo != nullptr)
    gizmo->Destroy( );

  mGizmo = nullptr;

  GizmoCreator* gizmoCreator = GetOwner( )->has(GizmoCreator);
  if(gizmoCreator != nullptr)
    gizmoCreator->DestroyGizmo();
}

/******************************************************************************/
void ObjectTransformTool::RegisterNewGizmo( )
{
  MetaSelection* selection = Z::gEditor->GetSelection( );
  Space* space = GetSpaceFromSelection(selection);

  if(space == nullptr)
    return;

    // need to change space?
  GizmoCreator* gizmoCreator = GetOwner( )->has(GizmoCreator);
  if(space != gizmoCreator->mSpace)
    gizmoCreator->DestroyGizmo( );

    // if the gizmo doesn't exist, create it
  Cog* gizmo = GetOwner( )->has(GizmoCreator)->GetGizmo(space);

    // clear any old objects in the Gizmo
  ObjectTransformGizmo* transformGizmo = gizmo->has(ObjectTransformGizmo);
  transformGizmo->ClearObjects( );

    // add each object in the selection to the gizmo
  forRange(Handle object, mRegisteredObjects.All())
  {
    transformGizmo->AddObject(object);
  }
}

/******************************************************************************/
void ObjectTransformTool::OnToolActivate(Event* event)
{
  MetaSelection* selection = Z::gEditor->GetSelection( );

  ConnectThisTo(selection, Events::SelectionChanged, OnSelectionChanged);
  ConnectThisTo(selection, Events::SelectionFinal, OnFinalSelectionChanged);
  ConnectThisTo(GetSpace(), Events::FrameUpdate, OnFrameUpdate);

  OnSelectionChanged(nullptr);
}

/******************************************************************************/
void ObjectTransformTool::OnToolDeactivate(Event* event)
{
  DestroyGizmo( );

  MetaSelection* selection = Z::gEditor->GetSelection( );

  selection->GetDispatcher()->DisconnectEvent(Events::SelectionChanged, this);
  selection->GetDispatcher( )->DisconnectEvent(Events::SelectionFinal, this);
  GetSpace()->GetDispatcher( )->DisconnectEvent(Events::FrameUpdate, this);

  // Remove all selected object connections when deactivated
  GetReceiver()->Disconnect(Events::ComponentsModified);
}

/******************************************************************************/
void ObjectTransformTool::OnToolGizmoCreated(ToolGizmoEvent* event)
{
  mGizmo = event->mGizmo;

  Cog* gizmo = mGizmo.ToCog( );
  ObjectTransformGizmo* transformGizmo = gizmo->has(ObjectTransformGizmo);
    // use the editors primary operation queue
  transformGizmo->SetOperationQueue(Z::gEditor->GetOperationQueue());

    // update the new gizmo with the tool settings
  CopyPropertiesToGizmo( );

  GizmoCreated(gizmo);
}

/******************************************************************************/
void ObjectTransformTool::OnFrameUpdate(UpdateEvent* event)
{
  Cog* gizmo = mGizmo.ToCog();
  if(gizmo != nullptr)
  {
    ObjectTransformGizmo* transformGizmo = gizmo->has(ObjectTransformGizmo);
    int count = transformGizmo->GetObjectCount();
    for(int i = 0; i < count; ++i)
    {
      Handle object = transformGizmo->GetObjectAtIndex(i);
      if(object.IsNotNull())
        return;
    }

    DestroyGizmo();
  }
}

/******************************************************************************/
/// Responds to objects having a 'Transform' component removed or added
void ObjectTransformTool::OnComponentsChanged(Event* event)
{
  bool transformFound = false;
  forRange(Handle object, mRegisteredObjects.All())
  {
    if(MetaTransform* transform = object.StoredType->HasInherited<MetaTransform>())
    {
      MetaTransformInstance transformInstance = transform->GetInstance(object);
      if(transformInstance.IsNotNull())
      {
        transformFound = true;
        break;
      }
    }
  }

    // if the selection doesn't contain a transform, destroy the gizmo
  if(!transformFound)
  {
    DestroyGizmo( );
    return;
  }

  GizmoCreator* gizmoCreator = GetOwner( )->has(GizmoCreator);

    // make sure there is a gizmo
    //  - [ex: could be undoing 'Transform' remove, so put the gizmo back as well]
  if(gizmoCreator->mGizmo == nullptr)
    RegisterNewGizmo();
}

/******************************************************************************/
/// Responds to selection being created [ex: multi-select dragging]
void ObjectTransformTool::OnSelectionChanged(Event* event)
{
  if(mChangeFromUs)
    return;

  MetaSelection* selection = Z::gEditor->GetSelection( );

    // disconnect all previous objects
  forRange(Handle objectHandle, mRegisteredObjects.All())
  {
    if(Object* object = objectHandle.Get<Object*>())
      object->GetDispatcher()->Disconnect(this);
  }

  mRegisteredObjects.Copy(*selection);

  Cog* primary = selection->GetPrimaryAs<Cog>( );

    // if no objects are selected (or if the selected object is the editor camera),
    // destroy the gizmo
  if(selection->Empty( ) || (primary && primary->has(EditorCameraController)))
  {
    DestroyGizmo( );
    return;
  }

  bool transformFound = false;

    // register new object connections
  forRange(Handle objectHandle, selection->All())
  {
    if(Object* object = objectHandle.Get<Cog*>())
      ConnectThisTo(object, Events::ComponentsModified, OnComponentsChanged);

    if(MetaTransform* transform = objectHandle.StoredType->HasInherited<MetaTransform>())
    {
      MetaTransformInstance transformInstance = transform->GetInstance(objectHandle);
      if(transformInstance.IsNotNull())
        transformFound = true;
    }
  }

    // if the selection doesn't contain a transform, destroy the gizmo
  if(!transformFound)
  {
    DestroyGizmo( );
    return;
  }

  RegisterNewGizmo();
}

/******************************************************************************/
/// Responds to finalized selections [ie, mouse up]
void ObjectTransformTool::OnFinalSelectionChanged(Event* event)
{
    // @RYAN: TEMP [ie, might need separate "Final" version]
  OnSelectionChanged(event);
}

//******************************************************************************
void ObjectTransformTool::OnKeyDown(KeyboardEvent* e)
{
  if(!e->CtrlPressed && e->Key == Keys::X)
  {
    mBasis = (GizmoBasis::Enum)!mBasis;

    Cog* gizmo = mGizmo.ToCog();
    if(gizmo != nullptr)
    {
      gizmo->has(ObjectTransformGizmo)->mBasis = mBasis;

      if(mBasis == GizmoBasis::Local)
      {
        MetaSelection* selection = Z::gEditor->GetSelection();
        Handle object = selection->GetPrimary();

        MetaTransform* metaTransform = object.StoredType->HasInherited<MetaTransform>();
        MetaTransformInstance transform = metaTransform->GetInstance(object);

        if(transform.IsNotNull())
        {
          Transform* t = gizmo->has(Transform);
          Vec3 translation = transform.GetWorldTranslation();
          t->SetWorldTranslation(translation);

          Quat rotation = transform.GetWorldRotation();
          t->SetWorldRotation(rotation);
        }
      }
    }

    e->Handled = true;
  }

}

//******************************************************************************
void ObjectTransformTool::UpdateGrabState(GizmoGrabMode::Enum state)
{
  Cog* gizmo = mGizmo.ToCog( );
  forRange(Cog& child, gizmo->GetChildren( ))
  {
    GizmoDrag* drag;
    if(drag = child.has(GizmoDrag))
      drag->mGrabMode = state;
  }

}

/******************************************************************************/
GizmoGrabMode::Enum ObjectTransformTool::GetGrab()
{
  return mGrab;
}

void ObjectTransformTool::SetGrab(GizmoGrabMode::Enum grab)
{
  mGrab = grab;
  UpdateGrabState(grab);
}

/******************************************************************************/
GizmoBasis::Enum ObjectTransformTool::GetBasis()
{
  return mBasis;
}

void ObjectTransformTool::SetBasis(GizmoBasis::Enum basis)
{
  mBasis = basis;
  Cog* gizmo = mGizmo.ToCog();
  if(gizmo)
    gizmo->has(ObjectTransformGizmo)->mBasis = basis;
}

/******************************************************************************/
GizmoPivot::Enum ObjectTransformTool::GetPivot()
{
  return mPivot;
}

void ObjectTransformTool::SetPivot(GizmoPivot::Enum pivot)
{
  mPivot = pivot;
  Cog* gizmo = mGizmo.ToCog();
  if(gizmo)
    gizmo->has(ObjectTransformGizmo)->mPivot = pivot;
}

/******************************************************************************/
Space* ObjectTransformTool::GetSpaceFromSelection(MetaSelection* selection)
{
  Cog* primaryCog = selection->GetPrimaryAs<Cog>();
  if(primaryCog != nullptr)
    return primaryCog->GetSpace();

  return nullptr;
}

//------------------------------------------------------ ObjectTranslateTool ---
ZilchDefineType(ObjectTranslateTool, builder, type)
{
  ZeroBindComponent();

  ZilchBindGetterSetterProperty(Snapping);
  ZilchBindGetterSetterProperty(SnapDistance);
  ZilchBindGetterSetterProperty(SnapMode);

  ZeroBindTag(Tags::Tool);
}

/******************************************************************************/
ObjectTranslateTool::ObjectTranslateTool()
{
}

/******************************************************************************/
void ObjectTranslateTool::Serialize(Serializer& stream)
{
  ObjectTransformTool::Serialize(stream);
  SerializeEnumName(GizmoSnapMode, mSnapMode);
}

/******************************************************************************/
void ObjectTranslateTool::Initialize(CogInitializer& initializer)
{
  ObjectTransformTool::Initialize(initializer);
}

/******************************************************************************/
GizmoDragMode::Enum ObjectTranslateTool::GetDragMode( )
{
  Cog* gizmo = mGizmo.ToCog( );
  if(gizmo)
    return gizmo->has(TranslateGizmo)->mDragMode;
  else
    return GizmoDragMode::Line;  // Default, even though gizmo doesn't exist? Sure.
}

/******************************************************************************/
bool ObjectTranslateTool::GetSnapping( )
{
  return mSnapping;
}

void ObjectTranslateTool::SetSnapping(bool snapping)
{
  mSnapping = snapping;

  Cog* gizmo = mGizmo.ToCog( );
  if(gizmo)
    gizmo->has(TranslateGizmo)->SetSnapping(snapping);
}

/******************************************************************************/

float ObjectTranslateTool::GetSnapDistance()
{
  return mSnapDistance;
}

void ObjectTranslateTool::SetSnapDistance(float distance)
{
  mSnapDistance = distance;
  Cog* gizmo = mGizmo.ToCog( );
  if(gizmo)
    gizmo->has(TranslateGizmo)->mSnapDistance = distance;
}

/******************************************************************************/
GizmoSnapMode::Enum ObjectTranslateTool::GetSnapMode( )
{
  return mSnapMode;
}

void ObjectTranslateTool::SetSnapMode(GizmoSnapMode::Enum mode)
{
  mSnapMode = mode;
  Cog* gizmo = mGizmo.ToCog( );
  if(gizmo)
    gizmo->has(TranslateGizmo)->mSnapMode = mode;
}

/******************************************************************************/
void ObjectTranslateTool::OnGizmoObjectsDuplicated(Event* event)
{
  // The Translate Gizmo may copy the objects on drag start, so we should
  // make sure the objects on the gizmo are the objects on the gizmo
  mChangeFromUs = true;
  MetaSelection* selection = Z::gEditor->GetSelection( );
  selection->Clear(SendsEvents::False);

  Cog* gizmo = mGizmo.ToCog( );
  if(gizmo == nullptr)
    return;

  ObjectTransformGizmo* transformGizmo = mGizmo.ToCog( )->has(ObjectTransformGizmo);
  transformGizmo->GetSpace()->DispatchEvent(Events::DuplicateFirstChance, event);

  int count = transformGizmo->GetObjectCount( );
  for(int i = 0; i < count; ++i)
    selection->Add(transformGizmo->GetObjectAtIndex(i));

  selection->FinalSelectionChanged( );
  mChangeFromUs = false;
}

/******************************************************************************/
void ObjectTranslateTool::GizmoCreated(Cog* gizmo)
{
  ConnectThisTo(gizmo, Events::GizmoObjectsDuplicated, OnGizmoObjectsDuplicated);
}

/******************************************************************************/
void ObjectTranslateTool::CopyPropertiesToGizmo()
{
  Cog* gizmo = mGizmo.ToCog( );
  if(gizmo == nullptr)
    return;

  UpdateGrabState(mGrab);

  gizmo->has(TranslateGizmo)->mSnapping = mSnapping;
  gizmo->has(TranslateGizmo)->mSnapDistance = mSnapDistance;
  gizmo->has(TranslateGizmo)->mSnapMode = mSnapMode;

  gizmo->has(ObjectTranslateGizmo)->mBasis = mBasis;
  gizmo->has(ObjectTranslateGizmo)->mPivot = mPivot;
}

//---------------------------------------------------------- ObjectScaleTool ---
ZilchDefineType(ObjectScaleTool, builder, type)
{
  ZeroBindComponent();

  ZilchBindGetterSetterProperty(Snapping);
  ZilchBindGetterSetterProperty(SnapDistance);
  ZilchBindGetterSetterProperty(SnapMode);

  ZilchBindFieldProperty(mAffectTranslation);

  ZilchBindFieldGetter(mChangeInScale);

  ZeroBindTag(Tags::Tool);
}

/******************************************************************************/
ObjectScaleTool::ObjectScaleTool()
{
}

/******************************************************************************/
void ObjectScaleTool::Serialize(Serializer& stream)
{
  ObjectTransformTool::Serialize(stream);
  SerializeEnumName(GizmoSnapMode, mSnapMode);
  SerializeName(mAffectTranslation);
  SerializeName(mChangeInScale);
}

/******************************************************************************/
void ObjectScaleTool::Initialize(CogInitializer& initializer)
{
  ObjectTransformTool::Initialize(initializer);
}

/******************************************************************************/
void ObjectScaleTool::OnFrameUpdate(UpdateEvent* event)
{
  ObjectTransformTool::OnFrameUpdate(event);

  Cog* gizmo = mGizmo.ToCog( );
  if(gizmo == nullptr)
    return;

  ObjectTransformGizmo* transformGizmo = mGizmo.ToCog( )->has(ObjectTransformGizmo);
  int count = transformGizmo->GetObjectCount( );

  if(count == 1)
  {
    ObjectTransformState object = transformGizmo->GetObjectStateAtIndex(0);

    mChangeInScale = object.EndScale - object.StartScale;
  }
  else
  {
    mChangeInScale = gizmo->has(ScaleGizmo)->mChangeInScale - Vec3(1,1,1);
  }

}

/******************************************************************************/
GizmoDragMode::Enum ObjectScaleTool::GetDragMode( )
{
  Cog* gizmo = mGizmo.ToCog( );
  if(gizmo)
    return gizmo->has(ScaleGizmo)->mDragMode;
  else
    return GizmoDragMode::Line;  // Default, even though gizmo doesn't exist? Sure.
}

/******************************************************************************/
bool ObjectScaleTool::GetSnapping( )
{
  return mSnapping;
}

void ObjectScaleTool::SetSnapping(bool snapping)
{
  mSnapping = snapping;

  Cog* gizmo = mGizmo.ToCog( );
  if(gizmo)
    gizmo->has(ScaleGizmo)->SetSnapping(snapping);
}

/******************************************************************************/
float ObjectScaleTool::GetSnapDistance()
{
  return mSnapDistance;
}

void ObjectScaleTool::SetSnapDistance(float distance)
{
  mSnapDistance = distance;

  Cog* gizmo = mGizmo.ToCog( );
  if(gizmo) gizmo->has(ScaleGizmo)->mSnapDistance = distance;
}

/******************************************************************************/
GizmoSnapMode::Enum ObjectScaleTool::GetSnapMode( )
{
  return mSnapMode;
}

void ObjectScaleTool::SetSnapMode(GizmoSnapMode::Enum mode)
{
  mSnapMode = mode;

  Cog* gizmo = mGizmo.ToCog( );
  if(gizmo)
    gizmo->has(ScaleGizmo)->mSnapMode = mode;
}

/******************************************************************************/
Vec3 ObjectScaleTool::GetChangeInScale( )
{
  return mChangeInScale;
}

/******************************************************************************/
void ObjectScaleTool::CopyPropertiesToGizmo()
{
  Cog* gizmo = mGizmo.ToCog( );
  if(gizmo == nullptr)
    return;

  UpdateGrabState(mGrab);

  ScaleGizmo *scaleGizmo = gizmo->has(ScaleGizmo);

  scaleGizmo->mSnapping = mSnapping;
  scaleGizmo->mSnapDistance = mSnapDistance;
  scaleGizmo->mSnapMode = mSnapMode;

  scaleGizmo->mChangeInScale = mChangeInScale;

  ObjectScaleGizmo* objectGizmo = gizmo->has(ObjectScaleGizmo);

  objectGizmo->mBasis = mBasis;
  objectGizmo->mPivot = mPivot;
  objectGizmo->mAffectTranslation = mAffectTranslation;
}

//--------------------------------------------------------- ObjectRotateTool ---
ZilchDefineType(ObjectRotateTool, builder, type)
{
  ZeroBindComponent();
  ZilchBindGetterSetterProperty(Snapping);
  ZilchBindGetterSetterProperty(SnapAngle);
  ZilchBindFieldProperty(mAffectTranslation);

  ZilchBindFieldGetterProperty(mChangeInRotation);

  ZeroBindTag(Tags::Tool);
}

/******************************************************************************/
ObjectRotateTool::ObjectRotateTool()
{
}

/******************************************************************************/
void ObjectRotateTool::Serialize(Serializer& stream)
{
  ObjectTransformTool::Serialize(stream);
  SerializeName(mAffectTranslation);
  SerializeName(mChangeInRotation);
}

/******************************************************************************/
void ObjectRotateTool::Initialize(CogInitializer& initializer)
{
  ObjectTransformTool::Initialize(initializer);
}

/******************************************************************************/
void ObjectRotateTool::OnFrameUpdate(UpdateEvent* event)
{
  ObjectTransformTool::OnFrameUpdate(event);

  Cog* gizmo = mGizmo.ToCog( );
  if(gizmo == nullptr)
    return;

  ObjectTransformGizmo* transformGizmo = mGizmo.ToCog( )->has(ObjectTransformGizmo);
  int count = transformGizmo->GetObjectCount( );

  if(count == 1)
  {
    ObjectTransformState object = transformGizmo->GetObjectStateAtIndex(0);
    mChangeInRotation = Quat::AngleBetween(object.EndRotation, object.StartRotation);
  }
  else
  {
    mChangeInRotation = gizmo->has(RotateGizmo)->mChangeInRotation;
  }

}

/******************************************************************************/
bool ObjectRotateTool::GetSnapping( )
{
  return mSnapping;
}

void ObjectRotateTool::SetSnapping(bool snapping)
{
  mSnapping = snapping;

  Cog* gizmo = mGizmo.ToCog( );
  if(gizmo)
    gizmo->has(RotateGizmo)->SetSnapping(snapping);
}

/******************************************************************************/

float ObjectRotateTool::GetSnapAngle()
{
  return mSnapDistance;
}

void ObjectRotateTool::SetSnapAngle(float angle)
{
  mSnapDistance = angle;
  Cog* gizmo = mGizmo.ToCog();
  if(gizmo)
    gizmo->has(RotateGizmo)->mSnapAngle = angle;
}

/******************************************************************************/
void ObjectRotateTool::CopyPropertiesToGizmo()
{
  Cog* gizmo = mGizmo.ToCog( );
  if(gizmo == nullptr)
    return;

  UpdateGrabState(mGrab);

  gizmo->has(RotateGizmo)->mSnapping = mSnapping;
  gizmo->has(RotateGizmo)->mSnapAngle = mSnapDistance;

  gizmo->has(ObjectRotateGizmo)->mBasis = mBasis;
  gizmo->has(ObjectRotateGizmo)->mPivot = mPivot;
  gizmo->has(ObjectRotateGizmo)->mAffectTranslation = mAffectTranslation;
}

}// end namespace Zero

