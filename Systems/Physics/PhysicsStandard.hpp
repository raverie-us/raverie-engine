///////////////////////////////////////////////////////////////////////////////
///
/// \file PhysicsStandard.hpp
/// The standard includes header file for the physics project.
///
/// Authors: Joshua Davis, Joshua Claeys
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Math/MathStandard.hpp"
#include "Geometry/GeometryStandard.hpp"
#include "Engine/EngineStandard.hpp"
#include "SpatialPartition/SpatialPartitionStandard.hpp"

// Update these later!
#include "Geometry/Aabb.hpp"
#include "Geometry/DebugDraw.hpp"
#include "Engine/Time.hpp"

//Physics typedefs
namespace Zero
{

/// Defines a primary direction for an axis.
DeclareEnum3(AxisDirection, X, Y, Z);

// Physics library
class ZeroNoImportExport PhysicsLibrary : public Zilch::StaticLibrary
{
public:
  ZilchDeclareStaticLibraryInternals(PhysicsLibrary, "ZeroEngine");

  static void Initialize();
  static void Shutdown();
};

}//namespace Zero


#include "ForwardDeclarations.hpp"

#include "WorldTransformation.hpp"
#include "MeshFilterRange.hpp"
#include "Masses.hpp"
#include "Joints/JointData.hpp"
#include "Joints/JointEnums.hpp"
#include "PhysicsPairs.hpp"
#include "Joints/ConstraintAtoms.hpp"
#include "Joints/ConstraintAtomDefines.hpp"
#include "Joints/ConstraintMolecules.hpp"
#include "Physics/Joints/JointIncludes.hpp"
#include "Physics/ContactPoint.hpp"
#include "Physics/ConstraintRanges.hpp"

// Resources
#include "CollisionFilter.hpp"
#include "CollisionFilterBlocks.hpp"
#include "CollisionGroup.hpp"
#include "CollisionTable.hpp"
#include "Physics/PhysicsMeshBoundData.hpp"
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
#include "Joints/Joint.hpp"
#include "Joints/CustomJoint.hpp"
#include "Joints/FixedAngleJoint.hpp"
#include "Joints/GearJoint.hpp"
#include "Joints/LinearAxisJoint.hpp"
#include "Joints/ManipulatorJoint.hpp"
#include "Joints/PhyGunJoint.hpp"
#include "Joints/PositionJoint.hpp"
#include "Joints/PrismaticJoint.hpp"
#include "Joints/PrismaticJoint2d.hpp"
#include "Joints/PulleyJoint.hpp"
#include "Joints/RelativeVelocityJoint.hpp"
#include "Joints/RevoluteJoint.hpp"
#include "Joints/RevoluteJoint2d.hpp"
#include "Joints/StickJoint.hpp"
#include "Joints/UniversalJoint.hpp"
#include "Joints/UprightJoint.hpp"
#include "Joints/WeldJoint.hpp"
#include "Joints/WheelJoint.hpp"
#include "Joints/WheelJoint2d.hpp"

#include "Region.hpp"
#include "RigidBody.hpp"
#include "PhysicsCar.hpp"
#include "PhysicsCarWheel.hpp"
#include "DebugFlags.hpp"
#include "SpringSystem.hpp"

// Misc Joints Stuff
#include "JointCreator.hpp"
#include "Joints/ConstraintFragmentsSse.hpp"
#include "Joints/DebugDrawFragments.hpp"
#include "Joints/ConstraintFragments.hpp"
#include "Joints/ConstraintHelpers.hpp"
#include "Joints/Contact.hpp"
#include "Joints/IConstraintSolver.hpp"
#include "Joints/BasicSolver.hpp"
#include "Joints/GenericBasicSolver.hpp"
#include "Island.hpp"
#include "IslandManager.hpp"
#include "PhysicsSolverConfig.hpp"
#include "Joints/PositionCorrectionFragments.hpp"
#include "Joints/JointDebugDrawConfig.hpp"
#include "Joints/JointConfigOverride.hpp"
#include "Joints/JointLimit.hpp"
#include "Joints/JointMotor.hpp"
#include "Joints/NormalSolver.hpp"
#include "Joints/SerializationFragments.hpp"
#include "Joints/SolverFragments.hpp"
#include "Joints/JointSpring.hpp"
#include "Joints/JointEvents.hpp"
#include "Joints/TemplatedFragments.hpp"
#include "Joints/ThreadedFragments.hpp"
#include "Joints/ThreadedSolver.hpp"

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
