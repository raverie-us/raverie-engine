// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
DefineEvent(GizmoObjectsDuplicated);
DefineEvent(ObjectTransformGizmoStart);
DefineEvent(ObjectTranslateGizmoModified);
DefineEvent(ObjectScaleGizmoModified);
DefineEvent(ObjectRotateGizmoModified);
DefineEvent(ObjectTransformGizmoEnd);
} // namespace Events

ZilchDefineType(ObjectTransformGizmoEvent, builder, type)
{
  ZilchBindGetterSetter(FinalLocalTranslation);
  ZilchBindGetterSetter(FinalLocalScale);
  ZilchBindGetterSetter(FinalLocalRotation);
}

ObjectTransformGizmoEvent::ObjectTransformGizmoEvent(Component* sourceGizmo, Cog* owner, ViewportMouseEvent* base) :
    GizmoEvent(owner, base)
{
  mSource = sourceGizmo;
  mGizmoType = sourceGizmo->ZilchGetDerivedType();

  mFinalLocalTranslation = Vec3::cZero;
  mFinalLocalScale = Vec3::cZero;
  mFinalLocalRotation = Quat::cIdentity;

  mCanAlterScale = false;
  mCanAlterRotation = false;

  // Note: All transform gizmo's can alter translation.
  if (mGizmoType == ZilchTypeId(ObjectScaleGizmo))
    mCanAlterScale = true;
  else if (mGizmoType == ZilchTypeId(ObjectRotateGizmo))
    mCanAlterRotation = true;
}

Vec3 ObjectTransformGizmoEvent::GetFinalLocalTranslation()
{
  return mFinalLocalTranslation;
}

Vec3 ObjectTransformGizmoEvent::GetFinalLocalScale()
{
  if (!mCanAlterScale)
  {
    String message = BuildString(mGizmoType->Name, " doesn't operate on Scale.");
    DoNotifyErrorWithContext(message);
  }

  return mFinalLocalScale;
}

Quat ObjectTransformGizmoEvent::GetFinalLocalRotation()
{
  if (!mCanAlterRotation)
  {
    String message = BuildString(mGizmoType->Name, " doesn't operate on Rotation.");
    DoNotifyErrorWithContext(message);
  }

  return mFinalLocalRotation;
}

void ObjectTransformGizmoEvent::SetFinalLocalTranslation(Vec3Param translation)
{
  mFinalLocalTranslation = translation;
}

void ObjectTransformGizmoEvent::SetFinalLocalScale(Vec3Param scale)
{
  if (!mCanAlterScale)
  {
    String message = BuildString(mGizmoType->Name, " doesn't operate on Scale.");
    DoNotifyErrorWithContext(message);
  }

  mFinalLocalScale = scale;
}

void ObjectTransformGizmoEvent::SetFinalLocalRotation(QuatParam rotation)
{
  if (!mCanAlterRotation)
  {
    String message = BuildString(mGizmoType->Name, " doesn't operate on Rotation.");
    DoNotifyErrorWithContext(message);
  }

  mFinalLocalRotation = rotation;
}

ObjectTransformState::ObjectTransformState()
{
  StartWorldTranslation = Vec3::cZero;
  StartTranslation = Vec3::cZero;
  StartRotation = Quat::cIdentity;
  StartScale = Vec3::cZero;
  StartSize = Vec2::cZero;

  EndTranslation = Vec3::cZero;
  EndRotation = Quat::cIdentity;
  EndScale = Vec3::cZero;
  EndSize = Vec2::cZero;
}

static const float cPivotDistanceThreshold = 0.001f;

ZilchDefineType(ObjectTransformGizmo, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindTag(Tags::Gizmo);
  ZeroBindDocumented();

  ZeroBindEvent(Events::GizmoObjectsDuplicated, Event);
  ZeroBindEvent(Events::ObjectTransformGizmoStart, GizmoUpdateEvent);
  ZeroBindEvent(Events::ObjectTranslateGizmoModified, ObjectTransformGizmoEvent);
  ZeroBindEvent(Events::ObjectScaleGizmoModified, ObjectTransformGizmoEvent);
  ZeroBindEvent(Events::ObjectRotateGizmoModified, ObjectTransformGizmoEvent);
  ZeroBindEvent(Events::ObjectTransformGizmoEnd, ObjectTransformGizmoEvent);

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

void ObjectTransformGizmo::Serialize(Serializer& stream)
{
  SerializeEnumNameDefault(GizmoBasis, mBasis, GizmoBasis::World);
  SerializeEnumNameDefault(GizmoPivot, mPivot, GizmoPivot::Average);
}

void ObjectTransformGizmo::Initialize(CogInitializer& initializer)
{
  mRotateGizmo = false;
  mDuplicateCorrectIndex = unsigned(-1);

  mSizeBoxCollider = false;
  mDragging = false;

  mOperationQueue = nullptr;
  mTransform = GetOwner()->has(Transform);

  ConnectThisTo(GetOwner(), Events::MouseDragStart, OnMouseDragStart);
  ConnectThisTo(GetOwner(), Events::MouseDragEnd, OnMouseDragEnd);
  ConnectThisTo(GetSpace(), Events::FrameUpdate, OnFrameUpdate);
}

void ObjectTransformGizmo::AddObject(HandleParam object, bool updateBasis)
{
  if (object.IsNull())
    return;

  MetaTransform* metaTransform = object.StoredType->HasInherited<MetaTransform>();
  if (!metaTransform)
    return;

  mObjects.PushBack(object);

  if (updateBasis)
    UpdateGizmoBasis();
}

void ObjectTransformGizmo::RemoveObject(HandleParam object, bool updateBasis)
{
  if (object.IsNull())
    return;

  mObjects.EraseValue(object);

  if (updateBasis)
    UpdateGizmoBasis();
}

void ObjectTransformGizmo::ClearObjects()
{
  mObjects.Clear();
  mObjectStates.Clear();
  UpdateGizmoBasis();
}

uint ObjectTransformGizmo::GetObjectCount()
{
  return mObjects.Size();
}

Handle ObjectTransformGizmo::GetObjectAtIndex(uint index)
{
  if (index < mObjects.Size())
    return mObjects[index];

  DoNotifyException("Invalid Index", "Index out of range.");
  return Handle();
}

ObjectTransformState ObjectTransformGizmo::GetObjectStateAtIndex(uint index)
{
  if (index < mObjectStates.Size())
    return mObjectStates[index];

  return ObjectTransformState();
}

void ObjectTransformGizmo::SetOperationQueue(OperationQueue* opQueue)
{
  mOperationQueue = opQueue;
}

void ObjectTransformGizmo::ToggleCoordinateMode()
{
  mBasis = (GizmoBasis::Enum)!mBasis;
}

void ObjectTransformGizmo::OnMouseDragStart(ViewportMouseEvent* event)
{
  mDragging = true;
  mObjectStates.Clear();

  Array<Handle> metaObjects;
  FilterChildrenAndProtected(mObjects, metaObjects);

  GizmoUpdateEvent eventToSend(GetOwner(), event);

  // Store the state of all objects
  forRange (Handle target, metaObjects.All())
  {
    ObjectTransformState data;
    data.MetaObject = target;

    MetaTransform* metaTransform = target.StoredType->HasInherited<MetaTransform>();
    ErrorIf(metaTransform == nullptr, "No MetaTransform on object being transformed.");

    MetaTransformInstance transform = metaTransform->GetInstance(target);

    if (transform.mLocalTranslation != nullptr)
    {
      // local
      data.StartTranslation = transform.GetLocalTranslation();
      data.EndTranslation = data.StartTranslation;

      // world
      data.StartWorldTranslation = transform.GetWorldTranslation();
    }
    if (transform.mLocalRotation != nullptr)
    {
      // local
      data.StartRotation = transform.GetLocalRotation();
      data.EndRotation = data.StartRotation;
    }
    if (transform.mLocalScale != nullptr)
    {
      data.StartScale = transform.GetLocalScale();
      data.EndScale = data.StartScale;
    }

    // data.WorldAabb = GetAabb(target);

    data.StartSize = Vec2::cZero;
    data.EndSize = Vec2::cZero;

    if (Cog* cog = target.Get<Cog*>())
    {
      if (Area* area = cog->has(Area))
      {
        data.StartSize = area->mSize;
        data.EndSize = area->mSize;
      }
    }

    if (Object* object = target.Get<Object*>())
    {
      if (EventDispatcher* dispatcher = object->GetDispatcher())
        dispatcher->Dispatch(Events::ObjectTransformGizmoStart, &eventToSend);
    }

    mObjectStates.PushBack(data);
  }
}

void ObjectTransformGizmo::OnGizmoModified(GizmoUpdateEvent* e)
{
}

void ObjectTransformGizmo::OnMouseDragEnd(ViewportMouseEvent* event)
{
  mDragging = false;
  // No need to do anything if we don't have an operation queue or
  // were not transforming any objects
  OperationQueue* queue = mOperationQueue;
  if (mOperationQueue == nullptr || mObjectStates.Empty())
    return;

  ObjectTransformGizmoEvent eventToSend(this, GetOwner(), event);
  eventToSend.mOperationQueue = queue;

  // We want everything to be in the same operation batch so that it's
  // all undone at the same time
  queue->BeginBatch();
  queue->SetActiveBatchName("MultiTransform");

  forRange (ObjectTransformState& objectState, mObjectStates.All())
  {
    Handle target = objectState.MetaObject;
    if (target.IsNull())
      continue;

    MetaTransform* metaTransform = target.StoredType->HasInherited<MetaTransform>();
    MetaTransformInstance transform = metaTransform->GetInstance(target);

    if (transform.IsNull())
      continue;

    String name;
    if (MetaDisplay* display = target.StoredType->HasInherited<MetaDisplay>())
      name = display->GetName(target);
    else
      name = target.StoredType->Name;

    //@RYAN: COG_NOT_GENERIC
    // If the object was newly created, queue an operation for its creation
    Cog* cog = target.Get<Cog*>();
    if (cog && mNewObjects.Contains(target))
    {
      queue->SetActiveBatchName(BuildString("Create ", name));

      ObjectCreated(queue, cog);

      // single duplicated object proper hierarchy, emulates ctrl+c, ctrl+v
      // behavior
      if (mObjectStates.Size() == 1 && mDuplicateCorrectIndex != unsigned(-1))
      {
        MoveObjectIndex(queue, cog, mDuplicateCorrectIndex);
        mDuplicateCorrectIndex = unsigned(-1);
      }
    }

    EventDispatcher* dispatcher = nullptr;
    Object* object = target.Get<Object*>();
    if (object != nullptr)
      dispatcher = object->GetDispatcher();

    // Reset before each object as the previous object could have modified it.
    eventToSend.mFinalLocalTranslation = objectState.EndTranslation;
    eventToSend.mFinalLocalScale = objectState.EndScale;
    eventToSend.mFinalLocalRotation = objectState.EndRotation;

    if (dispatcher != nullptr)
      dispatcher->Dispatch(Events::ObjectTransformGizmoEnd, &eventToSend);

    // User modified?
    objectState.EndTranslation = eventToSend.mFinalLocalTranslation;
    objectState.EndScale = eventToSend.mFinalLocalScale;
    objectState.EndRotation = eventToSend.mFinalLocalRotation;

    queue->BeginBatch();

    // Send the final GizmoFinish transform update
    uint flag = 0;

    // When scaling or rotating multiple objects translation may changed so just
    // check
    Vec3 deltaT = objectState.StartTranslation - objectState.EndTranslation;
    if (deltaT.LengthSq() > Math::Epsilon())
    {
      transform.SetLocalTranslation(objectState.StartTranslation);

      // Queue the change
      PropertyPath propertyPath(transform.mLocalTranslation);
      ChangeAndQueueProperty(queue, transform.mInstance, propertyPath, objectState.EndTranslation);

      queue->SetActiveBatchName(BuildString("Translate '", name, "'"));
    }

    if (mRotateGizmo)
    {
      if ((objectState.EndRotation.Inverted() * objectState.StartRotation) != Quat::cIdentity)
      {
        transform.SetLocalRotation(objectState.StartRotation);

        // Queue the change
        PropertyPath propertyPath(transform.mLocalRotation);
        ChangeAndQueueProperty(queue, transform.mInstance, propertyPath, objectState.EndRotation);

        queue->SetActiveBatchName(BuildString("Rotate '", name, "'"));
      }
    }

    Vec3 deltaS = objectState.StartScale - objectState.EndScale;
    if (deltaS.LengthSq() > Math::Epsilon())
    {
      transform.SetLocalScale(objectState.StartScale);

      // Queue the change
      PropertyPath propertyPath(transform.mLocalScale);
      ChangeAndQueueProperty(queue, transform.mInstance, propertyPath, objectState.EndScale);

      queue->SetActiveBatchName(BuildString("Scale '", name, "'"));
    }

    if ((objectState.StartSize - objectState.EndSize).LengthSq() != 0.0f)
    {
      if (Cog* areaCog = target.Get<Cog*>())
      {
        if (Area* area = areaCog->has(Area))
        {
          Property* sizeProperty = ZilchVirtualTypeId(area)->GetProperty("Size");
          area->SetSize(objectState.StartSize);

          // Queue the change to size
          PropertyPath sizePath(sizeProperty);
          ChangeAndQueueProperty(queue, area, sizePath, objectState.EndSize);

          if (mSizeBoxCollider)
          {
            if (BoxCollider* collider = areaCog->has(BoxCollider))
            {
              BoundType* colliderType = ZilchVirtualTypeId(collider);

              Vec3 startOffset = collider->GetOffset();
              Vec3 startSize = collider->GetSize();
              Vec3 endOffset = Vec3(Location::GetDirection(area->GetOrigin(), Location::Center) * area->GetSize(), 0);

              Property* offsetProperty = colliderType->GetProperty("Offset");
              collider->SetOffset(startOffset);

              // Queue the change to offset
              PropertyPath offsetPath(offsetProperty);
              ChangeAndQueueProperty(queue, collider, offsetPath, endOffset);

              Vec3 endSize = Vec3(objectState.EndSize.x, objectState.EndSize.y, startSize.z);

              Property* colliderSizeProperty = colliderType->GetProperty("Size");
              collider->SetSize(startSize);

              // Queue the change to size
              PropertyPath colliderSizePath(colliderSizeProperty);
              ChangeAndQueueProperty(queue, collider, colliderSizePath, endSize);
            }

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

void ObjectTransformGizmo::UpdateGizmoBasis()
{
  if (mDragging)
    return;

  if (mObjects.Empty())
    return;

  Handle primary = mObjects.Front();
  if (primary.IsNull())
    return;

  Quat rotation = Quat::cIdentity;

  MetaTransform* metaTransform = primary.StoredType->HasInherited<MetaTransform>();
  MetaTransformInstance transform = metaTransform->GetInstance(primary);

  if (transform.IsNotNull())
  {
    if (mBasis == GizmoBasis::Local)
      rotation = transform.GetWorldRotation();
    else
      rotation = Quat::cIdentity;
  }

  Vec3 center = Vec3::cZero;

  if (mPivot == GizmoPivot::Primary)
  {
    if (transform.IsNotNull())
      center = transform.GetWorldTranslation();
  }
  else if (mPivot == GizmoPivot::Center)
  {
    Aabb aabb = GetAabb(primary);
    center = aabb.GetCenter();
  }
  else if (mPivot == GizmoPivot::Average)
  {
    uint count = 0;
    forRange (Handle object, mObjects.All())
    {
      if (object.IsNotNull())
      {
        MetaTransform* metaTransform = object.StoredType->HasInherited<MetaTransform>();
        MetaTransformInstance transform = metaTransform->GetInstance(object);
        if (transform.IsNotNull())
        {
          center += transform.GetWorldTranslation();
          ++count;
        }
      }
    }

    if (count != 0)
      center /= float(count);
  }

  mTransform->SetTranslation(center);
  mTransform->SetRotation(rotation);
}

void ObjectTransformGizmo::OnFrameUpdate(UpdateEvent* event)
{
  if (GetOwner()->GetMarkedForDestruction())
    return;

  UpdateGizmoBasis();
}

void ObjectTransformGizmo::SetBasis(GizmoBasis::Enum basis)
{
  mBasis = basis;
  UpdateGizmoBasis();
}

GizmoBasis::Enum ObjectTransformGizmo::GetBasis()
{
  return mBasis;
}

void ObjectTransformGizmo::SetPivot(GizmoPivot::Enum pivot)
{
  mPivot = pivot;
  UpdateGizmoBasis();
}

GizmoPivot::Enum ObjectTransformGizmo::GetPivot()
{
  return mPivot;
}

ZilchDefineType(ObjectTranslateGizmo, builder, type)
{
  ZeroBindComponent();
  ZeroBindInterface(ObjectTransformGizmo);
  ZeroBindDependency(TranslateGizmo);

  ZilchBindFieldProperty(mDuplicateOnCtrlDrag);
}

void ObjectTranslateGizmo::Serialize(Serializer& stream)
{
  ObjectTransformGizmo::Serialize(stream);

  SerializeNameDefault(mDuplicateOnCtrlDrag, true);
}

void ObjectTranslateGizmo::Initialize(CogInitializer& initializer)
{
  ObjectTransformGizmo::Initialize(initializer);

  ConnectThisTo(GetOwner(), Events::TranslateGizmoModified, OnGizmoModified);
}

void ObjectTranslateGizmo::OnMouseDragStart(ViewportMouseEvent* event)
{
  if (event->CtrlPressed && mDuplicateOnCtrlDrag)
  {
    mNewObjects.Clear();

    forRange (Handle object, mObjects.All())
    {
      Cog* cog = object.Get<Cog*>();
      if (cog && CogSerialization::ShouldSave(*cog))
      {
        Cog* copy = cog->Clone();
        mNewObjects.Insert(copy);

        // new object will go directly after the object it was copied from
        mDuplicateCorrectIndex = cog->GetHierarchyIndex() + 1;
        copy->PlaceInHierarchy(mDuplicateCorrectIndex);
      }
      else // currently unnecessary to check for protection on non-cogs
      {
        mNewObjects.Insert(object);
      }
    }

    ClearObjects();

    if (!mNewObjects.Empty())
    {
      forRange (Handle object, mNewObjects.All())
        AddObject(object, false);

      UpdateGizmoBasis();
    }

    // Signal that the objects were duplicated
    Event eventToSend;
    GetOwner()->DispatchEvent(Events::GizmoObjectsDuplicated, &eventToSend);
    GetOwner()->DispatchUp(Events::GizmoObjectsDuplicated, &eventToSend);
  }

  mStartPosition = mTransform->GetWorldTranslation();

  ObjectTransformGizmo::OnMouseDragStart(event);
}

void ObjectTranslateGizmo::OnGizmoModified(TranslateGizmoUpdateEvent* event)
{
  TranslateGizmo* baseGizmo = GetOwner()->has(TranslateGizmo);
  GizmoDrag* gizmoDrag = event->GetGizmo()->has(GizmoDrag);
  bool viewPlaneGizmo = (gizmoDrag->mDragMode == GizmoDragMode::ViewPlane);

  Vec3& movement = event->mGizmoWorldTranslation;

  // Cannot use singularly snapped ProcessedMovement when operating on multiple
  // objects.  They must be snapped individually in their own local space.
  bool multiTransform = mObjectStates.Size() > 1;
  if (multiTransform)
    movement = event->mConstrainedWorldMovement;

  // Special command, possibly modifies 'movement'
  if (viewPlaneGizmo && Keyboard::Instance->KeyIsDown(Keys::V))
    SnapToSurface(event, &movement);

  Vec3 final = movement;

  ObjectTransformGizmoEvent secondEvent(this, GetOwner(), event->GetViewportMouseEvent());

  forRange (ObjectTransformState& objectState, mObjectStates.All())
  {
    Handle target = objectState.MetaObject;
    if (target.IsNull())
      continue;

    MetaTransform* metaTransform = target.StoredType->HasInherited<MetaTransform>();
    if (metaTransform == nullptr)
      continue;

    MetaTransformInstance transform = metaTransform->GetInstance(target);
    if (transform.IsNull())
      continue;

    if (transform.mLocalTranslation == nullptr)
      continue;

    EventDispatcher* dispatcher = nullptr;
    Object* object = target.Get<Object*>();
    if (object != nullptr)
      dispatcher = object->GetDispatcher();

    // Reset before each object as the previous object could have modified it.
    event->mGizmoWorldTranslation = final;

    if (dispatcher != nullptr)
      dispatcher->Dispatch(Events::TranslateGizmoModified, event);

    // User modified?
    movement = event->mGizmoWorldTranslation;

    Mat4 inverseMatrix(Mat4::cIdentity);
    inverseMatrix = transform.GetParentWorldMatrix();
    inverseMatrix.Invert();

    Vec3 localMovement = Math::TransformNormal(inverseMatrix, movement);

    // If snapping multiple objects then snap them one at a time in their local
    // space.
    if (multiTransform)
    {
      objectState.EndTranslation = baseGizmo->TranslateFromDrag(
          gizmoDrag, objectState.StartTranslation, localMovement, objectState.StartRotation);
    }
    else // If single object, then the final movement will already be snapped.
    {
      objectState.EndTranslation = objectState.StartTranslation + localMovement;
    }

    // Reset before each object as the previous object could have modified it.
    secondEvent.mFinalLocalTranslation = objectState.EndTranslation;

    if (dispatcher != nullptr)
      dispatcher->Dispatch(Events::ObjectTranslateGizmoModified, &secondEvent);

    // User modified?
    objectState.EndTranslation = secondEvent.mFinalLocalTranslation;
    transform.SetLocalTranslation(objectState.EndTranslation);

    if (dispatcher != nullptr)
    {
      PropertyEvent propertyEvent(
          transform.mInstance, transform.mLocalTranslation, objectState.StartTranslation, objectState.EndTranslation);

      dispatcher->Dispatch(Events::PropertyModifiedIntermediate, &propertyEvent);
    }
  }
}

static void AddHierarchyIntoSet(Cog* cog, HashSet<Cog*>& set)
{
  set.Insert(cog);

  Hierarchy* hierarchy = cog->has(Hierarchy);
  if (hierarchy == nullptr)
    return;

  forRange (Cog& child, hierarchy->GetChildren())
  {
    AddHierarchyIntoSet(&child, set);
  }
}

void ObjectTranslateGizmo::SnapToSurface(GizmoUpdateEvent* event, Vec3* movementOut)
{
  ViewportMouseEvent* vpEvent = event->GetViewportMouseEvent();
  if (vpEvent == nullptr)
    return;

  // Ignore all selected objects and their children
  HashSet<Cog*> ignoredObjects;
  forRange (ObjectTransformState& objectState, mObjectStates.All())
  {
    Handle target = objectState.MetaObject;
    if (target.IsNull())
      continue;

    //@RYAN: COG_NOT_GENERIC
    Cog* object = (Cog*)target.Dereference();

    if (object)
      AddHierarchyIntoSet(object, ignoredObjects);
  }

  RaycastResultList results(ignoredObjects.Size() + 1);

  Space* space = GetSpace();
  CameraViewport* viewport = vpEvent->GetCameraViewport();
  Vec2 mousePosition = vpEvent->Position;

  CastInfo castInfo(space, viewport->GetCameraCog(), mousePosition);
  Ray ray = vpEvent->mWorldRay;

  // Raycast into both physics and graphics
  Raycaster rayCaster;
  rayCaster.AddProvider(new PhysicsRaycastProvider());
  rayCaster.AddProvider(new GraphicsRaycastProvider());
  rayCaster.RayCast(ray, castInfo, results);

  for (uint i = 0; i < results.mSize; ++i)
  {
    RayCastEntry& entry = results.mEntries[i];
    if (!ignoredObjects.Contains(entry.HitCog))
    {
      *movementOut = ray.GetPoint(entry.T) - event->mInitialGrabPoint;

      // Don't need to search for other hits, just take the first one.
      // First should have the lowest 'T' value, anyway.
      return;
    }
  }
}

ZilchDefineType(ObjectScaleGizmo, builder, type)
{
  ZeroBindComponent();
  ZeroBindInterface(ObjectTransformGizmo);
  ZeroBindDependency(ScaleGizmo);

  ZilchBindFieldProperty(mAffectTranslation);
}

void ObjectScaleGizmo::Serialize(Serializer& stream)
{
  ObjectTransformGizmo::Serialize(stream);

  SerializeNameDefault(mAffectTranslation, true);
  SerializeNameDefault(mAffectScale, true);
}

void ObjectScaleGizmo::Initialize(CogInitializer& initializer)
{
  ObjectTransformGizmo::Initialize(initializer);

  ConnectThisTo(GetOwner(), Events::ScaleGizmoModified, OnGizmoModified);
}

void ObjectScaleGizmo::OnMouseDragStart(ViewportMouseEvent* event)
{
  // mStartPosition = mTransform->GetWorldTranslation();

  Camera* camera = event->GetViewport()->GetCamera();
  if (camera)
    mEyeDirection = -Math::ToMatrix3(camera->mTransform->GetWorldRotation()).BasisZ();
  else
    mEyeDirection = Vec3::cZero;

  ObjectTransformGizmo::OnMouseDragStart(event);
}

void ObjectScaleGizmo::OnGizmoModified(ScaleGizmoUpdateEvent* event)
{
  Vec3 worldMovement = event->mConstrainedWorldMovement;

  GizmoDrag* gizmoDrag = event->GetGizmo()->has(GizmoDrag);
  ScaleGizmo* baseGizmo = GetOwner()->has(ScaleGizmo);

  // The speed at which scaling occurs is determined by how far away
  // the gizmo was grabbed from its center.
  Vec3 gizmoPosition = mTransform->GetWorldTranslation();
  Vec3 grabDirection = (event->mInitialGrabPoint - gizmoPosition);
  float distance = grabDirection.Length();

  Mat3 worldRotationBasis = Math::ToMatrix3(mTransform->GetWorldRotation());
  Mat3 inverseWorld = worldRotationBasis.Inverted();

  // Are multiple objects being transformed?
  bool multiTransform = mObjectStates.Size() > 1;

  ObjectTransformGizmoEvent secondEvent(this, GetOwner(), event->GetViewportMouseEvent());

  forRange (ObjectTransformState& objectState, mObjectStates.All())
  {
    Handle target = objectState.MetaObject;
    if (target.IsNull())
      continue;

    MetaTransform* metaTransform = target.StoredType->HasInherited<MetaTransform>();
    if (metaTransform == nullptr)
      continue;

    MetaTransformInstance transform = metaTransform->GetInstance(target);
    if (transform.IsNull())
      continue;

    if (transform.mLocalScale == nullptr)
      continue;

    Vec3 newScale =
        baseGizmo->ScaleFromDrag(mBasis, gizmoDrag, distance, worldMovement, objectState.StartScale, transform);

    Vec3 newPosition = objectState.EndTranslation;
    if (multiTransform && mAffectTranslation)
    {
      // Update the object's translation by scaling its offset about the basis
      // pivot point.
      Vec3 pivotOffset = objectState.StartWorldTranslation - gizmoPosition;
      if (pivotOffset.Length() > cPivotDistanceThreshold)
      {
        // Transform into local gizmo space.
        Math::Transform(inverseWorld, &pivotOffset);

        Vec3 scaleRatio = newScale / objectState.StartScale;
        pivotOffset *= scaleRatio;

        // Transform back into world space.
        Math::Transform(worldRotationBasis, &pivotOffset);

        newPosition = gizmoPosition + pivotOffset;

        // Put the new position in parent's space.
        newPosition = Math::TransformPoint(transform.GetParentWorldMatrix().Inverted(), newPosition);
      }
    }

    EventDispatcher* dispatcher = nullptr;
    Object* object = target.Get<Object*>();
    if (object != nullptr)
      dispatcher = object->GetDispatcher();

    // Reset before each object as the previous object could have modified it.
    secondEvent.mFinalLocalScale = newScale;
    secondEvent.mFinalLocalTranslation = newPosition;

    if (dispatcher != nullptr)
      dispatcher->Dispatch(Events::ObjectScaleGizmoModified, &secondEvent);

    if (mAffectScale || !multiTransform)
    {
      objectState.EndScale = secondEvent.mFinalLocalScale;
      transform.SetLocalScale(objectState.EndScale);

      if (dispatcher != nullptr)
      {
        PropertyEvent propertyEvent(
            transform.mInstance, transform.mLocalScale, objectState.StartScale, objectState.EndScale);

        dispatcher->Dispatch(Events::PropertyModifiedIntermediate, &propertyEvent);
      }
    }

    if (multiTransform && mAffectTranslation)
    {
      objectState.EndTranslation = secondEvent.mFinalLocalTranslation;
      transform.SetLocalTranslation(objectState.EndTranslation);

      if (dispatcher != nullptr)
      {
        PropertyEvent propertyEvent(
            transform.mInstance, transform.mLocalTranslation, objectState.StartTranslation, objectState.EndTranslation);

        dispatcher->Dispatch(Events::PropertyModifiedIntermediate, &propertyEvent);
      }
    }
  }
}

ZilchDefineType(ObjectRotateGizmo, builder, type)
{
  ZeroBindComponent();
  ZeroBindInterface(ObjectTransformGizmo);
  ZeroBindDependency(RotateGizmo);

  ZilchBindFieldProperty(mAffectTranslation);
}

void ObjectRotateGizmo::Serialize(Serializer& stream)
{
  ObjectTransformGizmo::Serialize(stream);

  SerializeNameDefault(mAffectTranslation, true);
  SerializeNameDefault(mAffectRotation, true);
}

void ObjectRotateGizmo::Initialize(CogInitializer& initializer)
{
  ObjectTransformGizmo::Initialize(initializer);

  ConnectThisTo(GetOwner(), Events::RotateGizmoModified, OnGizmoModified);

  mRotateGizmo = true;
}

void ObjectRotateGizmo::OnMouseDragStart(ViewportMouseEvent* event)
{
  ObjectTransformGizmo::OnMouseDragStart(event);
}

void ObjectRotateGizmo::OnGizmoModified(RotateGizmoUpdateEvent* event)
{
  Vec3 rotationAxis = event->mGizmoWorldRotationAxis;

  Vec3 gizmoPos = mTransform->GetWorldTranslation();
  RotateGizmo* baseGizmo = GetOwner()->has(RotateGizmo);

  float deltaOnAxis = event->mGizmoRotation;

  // If snapping is on, but there's no change in the snap-angle - then there's
  // no point to allow object updating.
  if (baseGizmo->GetSnapping() && deltaOnAxis == 0.0f)
    return;

  // If we're transforming multiple objects, we will have to translate them
  bool multiTransform = mObjectStates.Size() > 1;

  ObjectTransformGizmoEvent secondEvent(this, GetOwner(), event->GetViewportMouseEvent());

  // Apply the rotation transform to each selected object
  forRange (ObjectTransformState& objectState, mObjectStates.All())
  {
    Handle target = objectState.MetaObject;
    if (target.IsNull())
      continue;

    MetaTransform* metaTransform = target.StoredType->HasInherited<MetaTransform>();
    if (metaTransform == nullptr)
      continue;

    MetaTransformInstance transform = metaTransform->GetInstance(target);
    if (transform.IsNull())
      continue;

    if (transform.mLocalRotation == nullptr)
      continue;

    EventDispatcher* dispatcher = nullptr;
    Object* object = target.Get<Object*>();
    if (object != nullptr)
      dispatcher = object->GetDispatcher();

    // Reset before each object as the previous object could have modified it.
    event->mGizmoRotation = deltaOnAxis;
    event->mGizmoWorldRotationAxis = rotationAxis;

    if (dispatcher != nullptr)
      dispatcher->Dispatch(Events::RotateGizmoModified, event);

    // User modified?
    float finalDelta = event->mGizmoRotation;
    Vec3 finalAxis = event->mGizmoWorldRotationAxis;

    // Save the old transform so that deltas can be properly applied to in-world
    // objects
    Mat4 oldMat = transform.GetParentWorldMatrix();
    Mat4 inverseMatrix(Mat4::cIdentity);
    inverseMatrix = oldMat.Inverted();

    Quat newRotation = objectState.EndRotation;
    if (mAffectRotation || !multiTransform)
    {
      Vec3 localAxis = Math::TransformNormal(inverseMatrix, finalAxis).AttemptNormalized();

      // Construct a local rotation
      Quat localRotation;
      Math::ToQuaternion(localAxis, deltaOnAxis, &localRotation);

      // Add the current rotation to the starting rotation
      newRotation = localRotation * transform.GetLocalRotation();

      // Normalize to prevent rounding errors
      newRotation.Normalize();
    }

    Vec3 newLocalPosition = objectState.EndTranslation;
    if (multiTransform && mAffectTranslation)
    {
      Vec3 pivot = transform.GetWorldTranslation() - gizmoPos;
      if (pivot.Length() > cPivotDistanceThreshold)
      {
        Quat snappedRotation = Math::ToQuaternion(finalAxis, deltaOnAxis);
        Vec3 rotatedPivot = Math::Multiply(snappedRotation, pivot);
        Vec3 newPosition = gizmoPos + rotatedPivot;

        newLocalPosition = Math::TransformPoint(inverseMatrix, newPosition);
      }
    }

    // Reset before each object as the previous object could have modified it.
    secondEvent.mFinalLocalRotation = newRotation;
    secondEvent.mFinalLocalTranslation = newLocalPosition;

    if (dispatcher != nullptr)
      dispatcher->Dispatch(Events::ObjectRotateGizmoModified, &secondEvent);

    if (mAffectRotation || !multiTransform)
    {
      objectState.EndRotation = secondEvent.mFinalLocalRotation;
      transform.SetLocalRotation(objectState.EndRotation);

      if (dispatcher != nullptr)
      {
        PropertyEvent propertyEvent(
            transform.mInstance, transform.mLocalRotation, objectState.StartRotation, objectState.EndRotation);

        dispatcher->Dispatch(Events::PropertyModifiedIntermediate, &propertyEvent);
      }
    }

    if (multiTransform && mAffectTranslation)
    {
      objectState.EndTranslation = secondEvent.mFinalLocalTranslation;
      transform.SetLocalTranslation(objectState.EndTranslation);

      if (dispatcher != nullptr)
      {
        PropertyEvent propertyEvent(
            transform.mInstance, transform.mLocalTranslation, objectState.StartTranslation, objectState.EndTranslation);

        dispatcher->Dispatch(Events::PropertyModifiedIntermediate, &propertyEvent);
      }
    }
  }
}

} // namespace Zero
