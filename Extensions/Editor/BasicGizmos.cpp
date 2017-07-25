///////////////////////////////////////////////////////////////////////////////
///
/// \file TransformGizmos.cpp
/// Declaration of the Transform Gizmo classes.
/// 
/// Authors: Chris Peters, Joshua Claeys, Ryan Edgemon
/// Copyright 2010-2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//float GizmoGetViewScale(Camera* camera, Vec3Param location)
//{
//  Mat4 rotation = ToMatrix4(camera->mTransform->GetWorldRotation( ));
//  if(camera->mFacing == Facing::PositiveZ)
//  {
//    Mat4 localRotation;
//    localRotation.Rotate(Vec3::cYAxis, Math::DegToRad(180));
//    rotation = rotation * localRotation;
//  }
//
//  Vec3 viewDirection = Math::TransformNormal(rotation, Vec3::cZAxis);
//
//  return Debug::GetViewScale(location, camera->mTransform->GetWorldTranslation( ), viewDirection, camera->mSize, camera->mPerspectiveMode == PerspectiveMode::Orthographic);
//}

float GizmoGetViewScale(Camera* camera, Vec3Param location)
{
  float viewDistance = Debug::GetViewDistance(location, camera->GetWorldTranslation(), camera->GetWorldDirection());
  bool orthographic = camera->mPerspectiveMode == PerspectiveMode::Orthographic;
  return Debug::GetViewScale(viewDistance, camera->mFieldOfView, camera->mSize, orthographic);
}

//----------------------------------------------------------------------- Events
namespace Events
{
  DefineEvent(RingGizmoModified);
}

//----------------------------------------------------------------- GizmoHelpers
namespace GizmoHelpers
{

//******************************************************************************
int GetDragAxis(Vec3Param localMovement)
{
  for(int axis = 0; axis < 3; ++axis)
  {
    if(Math::Abs(localMovement[axis]) > Math::Epsilon( ))
      return axis;
  }

  return INT_MAX;
}

//******************************************************************************
Vec3 GetOffAxisMovement(Vec3Param movement)
{
  Vec3 v(movement);

  int dragAxis = GizmoHelpers::GetDragAxis(v);
  if(dragAxis != INT_MAX)
  {
    float selectedAxisMovement = v[dragAxis];
    Vec3 offAxis = Vec3(1, 1, 1);
    offAxis[dragAxis] = 0.0f;

    v = offAxis * selectedAxisMovement;
  }

  return v;
}

//******************************************************************************
Vec3 NormalToLocal(Vec3Param vector0, Transform* transform)
{
  if(transform->TransformParent)
  {
    //Transform into local space
    Mat4 parentWorldInverse = transform->TransformParent->GetWorldMatrix( );
    parentWorldInverse.Invert( );
    return TransformNormal(parentWorldInverse, vector0);//.Normalized();
  }
  return vector0;
}

//******************************************************************************
void SetLocalPositionFromWorld(ObjectTransformState& object, Vec3Param worldTranslation,
  Transform* transform)
{
  //if the object has a parent and is not in world
  //then transform the world translation into local space
  if(transform->TransformParent && !transform->GetInWorld( ))
  {
    Mat4 parentWorldInverse = transform->TransformParent->GetWorldMatrix( );
    parentWorldInverse.Invert( );
    transform->SetTranslation(TransformPoint(parentWorldInverse, worldTranslation));
  }
  else
  {
    transform->SetTranslation(worldTranslation);
  }

  object.EndTranslation = transform->GetTranslation( );
}

//******************************************************************************
//Current way of translating movement to a new scale
float ProcessScale(float movement, float startDis, float starting)
{
  return Math::Abs(starting + (movement / startDis) * starting);
}

//******************************************************************************
Vec3 ScaleVector(Vec3Param delta, float distance, Vec3Param start)
{
  Vec3 newValue;
  newValue.x = GizmoHelpers::ProcessScale(delta.x, distance, start.x);
  newValue.y = GizmoHelpers::ProcessScale(delta.y, distance, start.y);
  newValue.z = GizmoHelpers::ProcessScale(delta.z, distance, start.z);
  return newValue;
}


}  //namespace GizmoHelpers


//--------------------------------------------------------------- Gizmo Snapping
namespace GizmoSnapping
{

#define SNAP_ON_AXIS(in,out) \
  float ii = in*in;    out = (ii > Math::Epsilon() || ii < -Math::Epsilon())

//******************************************************************************
Vec3 GetSnappedPosition(Vec3Param currentPosition, Vec3Param deltaWorldMovement,
  QuatParam basis, GizmoDragMode::Enum dragMode,
  GizmoSnapMode::Enum snapMode, float snapDistance)
{
  Vec3 worldMovement = deltaWorldMovement;

  if(snapMode == GizmoSnapMode::Relative)
  {
    if(dragMode == GizmoDragMode::Line)
    {
      // Snap the distance that we're moving
      float distance = Math::Length(worldMovement);
      distance = Snap(distance, snapDistance);

      // Apply the new distance to the movement
      worldMovement.AttemptNormalize( );
      worldMovement = worldMovement * distance;
    }
    else  // Moving on a hyperplane, or the view-plane specifically
    {
      // Snap on all basis axes
      //  - note: Doing so accounts for 'GizmoBasis::Local'
      Mat3 m = ToMatrix3(basis);
      // X
      Vec3 axis = m.BasisX( ).AttemptNormalized( );
      float snap = Snap(Math::Dot(axis, worldMovement), snapDistance);
      Vec3 snappedMovement(snap * axis);
      // Y
      axis = m.BasisY( ).AttemptNormalized( );
      snap = Snap(Math::Dot(axis, worldMovement), snapDistance);
      snappedMovement += snap * axis;
      // Z
      axis = m.BasisZ( ).AttemptNormalized( );
      snap = Snap(Math::Dot(axis, worldMovement), snapDistance);
      snappedMovement += snap * axis;

      worldMovement = snappedMovement;
    }

  }
  else if(snapMode == GizmoSnapMode::WorldAxes)
  {
    Vec3 newPosition;
    for(int i = 0; i < 3; ++i)
    {
      bool snapOnAxis;
      SNAP_ON_AXIS(worldMovement[i], snapOnAxis);

      // If 'snapOnAxis * snapDistance' is 0, then 'currentPosition[i]' will be the final result.
      // [ie, don't snap on that axis]
      newPosition[i] = Snap(currentPosition[i] + worldMovement[i], snapOnAxis * snapDistance);
    }

    return newPosition;
  }
  else if(snapMode == GizmoSnapMode::WorldGrid)
  {
    // Apply the movement, then snap to world-grid with units defined by 'snapDistance'
    Vec3 worldGridPosition = Snap(currentPosition + worldMovement, snapDistance);
    return worldGridPosition;
  }

  return currentPosition + worldMovement;
}

//******************************************************************************
Vec3 GetSnappedScale(Vec3Param startPosition, Vec3Param mouseWorldMovement,
  GizmoSnapMode::Enum snapMode, float snapDistance,
  GizmoDragMode::Enum dragMode)
{
  Vec3 worldMovement = mouseWorldMovement;

  // Apply relative snapping to the world movement before
  if(snapMode == GizmoSnapMode::Relative)
  {
    if(dragMode == GizmoDragMode::Line)
    {
      // If we're moving in a line, snap the distance that we're moving
      float distance = Math::Length(worldMovement);
      distance = Snap(distance, snapDistance);

      // Apply the new distance to the movement
      worldMovement.AttemptNormalize( );
      worldMovement = worldMovement * distance;
    }
    else
    {
      // If we're moving on a plane, just snap the world movement
      worldMovement = Snap(worldMovement, snapDistance);
    }

  }
  else if(snapMode == GizmoSnapMode::WorldAxes)
  {
    Vec3 newScale;
    for(int i = 0; i < 3; ++i)
    {
      bool snapOnAxis;
      SNAP_ON_AXIS(worldMovement[i], snapOnAxis);

      // If 'snapOnAxis * snapDistance' is 0, then 'startPosition[i]' will be the final result.
      // [ie, don't snap on that axis]
      newScale[i] = Snap(startPosition[i] + worldMovement[i], snapOnAxis * snapDistance);
    }

    return newScale;
  }
  else if(snapMode == GizmoSnapMode::WorldGrid)
  {
    // Apply the movement, then snap to world-grid with units defined by 'snapDistance'
    Vec3 worldGridPosition = Snap(startPosition + worldMovement, snapDistance);
    return worldGridPosition;
  }

  return startPosition + worldMovement;
}


}  //namespace GizmoSnapping

//------------------------------------------------------------ Simple Gizmo Base
ZilchDefineType(SimpleGizmoBase, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindTag(Tags::Gizmo);
  ZeroBindDocumented();

  ZeroBindDependency(Transform);
  ZeroBindDependency(Gizmo);

  ZilchBindFieldProperty(mMouseInput);
  ZilchBindFieldProperty(mPickingPriority);
  ZilchBindFieldProperty(mColor);
  ZilchBindFieldProperty(mHoverColor);
  ZilchBindFieldProperty(mViewScaled);
  ZilchBindFieldProperty(mUseParentAsViewScaleOrigin);
  ZilchBindFieldProperty(mDrawOnTop);
}

//******************************************************************************
SimpleGizmoBase::SimpleGizmoBase()
{
  mGrabPoint = Vec3::cZero;
}

//******************************************************************************
void SimpleGizmoBase::Serialize(Serializer& stream)
{
  SerializeNameDefault(mMouseInput, true);
  SerializeNameDefault(mPickingPriority, 0);
  SerializeNameDefault(mColor, Vec4(0.8f,0.8f,0.8f,1));
  SerializeNameDefault(mHoverColor, Vec4(1,1,1,1));
  SerializeNameDefault(mViewScaled, true);
  SerializeNameDefault(mUseParentAsViewScaleOrigin, false);
  SerializeNameDefault(mDrawOnTop, true);
}

//******************************************************************************
void SimpleGizmoBase::Initialize(CogInitializer& initializer)
{
  mTransform = GetOwner()->has(Transform);
  mGizmo = GetOwner()->has(Gizmo);
}

//******************************************************************************
ByteColor SimpleGizmoBase::GetColor()
{
  if(mGizmo->mMouseOver)
    return ToByteColor(mHoverColor);
  return ToByteColor(mColor);
}

//----------------------------------------------------------------- Square Gizmo
ZilchDefineType(SquareGizmo, builder, type)
{
  ZeroBindComponent();
  ZilchBindFieldProperty(mSize);
  ZilchBindFieldProperty(mSnapDistance);
  ZilchBindFieldProperty(mViewAligned);
  ZilchBindFieldProperty(mBordered);
  ZilchBindFieldProperty(mFilled);
}

//******************************************************************************
void SquareGizmo::Serialize(Serializer& stream)
{
  SimpleGizmoBase::Serialize(stream);

  SerializeNameDefault(mSize, Vec3(0.4f));
  SerializeNameDefault(mSnapDistance, 0.25f);
  SerializeNameDefault(mViewAligned, true);
  SerializeNameDefault(mFilled, false);
  SerializeNameDefault(mBordered, false);
}

//******************************************************************************
void SquareGizmo::Initialize(CogInitializer& initializer)
{
  SimpleGizmoBase::Initialize(initializer);

  ConnectThisTo(GetOwner(), Events::GizmoRayTest, OnGizmoRayTest);
  ConnectThisTo(GetSpace(), Events::FrameUpdate, OnFrameUpdate);
}

//******************************************************************************
void SquareGizmo::OnGizmoRayTest(GizmoRayTestEvent* e)
{
  if(!mMouseInput)
    return;

  Camera* camera = e->mMouseEvent->GetCameraViewport()->mActiveCamera;
  if(camera == nullptr)
    return;

  Transform* t = GetOwner()->has(Transform);
  Vec3 center = t->GetWorldTranslation();

  Vec3 pivot = center;
  Cog* parent = GetOwner()->GetParent();

  if(mUseParentAsViewScaleOrigin)
  {
    if(Cog* parent = GetOwner()->GetParent())
    {
      if(Transform* parentTransform = parent->has(Transform))
        pivot = parentTransform->GetWorldTranslation();
    }
  }

  float viewScale = 1.0f;
  // Scale the gizmo by the view so it always appears the same size if specified
  if(mViewScaled)
    viewScale = GizmoGetViewScale(camera, pivot);

  Vec3 scaledCenter = pivot + ((center - pivot) * viewScale);
  Mat3 worldRotation = Math::ToMatrix3(t->GetWorldRotation());

  Ray ray = e->GetWorldRay();
  Intersection::IntersectionPoint point;
  Intersection::Type result = Intersection::RayObb(ray.Start, ray.Direction,
    scaledCenter, mSize * 0.5f * viewScale,
    worldRotation, &point);

  if(result != Intersection::None)
    e->RegisterResult(GetOwner(), point.T, mPickingPriority);
}

//******************************************************************************
void SquareGizmo::OnFrameUpdate(Event*)
{
  if(GetOwner()->has(Gizmo)->mActive == false)
    return;

  if(GetOwner()->mName == "XY")
    Cog* parent = GetOwner();

  ByteColor color = GetColor();

  Vec3 position = mTransform->GetWorldTranslation();
  Vec3 viewScaleOffset(Vec3::cZero);

  Vec3 size = mSize;
  Quat rot = mTransform->GetWorldRotation();

  if(mUseParentAsViewScaleOrigin)
    if(Cog* parent = GetOwner( )->GetParent( ))
      if(Transform* parentTransform = parent->has(Transform))
        viewScaleOffset = parentTransform->GetWorldTranslation( ) - position;

  gDebugDraw->Add(Debug::Obb(position, mSize * 0.5f, rot).OnTop(mDrawOnTop).Color(color).Filled(mFilled)
               .ViewScaled(mViewScaled).ViewAligned(mViewAligned).ViewScaleOffset(viewScaleOffset));

  gDebugDraw->Add(Debug::Obb(position, mSize * 0.5f, rot).OnTop(mDrawOnTop).Color(color).Border(mBordered)
               .ViewScaled(mViewScaled).ViewAligned(mViewAligned).ViewScaleOffset(viewScaleOffset));
}

//------------------------------------------------------------------ Arrow Gizmo
ZilchDefineType(ArrowGizmo, builder, type)
{
  ZeroBindComponent();
  // Used for defining the drag direction
  ZeroBindDependency(Orientation);

  ZilchBindFieldProperty(mHeadSize);
  ZilchBindFieldProperty(mLength);
  ZilchBindFieldProperty(mSelectRadius);
  ZilchBindFieldProperty(mHeadType);
  ZilchBindFieldProperty(mDualHeads);
  ZilchBindFieldProperty(mFilledHeads);
  ZilchBindFieldProperty(mLineDrawWidth);
}

//******************************************************************************
void ArrowGizmo::Serialize(Serializer& stream)
{
  SimpleGizmoBase::Serialize(stream);

  SerializeNameDefault(mHeadSize, 0.28f);
  SerializeNameDefault(mLength, 2.8f);
  SerializeNameDefault(mSelectRadius, 0.25f);
  SerializeEnumNameDefault(ArrowHeadType, mHeadType, ArrowHeadType::Arrow);
  SerializeNameDefault(mDualHeads, false);
  SerializeNameDefault(mFilledHeads, false);
  SerializeNameDefault(mLineDrawWidth, 2.0f);
}

//******************************************************************************
void ArrowGizmo::Initialize(CogInitializer& initializer)
{
  SimpleGizmoBase::Initialize(initializer);

  mOrientation = GetOwner()->has(Orientation);

  ConnectThisTo(GetOwner(), Events::GizmoRayTest, OnGizmoRayTest);
  ConnectThisTo(GetOwner(), Events::GizmoPreDrag, OnGizmoPreDrag);
  ConnectThisTo(GetSpace(), Events::FrameUpdate, OnFrameUpdate);
}

//******************************************************************************
void ArrowGizmo::OnGizmoRayTest(GizmoRayTestEvent* e)
{
  if(!mMouseInput)
    return;

  Camera* camera = e->mMouseEvent->GetCameraViewport()->mActiveCamera;
  if(camera == nullptr)
    return;

  Transform* t = GetOwner()->has(Transform);
  Vec3 start = t->GetWorldTranslation();

  float viewScale = 1.0f;

  // Scale the gizmo by the view so it always appears the same size if specified
  if(mViewScaled)
    viewScale = GizmoGetViewScale(camera, start);

  Orientation* orientation = GetOwner()->has(Orientation);
  Vec3 dir = orientation->GetWorldForward();

  Ray ray = e->mMouseEvent->mWorldRay;
  Vec3 end = start + dir * mLength * viewScale;
  float radius = mSelectRadius * viewScale;

  Intersection::IntersectionPoint point;
  Intersection::Type result = Intersection::RayCapsule(ray.Start, ray.Direction,
    start, end, radius,
    &point);
  if(result != Intersection::None)
    e->RegisterResult(GetOwner(), point.T, mPickingPriority);
}

//******************************************************************************
void ArrowGizmo::OnGizmoPreDrag(GizmoEvent* e)
{
  if(GizmoDrag* gizmoDrag = GetOwner()->has(GizmoDrag))
    gizmoDrag->mLineDirection = mOrientation->GetWorldForward();
}

//******************************************************************************
void ArrowGizmo::OnFrameUpdate(Event*)
{
  if(GetOwner()->has(Gizmo)->mActive == false)
    return;

  ByteColor color = ToByteColor(mColor);
  if(mGizmo->mMouseOver)
    color = ToByteColor(mHoverColor);

  Vec3 worldDirection = mOrientation->GetWorldForward();

  Vec3 worldStart = mTransform->GetWorldTranslation();
  Vec3 worldEnd = worldStart + worldDirection * mLength;

  bool boxHeads = mHeadType == ArrowHeadType::Cube;

    // lines/border
  gDebugDraw->Add(Debug::Line(worldStart, worldEnd).HeadSize(mHeadSize).Color(color)
                .OnTop(mDrawOnTop).ViewScaled(mViewScaled).DualHeads(mDualHeads)
                .BoxHeads(boxHeads)/*.BackShade(true)*/.Width(mLineDrawWidth).Border(true));

    // fill
  gDebugDraw->Add(Debug::Line(worldStart, worldEnd).HeadSize(mHeadSize).Color(color)
                .OnTop(mDrawOnTop).ViewScaled(mViewScaled).DualHeads(mDualHeads)
                .BoxHeads(boxHeads).Filled(mFilledHeads)/*.BackShade(true)*/);
}

//------------------------------------------------------------- Ring Gizmo Event
ZilchDefineType(RingGizmoEvent, builder, type)
{
  ZilchBindFieldProperty(mWorldRotation);
  ZilchBindFieldProperty(mWorldRotationAxis);
  ZilchBindFieldProperty(mRadiansAroundAxis);
  ZilchBindFieldProperty(mDeltaRadiansAroundAxis);
}

//******************************************************************************
RingGizmoEvent::RingGizmoEvent(GizmoUpdateEvent* e)
  : GizmoUpdateEvent(*e)
{

}

//------------------------------------------------------------------- Ring Gizmo
ZilchDefineType(RingGizmo, builder, type)
{
  ZeroBindComponent();
  // Used for defining the axis the ring is around
  ZeroBindDependency(Orientation);

  ZilchBindFieldProperty(mRadius);
  ZilchBindFieldProperty(mSelectRadius);
  ZilchBindFieldProperty(mDragRadiansPerPixel);

  ZilchBindFieldProperty(mBackShade);
  ZilchBindFieldProperty(mViewAligned);

  ZilchBindFieldProperty(mGrabArrowColor);
  ZilchBindFieldProperty(mGrabArrowLength);
  ZilchBindFieldProperty(mGrabArrowHeadSize);
  ZilchBindFieldProperty(mGrabArrowViewScaled);
  ZilchBindFieldProperty(mGrabArrowOnTop);

  ZeroBindEvent(Events::RingGizmoModified, RingGizmoEvent);
}

//******************************************************************************
void RingGizmo::Serialize(Serializer& stream)
{
  SimpleGizmoBase::Serialize(stream);

  SerializeNameDefault(mRadius, 1.0f);
  SerializeNameDefault(mSelectRadius, 0.25f);
  SerializeNameDefault(mDragRadiansPerPixel, 0.01f);
  SerializeNameDefault(mBackShade, false);
  SerializeNameDefault(mViewAligned, false);

  SerializeNameDefault(mGrabArrowColor, ToFloatColor(Color::Yellow));
  SerializeNameDefault(mGrabArrowLength, 1.6f);
  SerializeNameDefault(mGrabArrowHeadSize, 0.12f);
  SerializeNameDefault(mGrabArrowViewScaled, true);
  SerializeNameDefault(mGrabArrowOnTop, true);
}

//******************************************************************************
void RingGizmo::Initialize(CogInitializer& initializer)
{
  SimpleGizmoBase::Initialize(initializer);

  mOrientation = GetOwner()->has(Orientation);

  ConnectThisTo(GetOwner(), Events::GizmoRayTest, OnGizmoRayTest);
  ConnectThisTo(GetOwner(), Events::GizmoPreDrag, OnGizmoPreDrag);
  ConnectThisTo(GetSpace(), Events::FrameUpdate, OnFrameUpdate);
  ConnectThisTo(GetOwner(), Events::GizmoModified, OnGizmoModified);
}

//******************************************************************************
void RingGizmo::OnGizmoRayTest(GizmoRayTestEvent* e)
{
  if(!mMouseInput)
    return;

  Camera* camera = e->mMouseEvent->GetCameraViewport()->mActiveCamera;
  if(camera == nullptr)
    return;

  Vec3 eyePosition = camera->mTransform->GetWorldTranslation();

  Transform* t = GetOwner()->has(Transform);
  Vec3 center = t->GetWorldTranslation();
  Vec3 pivot = center;

  if(mUseParentAsViewScaleOrigin)
  {
    if(Cog* parent = GetOwner( )->GetParent( ))
    {
      if(Transform* parentTransform = parent->has(Transform))
        pivot = parentTransform->GetWorldTranslation( );
    }
  }

  float viewScale = 1.0f;
  // Scale the gizmo by the view so it always appears the same size if specified
  if(mViewScaled)
    viewScale = GizmoGetViewScale(camera, pivot);

  center = pivot + ((center - pivot) * viewScale);

  float sphereRadius = mRadius * viewScale;
  float tubeRadius = mSelectRadius * viewScale;

  // Expand the sphere buy the torus radius
  float outerSphereRadius = sphereRadius + tubeRadius;

  Vec3 axis = mOrientation->GetWorldForward();

  // The cylinder should be aligned towards the camera if we're view aligned
  if(mViewAligned)
    axis = (eyePosition - center).Normalized();

  Ray mouseRay = e->GetWorldRay();

  Intersection::Interval interval;
  Intersection::Type result = Intersection::RayCylinder(mouseRay.Start, mouseRay.Direction,
    center - axis * tubeRadius, center + axis * tubeRadius, outerSphereRadius, &interval);

  Intersection::IntersectionPoint point;
  if(interval.Min > real(0.0))
  {
    point.Points[0] = mouseRay.Start + interval.Min * mouseRay.Direction;
    point.Points[1] = mouseRay.Start + interval.Max * mouseRay.Direction;
    point.T = interval.Min;
  }
  else if(interval.Max > real(0.0))
  {
    point.Points[0] = mouseRay.Start + interval.Max * mouseRay.Direction;
    point.Points[1] = point.Points[0];
    point.T = interval.Max;
  }

  //if(result != Intersection::None)
  {
    const float cAngleSlop = 0.3f;

    // Check both points of the ray intersection
    // We need to check where the ray left the cylinder to properly
    // pick the inside of the ring on the side facing away from the view
    for(uint i = 0; i < 2; ++i)
    {
      Vec3 dirToIntersection = point.Points[i] - center;
      float pointToAxisProjection = Dot(axis, dirToIntersection);

        // Intersection point is outside cylinder caps
        //  - NOTE: this is to catch the degenerate case (false positive from
        //          'RayCylinder') when the caps are parallel to the ray
        //             > ex: orthographic view
        //
        //  - @RYAN: remove this when 'RayCylinder gets fixed'
      if(pointToAxisProjection*pointToAxisProjection - tubeRadius*tubeRadius > 0.000001f)
        continue;

      Vec3 pointAlongAxis = center + pointToAxisProjection * axis;
      float distance = (point.Points[i] - pointAlongAxis).Length( );

      // Check to see if the intersection is with the tube radius
      // of the edge to make it a donut
      float distanceToEdge = Math::Abs(sphereRadius - distance);
      bool withinEdge = (distanceToEdge <= tubeRadius+0.0001f);

      dirToIntersection.AttemptNormalize( );
      real angle = Dot(dirToIntersection, mouseRay.Direction);

      // Only accept intersections of the side of the
      // sphere facing the view
      bool frontFacing = (angle < cAngleSlop);
      if(withinEdge && (!mBackShade || frontFacing))
      {
        e->RegisterResult(GetOwner(), point.T, mPickingPriority);

        mGrabPoint = point.Points[i];

        // Snap to plane of ring
        Plane ringPlane(axis, center);
        Intersection::ClosestPointOnPlaneToPoint(ringPlane.GetNormal(),
          ringPlane.GetDistance(), &mGrabPoint);

        // Snap to radius of ring
        Vec3 grabDir = mGrabPoint - center;
        mGrabPoint = center + grabDir.Normalized() * sphereRadius;

        // Compute the axis of movement for the mouse
        mGrabMoveAxis = Math::Cross(axis, (mGrabPoint - center).Normalized());
        mGrabMoveAxis.Normalize();

        // No need to check the other point
        break;
      }
    }
  }
}

//******************************************************************************
void RingGizmo::OnGizmoPreDrag(GizmoEvent* e)
{
  if(GizmoDrag* gizmoDrag = GetOwner()->has(GizmoDrag))
    gizmoDrag->mLineDirection = mGrabMoveAxis;

  mPreviousMouseDragSample = mMouseDragStart = e->mMouseEvent->Position;
  mWorldRotationAxis = mOrientation->GetWorldForward();
}

//******************************************************************************
void RingGizmo::OnGizmoModified(GizmoUpdateEvent* e)
{
  ViewportMouseEvent* mouseEvent = e->mMouseEvent;

  Viewport* viewport = mouseEvent->GetViewport();
  Vec2 onScreen0 = viewport->WorldToScreen(mGrabPoint);
  Vec2 onScreen1 = viewport->WorldToScreen(mGrabPoint + mGrabMoveAxis * 0.01f);

  Vec2 mouseDelta = e->mMouseEvent->Position - mMouseDragStart;
  Vec2 screenDir = (onScreen1 - onScreen0).Normalized();
  float moveOnAxis = Dot(mouseDelta, screenDir);
  moveOnAxis *= mDragRadiansPerPixel;

  mouseDelta = e->mMouseEvent->Position - mPreviousMouseDragSample;
  float deltaOnAxis = Dot(mouseDelta, screenDir);
  deltaOnAxis *= mDragRadiansPerPixel;

  mPreviousMouseDragSample = e->mMouseEvent->Position;

  // Construct a rotation using angle basis on the mouse movement
  // and axis of the currently selected axis for the world rotation
  Quat worldRotation;
  Math::ToQuaternion(mWorldRotationAxis, moveOnAxis, &worldRotation);

  RingGizmoEvent eventToSend(e);
  eventToSend.mWorldRotation = worldRotation;
  eventToSend.mWorldRotationAxis = mWorldRotationAxis;
  eventToSend.mRadiansAroundAxis = moveOnAxis;
  eventToSend.mDeltaRadiansAroundAxis = deltaOnAxis;

  GetOwner()->DispatchEvent(Events::RingGizmoModified, &eventToSend);
  GetOwner()->DispatchUp(Events::RingGizmoModified, &eventToSend);
}

//******************************************************************************
void RingGizmo::OnFrameUpdate(Event*)
{
  if(GetOwner()->has(Gizmo)->mActive == false)
    return;

  ByteColor color = ToByteColor(mColor);
  if(mGizmo->mMouseOver)
    color = ToByteColor(mHoverColor);

  Vec3 center = mTransform->GetWorldTranslation();
  Vec3 axis = mOrientation->GetWorldForward();
  Quat rot = mTransform->GetWorldRotation( );

  Vec3 viewScaleOffset(Vec3::cZero);

  if(mUseParentAsViewScaleOrigin)
    if(Cog* parent = GetOwner( )->GetParent( ))
      if(Transform* parentTransform = parent->has(Transform))
        viewScaleOffset = parentTransform->GetWorldTranslation( ) - center;

  // draw the ring gizmo
  {
    gDebugDraw->Add(Debug::Circle(center, axis, mRadius)
      .ViewScaled(mViewScaled)
      .ViewScaleOffset(viewScaleOffset)
      .BackShade(mBackShade)
      .OnTop(mDrawOnTop).Color(color)
      .ViewAligned(mViewAligned)
      .Border(mGizmo->mMouseOver));  // double thickness
  }

  if(mGizmo->mMouseOver)
  {
    Vec3 end0 = mGrabPoint + mGrabMoveAxis * mGrabArrowLength * 0.5f;
    Vec3 end1 = mGrabPoint - mGrabMoveAxis * mGrabArrowLength * 0.5f;

    gDebugDraw->Add(Debug::Line(mGrabPoint, end0).ViewScaled(mGrabArrowViewScaled)
      .HeadSize(mGrabArrowHeadSize)
      .Color(ToByteColor(mGrabArrowColor))
      .OnTop(mGrabArrowOnTop)
      /*.Border(true)*/);  // double thickness
    gDebugDraw->Add(Debug::Line(mGrabPoint, end1).ViewScaled(mGrabArrowViewScaled)
      .HeadSize(mGrabArrowHeadSize)
      .Color(ToByteColor(mGrabArrowColor))
      .OnTop(mGrabArrowOnTop)
      /*.Border(true)*/);  // double thickness
  }
}

//-------------------------------------------------------------- Translate Gizmo
ZilchDefineType(TranslateGizmo, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindTag(Tags::Gizmo);
  ZeroBindDocumented();

  ZeroBindDependency(Transform);

  ZilchBindFieldProperty(mUpdateMode);
  ZilchBindGetterSetterProperty(Snapping);
  ZilchBindFieldProperty(mSnapMode);
  ZilchBindFieldProperty(mSnapDistance);
}

//******************************************************************************
void TranslateGizmo::Serialize(Serializer& stream)
{
  SerializeEnumNameDefault(UpdateMode, mUpdateMode, UpdateMode::TranslateSelf);
  SerializeNameDefault(mSnapping, false);
  SerializeEnumName(GizmoSnapMode, mSnapMode);
  SerializeNameDefault(mSnapDistance, 0.25f);
}

//******************************************************************************
void TranslateGizmo::Initialize(CogInitializer& initializer)
{
  Cog* owner = GetOwner();

  mTransform = owner->has(Transform);

  ConnectThisTo(owner, Events::MouseDragStart, OnMouseDragStart);
  ConnectThisTo(owner, Events::GizmoModified, OnGizmoModified);
}

//******************************************************************************
bool TranslateGizmo::GetSnapping( )
{
  // Shift key modifies the snapping flag temporarily
  bool nonShiftModified = (mSnapping && Keyboard::Instance->KeyIsUp(Keys::Shift));
  bool tempSnappingOn = (!mSnapping && Keyboard::Instance->KeyIsDown(Keys::Shift));

  return (nonShiftModified || tempSnappingOn);
}

//******************************************************************************
void TranslateGizmo::SetSnapping(bool snapping)
{
  mSnapping = snapping;
}

//******************************************************************************
void TranslateGizmo::OnMouseDragStart(Event* e)
{
  mStartPosition = mTransform->GetWorldTranslation();
}

//******************************************************************************
void TranslateGizmo::OnGizmoModified(GizmoUpdateEvent* e)
{
  // Don't need to do anything if we aren't updating ourselves
  if(mUpdateMode == UpdateMode::None)
    return;

  Cog* objectToUpdate = GetOwner();

  if(mUpdateMode == UpdateMode::TranslateRoot)
    objectToUpdate = GetOwner()->FindRoot();

  Transform* t = objectToUpdate->has(Transform);
  ReturnIf(t == nullptr, , "TranslateGizmo has no Transform. This should never happen.");

  // Pull the drag mode off the gizmo
  Cog* gizmo = e->GetGizmo();
  GizmoDrag* gizmoDrag = gizmo->has(GizmoDrag);
  ReturnIf(gizmoDrag == nullptr, , "Dragging gizmo has no GizmoDrag Component.");

  mDragMode = gizmoDrag->mDragMode;

  Vec3 delta = ((mStartPosition + e->mMouseWorldMovement) - t->GetWorldTranslation( ));

  Vec3 newPosition = TranslateFromDrag(gizmoDrag, t->GetWorldTranslation( ),
    delta, t->GetWorldRotation());

  // Update the final translation
  t->SetWorldTranslation(newPosition);
}

//******************************************************************************
Vec3 TranslateGizmo::TranslateFromDrag(GizmoDrag* gizmoDrag, Vec3Param startPosition,
  Vec3Param localMovement, QuatParam rotation)
{
  Vec3 newPosition;
  if(GetSnapping( ))
  {
    newPosition = GizmoSnapping::GetSnappedPosition(startPosition,
      localMovement, rotation, gizmoDrag->mDragMode, mSnapMode, mSnapDistance);
  }
  else
  {
    newPosition = startPosition + localMovement;
  }

  return newPosition;
}

//******************************************************************************
Vec3 TranslateGizmo::GetStartPosition()
{
  return mStartPosition;
}

//------------------------------------------------------------------ Scale Gizmo

ZilchDefineType(ScaleGizmo, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindTag(Tags::Gizmo);
  ZeroBindDocumented( );

  ZeroBindDependency(Transform);

  ZilchBindGetterSetterProperty(Snapping);
  ZilchBindFieldProperty(mSnapMode);
  ZilchBindFieldProperty(mSnapDistance);

  //outputs of gizmo modified
  ZilchBindField(mDirection);
  ZilchBindFieldProperty(mChangeInScale);
}

//******************************************************************************
void ScaleGizmo::Serialize(Serializer& stream)
{
  SerializeNameDefault(mSnapping, false);
  SerializeEnumName(GizmoSnapMode, mSnapMode);
  SerializeNameDefault(mSnapDistance, 1.0f);
}

//******************************************************************************
void ScaleGizmo::Initialize(CogInitializer& initializer)
{
  ConnectThisTo(GetOwner(), Events::MouseDragStart, OnMouseDragStart);
  ConnectThisTo(GetOwner(), Events::GizmoModified, OnGizmoModified);
}

//******************************************************************************
bool ScaleGizmo::GetSnapping( )
{
  // Shift key modifies the snapping flag temporarily
  bool nonShiftModified = (mSnapping && Keyboard::Instance->KeyIsUp(Keys::Shift));
  bool tempSnappingOn = (!mSnapping && Keyboard::Instance->KeyIsDown(Keys::Shift));

  return (nonShiftModified || tempSnappingOn);
}

//******************************************************************************
void ScaleGizmo::SetSnapping(bool snapping)
{
  mSnapping = snapping;
}

//******************************************************************************
void ScaleGizmo::OnMouseDragStart(ViewportMouseEvent* e)
{
  Viewport* viewport = e->GetViewport( );
  
  Mat3 rotation = Math::ToMatrix3(viewport->GetCamera()->mTransform->GetWorldRotation( ));
  mEyeDirection = -rotation.BasisZ( );

  mChangeInScale.Set(1, 1, 1);
}

//******************************************************************************
void ScaleGizmo::OnGizmoModified(GizmoUpdateEvent* e)
{
  Cog* owner = GetOwner( );

  Transform* t = owner->has(Transform);
  ReturnIf(t == nullptr, , "ScaleGizmo has no Transform. This should never happen.");

  GizmoDrag* gizmoDrag = e->GetGizmo( )->has(GizmoDrag);
  ReturnIf(gizmoDrag == nullptr, , "Dragging gizmo has no GizmoDrag Component.");

  mDragMode = gizmoDrag->mDragMode;

  // Transform of 'this' gizmo.
  Mat3 m = Math::ToMatrix3(t->GetWorldRotation( ));

  Vec3 worldMovement = e->mMouseWorldMovement;
  Vec3 localMovement = Math::Transform(m.Inverted( ), worldMovement);

  // The speed at which we scale will be determined by how far away
  // it was grabbed from its center
  Vec3 grabDirection = (e->mInitialGrabPoint - t->GetWorldTranslation( ));
  float distance = grabDirection.Length( );

  bool viewPlaneGizmo = (mDragMode == GizmoDragMode::ViewPlane);

  mDirection.Set(1, 1, 1);
  mViewPlaneMove = 0.0f;

  Vec3 startScale(1, 1, 1);

    // Dragging on view plane.
  if(viewPlaneGizmo)
  {
    Vec3 up = Vec3(0, 1, 0);
    Vec3 side = Cross(up, mEyeDirection);
    Vec3 screenUp = Cross(side, mEyeDirection);
    float a = Dot(side, worldMovement);
    float b = Dot(screenUp, worldMovement);

      // Line from top-left to bottom-right, across view-plane, passing through grab-point.
      //  - Drag movement in half-space to the left is scaling down
      //  - Drag movement in half-space to the right is scaling up
    mViewPlaneMove = -Dot(Vec2(a, b), Vec2(1, 1)) * distance;

      // If false: 0 * 2 - 1 = -1 [ie, dir is scaling down].
      // If true:  1 * 2 - 1 =  1 [ie, dir is scaling up].
    float scalingDir = (float)((mViewPlaneMove > 0.0f) * 2 - 1);
    mDirection.Set(scalingDir, scalingDir, scalingDir);

    Vec3 viewPlaneVector(mViewPlaneMove, mViewPlaneMove, mViewPlaneMove);
    mScaledLocalMovement = GizmoHelpers::ScaleVector(viewPlaneVector, distance, startScale);
  }
  else  // Dot on all gizmo-basis axes.
  {
    for(int axis = 0; axis < 3; ++axis)
    {
      Vec3 v = m.GetBasis(axis).AttemptNormalized( );

      float moveDelta = Math::Dot(v, grabDirection + worldMovement);
      float grab = Math::Dot(v, grabDirection);

        // If false: 0 * 2 - 1 = -1 [ie, dir is scaling down].
        // If true:  1 * 2 - 1 =  1 [ie, dir is scaling up].
      mDirection[axis] = (float)((moveDelta > grab) * 2 - 1);
    }

    mScaledLocalMovement = GizmoHelpers::ScaleVector(localMovement, distance, startScale);
  }

  mChangeInScale = ScaleFromDrag(gizmoDrag, distance, localMovement,
    startScale, t->GetWorldRotation());
}

//******************************************************************************
Vec3 ScaleGizmo::ScaleFromDrag(GizmoDrag* gizmoDrag, float distance,
  Vec3Param localMovement, Vec3Param startScale, QuatParam rotation)
{
  GizmoDragMode::Enum dragMode = gizmoDrag->mDragMode;
  bool viewPlaneGizmo = (dragMode == GizmoDragMode::ViewPlane);

  // Off-axis scale if control is pressed
  //   - [ie, scale on each axis except the one being dragged]
  bool singleAxis = (dragMode == GizmoDragMode::Line);
  bool offAxis = (Keyboard::Instance->KeyIsDown(Keys::Control) && singleAxis);

    // Generate a new scale based on drag-type [ie: viewplane, gizmo-basis-plane, gizmo-axis]
  Vec3 newScale;
  if(viewPlaneGizmo)
  {
    Vec3 viewPlaneVector(mViewPlaneMove, mViewPlaneMove, mViewPlaneMove);
    newScale = GizmoHelpers::ScaleVector(viewPlaneVector, distance, startScale);
  }
  else
  {
    Vec3 movement(localMovement);

      // Negative scale not allowed.
    movement = Math::Abs(movement);

      // Scaling up or down?
    movement = mDirection * movement;

      // Special command.
    if(offAxis)
      movement = GizmoHelpers::GetOffAxisMovement(movement);

    newScale = GizmoHelpers::ScaleVector(movement, distance, startScale);
  }

  Vec3 scaleDelta = (newScale - startScale);
  newScale = startScale + scaleDelta;

  if(GetSnapping())
  {
    if(offAxis)
    {
      // Plane will snap on each off-axis individually if snapMode is RelativeUnits.
      newScale = GizmoSnapping::GetSnappedPosition(startScale, scaleDelta,
        rotation, GizmoDragMode::Plane, mSnapMode, mSnapDistance);
    }
    else
    {
      newScale = GizmoSnapping::GetSnappedPosition(startScale, scaleDelta,
        rotation, dragMode, mSnapMode, mSnapDistance);
    }

  }

  return newScale;
}

//------------------------------------------------------------------Rotate Gizmo
ZilchDefineType(RotateGizmo, builder, type)
{
  ZeroBindComponent( );
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindTag(Tags::Gizmo);
  ZeroBindDocumented( );

  ZeroBindDependency(Transform);

  ZilchBindGetterSetterProperty(Snapping);
  ZilchBindFieldProperty(mSnapAngle);

  //outputs of gizmo modified
  ZilchBindField(mDirection);
  ZilchBindField(mChangeInRotation);
}

//******************************************************************************
void RotateGizmo::Serialize(Serializer& stream)
{
  SerializeNameDefault(mSnapping, false);
  SerializeNameDefault(mSnapAngle, 15.0f);
}

//******************************************************************************
void RotateGizmo::Initialize(CogInitializer& initializer)
{
  ConnectThisTo(GetOwner( ), Events::MouseDragStart, OnMouseDragStart);
  ConnectThisTo(GetOwner( ), Events::RingGizmoModified, OnGizmoModified);
}

//******************************************************************************
bool RotateGizmo::GetSnapping( )
{
  // Shift key modifies the snapping flag temporarily
  bool nonShiftModified = (mSnapping && Keyboard::Instance->KeyIsUp(Keys::Shift));
  bool tempSnappingOn = (!mSnapping && Keyboard::Instance->KeyIsDown(Keys::Shift));

  return (nonShiftModified || tempSnappingOn);
}

//******************************************************************************
void RotateGizmo::SetSnapping(bool snapping)
{
  mSnapping = snapping;
}

//******************************************************************************
void RotateGizmo::OnMouseDragStart(ViewportMouseEvent* e)
{
  mPreviousSnap = 0.0f;
  mChangeInRotation = 0.0f;
}

//******************************************************************************
void RotateGizmo::OnGizmoModified(RingGizmoEvent* e)
{
  Transform* t = GetOwner()->has(Transform);
  ReturnIf(t == nullptr, , "RotateGizmo has no Transform. This should never happen.");

  mDirection = e->mRadiansAroundAxis - mChangeInRotation;
  mDirection /= Math::Abs(mDirection);

  mChangeInRotation = e->mRadiansAroundAxis;

  float deltaOnAxis = e->mDeltaRadiansAroundAxis;
  Vec3 selectedAxis = e->mWorldRotationAxis;

  if(GetSnapping())
  {
    mChangeInRotation = Snap(mChangeInRotation, Math::DegToRad(mSnapAngle));

    float deltaSnap = Math::Abs(mChangeInRotation) - Math::Abs(mPreviousSnap);
    if(deltaSnap > Math::Epsilon( ) || deltaSnap < -Math::Epsilon( ))
    {
      deltaOnAxis = Math::DegToRad((mChangeInRotation - mPreviousSnap < 0.0f) ? -mSnapAngle : mSnapAngle);
      mPreviousSnap = mChangeInRotation;
    }
    else
    {
      mPreviousSnap = mChangeInRotation;
      return;
    }

  }

  Quat deltaRotation;
  Math::ToQuaternion(selectedAxis, deltaOnAxis, &deltaRotation);
  // sanity normalize to prevent rounding errors
  deltaRotation.Normalize( );

  // Rotate display
  t->RotateWorld(deltaRotation);
}

}//namespace Zero
