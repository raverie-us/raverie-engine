///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Joshua Claeys
/// Copyright 2010-2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//////////////Math Helpers/////////////
float ToolGetViewScale(Camera* camera, Vec3Param location)
{
  float viewDistance = Debug::GetViewDistance(location, camera->GetWorldTranslation(), camera->GetWorldDirection());
  bool orthographic = camera->mPerspectiveMode == PerspectiveMode::Orthographic;
  return Debug::GetViewScale(viewDistance, camera->mFieldOfView, camera->mSize, orthographic);
}

Vec3 NormalToLocal(Vec3Param vector0, Transform* transform)
{
  if(transform->TransformParent)
  {
    //Transform into local space
    Mat4 parentWorldInverse = transform->TransformParent->GetWorldMatrix();
    parentWorldInverse.Invert();
    return TransformNormal(parentWorldInverse, vector0);//.Normalized();
  }
  return vector0;
}

void SetLocalPositionFromWorld(TransformingObject& object,
                               Vec3Param worldTranslation, Transform* transform)
{
  //if the object has a parent and is not in world
  //then transform the world translation into local space
  if(transform->TransformParent && !transform->GetInWorld())
  {
    Mat4 parentWorldInverse = transform->TransformParent->GetWorldMatrix();
    parentWorldInverse.Invert();
    transform->SetTranslation(TransformPoint(parentWorldInverse,
                                            worldTranslation));
  }
  else
  {
    transform->SetTranslation(worldTranslation);
  }

  object.EndTranslation = transform->GetTranslation();
}


Vec3 ProjectOntoPlane(Vec2Param mousePosition, Vec3Param eyeDir, Viewport* viewport,
                      TransformBasis& basis, GizmoAxis::Type selectedAxis,
                      GizmoAxis::Type movementAxis)
{
  Ray worldRay = viewport->ScreenToWorldRay(mousePosition);
  Intersection::IntersectionPoint intersection;

  if(selectedAxis == GizmoAxis::ViewAxis)
  {
    //Project onto the view plane
    RayPlane(worldRay.Start, worldRay.Direction.Normalized(), eyeDir,
      Dot(eyeDir, basis.Translation), &intersection);
    return intersection.Points[0];
  }
  else
  {
    //Project onto the selected axis plane
    RayPlane(worldRay.Start, worldRay.Direction.Normalized(), basis.Axis[movementAxis],
      Dot(basis.Axis[movementAxis], basis.Translation),
      &intersection);
    return intersection.Points[0];
  }
}

//------------------------------------------------------------------------ Gizmo
ZilchDefineType(TransformTool, builder, type)
{
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDependency(MouseCapture);
  ZeroBindTag(Tags::Tool);
}

//******************************************************************************
TransformTool::TransformTool()
{
  //Options
  mGrabMode = GizmoGrab::Hold;
  mSelectedAxis = GizmoAxis::None;
  mGizmoMode = GizmoMode::Inactive;
  mBasis = GizmoBasis::World;
  mValidSelection = false;
  mAffectTranslation = true;
  mPivot = GizmoPivot::Average;
  mSnapping = false;
  mSnapAngle = 15.0f;
  mSnapDistance = 0.5f;
  mSizeBoxCollider = true;
  mActiveBasis.Axis[0] = Vec3::cXAxis;
  mActiveBasis.Axis[1] = Vec3::cYAxis;
  mActiveBasis.Axis[2] = Vec3::cZAxis;
  mPlaneAxis = 0;
}

//******************************************************************************
void TransformTool::Serialize(Serializer& stream)
{
  
}

//******************************************************************************
void TransformTool::Initialize(CogInitializer& initializer)
{
  ConnectThisTo(GetOwner(), Events::ToolActivate, OnToolActivate);
  ConnectThisTo(GetOwner(), Events::ToolDeactivate, OnToolDeactivate);
  ConnectThisTo(GetOwner(), Events::LeftMouseDown, OnLeftMouseDown);
  ConnectThisTo(GetOwner(), Events::LeftMouseUp, OnLeftMouseUp);
  ConnectThisTo(GetOwner(), Events::MouseMove, OnMouseMove);
  ConnectThisTo(GetOwner(), Events::KeyDown, OnKeyDown);

  /// Drag Events
  ConnectThisTo(GetOwner(), Events::MouseDragStart, OnMouseDragStart);
  ConnectThisTo(GetOwner(), Events::MouseDragEnd, OnMouseDragEnd);
}

//******************************************************************************
void TransformTool::OnToolActivate(Event*)
{
  mGizmoMode = GizmoMode::Active;
}

//******************************************************************************
void TransformTool::OnToolDeactivate(Event*)
{
  mGizmoMode = GizmoMode::Inactive;
}

//******************************************************************************
void TransformTool::ToggleCoordinateMode()
{
  mBasis = (GizmoBasis::Enum)!mBasis;
}

//******************************************************************************
void TransformTool::OnLeftMouseDown(ViewportMouseEvent* e)
{
  if(mGrabMode == GizmoGrab::Hold)
    e->Handled = CheckMouseManipulation(e);
  else
    e->Handled = OnGizmo();
}

//******************************************************************************
void TransformTool::OnLeftMouseUp(ViewportMouseEvent* e)
{
  if(mGrabMode == GizmoGrab::Toggle)
    e->Handled = CheckMouseManipulation(e);
}

//******************************************************************************
void TransformTool::OnMouseMove(ViewportMouseEvent* e)
{
  if(Active())
    TestMouseMove(e);
}

//******************************************************************************
void TransformTool::OnKeyDown(KeyboardEvent* e)
{
  if(!e->CtrlPressed && e->Key == Keys::X)
  {
    ToggleCoordinateMode();
    e->Handled = true;
  }
}

//******************************************************************************
void TransformTool::OnMouseDragStart(ViewportMouseEvent* e)
{
  if(!mValidSelection)
    return;

  MetaSelection* selection = Z::gEditor->GetSelection();

  Array<Cog*> cogs;
  FilterChildrenAndProtected(cogs, selection);
  mTransformingObjects.Clear();

  Array<Cog*>::range r = cogs.All();
  while(!r.Empty())
  {
    Cog* target = r.Front();

    //Store the staring values for transformations
    if(Transform* tx = target->has(Transform))
    {
      TransformingObject data;
      data.ObjectId = target;
      data.StartWorldTranslation = tx->GetWorldTranslation();
      data.StartTranslation = tx->GetTranslation();
      data.StartRotation = tx->GetRotation();
      data.StartScale = tx->GetScale();
      data.EndTranslation = data.StartTranslation;
      data.EndRotation = data.StartRotation;
      data.EndScale = data.StartScale;
      data.WorldAabb = GetAabb(target);

      if(Area* area = target->has(Area))
      {
        data.StartSize = area->mSize;
        data.EndSize = area->mSize;
      }
      else
      {
        data.StartSize = Vec2::cZero;
        data.EndSize = Vec2::cZero;
      }

      mTransformingObjects.PushBack(data);
    }
    r.PopFront();
  }

  //Store the starting basis for moving the gizmo
  mStartingBasis = mActiveBasis;

  //Gizmo is now moving
  mGizmoMode = GizmoMode::Transforming;
}

//******************************************************************************
void TransformTool::OnMouseDragEnd(Event*)
{
  BoundType* transformType = ZilchTypeId(Transform);
  Property* translationProperty = transformType->GetProperty("Translation");
  Property* rotationProperty = transformType->GetProperty("Rotation");
  Property* scaleProperty = transformType->GetProperty("Scale");

  OperationQueue* queue = Z::gEditor->GetOperationQueue();

  queue->BeginBatch();
  queue->SetActiveBatchName("TransformTool_OnMouseDragEnd");

  Array<TransformingObject>::range r = mTransformingObjects.All();
  while(!r.Empty())
  {
    Cog* target = r.Front().ObjectId;
    if(target)
    {
      TransformingObject& transformObject = r.Front();
      Transform* transform = target->has(Transform);

      transform->GetSpace()->MarkModified();

      queue->BeginBatch();
      queue->SetActiveBatchName("TransformTool");

      // Send the final GizmoFinish transform update
      uint flag = 0;

      // When scaling or rotating multiple objects translation may changed so just check
      if((transformObject.StartTranslation - transformObject.EndTranslation).LengthSq()!=0.0f)
      {
        flag |= TransformUpdateFlags::Translation;
        transform->SetLocalTranslationInternal(transformObject.StartTranslation);
        PropertyPath propertyPath(transform, translationProperty);
        ChangeAndQueueProperty(queue,
                            target,
                            propertyPath,
                            transformObject.EndTranslation);
      }

      if(mGizmoType == GizmoType::Rotate)
      {
        flag |= TransformUpdateFlags::Rotation;
        transform->SetLocalRotationInternal(transformObject.StartRotation);
        PropertyPath propertyPath(transform, rotationProperty);
        ChangeAndQueueProperty(queue,
                               target,
                               propertyPath,
                               transformObject.EndRotation);
      }

      if((transformObject.StartScale - transformObject.EndScale).LengthSq()!=0.0f)
      {
        flag |= TransformUpdateFlags::Scale;
        transform->SetLocalScaleInternal(transformObject.StartScale);
        PropertyPath propertyPath(transform, scaleProperty);
        ChangeAndQueueProperty(queue,
                            target,
                            propertyPath,
                            transformObject.EndScale);
      }

      if((transformObject.StartSize - transformObject.EndSize).LengthSq()!=0.0f)
      {
        Area* area = target->has(Area);
        if(area)
        {
          Property* sizeProperty = ZilchVirtualTypeId(area)->GetProperty("Size");
          area->SetSize(transformObject.StartSize);
          PropertyPath areaPath(area, sizeProperty);
          ChangeAndQueueProperty(queue, target, areaPath,
                                 transformObject.EndSize);

          if(mSizeBoxCollider)
          {
            BoxCollider* collider = target->has(BoxCollider);

            if(collider)
            {
              Vec3 startOffset = collider->GetOffset();
              Vec3 startSize = collider->GetSize();
              Vec3 endOffset = Vec3( OffsetOfOffset(area->GetOrigin(), Location::Center) * area->GetSize(), 0);

              Property* offsetProperty = ZilchVirtualTypeId(collider)->GetProperty("Offset");
              collider->SetOffset(startOffset);

              PropertyPath offsetPath(collider, offsetProperty);
              ChangeAndQueueProperty(queue, target, offsetPath,
                                     endOffset);

              Vec3 endSize = Vec3(transformObject.EndSize.x, transformObject.EndSize.y, startSize.z);

              Property* sizeProperty = ZilchVirtualTypeId(collider)->GetProperty("Size");
              collider->SetSize(startSize);
              PropertyPath sizePath(collider, offsetProperty);
              ChangeAndQueueProperty(queue, target, sizePath, endSize);
            }

          }
        }
      }

      queue->EndBatch();

      transform->Update(flag | TransformUpdateFlags::GizmoFinish);
    }
    r.PopFront();
  }

  queue->EndBatch();


  mGizmoMode = GizmoMode::Active;
}

//******************************************************************************
bool TransformTool::OnGizmo()
{
  MetaSelection* selection = Z::gEditor->GetSelection();
  return selection->Count() > 0 && Active() && IsAxisSelected();
}

//******************************************************************************
bool TransformTool::CheckMouseManipulation(ViewportMouseEvent* e)
{
  if(OnGizmo())
  {
    MouseCapture* capture = GetOwner()->has(MouseCapture);
    capture->Capture(e);
    return true;
  }
  return false;
}

//******************************************************************************
void TransformTool::RecomputeTransformBasis()
{
  //Do not ComputeTransformBasis when Transforming
  //Or the basis will move around while transforming
  if(mGizmoMode == GizmoMode::Transforming)
    return;

  MetaSelection* selection = Z::gEditor->GetSelection();

  //Select basis (rotation)
  mActiveBasis.Axis[0] = Vec3::cXAxis;
  mActiveBasis.Axis[1] = Vec3::cYAxis;
  mActiveBasis.Axis[2] = Vec3::cZAxis;

  //Use the primary object for rotation
  //(averaging rotation is not really valid)
  if(Cog* primary = selection->GetPrimaryAs<Cog>())
  {
    if(Transform* tx = primary->has(Transform))
    {
      if(mBasis == GizmoBasis::Local)
      {
        mActiveBasis.Translation = tx->GetWorldTranslation();
        Mat3 m = Math::ToMatrix3(tx->GetWorldRotation());
        mActiveBasis.Axis[0] = m.BasisX();
        mActiveBasis.Axis[1] = m.BasisY();
        mActiveBasis.Axis[2] = m.BasisZ();
      }
    }
    else
    {
      mValidSelection = false;
    }
  }

  //Center of current object
  if(mPivot == GizmoPivot::Center)
  {
    Aabb aabb = GetAabb(selection->GetPrimaryAs<Cog>());
    mActiveBasis.Translation = aabb.GetCenter();
  }

  //Select translation of basis by averaging positions.
  //(Could also use middle of bounding volume).
  if(mPivot == GizmoPivot::Average)
  {
    Vec3 center = Vec3::cZero;
    uint transformCount = 0;

    forRange(Cog* target, selection->AllOfType<Cog>())
    {
      if(Transform* tx = target->has(Transform))
      {
        center += tx->GetWorldTranslation();
        ++transformCount;
      }
    }

    if(transformCount > 0)
    {
      float invCount = 1.0f / float(transformCount);
      center *= invCount;
      mActiveBasis.Translation = center;

      //Selection is a valid.
      mValidSelection = true;
    }
    else
    {
      //No objects with transform
      mActiveBasis.Translation = Vec3::cZero;
      mValidSelection = false;
    }
  }
}

//******************************************************************************
bool TransformTool::GetSnapping()
{
  return mSnapping;
}

//******************************************************************************
float TransformTool::GetSnapAngle()
{
  return mSnapAngle;
}

//******************************************************************************
float TransformTool::GetSnapDistance()
{
  return mSnapDistance;
}

//******************************************************************************
void TransformTool::SetSnapAngle(float angle)
{
  if (angle < 0.001f)
    angle = 0.001f;
  mSnapAngle = angle;
}

//******************************************************************************
void TransformTool::SetSnapDistance(float distance)
{
  if (distance < 0.001f)
    distance = 0.001f;
  mSnapDistance = distance;
}

//--------------------------------------------------------------- Translate Tool
const float cAxisSize = 2.8f;
const float cBoxSize = 0.15f * cAxisSize;
const float cHeadSize = 0.1f * cAxisSize;
const float cAxisSelectRadius = 0.25f;

//******************************************************************************
ZilchDefineType(TranslateTool, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);

  ZilchBindFieldProperty(mBasis);
  ZilchBindFieldProperty(mPivot);

  ZilchBindFieldProperty(mSnapping);
  ZilchBindGetterSetterProperty(SnapDistance);
  ZilchBindFieldProperty(mSnapToXPlane);
  ZilchBindFieldProperty(mSnapToYPlane);
  ZilchBindFieldProperty(mSnapToZPlane);

  ZilchBindFieldProperty(mGrabMode);
}
//******************************************************************************
TranslateTool::TranslateTool()
{
  mGizmoType = GizmoType::Translate;
  mDuplicateOnCtrlDrag = true;
}

//******************************************************************************
void TranslateTool::Serialize(Serializer& stream)
{
  TransformTool::Serialize(stream);
  SerializeNameDefault(mSnapDistance, 0.5f);
  SerializeNameDefault(mSnapToXPlane, false);
  SerializeNameDefault(mSnapToYPlane, false);
  SerializeNameDefault(mSnapToZPlane, false);
}

//******************************************************************************
void TranslateTool::Initialize(CogInitializer& initializer)
{
  TransformTool::Initialize(initializer);
  ConnectThisTo(GetOwner(), Events::MouseDragMove, OnMouseDragMove);
  ConnectThisTo(GetOwner(), Events::ToolDraw, OnToolDraw);
}

//******************************************************************************
void TranslateTool::TestMouseMove(ViewportMouseEvent* e)
{
  if(!Active())
    return;

  Camera* camera = e->GetViewport()->GetCamera();
  if (camera == nullptr)
    return;

  //Scale the gizmo by the view so it always appears the same size
  float viewScale = ToolGetViewScale(camera, mActiveBasis.Translation);

  Ray mouseRay = e->mWorldRay;

  //Test with the move on view plane in the center of the gizmo
  Vec3 min = mActiveBasis.Translation -
             Vec3(cBoxSize, cBoxSize, cBoxSize) * viewScale;
  Vec3 max = mActiveBasis.Translation +
             Vec3(cBoxSize, cBoxSize, cBoxSize) * viewScale;
  Intersection::Type type = Intersection::RayAabb(mouseRay.Start, mouseRay.Direction, min, max);

  if(type != Intersection::None)
  {
    //Selected the move on view box.
    mSelectedAxis = GizmoAxis::ViewAxis;
    return;
  }

  float radius = viewScale * cAxisSelectRadius;
  //Check each axis for the mouse
  for(uint i = 0; i < 3; ++i)
  {
    Vec3 end = mActiveBasis.Translation + mActiveBasis.Axis[i] * cAxisSize * viewScale;
    type = Intersection::RayCapsule(mouseRay.Start, mouseRay.Direction, mActiveBasis.Translation,
                                    end, radius);
    if(type != Intersection::None)
    {
      mSelectedAxis = (GizmoAxis::Enum)i;
      return;
    }
  }

  //Not on an axis
  mSelectedAxis = GizmoAxis::None;
}

//******************************************************************************
void TranslateTool::OnMouseDragStart(ViewportMouseEvent* e)
{
  Camera* camera = e->GetViewport()->GetCamera();
  if (camera == nullptr)
    return;

  mEyeDirection = -Math::ToMatrix3(camera->mTransform->GetWorldRotation()).BasisZ();

  bool ctrlIsDown = Keyboard::Instance->KeyIsDown(Keys::Control);

  Viewport* viewport = e->GetViewport();

  if(mDuplicateOnCtrlDrag && ctrlIsDown)
    DuplicateSelection(Z::gEditor, viewport->GetTargetSpace());

  TransformTool::OnMouseDragStart(e);

  if(mSelectedAxis != GizmoAxis::ViewAxis)
  {
    uint selectedAxis = mSelectedAxis;
    mPlaneAxis = (selectedAxis + 1) % 3;
    uint otherAxis = (selectedAxis + 2) % 3;

    // Choose the best plane (the most visible or biggest absolute dot product
    // with the eye) for movement projection
    float axisA = Dot(mEyeDirection, mActiveBasis.Axis[mPlaneAxis]);
    float axisB = Dot(mEyeDirection, mActiveBasis.Axis[otherAxis]);

    if(fabs(axisA) < fabs(axisB))
      Swap(mPlaneAxis, otherAxis);
  }

  mGrabPoint = ProjectOntoPlane(e->Position, mEyeDirection, viewport,
                                mActiveBasis, mSelectedAxis, mPlaneAxis);
}

//******************************************************************************
void AddHierarchyIntoSet(Cog* cog, HashSet<Cog*>& set)
{
  set.Insert(cog);
  if(Hierarchy* hierarchy = cog->has(Hierarchy))
  {
    forRange(Cog& child, hierarchy->GetChildren())
    {
      AddHierarchyIntoSet(&child, set);
    }
  }
}

//******************************************************************************
void TranslateTool::OnMouseDragMove(ViewportMouseEvent* e)
{
  Viewport* viewport = e->GetViewport();
  Vec2 newMousePosition = e->Position;

  if(mSelectedAxis == GizmoAxis::ViewAxis && Keyboard::Instance->KeyIsDown(Keys::V))
  {
    Space* targetSpace = viewport->GetTargetSpace();

    // Ray cast into both physics and graphics
    Raycaster rayCaster;
    rayCaster.AddProvider(new PhysicsRaycastProvider());
    rayCaster.AddProvider(new GraphicsRaycastProvider());

    // We want to ignore all selected objects and their children
    HashSet<Cog*> ignoredObjects;
    for(uint i = 0; i < mTransformingObjects.Size(); ++i)
    {
      TransformingObject& object = mTransformingObjects[i];
      if(Cog* cog = object.ObjectId)
        AddHierarchyIntoSet(cog, ignoredObjects);
    }

    RaycastResultList results(ignoredObjects.Size() + 1);

    CastInfo castInfo(targetSpace, viewport->mViewportInterface->GetCameraCog(), newMousePosition);
    Ray ray = viewport->ScreenToWorldRay(newMousePosition);

    rayCaster.RayCast(ray, castInfo, results);

    for(uint i = 0; i < results.mSize; ++i)
    {
      RayCastEntry& entry = results.mEntries[i];
      if(!ignoredObjects.Contains(entry.HitCog))
      {
        Vec3 pos = ray.GetPoint(entry.T);
        this->ApplyWorldMovement(pos - mGrabPoint);
        e->Handled = true;
      }
    }
  }

  //Get the new position of the mouse projected onto the movement plane
  Vec3 newPosition = ProjectOntoPlane(newMousePosition, mEyeDirection, viewport,
                                      mActiveBasis, mSelectedAxis, mPlaneAxis);
  Vec3 movementInPlane = newPosition - mGrabPoint;

  //Project to the selected axis if not in plane mode
  if(mSelectedAxis != GizmoAxis::ViewAxis)
  {
    movementInPlane = Math::ProjectOnVector(movementInPlane, mActiveBasis.Axis[mSelectedAxis]);
  }

  //Axis snapping.
  if(mSnapToXPlane)
    movementInPlane.x = 0;

  if(mSnapToYPlane)
    movementInPlane.y = 0;

  if(mSnapToZPlane)
    movementInPlane.z = 0;

  //Process the plane movement for scale or translate
  this->ApplyWorldMovement(movementInPlane);

  e->Handled = true;
}

//******************************************************************************
void TranslateTool::OnToolDraw(Event*)
{
  if(!Active())
    return;

  RecomputeTransformBasis();

  if(mValidSelection)
  {
    ByteColor colors[3];
    colors[0] = Color::Red;
    colors[1] = Color::Green;
    colors[2] = Color::Blue;

    ByteColor boxColor = Color::LightYellow;

    if(mSelectedAxis == GizmoAxis::ViewAxis)
      boxColor = Color::Yellow;

    //Draw transform view plane box
    gDebugDraw->Add(Debug::Box(mActiveBasis.Translation, Vec2(cBoxSize, cBoxSize)).ViewScaled(true).OnTop(true).Color(boxColor).ViewAligned(true));

    // Don't draw each axis if the V key is down
    if(!Keyboard::Instance->KeyIsDown(Keys::V))
    {
      //Draw each axis
      for(uint i = 0; i < 3; ++i)
      {
        ByteColor color = colors[i];

        if(i == mSelectedAxis)
          color = Color::Yellow;

        Vec3 end = mActiveBasis.Translation + mActiveBasis.Axis[i] * cAxisSize;

        //Draw an arrow for translate and a box for scale
        if (mGizmoType == GizmoType::Translate)
          gDebugDraw->Add(Debug::Line(mActiveBasis.Translation, end).HeadSize(cHeadSize).Color(color).OnTop(true).ViewScaled(true));
        else
          gDebugDraw->Add(Debug::Line(mActiveBasis.Translation, end).HeadSize(cHeadSize*0.5f).Color(color).OnTop(true).ViewScaled(true).BoxHeads(true));
      }
    }
  }
}

//******************************************************************************
void TranslateTool::ApplyWorldMovement(Vec3 worldMovement)
{
  forRange(TransformingObject& transformObject, mTransformingObjects.All( ))
  {
    Handle& meta = transformObject.ObjectMeta;
    Cog* target = transformObject.ObjectId;

    if(target)
    {
      Transform* tf = target->has(Transform);

      //save the old transform so that we can properly apply the deltas to in-world objects
      Mat4 oldMat = tf->GetWorldMatrix();

      Vec3 localMovement = NormalToLocal(worldMovement, tf);
      transformObject.EndTranslation = transformObject.StartTranslation +
                                       localMovement;

      //Snap the translation if snapping enabled
      if(mSnapping)
      {
        transformObject.EndTranslation = Snap(transformObject.EndTranslation,
                                              mSnapDistance);
      }

      // Set the translation
      tf->SetLocalTranslationInternal(transformObject.EndTranslation);
      // Send the transform update event with the old matrix so a delta can be computed
      tf->Update(TransformUpdateFlags::Translation | TransformUpdateFlags::GizmoIncremental, oldMat);
    }

  }

  //Update the display of the gizmo
  mActiveBasis.Translation = mStartingBasis.Translation + worldMovement;
}

//------------------------------------------------------------- Manipulator Tool
DeclareBitField4(DirectionFlags, Left, Right, Top, Bottom);

const uint MiddlePoint = 8;
const uint PointCount = 9;

const uint GrabPoints[] =
{
  DirectionFlags::Left,
  DirectionFlags::Left | DirectionFlags::Top,
  DirectionFlags::Top,
  DirectionFlags::Top | DirectionFlags::Right,
  DirectionFlags::Right,
  DirectionFlags::Right| DirectionFlags::Bottom,
  DirectionFlags::Bottom,
  DirectionFlags::Bottom | DirectionFlags::Left,
  0,
};

const float GripSize = 0.05f;

//******************************************************************************
ZilchDefineType(ManipulatorTool, builder, type)
{
  ZeroBindComponent();
  ZeroBindTag(Tags::Tool);
  ZilchBindFieldProperty(mSizeBoxCollider);
  ZilchBindFieldProperty(mScalingMode);
  ZilchBindFieldProperty(mSnapping);
  ZilchBindGetterSetterProperty(SnapDistance);
}

//******************************************************************************
ManipulatorTool::ManipulatorTool()
{
  mSelectedPoint = -1;
  mScalingMode = false;
}

//******************************************************************************
void ManipulatorTool::Serialize(Serializer& stream)
{
  TranslateTool::Serialize(stream);
  SerializeEnumNameDefault(GizmoBasis, mBasis, GizmoBasis::World);
  SerializeEnumNameDefault(GizmoPivot, mPivot, GizmoPivot::Average);
  SerializeNameDefault(mAffectTranslation, true);
  SerializeNameDefault(mSnapping, false);
}

//******************************************************************************
void ManipulatorTool::TestMouseMove(ViewportMouseEvent* e)
{
  MetaSelection* selection = Z::gEditor->mSelection;
  Cog* primary = selection->GetPrimaryAs<Cog>();
  Aabb aabb = mActiveAabb;
  Vec3 center = aabb.GetCenter();
  Vec3 size = aabb.GetHalfExtents();
  Quat rot = Quat::cIdentity;

  mSelectedPoint = -1;
  mSelectedAxis = GizmoAxis::None;

  for(uint i=0;i<PointCount;++i)
  {
    Vec3 point = center;

    if( GrabPoints[i] & DirectionFlags::Left )
      point -= Vec3(size.x, 0, 0);

    if( GrabPoints[i] & DirectionFlags::Top )
      point += Vec3(0, size.y, 0);

    if( GrabPoints[i] & DirectionFlags::Right )
      point += Vec3(size.x, 0, 0);

    if( GrabPoints[i] & DirectionFlags::Bottom )
      point -= Vec3(0, size.y, 0);
    
    Camera* camera = e->GetViewport()->GetCamera();
    float scaledGripSize = ToolGetViewScale(camera, point) * 0.1f;

    Vec3 boxSize(scaledGripSize, scaledGripSize, scaledGripSize);

    //if(i == MiddlePoint)
    //  boxSize = size;

    Mat3 noRot = Mat3::cIdentity;
    Ray mouseRay = e->mWorldRay;
    Intersection::Type result = Intersection::RayObb(mouseRay.Start, mouseRay.Direction, point, boxSize, noRot);

    if(result != Intersection::None)
    {
      mSelectedPoint = i;
      mValidSelection = true;
      mSelectedAxis = GizmoAxis::ZAxis;
      mPlaneAxis = GizmoAxis::ZAxis;
      mActiveBasis = TransformBasis();
      return;
    }
  }
}

//******************************************************************************
void ManipulatorTool::OnMouseDragStart(ViewportMouseEvent* e)
{
  mStartAabb = mActiveAabb;
  mMouseDragStart = e->Position;
  TransformTool::OnMouseDragStart(e);
}

//******************************************************************************
void ManipulatorTool::OnMouseDragMove(ViewportMouseEvent* e)
{
  Viewport* viewport = e->GetViewport();
  // Compute plane movement
  Vec3 oldPosition = ProjectOntoPlane(mMouseDragStart, mEyeDirection, viewport, mActiveBasis, mSelectedAxis, mPlaneAxis);
  Vec3 newPosition = ProjectOntoPlane(e->Position, mEyeDirection, viewport, mActiveBasis, mSelectedAxis, mPlaneAxis);

  // All movement computed relative to bottom left of
  // of staring aabb
  Vec3 worldOrigin = mStartAabb.mMin;
  Vec3 movementInPlane = newPosition - oldPosition;
  Vec3 orginMovement = Vec3(0,0,0);

  if(mSnapping)
    movementInPlane = Snap(movementInPlane, mSnapDistance);

  // Determine the movement of the origin
  // and update the movement in plane to be relative
  if(mSelectedPoint != MiddlePoint)
  {
    uint grabFlags = GrabPoints[mSelectedPoint];

    if( !( grabFlags & (DirectionFlags::Left | DirectionFlags::Right) ) )
      movementInPlane[0] = 0.0f;

    if( !(grabFlags & (DirectionFlags::Top | DirectionFlags::Bottom) ) )
      movementInPlane[1] = 0.0f;

    if( grabFlags & DirectionFlags::Left )
    {
      orginMovement.x = movementInPlane.x;
      movementInPlane.x = -movementInPlane.x;
    }

    if( grabFlags & DirectionFlags::Bottom )
    {
      orginMovement.y = movementInPlane.y;
      movementInPlane.y = -movementInPlane.y;
    }
  }
  else
  {
    // Middle mode just translate
    orginMovement = movementInPlane;
    movementInPlane = Vec3(0,0,0);
  }

  // Compute new Aabb
  Vec3 startExtents = mStartAabb.GetExtents();
  Vec3 startCenter = mStartAabb.GetCenter();
  Vec3 newExtents = startExtents + movementInPlane;
  Vec3 scaleChange = newExtents / startExtents;
  Vec3 aabbOffset = startCenter - worldOrigin;
  Vec3 newCenter = worldOrigin + aabbOffset * scaleChange + orginMovement;

  mActiveAabb.SetCenter(newCenter);
  mActiveAabb.SetExtents(newExtents);

  Array<TransformingObject>::range r = mTransformingObjects.All();
  for(;!r.Empty();r.PopFront())
  {
    Cog* target = r.Front().ObjectId;
    if(target)
    {
      TransformingObject& transformObject = r.Front();
      Transform* tf = target->has(Transform);
      Area* area = target->has(Area);

      Vec3 worldOffset = transformObject.StartWorldTranslation - worldOrigin;
      Vec3 newTranslation = worldOrigin + worldOffset * scaleChange + orginMovement;

      // Change to local
      if(tf->TransformParent)
        newTranslation = tf->TransformParent->TransformPointInverse(newTranslation);

      transformObject.EndTranslation = newTranslation;

      if(area)
      {
        // Change Size
        transformObject.EndSize = transformObject.StartSize * Vec2(scaleChange.x, scaleChange.y);


        area->SetSize(transformObject.EndSize);
      }
      else
      {
        // Change Scale
        transformObject.EndScale = transformObject.StartScale * scaleChange;
        tf->SetScale(transformObject.EndScale);
      }

      tf->SetTranslation(transformObject.EndTranslation);
    }
  }

  e->Handled = true;
}

//******************************************************************************
void ManipulatorTool::OnToolDraw(Event*)
{
  MetaSelection* selection = Z::gEditor->mSelection;

  Aabb aabb;

  if(mGizmoMode != GizmoMode::Transforming)
  {
    aabb = GetAabb(selection, IncludeMode::Children);
    mActiveAabb = aabb;
  }
  else
  {
    aabb = mActiveAabb;
  }

  Vec3 center = aabb.GetCenter();
  Vec3 size = aabb.GetHalfExtents();
  Quat rotation = Quat::cIdentity;

  Vec3 gripSize(GripSize, GripSize, GripSize * 0.1f);

  gDebugDraw->Add(Debug::Obb(center, size, rotation).Color(Color::Red));

  for(uint i=0;i<PointCount;++i)
  {
    Vec3 point = center;

    if( GrabPoints[i] & DirectionFlags::Left )
      point -= Vec3(size.x, 0, 0);

    if( GrabPoints[i] & DirectionFlags::Top )
      point += Vec3(0, size.y, 0);

    if( GrabPoints[i] & DirectionFlags::Right )
      point += Vec3(size.x, 0, 0);

    if( GrabPoints[i] & DirectionFlags::Bottom )
      point -= Vec3(0, size.y, 0);

    if(i == mSelectedPoint)
      gDebugDraw->Add(Debug::Box(point, gripSize, rotation).ViewScaled(true).Color(Color::Green).OnTop(true));
    else
      gDebugDraw->Add(Debug::Box(point, gripSize, rotation).ViewScaled(true).Color(Color::Red).OnTop(true));
  }
}

}//namespace Zero
