///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Physics
{

///Action used to store various transform update commands.
struct TransformAction
{
  enum Actions
  {
    Empty                = 0,
    ReadTransform        = 1 << 0,
    WorldTransform       = 1 << 1,
    OverrideOldTransform = 1 << 2,//replaces the old transform values with the current
    KinematicVelocity    = 1 << 3//effects the rigid body
  };

  TransformAction();
  void QueueState(byte state);
  ///Performs the queued actions on the collider and body passed in.
  void CommitState(PhysicsNode* node);
  void EmptyState();

  void Validate();

  byte mState;
};

///Action to keep track of what mass properties to recalculate.
struct MassAction
{
  enum Actions
  {
    Empty                  = 0,
    RecomputeCenterMass    = 1 << 0,
    RecomputeInertiaTensor = 1 << 1,
    CenterMassUpdate       = 1 << 2,//The object has moved, we have to move the center of mass with the object
    CenterMassRotation     = 1 << 3,
    WorldInertiaTensor     = 1 << 4,
  };

  MassAction();
  void QueueState(byte state);
  ///Performs the queue action on the body passed in.
  void CommitState(RigidBody* owner);
  void EmptyState();

  void Validate();

  byte mState;
};

struct BroadPhaseBatch
{
  ProxyHandleArray removals;
  BroadPhaseObjectArray updates, inserts;
};

///Action to keep track of BroadPhase changes.
struct BroadPhaseAction
{
  enum Actions
  {
    Empty            = 0,
    DynamicInsert    = 1 << 0,
    DynamicRemoval   = 1 << 1,
    StaticInsert     = 1 << 2,
    StaticRemoval    = 1 << 3,
    Update           = 1 << 4,
    CurrStateStatic  = 1 << 5,
    CurrStateDynamic = 1 << 6,
    CurrStateQueued  = 1 << 7,
    Dynamic = DynamicInsert | DynamicRemoval,
    Static  = StaticInsert | StaticRemoval,
    Insert  = DynamicInsert | StaticInsert,
    Removal = DynamicRemoval | StaticRemoval,
  };

  BroadPhaseAction();

  void PushAction(byte state);
  ///Insert the collider based upon it's static/dynamic state.
  void InsertAction(Collider* collider);
  ///Remove the collider based upon it's static/dynamic state.
  void RemoveAction(Collider* collider);
  ///Update the collider based upon it's static/dynamic state.
  void UpdateAction(Collider* collider);
  void QueueState(byte state);
  ///Empties the state but keeps the CurrState info (except for queued).
  void EmptyState();
  void ClearAction();

  //Helpers
  bool IsSet(byte state) const;
  void SetState(byte state);
  void ClearState(byte state);
  bool IsActionQueued() const;
  bool IsInBroadPhase() const;
  bool IsInDynamicBroadPhase() const;
  //returns the enum value of DynamicRemoval, StaticRemoval or 0 (not in a broadphase)
  uint BroadPhaseToRemoveFrom() const;

  void Validate();

  byte mState;
};

}//namespace Physics

}//namespace Zero
