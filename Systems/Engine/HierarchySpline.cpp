///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------HierarchySpline
ZilchDefineType(HierarchySpline, builder, type)
{
  ZeroBindComponent();
  ZeroBindSetup(SetupMode::DefaultSerialization);
  ZeroBindDocumented();
  
  ZeroBindEvent(Events::QuerySpline, SplineEvent);
  ZeroBindEvent(Events::SplineModified, SplineEvent);

  ZilchBindGetter(Spline);
  ZilchBindGetterSetterProperty(Closed)->ZeroSerialize(false);
  ZilchBindGetterSetterProperty(SplineType)->ZeroSerialize(SplineType::CatmulRom);
  ZilchBindGetterSetterProperty(Error)->ZeroSerialize(real(0.01f));
  ZilchBindGetterSetterProperty(DebugDrawSpline)->ZeroSerialize(true);
  ZilchBindGetterSetterProperty(SplineColor)->ZeroSerialize(Vec4(0, 0, 0, 1));
}

HierarchySpline::HierarchySpline()
{
  mSpline = new Spline();
  mSpline->SetError(real(0.01f));
  mDebugDrawSpline = true;
  mSplineColor = Vec4(0, 0, 0, 1);
  mIsModified = true;
}

void HierarchySpline::Serialize(Serializer& stream)
{
  MetaSerializeProperties(this, stream);
}

void HierarchySpline::Initialize(CogInitializer& initializer)
{
  Component::Initialize(initializer);

  Cog* owner = GetOwner();
  // Listen for someone who wants to know if we have a spline
  ConnectThisTo(owner, Events::QuerySpline, OnQuerySpline);
  // Listen for any children changing to so we can recompute the spline
  ConnectThisTo(owner, Events::ChildAttached, OnChildAttached);
  ConnectThisTo(owner, Events::ChildDetached, OnChildDetached);
  ConnectThisTo(owner, Events::ChildrenOrderChanged, OnMarkModified);
  ConnectThisTo(GetSpace(), Events::FrameUpdate, OnFrameUpdate);
}

void HierarchySpline::OnAllObjectsCreated(CogInitializer& initializer)
{
  GetChildrenConnections();
  mIsModified = true;
}

void HierarchySpline::DebugDraw()
{
  if(!mDebugDrawSpline)
    return;

  mSpline->DebugDraw(mSplineColor);
}

Spline* HierarchySpline::GetSpline() const
{
  return mSpline;
}

bool HierarchySpline::GetClosed() const
{
  return mSpline->GetClosed();
}

void HierarchySpline::SetClosed(bool closed)
{
  mSpline->SetClosed(closed);
  OnMarkModified(nullptr);
}

SplineType::Enum HierarchySpline::GetSplineType() const
{
  return mSpline->GetSplineType();
}

void HierarchySpline::SetSplineType(SplineType::Enum splineType)
{
  mSpline->SetSplineType(splineType);
  OnMarkModified(nullptr);
}

real HierarchySpline::GetError() const
{
  return mSpline->GetError();
}

void HierarchySpline::SetError(real error)
{
  mSpline->SetError(error);
  OnMarkModified(nullptr);
}

bool HierarchySpline::GetDebugDrawSpline() const
{
  return mDebugDrawSpline;
}

void HierarchySpline::SetDebugDrawSpline(bool debugDrawSpline)
{
  mDebugDrawSpline = debugDrawSpline;
}

Vec4 HierarchySpline::GetSplineColor() const
{
  return mSplineColor;
}

void HierarchySpline::SetSplineColor(Vec4 splineColor)
{
  mSplineColor = splineColor;
}

void HierarchySpline::RebuildIfModified()
{
  if(!mIsModified)
    return;

  ForceRebuild();
}

void HierarchySpline::ForceRebuild()
{
  mIsModified = false;
  // Remove all of the old control points
  SplineControlPoints* controlPoints = mSpline->GetControlPoints();
  controlPoints->Clear();
  // Add all children's positions as control points
  AutoDeclare(childrenRange, GetOwner()->GetChildren());
  for(; !childrenRange.Empty(); childrenRange.PopFront())
  {
    Cog& child = childrenRange.Front();
    // When we're rebuilding because of a child being destroyed, the child
    // still exists until the end of the function call. To avoid this we simply
    // skip any child that is being destroyed.
    if(child.GetMarkedForDestruction())
      continue;

    // Add the world translation of this cog (if it has a transform)
    Transform* transform = child.has(Transform);
    if(transform != nullptr)
      controlPoints->Add(transform->GetWorldTranslation());
  }

  // Notify anyone who cares that the spline was modified
  SplineEvent toSend;
  toSend.mSpline = mSpline;
  DispatchEvent(Events::SplineModified, &toSend);
}

real HierarchySpline::GetTotalDistance() const
{
  return mSpline->GetTotalDistance();
}

SplineSampleData HierarchySpline::SampleDistance(real distance)
{
  return mSpline->SampleDistance(distance);
}

void HierarchySpline::OnQuerySpline(SplineEvent* e)
{
  e->mSpline = mSpline;
}

void HierarchySpline::OnFrameUpdate(UpdateEvent* e)
{
  // To avoid traversing children multiple times during transform updates, 
  // we rebuild every frame. The user can also manually rebuild if they desire
  RebuildIfModified();
  DebugDraw();
}

void HierarchySpline::OnChildAttached(HierarchyEvent* e)
{
  // A new child was added to us, connect all the required evens and mark ourself as modified
  ConnectChildEvents(e->Child);
  mIsModified = true;
}

void HierarchySpline::OnChildDetached(HierarchyEvent* e)
{
  // A child was removed, stop listening to it and mark ourself as modified
  DisconnectAll(e->Child, this);
  mIsModified = true;
}

void HierarchySpline::OnMarkModified(Event* e)
{
  mIsModified = true;
}

void HierarchySpline::ConnectChildEvents(Cog* child)
{
  // Disconnect any connections we have to this child (so we don't get duplicate events)
  DisconnectAll(child, this);
  // Listen for transform update so we can rebuild the spline when a child moves
  ConnectThisTo(child, Events::TransformUpdated, OnMarkModified);
  // Listen for when destroy is called on a child so we can rebuild the spline
  ConnectThisTo(child, Events::CogDestroy, OnMarkModified);
}

void HierarchySpline::GetChildrenConnections()
{
  // Add connections for all children
  AutoDeclare(childrenRange, GetOwner()->GetChildren());
  for(; !childrenRange.Empty(); childrenRange.PopFront())
  {
    Cog& child = childrenRange.Front();
    ConnectChildEvents(&child);
  }
}

}//namespace Zero
