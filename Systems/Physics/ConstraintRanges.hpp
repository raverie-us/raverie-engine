///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//-------------------------------------------------------------------BaseConstraintGraphEdge
/// A bi-directional graph edge between a collider and a constraint. This is a helper class 
/// to wrap edges for traversing constraints. Provides helpers for traversing from the edge
/// to the connected objects. Also exposes helpers to the underlying constraint.
template <typename ConstraintType, typename EdgeType>
struct BaseConstraintGraphEdge
{
  typedef ConstraintType ConstraintTypeDef;

  BaseConstraintGraphEdge()
  {
    mEdge = nullptr;
    mConstraint = nullptr;
  }

  BaseConstraintGraphEdge(EdgeType* edge)
  {
    mEdge = edge;
    mConstraint = static_cast<ConstraintType*>(edge->mJoint);
  }

  ConstraintType& GetConstraint()
  {
    return *mConstraint;
  }

  /// Returns whether or not this was a ghost constraint.
  bool GetIsGhost()
  {
    return mConstraint->GetGhost();
  }

  /// Returns the collider that is iterating through its constraints.
  Collider* GetCollider()
  {
    return mEdge->mCollider;
  }

  /// Returns the Cog that is iterating through its constraints.
  Cog* GetObject()
  {
    return mEdge->mCollider->GetOwner();
  }

  /// Returns the other collider in the constraint. Since this edge is part of a
  /// range on one collider, we know who the other collider is.
  Collider* GetOtherCollider()
  {
    return mEdge->mOther;
  }
  
  /// Returns the other Cog in the constraint.
  Cog* GetOtherObject()
  {
    return mEdge->mOther->GetOwner();
  }

  /// Returns the other RigidBody in the constraint. May be null.
  RigidBody* GetOtherBody()
  {
    return mEdge->mOther->GetActiveBody();
  }

protected:

  uint GetColliderIndex(Collider* collider)
  {
    if(collider == mConstraint->GetCollider(0))
      return 0;
    return 1;
  }

  uint GetObjectIndex(Cog* cog)
  {
    if(cog == mConstraint->GetCollider(0)->GetOwner())
      return 0;
    return 1;
  }

  EdgeType* mEdge;
  ConstraintType* mConstraint;
};

//-------------------------------------------------------------------ContactGraphEdge
/// A bi-directional graph edge between a collider and a contact.
/// Exposes some internals to Contact which currently can't be exposed.
struct ContactGraphEdge : public BaseConstraintGraphEdge<Physics::Contact, Physics::ContactEdge>
{
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  typedef Physics::Contact Contact;
  typedef Physics::ContactEdge EdgeType;
  typedef BaseConstraintGraphEdge<Contact, EdgeType> BaseType;

  ContactGraphEdge() {}
  ContactGraphEdge(EdgeType* edge);

  /// Was this a ghost collision?
  bool GetIsGhost();
  /// How many points of contact were in this collision.
  uint GetContactPointCount();
  /// Returns a range of all contact points in the collision.
  ContactPointRange GetContactPoints();
  /// Convenience function to return the first ContactPoint. Some logic only cares about
  /// one point of information. In a more general case all points should be iterated over.
  ContactPoint GetFirstPoint();
  
  // Internal helpers for C++
  bool GetIsNew();
  bool GetSkipsResolution();

  Contact& GetContact();
};

//-------------------------------------------------------------------BaseJointGraphEdge
/// Templated base for a graph edge between a joint type. Is specialized for individual joint types.
template <typename JointType>
struct BaseJointGraphEdge : public BaseConstraintGraphEdge<JointType, JointEdge>
{
  typedef JointType JointTypeDef;
  typedef JointEdge EdgeType;
  typedef BaseConstraintGraphEdge<JointType, EdgeType> BaseType;

  using BaseType::mConstraint;

  BaseJointGraphEdge() {}
  BaseJointGraphEdge(EdgeType* edge) : BaseType(edge) {}

  /// Is this joint valid? Invalid joints are ones missing connecting objects (or missing colliders).
  bool GetValid()
  {
    return mConstraint->GetValid();
  }

  /// The joint this edge connects to.
  JointType& GetJoint()
  {
    return BaseType::GetConstraint();
  }

  /// Returns the owning object of the joint edge.
  Cog* GetOwner()
  {
    return BaseType::GetConstraint().GetOwner();
  }
};

//-------------------------------------------------------------------JointGraphEdge
/// A bi-directional graph edge between a collider and a joint.
/// Exposes convenience functions for iterating over the graph.
struct JointGraphEdge : public BaseJointGraphEdge<Joint>
{
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  typedef BaseJointGraphEdge<Joint> BaseType;
  JointGraphEdge() {}
  JointGraphEdge(EdgeType* edge) : BaseType(edge) {}
};

//-------------------------------------------------------------------ConstraintGraphEdgePolicy
/// Policy for ConstraintGraphEdges to help write generic iteration code. This will
/// iterate over only Joints of the provided template type.
template <typename JointType>
struct ConstraintGraphEdgePolicy
{
  typedef JointType JointTypeDef;
  typedef BaseJointGraphEdge<JointType> GraphEdgeType;
  typedef typename GraphEdgeType::EdgeType EdgeType;
  typedef InList<EdgeType, &EdgeType::ColliderLink> EdgeList;
  typedef typename EdgeList::range RangeType;
  
  GraphEdgeType CreateGraphEdge(EdgeType* edge)
  {
    return GraphEdgeType(edge);
  }

  /// Skips over all joints that are not the template type.
  void SkipDead(RangeType& range)
  {
    while(!range.Empty() && range.Front().mJoint->GetJointType() != JointType::StaticGetJointType())
      range.PopFront();
  }
};

//-------------------------------------------------------------------ConstraintGraphEdgePolicy<Contact>
/// Specialization for contact's graph edge.
template <>
struct ConstraintGraphEdgePolicy<Physics::Contact>
{
  typedef Physics::Contact JointType;
  typedef ContactGraphEdge GraphEdgeType;
  typedef GraphEdgeType::EdgeType EdgeType;
  typedef InList<EdgeType,&EdgeType::ColliderLink> EdgeList;
  typedef EdgeList::range RangeType;

  GraphEdgeType CreateGraphEdge(EdgeType* edge)
  {
    return GraphEdgeType(edge);
  }

  void SkipDead(RangeType& range)
  {

  }
};

//-------------------------------------------------------------------ConstraintGraphEdgePolicy<Joint>
/// Specialization for the Joint's graph edges. Iterates over all joint types.
template <>
struct ConstraintGraphEdgePolicy<Joint>
{
  typedef Joint JointType;
  typedef JointGraphEdge GraphEdgeType;
  typedef GraphEdgeType::EdgeType EdgeType;
  typedef InList<EdgeType, &EdgeType::ColliderLink> EdgeList;
  typedef EdgeList::range RangeType;

  GraphEdgeType CreateGraphEdge(EdgeType* edge)
  {
    return GraphEdgeType(edge);
  }

  void SkipDead(RangeType& range)
  {

  }
};

//-------------------------------------------------------------------BodyFilterPolicy
/// A policy for filtering constraints and returning only unique rigid bodies.
template <typename JointType>
struct BodyFilterPolicy
{
  typedef JointType JointTypeDef;
  typedef BodyFilterPolicy<JointType> SelfType;
  typedef BaseJointGraphEdge<JointType> GraphEdgeType;
  typedef typename GraphEdgeType::EdgeType EdgeType;
  typedef InList<EdgeType, &EdgeType::ColliderLink> EdgeList;
  typedef typename EdgeList::range RangeType;

  BodyFilterPolicy() {}

  GraphEdgeType CreateGraphEdge(EdgeType* edge)
  {
    return GraphEdgeType(edge);
  }

  void SkipDead(RangeType& range)
  {
    while(!range.Empty())
    {
      JointType* joint = range.Front().mJoint;
      if(joint->GetJointType() == JointType::StaticGetJointType())
      {
        // Uniquely map rigid bodies
        RigidBody* body = range.Front().mOther->GetActiveBody();
        if(body && mBodies.Find(body).Empty())
        {
          mBodies.Insert(body);
          break;
        }
      }
      range.PopFront();
    }
  }
private:
  BodyFilterPolicy(const SelfType& other);
  void operator=(const SelfType& other);
  
  HashSet<RigidBody*> mBodies;
};

//-------------------------------------------------------------------BodyFilterPolicy<Contact>
/// Specialization for contact constraints.
template <>
struct BodyFilterPolicy<Physics::Contact>
{
  typedef Physics::Contact JointType;
  typedef BodyFilterPolicy<JointType> SelfType;
  typedef ContactGraphEdge GraphEdgeType;
  typedef GraphEdgeType::EdgeType EdgeType;
  typedef InList<EdgeType, &EdgeType::ColliderLink> EdgeList;
  typedef EdgeList::range RangeType;

  BodyFilterPolicy() {};

  GraphEdgeType CreateGraphEdge(EdgeType* edge);
  void SkipDead(RangeType& range);

private:
  BodyFilterPolicy(const SelfType& other);
  void operator=(const SelfType& other);

  HashSet<RigidBody*> mBodies;
};

//-------------------------------------------------------------------BodyFilterPolicy<Joint>
/// Specialization for Joint.
template <>
struct BodyFilterPolicy<Joint>
{
  typedef Joint JointType;
  typedef BodyFilterPolicy<JointType> SelfType;
  typedef BaseJointGraphEdge<Joint> GraphEdgeType;
  typedef GraphEdgeType::EdgeType EdgeType;
  typedef InList<EdgeType, &EdgeType::ColliderLink> EdgeList;
  typedef EdgeList::range RangeType;

  BodyFilterPolicy() {};

  GraphEdgeType CreateGraphEdge(EdgeType* edge);
  void SkipDead(RangeType& range);

private:
  BodyFilterPolicy(const SelfType& other);
  void operator=(const SelfType& other);

  HashSet<RigidBody*> mBodies;
};

//-------------------------------------------------------------------BaseConstraintRange
/// A base constraint range. Works for Joints and Contacts. Used
/// to make cleaner and easier looping over constraints on a collider.
template <typename BaseConstraintType, typename FilterConstraintType, typename PolicyType = ConstraintGraphEdgePolicy<FilterConstraintType> >
struct BaseConstraintRange
{
  typedef BaseConstraintRange<BaseConstraintType, FilterConstraintType, PolicyType> self_type;

  typedef BaseConstraintType BaseConstraintTypeDef;
  typedef FilterConstraintType ConstraintType;
  typedef PolicyType PolicyTypeDef;

  typedef typename PolicyType::GraphEdgeType GraphEdgeType;
  typedef GraphEdgeType value_type;
  typedef typename PolicyType::EdgeType EdgeType;
  typedef InList<EdgeType, &EdgeType::ColliderLink> EdgeList;
  typedef typename EdgeList::range JointRangeType;

  typedef typename GraphEdgeType& FrontResult;

  BaseConstraintRange(){}
  BaseConstraintRange(const BaseConstraintRange& rhs)
  {
    mConstraintRange = rhs.mConstraintRange;
    mPolicy = rhs.mPolicy;
    mGraphEdge = rhs.mGraphEdge;
  }

  BaseConstraintRange(const JointRangeType& range)
  {
    mConstraintRange = range;
    mPolicy.SkipDead(mConstraintRange);
  }

  BaseConstraintRange(const JointRangeType& range, PolicyType& policy)
    : mPolicy(policy)
  {
    mConstraintRange = range;
    mPolicy.SkipDead(mConstraintRange);
  }

  bool Empty()
  {
    return mConstraintRange.Empty();
  }

  void PopFront()
  {
    ErrorIf(mConstraintRange.Empty(), "Popped an empty range.");
    mConstraintRange.PopFront();
    mPolicy.SkipDead(mConstraintRange);
  }

  FrontResult Front()
  {
    mGraphEdge = mPolicy.CreateGraphEdge(&mConstraintRange.Front());
    return mGraphEdge;
  }

  /// Internal helper when I don't care about the
  /// graph edge and don't want to construct it.
  ConstraintType& GetConstraint()
  {
    return *static_cast<ConstraintType*>(mConstraintRange.Front().mJoint);
  }

protected:

  JointRangeType mConstraintRange;
  PolicyType mPolicy;
  GraphEdgeType mGraphEdge;
};

typedef BaseConstraintRange<Physics::Contact,Physics::Contact> ContactRange;
typedef BaseConstraintRange<Joint, Joint> JointRange;

typedef BaseConstraintRange<Physics::Contact, Physics::Contact, BodyFilterPolicy<Physics::Contact> > ContactBodyRange;
typedef BaseConstraintRange<Joint, Joint, BodyFilterPolicy<Joint> > JointBodyRange;


/// Define a range for iterating over specific range types (filters out the other types).
/// This macro automatically declares the range for all joint types. This does
/// however, assume that you include what joints are needed.
#define JointType(type)                                                              \
  typedef BaseConstraintRange<Joint,type> type##Range;                               \
  typedef BaseConstraintRange<Joint, type, BodyFilterPolicy<type> > type##BodyRange; \
  typedef BaseJointGraphEdge<type> type##GraphEdge;

#include "Physics/Joints/JointList.hpp"

#undef JointType

InList<JointEdge, &JointEdge::ColliderLink>::range GetJointEdges(Collider* collider);

template <typename JointType>
inline BaseConstraintRange<Joint, JointType> FilterJointRange(Collider* collider)
{
  return BaseConstraintRange<Joint, JointType>(GetJointEdges(collider));
}

ContactRange FilterContactRange(Collider* collider);

template <typename JointType>
inline BaseConstraintRange<Joint, JointType> FilterJointBodyRange(Collider* collider)
{
  return BaseConstraintRange<Joint, JointType, BodyFilterPolicy<JointType> >(GetJointEdges(collider));
}

ContactBodyRange FilterContactBodyRange(Collider* collider);

}//namespace Zero
