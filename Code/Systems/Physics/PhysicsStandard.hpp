// MIT Licensed (see LICENSE.md).
#pragma once

#include "Foundation/Common/CommonStandard.hpp"
#include "Foundation/Geometry/GeometryStandard.hpp"
#include "Systems/Engine/EngineStandard.hpp"
#include "Foundation/SpatialPartition/SpatialPartitionStandard.hpp"

// Update these later!
#include "Foundation/Geometry/Aabb.hpp"
#include "Foundation/Geometry/DebugDraw.hpp"
#include "Systems/Engine/Time.hpp"

// Physics typedefs
namespace Zero
{

/// Defines a primary direction for an axis.
DeclareEnum3(AxisDirection, X, Y, Z);

// Physics library
class PhysicsLibrary : public Zilch::StaticLibrary
{
public:
  ZilchDeclareStaticLibraryInternals(PhysicsLibrary);

  static void Initialize();
  static void Shutdown();
};

} // namespace Zero

#include "ForwardDeclarations.hpp"

#include "WorldTransformation.hpp"
#include "MeshFilterRange.hpp"
#include "Masses.hpp"
#include "JointData.hpp"
#include "JointEnums.hpp"
#include "PhysicsPairs.hpp"
#include "ConstraintAtoms.hpp"
#include "ConstraintAtomDefines.hpp"
#include "ConstraintMolecules.hpp"
#include "JointIncludes.hpp"
#include "ContactPoint.hpp"
#include "ConstraintRanges.hpp"

// Resources
#include "CollisionFilter.hpp"
#include "CollisionFilterBlocks.hpp"
#include "CollisionGroup.hpp"
#include "CollisionTable.hpp"
#include "PhysicsMeshBoundData.hpp"
#include "GenericPhysicsMesh.hpp"
#include "ConvexMesh.hpp"
#include "MultiConvexMesh.hpp"
#include "PhysicsMesh.hpp"
#include "PhysicsMaterial.hpp"

// PhysicsEffects
#include "PhysicsEffect.hpp"
#include "BasicDirectionEffects.hpp"
#include "BasicPointEffects.hpp"
#include "BuoyancyEffect.hpp"
#include "CustomPhysicsEffect.hpp"
#include "DragEffect.hpp"
#include "FlowEffect.hpp"
#include "IgnoreSpaceEffects.hpp"
#include "ThrustEffect.hpp"
#include "TorqueEffect.hpp"
#include "VortexEffect.hpp"
#include "WindEffect.hpp"
// Colliders
#include "Collider.hpp"
#include "BoxCollider.hpp"
#include "CapsuleCollider.hpp"
#include "ConvexMeshCollider.hpp"
#include "CylinderCollider.hpp"
#include "EllipsoidCollider.hpp"
#include "HeightMapCollider.hpp"
#include "MassOverride.hpp"
#include "MeshCollider.hpp"
#include "MultiConvexMeshCollider.hpp"
#include "SphereCollider.hpp"
// Joints
#include "Joint.hpp"
#include "CustomJoint.hpp"
#include "FixedAngleJoint.hpp"
#include "GearJoint.hpp"
#include "LinearAxisJoint.hpp"
#include "ManipulatorJoint.hpp"
#include "PhyGunJoint.hpp"
#include "PositionJoint.hpp"
#include "PrismaticJoint.hpp"
#include "PrismaticJoint2d.hpp"
#include "PulleyJoint.hpp"
#include "RelativeVelocityJoint.hpp"
#include "RevoluteJoint.hpp"
#include "RevoluteJoint2d.hpp"
#include "StickJoint.hpp"
#include "UniversalJoint.hpp"
#include "UprightJoint.hpp"
#include "WeldJoint.hpp"
#include "WheelJoint.hpp"
#include "WheelJoint2d.hpp"

#include "Region.hpp"
#include "RigidBody.hpp"
#include "PhysicsCar.hpp"
#include "PhysicsCarWheel.hpp"
#include "DebugFlags.hpp"
#include "SpringSystem.hpp"

// Misc Joints Stuff
#include "JointCreator.hpp"
#include "DebugDrawFragments.hpp"
#include "ConstraintFragments.hpp"
#include "ConstraintHelpers.hpp"
#include "Contact.hpp"
#include "IConstraintSolver.hpp"
#include "BasicSolver.hpp"
#include "GenericBasicSolver.hpp"
#include "Island.hpp"
#include "IslandManager.hpp"
#include "PhysicsSolverConfig.hpp"
#include "PositionCorrectionFragments.hpp"
#include "JointDebugDrawConfig.hpp"
#include "JointConfigOverride.hpp"
#include "JointLimit.hpp"
#include "JointMotor.hpp"
#include "NormalSolver.hpp"
#include "SerializationFragments.hpp"
#include "SolverFragments.hpp"
#include "JointSpring.hpp"
#include "JointEvents.hpp"
#include "TemplatedFragments.hpp"
#include "ThreadedFragments.hpp"
#include "ThreadedSolver.hpp"

#include "RayCast.hpp"
#include "Manifold.hpp"
#include "PhysicsSpace.hpp"

// BroadPhase
#include "Analyzer.hpp"
// NarrowPhase
#include "CollisionManager.hpp"
#include "ContactManager.hpp"
#include "CustomCollisionEventTracker.hpp"
#include "TimeOfImpact.hpp"

#include "BodyMassCalculations.hpp"
#include "CoreActions.hpp"
#include "BasicActions.hpp"
#include "DynamicMotor.hpp"
#include "PhysicsComposition.hpp"
#include "PhysicsEvents.hpp"
#include "PhysicsEventManager.hpp"
#include "PhysicsQueues.hpp"
#include "PhysicsNode.hpp"
#include "PhysicsQueueManager.hpp"
#include "PhysicsRaycastProvider.hpp"
#include "Integrators.hpp"
#include "PhysicsEngine.hpp"
// Debug
#include "GjkDebug.hpp"
#include "MeshDebug.hpp"
#include "TimeOfImpactDebug.hpp"
#include "ComponentInspectors.hpp"
