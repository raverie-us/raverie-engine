///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//----------------------------------------------------------------------- Events
namespace Events
{
  DefineEvent(GizmoPreDrag);
}

//----------------------------------------------------------- Gizmo Update Event
ZilchDefineType(GizmoUpdateEvent, builder, type)
{
  ZilchBindFieldProperty(mMouseWorldMovement);
  ZilchBindFieldProperty(mInitialGrabPoint);
  ZilchBindFieldProperty(mMouseWorldDelta);
}

//******************************************************************************
GizmoUpdateEvent::GizmoUpdateEvent(Cog* gizmoCog, ViewportMouseEvent* e) :
  GizmoEvent(gizmoCog, e)
{

}

//------------------------------------------------------------------- Gizmo Drag
ZilchDefineType(GizmoDrag, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindTag(Tags::Gizmo);

  ZeroBindDependency(Gizmo);
  ZeroBindDependency(MouseCapture);

  ZeroBindEvent(Events::GizmoModified, GizmoUpdateEvent);
  ZeroBindEvent(Events::GizmoPreDrag, GizmoEvent);

  ZilchBindFieldProperty(mDragMode)->AddAttribute(PropertyAttributes::cInvalidatesObject);
  ZilchBindFieldProperty(mLineDirection)->ZeroFilterEquality(mDragMode, GizmoDragMode::Enum, GizmoDragMode::Line);
  ZilchBindFieldProperty(mPlaneNormal)->ZeroFilterEquality(mDragMode, GizmoDragMode::Enum, GizmoDragMode::Plane);
  ZilchBindFieldProperty(mNormalInWorld)->ZeroFilterEquality(mDragMode, GizmoDragMode::Enum, GizmoDragMode::Plane);
  ZilchBindGetter(GrabPoint);
  ZilchBindFieldProperty(mGrabMode);
  ZilchBindFieldProperty(mAutoDrag)->AddAttribute(PropertyAttributes::cInvalidatesObject);
  ZilchBindFieldProperty(mDragDistance)->ZeroFilterBool(mAutoDrag);

  ZilchBindGetter(DragActive);
}

//******************************************************************************
void GizmoDrag::Serialize(Serializer& stream)
{
  SerializeEnumNameDefault(GizmoDragMode, mDragMode, GizmoDragMode::ViewPlane);
  SerializeNameDefault(mLineDirection, Vec3::cXAxis);
  SerializeNameDefault(mPlaneNormal, Vec3::cYAxis);
  SerializeNameDefault(mNormalInWorld, false);
  SerializeEnumNameDefault(GizmoGrabMode, mGrabMode, GizmoGrabMode::Hold);
  SerializeNameDefault(mAutoDrag, true);
  SerializeNameDefault(mDragDistance, 6.0f);
}

//******************************************************************************
void GizmoDrag::Initialize(CogInitializer& initializer)
{
  mMouseDown = false;

  mTransform = GetOwner()->has(Transform);
  mMouseCapture = GetOwner()->has(MouseCapture);

  ConnectThisTo(GetOwner(), Events::MouseDragMove, OnMouseDragMove);
  ConnectThisTo(GetOwner(), Events::MouseDragEnd, OnMouseDragEnd);
  ConnectThisTo(GetOwner(), Events::LeftMouseDown, OnLeftMouseDown);
  ConnectThisTo(GetOwner(), Events::LeftMouseUp, OnLeftMouseUp);
  ConnectThisTo(GetOwner(), Events::MouseMove, OnMouseMove);
  ConnectThisTo(GetOwner(), Events::MouseExitGizmo, OnMouseExitGizmo);
  ConnectThisTo(GetOwner(), Events::LeftMouseDrag, OnLeftMouseDrag);
}

//******************************************************************************
Vec3 GetEyeDirection(ViewportMouseEvent* e)
{
  if(Camera* camera = e->GetCameraViewport()->mActiveCamera)
  {
    Mat3 rotation = Math::ToMatrix3(camera->mTransform->GetWorldRotation( ));
    Vec3 eyeDirection = -rotation.BasisZ( );

    return eyeDirection;
  }
  //Viewport* viewport = e->GetViewport();
  //ViewState* viewState = viewport->GetViewState();

  //if (viewState)
  //  return viewState->EyeDirection;
  //return Vec3::cZero;
  return Vec3::cZero;
}

//******************************************************************************
void GizmoDrag::StartDrag(ViewportMouseEvent* e)
{
  // Cannot start a new drag if one is already active
  if(GetDragActive())
  {
    DoNotifyException("Cannot start drag", "Drag already active.");
    return;
  }

  // Allow others to change properties before the drag starts
  GizmoEvent eventToSend(GetOwner(), e);
  GetOwner()->DispatchEvent(Events::GizmoPreDrag, &eventToSend);

  Vec3 position = mTransform->GetWorldTranslation();

  // Build the drag plane based on the current mode and properties
  if(mDragMode == GizmoDragMode::Line)
  {
    Vec3 eyeDirection = GetEyeDirection(e);
    Vec3 planeNormal = GetLineDragPlane(mLineDirection, eyeDirection);

    // Store the plane 
    mDragPlane.Set(planeNormal, position);
  }
  else if(mDragMode == GizmoDragMode::Plane)
  {
    Vec3 worldNormal = mPlaneNormal;
    if(!mNormalInWorld)
      worldNormal = mTransform->TransformNormal(mPlaneNormal);
    mDragPlane.Set(worldNormal, position);
  }
  else // mDragMode == GizmoDragMode::ViewPlane
  {
    Camera* camera = e->GetCameraViewport()->mActiveCamera;
    if(camera == nullptr)
      return;

    Mat3 rotation = Math::ToMatrix3(camera->mTransform->GetWorldRotation( ));
    Vec3 eyeDirection = -rotation.BasisZ( );
    //ViewState* viewState = e->GetViewport()->GetViewState();
    //if(!viewState)
    //  return;

    Vec3 planeNormal = -eyeDirection;
    mDragPlane.Set(planeNormal, position);
  }

  // Test the ray against the plane to find the drag point
  mInitialGrabPoint = CastRayAgainstDragPlane(e->mWorldRay);

  // Start the mouse capture
  mMouseCapture->Capture(e);
  e->Handled = true;
}

//******************************************************************************
Vec3 GizmoDrag::GetLineDragPlane(Vec3Param dragDirection,Vec3Param eyeDirection)
{
  Vec3 a = Math::Cross(dragDirection, eyeDirection).Normalized();
  return Math::Cross(dragDirection, a).Normalized();
}

//******************************************************************************
Vec3 GizmoDrag::GetGrabPoint()
{
  return mInitialGrabPoint;
}

//******************************************************************************
bool GizmoDrag::GetDragActive()
{
  return mMouseCapture->IsCaptured();
}

//******************************************************************************
void GizmoDrag::OnLeftMouseDown(ViewportMouseEvent* e)
{
  if(mAutoDrag)
  {
    if(mGrabMode == GizmoGrabMode::Hold && !GetDragActive( ) && mDragDistance < Math::Epsilon())
    {
      StartDrag(e);
      return;
    }

    mMouseDown = true;
    mMouseDownPosition = e->Position;
    e->Handled = true;
  }

}

//******************************************************************************
void GizmoDrag::OnLeftMouseUp(ViewportMouseEvent* e)
{
  if(mAutoDrag)
  {
    if(mGrabMode == GizmoGrabMode::Toggle && !GetDragActive( ))
    {
      StartDrag(e);
      return;
    }

    e->Handled = true;
  }

  mMouseDown = false;
}

//******************************************************************************
void GizmoDrag::OnMouseMove(ViewportMouseEvent* e)
{
  // Check auto drag again in case it was changed since the mouse was down
  if(mAutoDrag && mGrabMode == GizmoGrabMode::Hold && mMouseDown && !GetDragActive())
  {
    float mouseDistance = Math::Distance(mMouseDownPosition, e->Position);

    if(mouseDistance > mDragDistance)
      StartDrag(e);
  }

}

//******************************************************************************
void GizmoDrag::OnLeftMouseDrag(MouseEvent* e)
{
  // Handle this so that the viewport does not start a drag selection
  e->Handled = true;
}

//******************************************************************************
void GizmoDrag::OnMouseExitGizmo(Event*)
{
  mMouseDown = false;
}

//******************************************************************************
Vec3 ProjectAOntoB(Vec3Param a, Vec3Param b)
{
  return (Dot(a, b) / b.Length()) * b;
}

//******************************************************************************
void GizmoDrag::OnMouseDragMove(ViewportMouseEvent* e)
{
  Viewport* viewport = e->GetViewport();

  // Get the new position on the drag plane
  Vec3 newPosition = CastRayAgainstDragPlane(e->mWorldRay);

  Vec3 movement = newPosition - mInitialGrabPoint;

  // Project the mouse movement to the movement axis
  if(mDragMode == GizmoDragMode::Line)
    movement = ProjectAOntoB(movement, mLineDirection);

  newPosition = mInitialGrabPoint;
  newPosition += movement;

  GizmoUpdateEvent eventToSend(GetOwner(), e);
  eventToSend.mMouseWorldMovement = movement;
  eventToSend.mInitialGrabPoint = mInitialGrabPoint;
  eventToSend.mMouseWorldDelta = newPosition - mPreviousMouseWorldPosition;

  mPreviousMouseWorldPosition = newPosition;

  GetOwner()->DispatchEvent(Events::GizmoModified, &eventToSend);
  GetOwner()->DispatchUp(Events::GizmoModified, &eventToSend);
}

//******************************************************************************
void GizmoDrag::OnMouseDragEnd(Event*)
{
  mMouseDown = false;
}

//******************************************************************************
Vec3 GizmoDrag::CastRayAgainstDragPlane(const Ray& worldRay)
{
  Intersection::IntersectionPoint intersection;
  Intersection::RayPlane(worldRay.Start, worldRay.Direction,
                         mDragPlane.GetNormal(), mDragPlane.GetDistance(),
                         &intersection);

  return intersection.Points[0];
}

}//namespace Zero
