///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Joshua Claeys, Ryan Edgemon
/// Copyright 2010-2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

// ManipulatorTool Events
namespace Events
{
DefineEvent(ManipulatorToolStart);
DefineEvent(ManipulatorToolModified);
DefineEvent(ManipulatorToolEnd);
}

//----------------------------------------------------- ManipulatorToolEvent ---
ZilchDefineType(ManipulatorToolEvent, builder, type)
{
  ZilchBindGetter(OperationQueue);
  ZilchBindGetter(Finished);

  ZilchBindFieldGetter(mGrabLocation);
  ZilchBindFieldGetter(mStartWorldRectangle);
  ZilchBindField(mEndWorldRectangle);
}

//******************************************************************************
ManipulatorToolEvent::ManipulatorToolEvent(ViewportMouseEvent* event)
  : ViewportMouseEvent(*event), mOperationQueue(nullptr),
    mStartWorldRectangle(Rectangle::cZero), mEndWorldRectangle(Rectangle::cZero)
{
}

//******************************************************************************
OperationQueue* ManipulatorToolEvent::GetOperationQueue()
{
  return mOperationQueue;
}

//******************************************************************************
bool ManipulatorToolEvent::GetFinished()
{
  return mOperationQueue.IsNotNull();
}

//------------------------------------------------------------- Manipulator Tool
DeclareBitField4(DirectionFlags, Left, Right, Top, Bottom);

static const uint cMiddlePoint = 8;
static const uint cPointCount = 9;

static const uint cGrabPoints[] =
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

static const Location::Enum cLocations[] =
{
  Location::CenterLeft,
  Location::TopLeft,
  Location::TopCenter,
  Location::TopRight,
  Location::CenterRight,
  Location::BottomRight,
  Location::BottomCenter,
  Location::BottomLeft,
  Location::Center,
};

static const float cGripSize = 0.075f;
static const float cToolEpsilon = 0.0001f;

//******************************************************************************
ZilchDefineType(ManipulatorTool, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDependency(MouseCapture);
  type->AddAttribute(ObjectAttributes::cTool);

  ZeroBindEvent(Events::ManipulatorToolStart, ManipulatorToolEvent);
  ZeroBindEvent(Events::ManipulatorToolModified, ManipulatorToolEvent);
  ZeroBindEvent(Events::ManipulatorToolEnd, ManipulatorToolEvent);

  ZilchBindFieldProperty(mGrabMode);
  ZilchBindFieldProperty(mSnapping);
  ZilchBindGetterSetterProperty(SnapDistance);
  ZilchBindFieldProperty(mSizeBoxCollider);
}

//******************************************************************************
ManipulatorTool::ManipulatorTool()
{
  mSnapping = false;
  mSnapDistance = 0.5f;

  mSizeBoxCollider = true;

  mValidSelection = false;
  mGizmoMode = GizmoMode::Inactive;
  mGrabMode = GizmoGrab::Hold;
  
  mSelectedPoint = -1;
  mLocation = Location::Center;
}

//******************************************************************************
void ManipulatorTool::Serialize(Serializer& stream)
{
  SerializeNameDefault(mSnapping, false);
  SerializeNameDefault(mSnapDistance, 0.5f);

  SerializeNameDefault(mSizeBoxCollider, true);
}

//******************************************************************************
void ManipulatorTool::Initialize(CogInitializer& initializer)
{
  ConnectThisTo(GetOwner( ), Events::ToolActivate, OnToolActivate);
  ConnectThisTo(GetOwner( ), Events::ToolDeactivate, OnToolDeactivate);
  ConnectThisTo(GetOwner( ), Events::LeftMouseDown, OnLeftMouseDown);
  ConnectThisTo(GetOwner( ), Events::LeftMouseUp, OnLeftMouseUp);
  ConnectThisTo(GetOwner( ), Events::MouseMove, OnMouseMove);

  /// Drag Events
  ConnectThisTo(GetOwner( ), Events::MouseDragStart, OnMouseDragStart);
  ConnectThisTo(GetOwner( ), Events::MouseDragEnd, OnMouseDragEnd);
  ConnectThisTo(GetOwner( ), Events::MouseDragMove, OnMouseDragMove);

  ConnectThisTo(GetOwner( ), Events::ToolDraw, OnToolDraw);
}

//******************************************************************************
float ManipulatorTool::GetSnapDistance( )
{
  return mSnapDistance;
}

//******************************************************************************
void ManipulatorTool::SetSnapDistance(float distance)
{
  if(distance < 0.001f)
    distance = 0.001f;

  mSnapDistance = distance;
}

//******************************************************************************
bool ManipulatorTool::IsActiveAndHasValidSelection( )
{
  if(mValidSelection == false)
    return false;

  MetaSelection* selection = Z::gEditor->GetSelection( );
  return selection->Count( ) > 0 && Active( );
}

//******************************************************************************
bool ManipulatorTool::CheckMouseManipulation(ViewportMouseEvent* e)
{
  if(IsActiveAndHasValidSelection( ))
  {
    MouseCapture* capture = GetOwner( )->has(MouseCapture);
    capture->Capture(e);
    return true;
  }
  return false;
}

//******************************************************************************
void ManipulatorTool::OnToolActivate(Event*)
{
  mGizmoMode = GizmoMode::Active;
}

//******************************************************************************
void ManipulatorTool::OnToolDeactivate(Event*)
{
  mGizmoMode = GizmoMode::Inactive;
}


//******************************************************************************
void ManipulatorTool::OnLeftMouseDown(ViewportMouseEvent* e)
{
  if(mGrabMode == GizmoGrab::Hold)
    e->Handled = CheckMouseManipulation(e);
  else
    e->Handled = IsActiveAndHasValidSelection( );
}

//******************************************************************************
void ManipulatorTool::OnLeftMouseUp(ViewportMouseEvent* e)
{
  if(mGrabMode == GizmoGrab::Toggle)
    e->Handled = CheckMouseManipulation(e);
}

//******************************************************************************
void ManipulatorTool::OnMouseMove(ViewportMouseEvent* e)
{
  if(Active( ))
    TestMouseMove(e);
}

//******************************************************************************
void ManipulatorTool::TestMouseMove(ViewportMouseEvent* e)
{
  MetaSelection* selection = Z::gEditor->mSelection;
  Cog* primary = selection->GetPrimaryAs<Cog>();

  Vec3 center = mActiveAabb.GetCenter();
  Vec3 size = mActiveAabb.GetHalfExtents();

  Camera* camera = e->GetViewport()->GetCamera();
  Ray mouseRay = e->mWorldRay;

  mValidSelection = false;
  mSelectedPoint = -1;

  for(uint i = 0; i < cPointCount; ++i)
  {
    Vec3 point = center;
    mLocation = Location::Center;

    if(cGrabPoints[i] & DirectionFlags::Left)
      point -= Vec3(size.x, 0, 0);
    else if(cGrabPoints[i] & DirectionFlags::Right)
      point += Vec3(size.x, 0, 0);

    if(cGrabPoints[i] & DirectionFlags::Top)
      point += Vec3(0, size.y, 0);
    else if(cGrabPoints[i] & DirectionFlags::Bottom)
      point -= Vec3(0, size.y, 0);

    float scaledGripSize = GizmoHelpers::GetViewScale(camera, point) * 0.1f;
    Vec3 boxSize(scaledGripSize, scaledGripSize, scaledGripSize);

    Intersection::Type result =
      Intersection::RayObb(mouseRay.Start, mouseRay.Direction, point, boxSize, Mat3::cIdentity);

    if(result != Intersection::None)
    {
      mSelectedPoint = i;
      mLocation = cLocations[i];
      mValidSelection = true;
      return;
    }

  }

}

//******************************************************************************
void ManipulatorTool::OnMouseDragStart(ViewportMouseEvent* e)
{
  mStartAabb = mActiveAabb;
  mMouseDragStart = e->Position;
  
  if(!mValidSelection)
    return;

  MetaSelection* selection = Z::gEditor->GetSelection( );

  Array<Cog*> cogs;
  FilterChildrenAndProtected(cogs, selection);
  mTransformingObjects.Clear( );

  ManipulatorToolEvent eventToSend(e);
  eventToSend.mGrabLocation = mLocation;
  eventToSend.mStartWorldRectangle = Rectangle::MinAndMax(mStartAabb.mMin, mStartAabb.mMax);
  eventToSend.mEndWorldRectangle = eventToSend.mStartWorldRectangle;

  Array<Cog*>::range r = cogs.All( );
  while(!r.Empty( ))
  {
    Cog* target = r.Front( );
    target->DispatchEvent(Events::ManipulatorToolStart, &eventToSend);

    //Store the staring values for transformations
    if(Transform* tx = target->has(Transform))
    {
      TransformingObject data;
      data.ObjectId = target;
      data.StartWorldTranslation = tx->GetWorldTranslation( );
      data.StartTranslation = tx->GetTranslation( );
      data.StartRotation = tx->GetRotation( );
      data.StartScale = tx->GetScale( );
      data.EndTranslation = data.StartTranslation;
      data.EndRotation = data.StartRotation;
      data.EndScale = data.StartScale;

      if(Area* area = target->has(Area))
      {
        data.StartSize = area->mSize;
        data.EndSize = area->mSize;

        if(BoxCollider* collider = target->has(BoxCollider))
          data.StartColliderSize = collider->GetSize();

        data.StartRect = area->GetWorldRectangle();
      }
      else
      {
        data.StartSize = Vec2::cZero;
        data.EndSize = Vec2::cZero;
      }

      mTransformingObjects.PushBack(data);
    }

    r.PopFront( );
  }

  //Gizmo is now moving
  mGizmoMode = GizmoMode::Transforming;
}

//******************************************************************************
void ManipulatorTool::OnMouseDragMove(ViewportMouseEvent* e)
{
  Vec3 startExtents = mStartAabb.GetExtents();
  Vec3 startCenter = mStartAabb.GetCenter();

  Rectangle toolStartRect = Rectangle::MinAndMax(mStartAabb.mMin, mStartAabb.mMax);
  Rectangle toolUpdateRect = toolStartRect;

  // Compute plane movement
  Viewport* viewport = e->GetViewport();
  Vec3 oldPosition = viewport->ScreenToWorldPlane(mMouseDragStart, Vec3::cZAxis, startCenter);
  Vec3 newPosition = viewport->ScreenToWorldPlane(e->Position, Vec3::cZAxis, startCenter);
  Vec3 movement = newPosition - oldPosition;
  Vec2 move2D(movement.x, movement.y);

  if(mSnapping)
    move2D = Snap(move2D, mSnapDistance);

  Vec3 translationChange;

  // Translation only.
  if(mSelectedPoint == cMiddlePoint)
  {
    translationChange = Vec3(move2D, 0);
  }
  else
  {
    uint grabFlags = cGrabPoints[mSelectedPoint];

    // If true:  1 * value = value [ie, use movement on axis].
    // If false: 0 * value =     0 [ie, no movement on axis].
    Vec2 moveOnAxis;
    moveOnAxis.x = (bool)(grabFlags & (DirectionFlags::Left | DirectionFlags::Right));
    moveOnAxis.y = (bool)(grabFlags & (DirectionFlags::Top | DirectionFlags::Bottom));

    move2D.x = moveOnAxis.x * move2D.x;
    move2D.y = moveOnAxis.y * move2D.y;

    // Maintain aspect ratio.
    if(e->ShiftPressed && moveOnAxis.x && moveOnAxis.y)
    {
      Vec2 grabPosition = toolStartRect.GetLocation(mLocation);
      Vec2 axis = grabPosition - toolStartRect.GetCenter();
      float length = axis.AttemptNormalize();

      // Prevent sizer gizmo not being able to recover from a "zero-size" state.
      if(length >= Math::Epsilon() * Math::Epsilon())
        move2D = axis * axis.Dot(move2D);
    }

    Vec2 startSize = toolStartRect.GetSize();

    // Moving left is negative on the x-axis, but the size needs to increase
    // [ie, move the min farther from the max]. So, flip the sign of the move.
    if(grabFlags & DirectionFlags::Left)
      toolUpdateRect.SetSize(Location::CenterRight, startSize - move2D);
    else
      toolUpdateRect.SetSize(Location::CenterLeft, startSize + move2D);

    // Moving down is negative on the y-axis, but the size needs to increase
    // [ie, move the max farther from the min]. So, flip the sign of the move.
    if(grabFlags & DirectionFlags::Bottom)
      toolUpdateRect.SetSize(Location::TopCenter, startSize - move2D);
    else
      toolUpdateRect.SetSize(Location::BottomCenter, startSize + move2D);

    translationChange = Vec3(toolUpdateRect.GetCenter() - toolStartRect.GetCenter(), 0);
  }

  // Compute the new tool-aabb.
  Vec3 newCenter = startCenter + translationChange;
  Vec3 newExtents = Vec3(toolUpdateRect.GetSize(), startExtents.z);

  mActiveAabb.SetCenter(newCenter);
  mActiveAabb.SetExtents(newExtents);

  Vec3 toolOrigin = Vec3(toolStartRect.GetLocation(Location::GetOpposite(mLocation)), startCenter.z);

  ManipulatorToolEvent eventToSend(e);
  eventToSend.mGrabLocation = mLocation;
  eventToSend.mStartWorldRectangle = toolStartRect;
  eventToSend.mEndWorldRectangle = toolUpdateRect;

  // Compute updated transforms for the objects the tool is affecting.
  forRange(TransformingObject& object, mTransformingObjects.All())
  {
    Cog* cog = object.ObjectId;
    if(cog == nullptr)
      continue;

    // Reset for each object in case 'EndWorldRectangle' was modified, or
    // 'HandledEventScript' was set.
    eventToSend.mEndWorldRectangle = toolUpdateRect;
    eventToSend.HandledEventScript = false;

    cog->DispatchEvent(Events::ManipulatorToolModified, &eventToSend);

    cog->DispatchEvent(Events::ManipulatorToolModified, &eventToSend);
    if(eventToSend.Handled || eventToSend.HandledEventScript)
      continue;

    Vec2 returnedCenter = eventToSend.mEndWorldRectangle.GetCenter();
    Vec2 returnedSize = eventToSend.mEndWorldRectangle.GetSize();

    // Update values to conform to modifications a user may have made to
    // 'EndWorldRectangle' when handling 'ManipulatorToolModified'.
    translationChange = Vec3(returnedCenter - toolStartRect.GetCenter(), 0);
    newExtents = Vec3(returnedSize, startExtents.z);

    Vec3 scaleChange = newExtents / startExtents;

    Transform* tf = cog->has(Transform);
    Vec3 newTranslation = object.StartWorldTranslation + translationChange;

    if(mSelectedPoint != cMiddlePoint)
    {
      Vec3 originalOffset = object.StartWorldTranslation - toolOrigin;
      newTranslation = toolOrigin + scaleChange * originalOffset;
    }

    // Change to local
    if(tf->TransformParent)
      newTranslation = tf->TransformParent->TransformPointInverse(newTranslation);

    // Change Translation
    object.EndTranslation = newTranslation;
    tf->SetTranslation(object.EndTranslation);

    Area* area = cog->has(Area);
    if(area != nullptr)
    {
      object.EndSize = object.StartSize * ToVector2(scaleChange);
      area->SetSize(object.EndSize);

      // No need to update an object's BoxCollider if there isn't one, or if
      // that option isn't enabled.
      BoxCollider* collider = cog->has(BoxCollider);
      if(collider != nullptr && mSizeBoxCollider)
      {
        Vec3 endOffset(area->OffsetOfOffset(Location::Center) * area->GetSize());
        collider->SetOffset(endOffset);

        Vec3 endSize = Vec3(object.EndSize.x, object.EndSize.y, object.StartColliderSize.z);
        collider->SetSize(endSize);
      }

    }
    else
    {
      // Change Scale
      object.EndScale = object.StartScale * scaleChange;
      tf->SetScale(object.EndScale);
    }

  }

  e->Handled = true;
}

//******************************************************************************
void ManipulatorTool::OnMouseDragEnd(ViewportMouseEvent* e)
{
  BoundType* transformType = ZilchTypeId(Transform);
  Property* translationProperty = transformType->GetProperty("Translation");
  Property* rotationProperty = transformType->GetProperty("Rotation");
  Property* scaleProperty = transformType->GetProperty("Scale");

  Property* areaSizeProperty = ZilchTypeId(Area)->GetProperty("Size");

  Property* colliderOffsetProperty = ZilchTypeId(BoxCollider)->GetProperty("Offset");
  Property* colliderSizeProperty = ZilchTypeId(BoxCollider)->GetProperty("Size");

  MetaDisplay* cogDisplay = ZilchTypeId(Cog)->HasInherited<MetaDisplay>();

  OperationQueue* queue = Z::gEditor->GetOperationQueue();

  ManipulatorToolEvent eventToSend(e);
  eventToSend.mGrabLocation = mLocation;
  eventToSend.mOperationQueue = queue;
  eventToSend.mStartWorldRectangle = Rectangle::MinAndMax(mStartAabb.mMin, mStartAabb.mMax);
  eventToSend.mEndWorldRectangle = Rectangle::MinAndMax(mActiveAabb.mMin, mActiveAabb.mMax);

  queue->BeginBatch();
  queue->SetActiveBatchName("MultiObjectManipulation");

  forRange(TransformingObject& object, mTransformingObjects.All())
  {
    Cog* cog = object.ObjectId;
    if(cog == nullptr)
      continue;

    cog->DispatchEvent(Events::ManipulatorToolEnd, &eventToSend);
    if(eventToSend.Handled || eventToSend.HandledEventScript)
      continue;

    Transform* transform = cog->has(Transform);
    transform->GetSpace()->MarkModified();

    String name = cogDisplay->GetName(cog);

    queue->BeginBatch();
    queue->SetActiveBatchName(BuildString(name, "Manipulation"));

    // Send the final GizmoFinish transform update
    uint flag = 0;

    // When manipulating multiple objects, translation might have changed.
    if((object.StartTranslation - object.EndTranslation).LengthSq() != 0.0f)
    {
      flag |= TransformUpdateFlags::Translation;
      transform->SetLocalTranslationInternal(object.StartTranslation);

      PropertyPath propertyPath(transform, translationProperty);
      ChangeAndQueueProperty(queue, cog, propertyPath, object.EndTranslation);
    }

    // Scale might have changed.
    if((object.StartScale - object.EndScale).LengthSq() != 0.0f)
    {
      flag |= TransformUpdateFlags::Scale;
      transform->SetLocalScaleInternal(object.StartScale);

      PropertyPath propertyPath(transform, scaleProperty);
      ChangeAndQueueProperty(queue, cog, propertyPath, object.EndScale);
    }

    // Area might have changed.
    Area* area = cog->has(Area);
    if(area && (object.StartSize - object.EndSize).LengthSq() != 0.0f)
    {
      area->SetSize(object.StartSize);

      PropertyPath areaPath(area, areaSizeProperty);
      ChangeAndQueueProperty(queue, cog, areaPath, object.EndSize);

      // No need to update an object's BoxCollider if there isn't one, or if
      // that option isn't enabled.
      BoxCollider* collider = cog->has(BoxCollider);
      if(collider != nullptr && mSizeBoxCollider)
      {
        collider->SetOffset(collider->GetOffset());

        Vec3 endOffset(area->OffsetOfOffset(Location::Center) * area->GetSize());
        PropertyPath offsetPath(collider, colliderOffsetProperty);
        ChangeAndQueueProperty(queue, cog, offsetPath, endOffset);

        collider->SetSize(collider->GetSize());

        Vec3 endSize = Vec3(object.EndSize.x, object.EndSize.y, object.StartColliderSize.z);
        PropertyPath sizePath(collider, colliderSizeProperty);
        ChangeAndQueueProperty(queue, cog, sizePath, endSize);
      }

    }

    queue->EndBatch();  // end specific object Manipulation

    transform->Update(flag | TransformUpdateFlags::GizmoFinish);
  }

  queue->EndBatch();  // end MultiObjectManipulation

  mGizmoMode = GizmoMode::Active;
}

//******************************************************************************
void ManipulatorTool::OnToolDraw(Event*)
{
  MetaSelection* selection = Z::gEditor->mSelection;

  Aabb aabb;

  if(mGizmoMode != GizmoMode::Transforming)
  {
    Cog* primary = selection->GetPrimaryAs<Cog>();
    if (primary && primary->has(UiWidget) && selection->Count() == 1)
      aabb = GetAabb(primary, IncludeMode::OnlyRoot);
    else
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

  Vec3 gripSize(cGripSize, cGripSize, cGripSize * 0.1f);

  gDebugDraw->Add(Debug::Obb(center, size, rotation).Color(Color::Red));

  for(uint i=0;i<cPointCount;++i)
  {
    Vec3 point = center;

    if( cGrabPoints[i] & DirectionFlags::Left )
      point -= Vec3(size.x, 0, 0);

    if( cGrabPoints[i] & DirectionFlags::Top )
      point += Vec3(0, size.y, 0);

    if( cGrabPoints[i] & DirectionFlags::Right )
      point += Vec3(size.x, 0, 0);

    if( cGrabPoints[i] & DirectionFlags::Bottom )
      point -= Vec3(0, size.y, 0);

    if(i == mSelectedPoint)
      gDebugDraw->Add(Debug::Box(point, gripSize, rotation).Filled(true).ViewScaled(true).Color(Color::Green).OnTop(true));
    else
      gDebugDraw->Add(Debug::Box(point, gripSize, rotation).Filled(true).ViewScaled(true).Color(Color::Red).OnTop(true));
  }
}


}//namespace Zero
