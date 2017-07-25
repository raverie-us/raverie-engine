///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Joshua Claeys, Ryan Edgemon
/// Copyright 2010-2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//----------------------------------------------------------------------- Events
namespace Events
{
  DefineEvent(GizmoObjectsDuplicated);
}

//------------------------------------------------------- Object Transform Gizmo
ZilchDefineType(ObjectTransformGizmo, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindTag(Tags::Gizmo);
  ZeroBindDocumented();

  ZeroBindEvent(Events::GizmoObjectsDuplicated, Event);

  ZeroBindDependency(Transform);
  ZeroBindDependency(Gizmo);

  ZilchBindGetterSetterProperty(Basis);
  ZilchBindGetterSetterProperty(Pivot);

  ZilchBindMethod(AddObject);
  ZilchBindMethod(RemoveObject);
  ZilchBindMethod(ClearObjects);
  ZilchBindMethod(SetOperationQueue);
  ZilchBindMethod(ToggleCoordinateMode);
  ZilchBindGetterProperty(ObjectCount);
  ZilchBindMethod(GetObjectAtIndex);
}

//******************************************************************************
void ObjectTransformGizmo::Serialize(Serializer& stream)
{
  SerializeEnumNameDefault(GizmoBasis, mBasis, GizmoBasis::World);
  SerializeEnumNameDefault(GizmoPivot, mPivot, GizmoPivot::Average);
}

//******************************************************************************
void ObjectTransformGizmo::Initialize(CogInitializer& initializer)
{
  mRotateGizmo = false;
  mDuplicateCorrectIndex = unsigned(-1);

  mSizeBoxCollider = false;
  mDragging = false;

  mOperationQueue = nullptr;
  mTransform = GetOwner()->has(Transform);

  ConnectThisTo(GetOwner(), Events::MouseDragStart, OnMouseDragStart);
  ConnectThisTo(GetOwner(), Events::GizmoModified, OnGizmoModified);
  ConnectThisTo(GetOwner(), Events::MouseDragEnd, OnMouseDragEnd);
  ConnectThisTo(GetSpace(), Events::FrameUpdate, OnFrameUpdate);
}

//******************************************************************************
void ObjectTransformGizmo::AddObject(HandleParam object)
{
  if(object.IsNull())
    return;

  mObjects.PushBack(object);
  UpdateGizmoBasis();
}

//******************************************************************************
void ObjectTransformGizmo::RemoveObject(HandleParam object)
{
  if(object.IsNull())
    return;

  mObjects.EraseValueError(object);
  UpdateGizmoBasis();
}

//******************************************************************************
void ObjectTransformGizmo::ClearObjects()
{
  mObjects.Clear();
  mObjectStates.Clear();
}

//******************************************************************************
uint ObjectTransformGizmo::GetObjectCount()
{
  return mObjects.Size();
}

//******************************************************************************
Handle ObjectTransformGizmo::GetObjectAtIndex(uint index)
{
  if(index < mObjects.Size())
    return mObjects[index];

  DoNotifyException("Invalid Index", "Index out of range.");
  return Handle();
}

//******************************************************************************
ObjectTransformState ObjectTransformGizmo::GetObjectStateAtIndex(uint index)
{
  if(index < mObjectStates.Size( ))
    return mObjectStates[index];

  return ObjectTransformState( );
}

//******************************************************************************
void ObjectTransformGizmo::SetOperationQueue(OperationQueue* opQueue)
{
  mOperationQueue = opQueue;
}

//******************************************************************************
void ObjectTransformGizmo::ToggleCoordinateMode()
{
  mBasis = (GizmoBasis::Enum)!mBasis;
}

//******************************************************************************
void ObjectTransformGizmo::OnMouseDragStart(ViewportMouseEvent* e)
{
  mDragging = true;
  mObjectStates.Clear();

  Array<Handle> metaObjects;
  FilterChildrenAndProtected(mObjects, metaObjects);

  // Store the state of all objects
  forRange(Handle target, metaObjects.All())
  {
    ObjectTransformState data;
    data.MetaObject = target;

    MetaTransform* metaTransform = target.StoredType->HasInherited<MetaTransform>();
    ErrorIf(metaTransform == nullptr, "No MetaTransform on object being transformed.");

    MetaTransformInstance transform = metaTransform->GetInstance(target);
        
    if(transform.mLocalTranslation != nullptr)
    {
      // local
      data.StartTranslation = transform.GetLocalTranslation();
      data.EndTranslation = data.StartTranslation;

      // world
      data.StartWorldTranslation = transform.GetWorldTranslation();
    }
    if(transform.mLocalRotation != nullptr)
    { 
      // local
      data.StartRotation = transform.GetLocalRotation();
      data.EndRotation = data.StartRotation;
    }
    if(transform.mLocalScale != nullptr)
    {
      data.StartScale = transform.GetLocalScale();
      data.EndScale = data.StartScale;
    }

    //data.WorldAabb = GetAabb(target);

    data.StartSize = Vec2::cZero;
    data.EndSize = Vec2::cZero;

    if(Cog* cog = target.Get<Cog*>())
    {
      if(Area* area = cog->has(Area))
      {
        data.StartSize = area->mSize;
        data.EndSize = area->mSize;
      }
    }

    mObjectStates.PushBack(data);
  }
}

//******************************************************************************
void ObjectTransformGizmo::OnGizmoModified(GizmoUpdateEvent* e)
{

}

//******************************************************************************
void ObjectTransformGizmo::OnMouseDragEnd(Event* e)
{
  mDragging = false;
  // No need to do anything if we don't have an operation queue or
  // were not transforming any objects
  OperationQueue* queue = mOperationQueue;
  if(mOperationQueue == nullptr || mObjectStates.Empty())
    return;

  // We want everything to be in the same operation batch so that it's
  // all undone at the same time
  queue->BeginBatch();
  queue->SetActiveBatchName("TransformGizmo_Batch");

  forRange(ObjectTransformState& objectState, mObjectStates.All())
  {
    Handle target = objectState.MetaObject;
    if(target.IsNull())
      continue;

    MetaTransform* metaTransform = target.StoredType->HasInherited<MetaTransform>();
    MetaTransformInstance transform = metaTransform->GetInstance(target);

    if(transform.IsNull())
      continue;

    // Used for queuing property changes
    //Property* translationProperty = transform.mTranslation;
    //Property* rotationProperty = transform.mRotation;
    //Property* scaleProperty = transform.mScale;

    //@RYAN: COG_NOT_GENERIC
    // If the object was newly created, queue an operation for its creation
    Cog* cog = target.Get<Cog*>();
    if(cog && mNewObjects.Contains(target))
    {
      String name;
      if (MetaDisplay* display = target.StoredType->HasInherited<MetaDisplay>())
        name = display->GetName(target);
      else
        name = target.StoredType->Name;

      queue->SetActiveBatchName(BuildString("Create ", name));

      ObjectCreated(queue, cog);

        // single duplicated object proper hierarchy, emulates ctrl+c, ctrl+v behavior
      if(mObjectStates.Size() == 1 && mDuplicateCorrectIndex != unsigned(-1))
      {
        MoveObjectIndex(queue, cog, mDuplicateCorrectIndex);
        mDuplicateCorrectIndex = unsigned(-1);
      }

    }

    queue->BeginBatch();

    // Send the final GizmoFinish transform update
    uint flag = 0;

    // When scaling or rotating multiple objects translation may changed so just check
    if((objectState.StartTranslation - objectState.EndTranslation).LengthSq()!=0.0f)
    {
      transform.SetLocalTranslation(objectState.StartTranslation);

      // Queue the change
      PropertyPath propertyPath(transform.mLocalTranslation);
      ChangeAndQueueProperty(queue, transform.mInstance, propertyPath, objectState.EndTranslation);

      String name;
      if (MetaDisplay* display = target.StoredType->HasInherited<MetaDisplay>())
        name = display->GetName(target);
      else
        name = target.StoredType->Name;

      queue->SetActiveBatchName(BuildString("Translate '", name, "'"));
    }

    if(mRotateGizmo)
    {
      if((objectState.EndRotation.Inverted( ) * objectState.StartRotation) != Quat::cIdentity)
      {
        transform.SetLocalRotation(objectState.StartRotation);

        // Queue the change
        PropertyPath propertyPath(transform.mLocalRotation);
        ChangeAndQueueProperty(queue, transform.mInstance, propertyPath, objectState.EndRotation);

        String name;
        if(MetaDisplay* display = target.StoredType->HasInherited<MetaDisplay>( ))
          name = display->GetName(target);
        else
          name = target.StoredType->Name;

        queue->SetActiveBatchName(BuildString("Rotate '", name, "'"));
       }
    }

    if((objectState.StartScale - objectState.EndScale).LengthSq()!=0.0f)
    {
      transform.SetLocalScale(objectState.StartScale);

      // Queue the change
      PropertyPath propertyPath(transform.mLocalScale);
      ChangeAndQueueProperty(queue, transform.mInstance, propertyPath, objectState.EndScale);

      String name;
      if (MetaDisplay* display = target.StoredType->HasInherited<MetaDisplay>())
        name = display->GetName(target);
      else
        name = target.StoredType->Name;

      queue->SetActiveBatchName(BuildString("Scale '", name, "'"));
    }

    if((objectState.StartSize - objectState.EndSize).LengthSq()!=0.0f)
    {
      if(Cog* cog = target.Get<Cog*>())
      {
        if(Area* area = cog->has(Area))
        {
          Property* sizeProperty = ZilchVirtualTypeId(area)->GetProperty("Size");
          area->SetSize(objectState.StartSize);

          // Queue the change to size
          PropertyPath sizePath(sizeProperty);
          ChangeAndQueueProperty(queue, area, sizePath, objectState.EndSize);

          if(mSizeBoxCollider)
          {
            if(BoxCollider* collider = cog->has(BoxCollider))
            {
              BoundType* colliderType = ZilchVirtualTypeId(collider);

              Vec3 startOffset = collider->GetOffset();
              Vec3 startSize = collider->GetSize();
              Vec3 endOffset = Vec3(OffsetOfOffset(area->GetOrigin(), Location::Center) * area->GetSize(), 0);

              Property* offsetProperty = colliderType->GetProperty("Offset");
              collider->SetOffset(startOffset);

              // Queue the change to offset
              PropertyPath offsetPath(offsetProperty);
              ChangeAndQueueProperty(queue, collider, offsetPath, endOffset);

              Vec3 endSize = Vec3(objectState.EndSize.x, objectState.EndSize.y, startSize.z);

              Property* sizeProperty = colliderType->GetProperty("Size");
              collider->SetSize(startSize);

              // Queue the change to size
              PropertyPath colliderSizePath(sizeProperty);
              ChangeAndQueueProperty(queue, collider, colliderSizePath, endSize);
            }

            String name;
            if (MetaDisplay* display = target.StoredType->HasInherited<MetaDisplay>())
              name = display->GetName(target);
            else
              name = target.StoredType->Name;

            queue->SetActiveBatchName(BuildString("'", name, "' size change"));
          }
        }
      }
    }

    queue->EndBatch();
  }

  queue->EndBatch();
  mNewObjects.Clear();
}

//******************************************************************************
void ObjectTransformGizmo::UpdateGizmoBasis()
{
  if(mDragging)
    return;

  if(mObjects.Empty())
    return;

  Handle primary = mObjects.Front();
  if(primary.IsNull())
    return;

  Quat rotation = Quat::cIdentity;

  MetaTransform* metaTransform = primary.StoredType->HasInherited<MetaTransform>();
  MetaTransformInstance transform = metaTransform->GetInstance(primary);

  if(transform.IsNotNull())
  {
    if(mBasis == GizmoBasis::Local)
      rotation = transform.GetWorldRotation();
    else
      rotation = Quat::cIdentity;
  }

  Vec3 center = Vec3::cZero;

  if(mPivot == GizmoPivot::Primary)
  {
    if(transform.IsNotNull( ))
      center = transform.GetWorldTranslation( );
  }
  else if(mPivot == GizmoPivot::Center)
  {
    Aabb aabb = GetAabb(primary);
    center = aabb.GetCenter( );
  }
  else if(mPivot == GizmoPivot::Average)
  {
    uint count = 0;
    forRange(Handle object, mObjects.All( ))
    {
      if(object.IsNotNull( ))
      {
        MetaTransform* metaTransform = object.StoredType->HasInherited<MetaTransform>( );
        MetaTransformInstance transform = metaTransform->GetInstance(object);
        if(transform.IsNotNull( ))
        {
          center += transform.GetWorldTranslation( );
          ++count;
        }

      }

    }

    if(count != 0)
      center /= float(count);
  }

  mTransform->SetTranslation(center);
  mTransform->SetRotation(rotation);
}

//******************************************************************************
void ObjectTransformGizmo::OnFrameUpdate(UpdateEvent* e)
{
  if(GetOwner()->GetMarkedForDestruction())
    return;

  UpdateGizmoBasis();
}

//******************************************************************************
void ObjectTransformGizmo::SetBasis(GizmoBasis::Enum basis)
{
  mBasis = basis;
  UpdateGizmoBasis();
}

//******************************************************************************
GizmoBasis::Enum ObjectTransformGizmo::GetBasis()
{
  return mBasis;
}

//******************************************************************************
void ObjectTransformGizmo::SetPivot(GizmoPivot::Enum pivot)
{
  mPivot = pivot;
  UpdateGizmoBasis();
}

//******************************************************************************
GizmoPivot::Enum ObjectTransformGizmo::GetPivot()
{
  return mPivot;
}

//---------------------------------------------------------- Cog Translate Gizmo
ZilchDefineType(ObjectTranslateGizmo, builder, type)
{
  ZeroBindComponent();
  ZeroBindInterface(ObjectTransformGizmo);
  ZeroBindDependency(TranslateGizmo);

  ZilchBindFieldProperty(mDuplicateOnCtrlDrag);
}

//******************************************************************************
void ObjectTranslateGizmo::Serialize(Serializer& stream)
{
  ObjectTransformGizmo::Serialize(stream);

  SerializeNameDefault(mDuplicateOnCtrlDrag, true);
}

//******************************************************************************
void ObjectTranslateGizmo::Initialize(CogInitializer& initializer)
{
  ConnectThisTo(GetOwner(), Events::GizmoModified, OnGizmoModified);

  ObjectTransformGizmo::Initialize(initializer);
}

//******************************************************************************
void ObjectTranslateGizmo::OnMouseDragStart(ViewportMouseEvent* e)
{
  if(e->CtrlPressed && mDuplicateOnCtrlDrag)
  {
    mNewObjects.Clear();

    forRange(Handle object, mObjects.All())
    {
      Cog* cog = object.Get<Cog*>();
      if(cog && CogSerialization::ShouldSave(*cog))
      {
        Cog* copy = cog->Clone();
        mNewObjects.Insert(copy);

          // new object will go directly after the object it was copied from
        mDuplicateCorrectIndex = cog->GetHierarchyIndex( ) + 1;
      }
      else  // currently unnecessary to check for protection on non-cogs
      {
        mNewObjects.Insert(object);
      }
    }

    ClearObjects();

    forRange(Handle object, mNewObjects.All())
    {
      AddObject(object);
    }
    
    // Signal that the objects were duplicated
    Event eventToSend;
    GetOwner()->DispatchEvent(Events::GizmoObjectsDuplicated, &eventToSend);
    GetOwner()->DispatchUp(Events::GizmoObjectsDuplicated, &eventToSend);
  }

  mStartPosition = mTransform->GetWorldTranslation();

  ObjectTransformGizmo::OnMouseDragStart(e);
}

//******************************************************************************
void ObjectTranslateGizmo::OnGizmoModified(GizmoUpdateEvent* e)
{
  Vec3 worldMovement = e->mMouseWorldMovement;

  GizmoDrag* gizmoDrag = e->GetGizmo( )->has(GizmoDrag);
  bool viewPlaneGizmo = (gizmoDrag->mDragMode == GizmoDragMode::ViewPlane);

    // Special command, possibly modifies 'worldMovement'
  if(viewPlaneGizmo && Keyboard::Instance->KeyIsDown(Keys::V))
    SnapToSurface(e, &worldMovement);

  TranslateGizmo* baseGizmo = GetOwner()->has(TranslateGizmo);

  forRange(ObjectTransformState& objectState, mObjectStates.All())
  {
    Handle target = objectState.MetaObject;
    if(target.IsNull())
      continue;

    MetaTransform* metaTransform = target.StoredType->HasInherited<MetaTransform>();
    MetaTransformInstance transform = metaTransform->GetInstance(target);
    if(transform.IsNull())
      continue;

    Mat4 inverseMatrix(Mat4::cIdentity);
    inverseMatrix = transform.GetParentWorldMatrix();
    inverseMatrix.Invert();
  
    Vec3 localMovement = Math::TransformNormal(inverseMatrix, worldMovement);

    objectState.EndTranslation = baseGizmo->TranslateFromDrag(gizmoDrag,
      objectState.StartTranslation, localMovement, transform.GetWorldRotation( ));

    transform.SetLocalTranslation(objectState.EndTranslation);
  }

}

//------------------------------------------------------------------------------
static void AddHierarchyIntoSet(Cog* cog, HashSet<Cog*>& set)
{
  set.Insert(cog);

  Hierarchy* hierarchy = cog->has(Hierarchy);
  if(hierarchy ==  nullptr)
    return;

  forRange(Cog& child, hierarchy->GetChildren( ))
  {
    AddHierarchyIntoSet(&child, set);
  }

}

//******************************************************************************
void ObjectTranslateGizmo::SnapToSurface(GizmoUpdateEvent* e, Vec3* movementOut)
{
  ViewportMouseEvent* vpEvent = e->GetViewportMouseEvent( );
  if(vpEvent == nullptr)
    return;

    // Ignore all selected objects and their children
  HashSet<Cog*> ignoredObjects;
  forRange(ObjectTransformState& objectState, mObjectStates.All( ))
  {
    Handle target = objectState.MetaObject;
    if(target.IsNull( ))
      continue;

    //@RYAN: COG_NOT_GENERIC
    Cog* object = (Cog*)target.Dereference();

    if(object)
      AddHierarchyIntoSet(object, ignoredObjects);
  }

  RaycastResultList results(ignoredObjects.Size( ) + 1);

  Space* space = GetSpace( );
  CameraViewport* viewport = vpEvent->GetCameraViewport();
  Vec2 mousePosition = vpEvent->Position;

  CastInfo castInfo(space, viewport->GetCameraCog( ), mousePosition);
  Ray ray = vpEvent->mWorldRay;

  // Raycast into both physics and graphics
  Raycaster rayCaster;
  rayCaster.AddProvider(new PhysicsRaycastProvider( ));
  rayCaster.AddProvider(new GraphicsRaycastProvider( ));
  rayCaster.RayCast(ray, castInfo, results);

  for(uint i = 0; i < results.mSize; ++i)
  {
    RayCastEntry& entry = results.mEntries[i];
    if(!ignoredObjects.Contains(entry.HitCog))
    {
      *movementOut = ray.GetPoint(entry.T) - e->mInitialGrabPoint;

      // Don't need to search for other hits, just take the first one.
      // First should have the lowest 'T' value, anyway.
      return;
    }

  }

}

//----------------------------------------------------------- Object Scale Gizmo
ZilchDefineType(ObjectScaleGizmo, builder, type)
{
  ZeroBindComponent();
  ZeroBindInterface(ObjectTransformGizmo);
  ZeroBindDependency(ScaleGizmo);

  ZilchBindFieldProperty(mAffectTranslation);
}

//******************************************************************************
void ObjectScaleGizmo::Serialize(Serializer& stream)
{
  ObjectTransformGizmo::Serialize(stream);

  SerializeNameDefault(mAffectTranslation, true);
}

//******************************************************************************
void ObjectScaleGizmo::Initialize(CogInitializer& initializer)
{
  ConnectThisTo(GetOwner(), Events::GizmoModified, OnGizmoModified);

  ObjectTransformGizmo::Initialize(initializer);
}

//******************************************************************************
void ObjectScaleGizmo::OnMouseDragStart(ViewportMouseEvent* e)
{
  //mStartPosition = mTransform->GetWorldTranslation();

  Camera* camera = e->GetViewport()->GetCamera();
  if (camera)
    mEyeDirection = -Math::ToMatrix3(camera->mTransform->GetWorldRotation()).BasisZ();
  else
    mEyeDirection = Vec3::cZero;

  ObjectTransformGizmo::OnMouseDragStart(e);
}

//******************************************************************************
void ObjectScaleGizmo::OnGizmoModified(GizmoUpdateEvent* e)
{
  Vec3 worldMovement = e->mMouseWorldMovement;

  Vec3 gizmoPosition = mTransform->GetWorldTranslation( );
  Vec3 grabDirection = (e->mInitialGrabPoint - gizmoPosition);
  float distance = grabDirection.Length( );

  Mat3 worldRotationBasis = Math::ToMatrix3(mTransform->GetWorldRotation( ));
  Mat3 inverseWorld = worldRotationBasis.Inverted( );

  GizmoDrag* gizmoDrag = e->GetGizmo( )->has(GizmoDrag);
  ScaleGizmo* baseGizmo = GetOwner()->has(ScaleGizmo);

  // If we're transforming multiple objects, we will have to translate them
  bool multiTransform = mObjectStates.Size() > 1;

  forRange(ObjectTransformState& objectState, mObjectStates.All())
  {
    Handle target = objectState.MetaObject;
    if(target.IsNull())
    continue;

    MetaTransform* metaTransform = target.StoredType->HasInherited<MetaTransform>();
    MetaTransformInstance transform = metaTransform->GetInstance(target);

    if(transform.IsNull())
      continue;

    Mat4 inverseMatrix(Mat4::cIdentity);
    inverseMatrix = transform.GetParentWorldMatrix().Inverted( );

    const float cRotationLengthLimit = 0.001f;
    if(multiTransform && mAffectTranslation)
    {
      // Update the object's translation by scaling it about the basis
      // pivot Translation point
      Vec3 pivotOffset = objectState.StartWorldTranslation - gizmoPosition;
      if(pivotOffset.Length() > cRotationLengthLimit)
      {
        Math::Transform(inverseWorld, &pivotOffset);

        //Scale the pivotOffset
        pivotOffset *= baseGizmo->mScaledLocalMovement;

        //Transform it back into world space
        Math::Transform(worldRotationBasis, &pivotOffset);

        Vec3 newPosition = gizmoPosition + pivotOffset;
        newPosition = Math::TransformPoint(inverseMatrix, newPosition);

        transform.SetLocalTranslation(newPosition);
        objectState.EndTranslation = newPosition;
      }
    }

      // Mouse-drag movement in object local orientation.
    Vec3 localMovement = Math::TransformNormal(inverseMatrix, worldMovement);
    Quat localRotation(objectState.StartRotation);
    localRotation.Invert( );
    localMovement = Math::Multiply(localRotation, localMovement);

    Vec3 newScale = baseGizmo->ScaleFromDrag(gizmoDrag, distance,
      localMovement, objectState.StartScale, transform.GetWorldRotation( ));

      // Final object scale (at this step) of gizmo modification.
    objectState.EndScale = newScale;
    transform.SetLocalScale(newScale);
  }
}

//---------------------------------------------------------- Object Rotate Gizmo
ZilchDefineType(ObjectRotateGizmo, builder, type)
{
  ZeroBindComponent();
  ZeroBindInterface(ObjectTransformGizmo);
  ZeroBindDependency(RotateGizmo);

  ZilchBindFieldProperty(mAffectTranslation);
}

//******************************************************************************
void ObjectRotateGizmo::Serialize(Serializer& stream)
{
  ObjectTransformGizmo::Serialize(stream);

  SerializeNameDefault(mAffectTranslation, true);
}

//******************************************************************************
void ObjectRotateGizmo::Initialize(CogInitializer& initializer)
{
  ConnectThisTo(GetOwner(), Events::RingGizmoModified, OnGizmoModified);

  ObjectTransformGizmo::Initialize(initializer);

  mRotateGizmo = true;
}

//******************************************************************************
void ObjectRotateGizmo::OnMouseDragStart(ViewportMouseEvent* e)
{
  ObjectTransformGizmo::OnMouseDragStart(e);
}

//******************************************************************************
void ObjectRotateGizmo::OnGizmoModified(RingGizmoEvent* e)
{
  Quat worldRotation = e->mWorldRotation;
  Vec3 selectedAxis = e->mWorldRotationAxis;
  float deltaOnAxis = e->mDeltaRadiansAroundAxis;

  Vec3 gizmoPos = mTransform->GetWorldTranslation();
  RotateGizmo* baseGizmo = GetOwner()->has(RotateGizmo);

  float moveOnAxis = e->mRadiansAroundAxis;
  if(baseGizmo->GetSnapping( ))
    moveOnAxis = Snap(moveOnAxis, Math::DegToRad(baseGizmo->mSnapAngle));

  // If we're transforming multiple objects, we will have to translate them
  bool multiTransform = mObjectStates.Size() > 1;

  Quat newRotation;

  // Apply the rotation transform to each selected object
  forRange(ObjectTransformState& objectState, mObjectStates.All())
  {
    Handle target = objectState.MetaObject;
    if(target.IsNull())
      continue;

    MetaTransform* metaTransform = target.StoredType->HasInherited<MetaTransform>();
    MetaTransformInstance transform = metaTransform->GetInstance(target);

    if(transform.IsNull())
      continue;

    //save the old transform so that we can properly apply the deltas to in-world objects
    Mat4 oldMat = transform.GetParentWorldMatrix();
    Mat4 inverseMatrix(Mat4::cIdentity);
    inverseMatrix = oldMat.Inverted( );

    const float cRotationLengthLimit = 0.001f;
    if(multiTransform && mAffectTranslation)
    {
      Vec3 pivot = objectState.StartWorldTranslation - gizmoPos;
      if(pivot.Length() > cRotationLengthLimit)
      {
        Vec3 rotatedPivot = Math::Multiply(worldRotation, pivot);
        Vec3 newWorldPosition = gizmoPos + rotatedPivot;

        Vec3 newLocalPosition = Math::TransformPoint(inverseMatrix, newWorldPosition);
        transform.SetLocalTranslation(newLocalPosition);
        objectState.EndTranslation = newLocalPosition;
      }
    }

    Vec3 localAxis = Math::TransformNormal(inverseMatrix, selectedAxis).AttemptNormalized();

    //Construct a local rotation
    Quat localRotation;
    Math::ToQuaternion(localAxis, moveOnAxis, &localRotation);

    //Add the current rotation to the starting rotation
    Quat newRotation = localRotation * objectState.StartRotation;

    //Normalize to prevent rounding errors
    newRotation.Normalize();

    objectState.EndRotation = newRotation;

    // Set the rotation
    transform.SetLocalRotation(newRotation);
  }
}


}//namespace Zero
