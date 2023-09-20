// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

RaverieDefineType(ContactGraphEdge, builder, type)
{
  RaverieBindDefaultCopyDestructor();

  RaverieBindDocumented();

  RaverieBindGetterProperty(Object);
  RaverieBindGetterProperty(OtherObject);
  RaverieBindGetterProperty(IsGhost);
  RaverieBindGetterProperty(ContactPointCount);
  RaverieBindGetterProperty(ContactPoints);

  // RaverieBindGetterProperty(IsNew);
  RaverieBindGetterProperty(FirstPoint);
}

ContactGraphEdge::ContactGraphEdge(EdgeType* edge) : BaseType(edge)
{
}

bool ContactGraphEdge::GetIsGhost()
{
  return BaseType::GetIsGhost();
}

uint ContactGraphEdge::GetContactPointCount()
{
  return mConstraint->GetContactCount();
}

ContactPointRange ContactGraphEdge::GetContactPoints()
{
  // Find out if we're object 0 or object 1 in the manifold
  uint objectIndex = 0;
  if (mConstraint->mManifold->Objects[0] != GetCollider())
    objectIndex = 1;

  ContactPointRange range;
  range.Set(mConstraint->mManifold, objectIndex);
  return range;
}

ContactPoint ContactGraphEdge::GetFirstPoint()
{
  // Find out if we're object 0 or object 1 in the manifold
  uint objectIndex = 0;
  if (mConstraint->mManifold->Objects[0] != GetCollider())
    objectIndex = 1;

  ContactPoint proxyPoint;
  proxyPoint.Set(mConstraint->mManifold, &mConstraint->mManifold->Contacts[0], objectIndex);
  return proxyPoint;
}

bool ContactGraphEdge::GetIsNew()
{
  return mConstraint->GetIsNew();
}

bool ContactGraphEdge::GetSkipsResolution()
{
  return mConstraint->GetSkipResolution();
}

Physics::Contact& ContactGraphEdge::GetContact()
{
  return GetConstraint();
}

RaverieDefineType(JointGraphEdge, builder, type)
{
  RaverieBindDefaultCopyDestructor();
  RaverieBindDocumented();

  RaverieBindGetterProperty(Object);
  RaverieBindGetterProperty(OtherObject);

  RaverieBindGetterProperty(Valid);
  RaverieBindGetterProperty(Joint);
  RaverieBindGetterProperty(Owner);
}

typedef BodyFilterPolicy<Joint> JointBodyFilter;

JointBodyFilter::GraphEdgeType JointBodyFilter::CreateGraphEdge(EdgeType* edge)
{
  return GraphEdgeType(edge);
}

void JointBodyFilter::SkipDead(RangeType& range)
{
  while (!range.Empty())
  {
    RigidBody* body = range.Front().mOther->GetActiveBody();
    if (body && mBodies.Find(body).Empty())
    {
      mBodies.Insert(body);
      break;
    }
    range.PopFront();
  }
}

typedef BodyFilterPolicy<Physics::Contact> ContactBodyFilter;

ContactBodyFilter::GraphEdgeType ContactBodyFilter::CreateGraphEdge(EdgeType* edge)
{
  return GraphEdgeType(edge);
}

void ContactBodyFilter::SkipDead(RangeType& range)
{
  while (!range.Empty())
  {
    RigidBody* body = range.Front().mOther->GetActiveBody();
    if (body && mBodies.Find(body).Empty())
    {
      mBodies.Insert(body);
      break;
    }
    range.PopFront();
  }
}

// functions
InList<JointEdge, &JointEdge::ColliderLink>::range GetJointEdges(Collider* collider)
{
  return collider->mJointEdges.All();
}

ContactRange FilterContactRange(Collider* collider)
{
  return ContactRange(collider->mContactEdges.All());
}

} // namespace Raverie
