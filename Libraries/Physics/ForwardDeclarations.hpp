///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Claeys, Joshua Davis
/// Copyright 2010-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class Collider;
class BoxCollider;
class CapsuleCollider;
class CylinderCollider;
class EllipsoidCollider;
class HeightMapCollider;
class SphereCollider;
class ConvexMeshCollider;
class MeshCollider;
class MultiConvexMeshCollider;

class Region;
class RigidBody;

class GravityEffect;
class PointGravityEffect;
class ForceEffect;
class PointForceEffect;
class BuoyancyEffect;
class CustomPhysicsEffect;
class DragEffect;
class FlowEffect;
class IgnoreSpaceEffets;
class ThrustEffect;
class VortexEffect;
class WindEffect;
class PhysicsEffect;

class MassOverride;
class PhysicsSpace;
class CustomCollisionEventTracker;
class JointEvent;
class BaseCollisionEvent;
class CollisionEvent;
class CollisionGroupEvent;
class PreSolveEvent;
class Manifold;
class PhysicsSolverConfig;
class PhysicsEngine;

class PhysicsNode;
class CollisionTable;
struct CollisionGroupInstance;
class PhysicsMaterial;
class CollisionGroup;
class WorldTransformation;
struct ProxyResult;
struct BaseCastFilter;
struct CollisionFilter;
struct CollisionFilterBlock;
class SpringSystem;

namespace Physics
{

class PhysicsObject;
class IBroadPhase;
class ConstraintSolver;
class CollisionManager;
class ContactManager;
class IslandManager;
struct ConstraintSolverConfiguration;
struct PhysicsNodeManager;
struct PhysicsEventManager;
class Island;
struct PhysicsQueue;

}//namespace Physics

}//namespace Zero