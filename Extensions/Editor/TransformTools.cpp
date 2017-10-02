///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Chris Peters, Joshua Claeys, Ryan Edgemon
/// Copyright 2010-2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

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
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDependency(MouseCapture);
  ZeroBindTag(Tags::Tool);

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
bool ManipulatorTool::OnGizmo( )
{
  MetaSelection* selection = Z::gEditor->GetSelection( );
  return selection->Count( ) > 0 && Active( );
}

//******************************************************************************
bool ManipulatorTool::CheckMouseManipulation(ViewportMouseEvent* e)
{
  if(OnGizmo( ))
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
    e->Handled = OnGizmo( );
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
  Aabb aabb = mActiveAabb;
  Vec3 center = aabb.GetCenter();
  Vec3 size = aabb.GetHalfExtents();
  Quat rot = Quat::cIdentity;

  mSelectedPoint = -1;

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
    float scaledGripSize = GizmoHelpers::GetViewScale(camera, point) * 0.1f;

    Vec3 boxSize(scaledGripSize, scaledGripSize, scaledGripSize);

    Mat3 noRot = Mat3::cIdentity;
    Ray mouseRay = e->mWorldRay;
    Intersection::Type result = Intersection::RayObb(mouseRay.Start, mouseRay.Direction, point, boxSize, noRot);

    if(result != Intersection::None)
    {
      mSelectedPoint = i;
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

  Array<Cog*>::range r = cogs.All( );
  while(!r.Empty( ))
  {
    Cog* target = r.Front( );

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
    r.PopFront( );
  }

  //Gizmo is now moving
  mGizmoMode = GizmoMode::Transforming;
}

//******************************************************************************
void ManipulatorTool::OnMouseDragMove(ViewportMouseEvent* e)
{
  Viewport* viewport = e->GetViewport();
  Camera* camera = viewport->GetCamera();
  float viewDepth = 0.0f;

  Vec3 startExtents = mStartAabb.GetExtents( );
  Vec3 startCenter = mStartAabb.GetCenter( );

  if(camera != nullptr)
    viewDepth = (startCenter - camera->GetWorldTranslation( )).Length();

  // Compute plane movement
  Vec3 oldPosition = viewport->ScreenToWorldZPlane(mMouseDragStart, viewDepth);
  Vec3 newPosition = viewport->ScreenToWorldZPlane(e->Position, viewDepth);

  // All movement computed relative to bottom left of staring aabb.
  Vec3 worldOrigin = mStartAabb.mMin;
  Vec3 movementInPlane = newPosition - oldPosition;
  Vec3 orginMovement = Vec3(0,0,0);

  if(mSnapping)
    movementInPlane = Snap(movementInPlane, mSnapDistance);

  // Determine the movement of tool-origin and update the 'movementInPlane'
  // to be relative.
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

  // Compute the new tool-aabb.
  Vec3 newExtents = startExtents + movementInPlane;
  Vec3 scaleChange = newExtents / startExtents;
  Vec3 aabbOffset = startCenter - worldOrigin;
  Vec3 newCenter = worldOrigin + aabbOffset * scaleChange + orginMovement;

  mActiveAabb.SetCenter(newCenter);
  mActiveAabb.SetExtents(newExtents);

  // Compute all updated transform on the objects the tool is affecting.
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
void ManipulatorTool::OnMouseDragEnd(Event*)
{
  BoundType* transformType = ZilchTypeId(Transform);
  Property* translationProperty = transformType->GetProperty("Translation");
  Property* rotationProperty = transformType->GetProperty("Rotation");
  Property* scaleProperty = transformType->GetProperty("Scale");

  String toolName = ZilchVirtualTypeId(this)->Name;

  OperationQueue* queue = Z::gEditor->GetOperationQueue( );

  queue->BeginBatch( );
  queue->SetActiveBatchName("ManipulatorTool_OnMouseDragEnd");

  Array<TransformingObject>::range r = mTransformingObjects.All( );
  while(!r.Empty( ))
  {
    Cog* target = r.Front( ).ObjectId;
    if(target)
    {
      TransformingObject& transformObject = r.Front( );
      Transform* transform = target->has(Transform);

      transform->GetSpace( )->MarkModified( );

      queue->BeginBatch( );
      queue->SetActiveBatchName("ManipulatorTool");

      // Send the final GizmoFinish transform update
      uint flag = 0;

      // When scaling or rotating multiple objects translation may changed so just check
      if((transformObject.StartTranslation - transformObject.EndTranslation).LengthSq( )!=0.0f)
      {
        flag |= TransformUpdateFlags::Translation;
        transform->SetLocalTranslationInternal(transformObject.StartTranslation);
        PropertyPath propertyPath(transform, translationProperty);
        ChangeAndQueueProperty(queue,
          target,
          propertyPath,
          transformObject.EndTranslation);
      }

      if((transformObject.StartScale - transformObject.EndScale).LengthSq( )!=0.0f)
      {
        flag |= TransformUpdateFlags::Scale;
        transform->SetLocalScaleInternal(transformObject.StartScale);
        PropertyPath propertyPath(transform, scaleProperty);
        ChangeAndQueueProperty(queue,
          target,
          propertyPath,
          transformObject.EndScale);
      }

      if((transformObject.StartSize - transformObject.EndSize).LengthSq( )!=0.0f)
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
              Vec3 startOffset = collider->GetOffset( );
              Vec3 startSize = collider->GetSize( );
              Vec3 endOffset = Vec3(OffsetOfOffset(area->GetOrigin( ), Location::Center) * area->GetSize( ), 0);

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

      queue->EndBatch( );

      transform->Update(flag | TransformUpdateFlags::GizmoFinish);
    }

    r.PopFront( );
  }

  queue->EndBatch( );

  mGizmoMode = GizmoMode::Active;
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
