///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//lazy...cleanup later
typedef Physics::TransformAction TransformAction;
typedef Physics::MassAction MassAction;
typedef Physics::BroadPhaseAction BroadPhaseAction;

//Higher level primitives: Used for common actions that carry meaning
//across Transform, Mass and BroadPhase.

///Used to update an object after integration.
struct IntegrationAction
{
  IntegrationAction(Collider* collider);

  //Have to account for the object being moved due to integration. This means
  //that we have to update BroadPhase, update the world transform and also
  //recompute the world inertia tensor since we likely rotated.
  TransformAction mTransformAction;
  BroadPhaseAction mBroadPhaseAction;
  MassAction mMassAction;
};

///used to update an object whenever it is moved by the engine
struct MovementAction
{
  MovementAction(Collider* collider);

  //Have to account for the object being moved outside of physics.
  //This means that we have to update BroadPhase, update the world transform,
  //and we have to worry not only about the world inertia tensor, but the
  //center of mass needs to be moved to account for the position movement.
  TransformAction mTransformAction;
  BroadPhaseAction mBroadPhaseAction;
  MassAction mMassAction;
};

struct KinematicMovementAction
{
  KinematicMovementAction(RigidBody* body);

  //Kinematic object being moved, need to recompute the world matrix and compute
  //what the velocity should have been. The collider should file a separate
  //action to deal with its update (we may not be in BroadPhase if we are a point cloud).
  TransformAction mTransformAction;
};

struct ColliderCreationAction
{
  ColliderCreationAction(Collider* collider);

  TransformAction mTransformAction;
  BroadPhaseAction mBroadPhaseAction;
};

struct RigidBodyCreationAction
{
  RigidBodyCreationAction(RigidBody* body);

  MassAction mMassAction;
};

struct FullTransformAction
{
  FullTransformAction(Collider* collider);
  FullTransformAction(PhysicsNode* node);

  TransformAction mTransformAction;
};

struct WorldTransformationAction
{
  WorldTransformationAction(Collider* collider);

  TransformAction mTransformAction;
};

///Used when an object is inserted into a BroadPhase.
struct InsertionAction
{
  enum State
  {
    Static = BroadPhaseAction::StaticInsert,
    Dynamic = BroadPhaseAction::DynamicInsert,
  };

  ///Used when we want to be put in the correct BroadPhase (Static/Dynamic)
  InsertionAction(Collider* collider);

  ///Used when we know if we are going to static or dynamic.
  InsertionAction(Collider* collider, byte state);

private:
  void Queue(Collider* collider);

  //For now the insertion also implies creation, need to fix...
  //Since this is being created, we need to compute the world transform,
  //Insert in BroadPhase and also compute all mass properties along with
  //updating the world inertia tensor.
  TransformAction mTransformAction;
  BroadPhaseAction mBroadPhaseAction;
};

///Used when removing an object from a BroadPhase.
struct RemovalAction
{
  RemovalAction(Collider* collider);

  //We are being removed, simply remove from BroadPhase.
  BroadPhaseAction mBroadPhaseAction;
};

///Used upon creation or compositing to compute all mass values.
struct MassRecomputationAction
{
  MassRecomputationAction(RigidBody* body);

  //Just wraps recomputing the mass properties for a rigid body.
  MassAction mMassAction;
};

///Recomputes mass, body inertia and world inertia.
void QueueFullMassRecompuation(PhysicsNode* node);
///Recomputes body and world inertia.
void QueueInertiaRecompuation(PhysicsNode* node);
///Reads the transform and updates the world transform
void QueueTransformRead(PhysicsNode* node);
///Updates the world transform and the world inertia. Meant for when a body is integrated.
void QueueBodyIntegration(PhysicsNode* node);
///Removes from the current broadphase and inserts into the desired one
void ChangeBroadPhase(Collider* collider, bool toDynamicBroadphase);
///Inserts into the logically correct broadphase
void InsertIntoBroadPhase(Collider* collider);
///Removes from the last broadphase we were being inserted into
bool RemoveFromBroadPhase(Collider* collider);
///Queues an update in the current broadphase.
void UpdateInBroadPhase(Collider* collider);
///Queues the rigid body to override it's old transform values
///(so that we don't teleport on a first frame)
void QueueOverrideOldTransform(RigidBody* body);

}//namespace Zero
