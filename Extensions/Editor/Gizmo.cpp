///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//------------------------------------------------------------------------- Tags
namespace Tags
{
  DefineTag(Gizmo);
}

namespace Events
{
  DefineEvent(GizmoRayTest);
  DefineEvent(GizmoModified);
  DefineEvent(GizmoTargetSet);
  DefineEvent(MouseEnterGizmo);
  DefineEvent(MouseEnterGizmoHierarchy);
  DefineEvent(MouseExitGizmo);
  DefineEvent(MouseExitGizmoHierarchy);
}

//--------------------------------------------------------- Gizmo Ray Test Event
ZilchDefineType(GizmoRayTestEvent, builder, type)
{
}
//******************************************************************************
GizmoRayTestEvent::GizmoRayTestEvent() :
  mGizmoHit(nullptr),
  mHitDistance(Math::PositiveMax()),
  mPickingPriority(-Math::IntegerNegativeMin())
{
  
}

//******************************************************************************
void GizmoRayTestEvent::RegisterResult(Cog* gizmo, float distance,
                                       int pickingPriority)
{
  if(distance < mHitDistance && pickingPriority >= mPickingPriority)
  {
    mGizmoHit = gizmo;
    mHitDistance = distance;
    mPickingPriority = pickingPriority;
  }
}

//******************************************************************************
Ray GizmoRayTestEvent::GetWorldRay()
{
  return mMouseEvent->mWorldRay;
}

//------------------------------------------------------------------ Gizmo Event
ZilchDefineType(GizmoEvent, builder, type)
{
  ZilchBindGetterProperty(Gizmo);
  ZilchBindGetterProperty(ViewportMouseEvent);
}

//******************************************************************************
GizmoEvent::GizmoEvent(Cog* gizmoCog, ViewportMouseEvent* e) :
  mGizmo(gizmoCog),
  mMouseEvent(e)
{

}

//******************************************************************************
Cog* GizmoEvent::GetGizmo()
{
  return mGizmo;
}

//******************************************************************************
ViewportMouseEvent* GizmoEvent::GetViewportMouseEvent()
{
  return mMouseEvent;
}

//------------------------------------------------------------------------ Gizmo
ZilchDefineType(Gizmo, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindTag(Tags::Gizmo);

  ZilchBindGetterSetterProperty(Active);
  ZilchBindFieldProperty(mForwardEventsToChildren);
  ZilchBindGetterProperty(EditingObject);
  ZilchBindGetterProperty(MouseOver);

  ZeroBindEvent(Events::MouseEnterGizmo, GizmoEvent);
  ZeroBindEvent(Events::MouseEnterGizmoHierarchy, GizmoEvent);
  ZeroBindEvent(Events::MouseExitGizmo, GizmoEvent);
  ZeroBindEvent(Events::MouseExitGizmoHierarchy, GizmoEvent);
}

//******************************************************************************
Gizmo::Gizmo()
{
  mMouseOver = false;
}

//******************************************************************************
void Gizmo::Serialize(Serializer& stream)
{
  SerializeNameDefault(mActive, true);
  SerializeNameDefault(mForwardEventsToChildren, true);
}

//******************************************************************************
void Gizmo::Initialize(CogInitializer& initializer)
{
  GizmoSpace* gizmoSpace = HasOrAdd<GizmoSpace>(GetSpace());
  gizmoSpace->AddOrUpdateGizmo(GetOwner());
}

//******************************************************************************
void Gizmo::SetTargetObject(Cog* cog)
{
  mEditingObject = cog;
}

//******************************************************************************
void Gizmo::AttachTo(AttachmentInfo& info)
{
  GizmoSpace* gizmoSpace = GetSpace()->has(GizmoSpace);
  gizmoSpace->AddOrUpdateGizmo(GetOwner());
}

//******************************************************************************
void Gizmo::Detached(AttachmentInfo& info)
{
  GizmoSpace* gizmoSpace = GetSpace()->has(GizmoSpace);
  gizmoSpace->AddOrUpdateGizmo(GetOwner());
}

//******************************************************************************
// Some children in a Gizmo hierarchy may not have the Gizmo component, so
// we need to handle skipping over non-gizmo objects.
void ForwardGizmoEvent(Cog* cog, Event* e, bool forwardToChildren)
{
  // Dispatch the event on ourself
  cog->DispatchEvent(e->EventId, e);

  if(forwardToChildren)
  {
    forRange(Cog& child, cog->GetChildren())
    {
      if(Gizmo* childGizmo = child.has(Gizmo))
        childGizmo->ForwardEvent(e);
      else
        ForwardGizmoEvent(&child, e, true);
    }
  }
}

//******************************************************************************
void Gizmo::ForwardEvent(Event* e)
{
  // Do nothing if we aren't active
  if(!mActive)
    return;

  ForwardGizmoEvent(GetOwner(), e, mForwardEventsToChildren);
}

//******************************************************************************
Cog* Gizmo::GetEditingObject()
{
  Cog* cog = mEditingObject;
  return cog;
}

//******************************************************************************
void Gizmo::SetActive(bool state)
{
  mActive = state;

  // If we're being set to inactive, we need to check to see if the mouse is
  // currently over us
  if(state == false)
  {
    GizmoSpace* gizmoSpace = GetSpace()->has(GizmoSpace);
    if(gizmoSpace->mMouseOverGizmo == GetOwner()->GetId())
    {
      gizmoSpace->mMouseOverGizmo = nullptr;

      GizmoEvent eventToSend(GetOwner(), nullptr);
      GetOwner()->DispatchEvent(Events::MouseExitGizmo, &eventToSend);
    }
  }
}

//******************************************************************************
bool Gizmo::GetActive()
{
  return mActive;
}

//******************************************************************************
bool Gizmo::GetMouseOver()
{
  return mMouseOver;
}

//------------------------------------------------------------------ Gizmo Space
ZilchDefineType(GizmoSpace, builder, type)
{
  ZeroBindComponent();
  ZeroBindEvent(Events::GizmoTargetSet, ObjectEvent);
  type->AddAttribute(ObjectAttributes::cHidden);
  type->AddAttribute(ObjectAttributes::cCore);
}

//******************************************************************************
void GizmoSpace::Initialize(CogInitializer& initializer)
{

}

//******************************************************************************
void GizmoSpace::AddOrUpdateGizmo(Cog* cog)
{
  // Removing the object will handle the case of it used to be a root, but has
  // since been attached to a new object
  mRootGizmos.Erase(cog);
  mRootGizmos.Insert(cog->FindRoot());
}

//******************************************************************************
void GizmoSpace::OnMouseUpdate(ViewportMouseEvent* e)
{
  GizmoRayTestEvent rayEvent;
  rayEvent.EventId = Events::GizmoRayTest;
  rayEvent.mMouseEvent = e;

  ForwardEventToAllGizmos(&rayEvent);

  Cog* newGizmoCog = rayEvent.mGizmoHit;
  Cog* oldGizmoCog = mMouseOverGizmo;

  GizmoEvent newGizmoEvent(newGizmoCog, e);
  GizmoEvent oldGizmoEvent(oldGizmoCog, e);

  if(newGizmoCog)
  {
    Gizmo* newGizmo = newGizmoCog->has(Gizmo);

    // If it's a new Gizmo, send the mouse exit to the old one
    if(newGizmoCog != oldGizmoCog)
    {
      // In MouseExitGizmo we want to be able to look at what gizmo the mouse entered
      newGizmo->mMouseOver = true;

      if(oldGizmoCog)
      {
        // Mouse is no longer over the old cog
        if(Gizmo* oldGizmo = oldGizmoCog->has(Gizmo))
          oldGizmo->mMouseOver = false;

        oldGizmoCog->DispatchEvent(Events::MouseExitGizmo, &oldGizmoEvent);
        oldGizmoCog->DispatchEvent(Events::MouseExitGizmoHierarchy, &oldGizmoEvent);
        oldGizmoCog->DispatchUp(Events::MouseExitGizmoHierarchy, &oldGizmoEvent);
      }

      // Mouse has entered the new Gizmo
      newGizmoCog->DispatchEvent(Events::MouseEnterGizmo, &newGizmoEvent);
      newGizmoCog->DispatchEvent(Events::MouseEnterGizmoHierarchy, &newGizmoEvent);
      newGizmoCog->DispatchUp(Events::MouseEnterGizmoHierarchy, &newGizmoEvent);

      mMouseOverGizmo = newGizmoCog;
    }

    // No need to do anything if the mouse is still over the same Gizmo
  }
  // If there's no new Gizmo but there was an old one, we have left
  // the old gizmo
  else if(oldGizmoCog)
  {
    // Mouse is no longer over the old cog
    if(Gizmo* oldGizmo = oldGizmoCog->has(Gizmo))
      oldGizmo->mMouseOver = false;

    oldGizmoCog->DispatchEvent(Events::MouseExitGizmo, &newGizmoEvent);
    oldGizmoCog->DispatchEvent(Events::MouseExitGizmoHierarchy, &newGizmoEvent);
    oldGizmoCog->DispatchUp(Events::MouseExitGizmoHierarchy, &newGizmoEvent);
    mMouseOverGizmo = nullptr;
  }
}

//******************************************************************************
void GizmoSpace::ForwardEvent(Event* e)
{
  // Send all events to the mouse over Gizmo
  if(Cog* mouseOver = mMouseOverGizmo)
  {
    if(Gizmo* gizmo = mouseOver->has(Gizmo))
      gizmo->ForwardEvent(e);
  }
}

//******************************************************************************
void GizmoSpace::ForwardEventToAllGizmos(Event* e)
{
  forRange(CogId cogId, mRootGizmos.All())
  {
    if(Cog* cog = cogId)
    {
      if(Gizmo* gizmo = cog->has(Gizmo))
        gizmo->ForwardEvent(e);
    }
  }
}

}//namespace Zero
