// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

// Ranges
RaverieDefineRange(CastResultsRange);
RaverieDefineRange(ContactPointRange);
RaverieDefineRange(SweepResultRange);
RaverieDefineRange(CastResults::range);
RaverieDefineRange(ContactRange);
RaverieDefineRange(JointRange);
RaverieDefineRange(PhysicsMeshVertexData::RangeType);
RaverieDefineRange(PhysicsMeshIndexData::RangeType);
RaverieDefineRange(MultiConvexMeshVertexData::RangeType);
RaverieDefineRange(MultiConvexMeshIndexData::RangeType);
RaverieDefineRange(MultiConvexMeshSubMeshData::RangeType);

// Enums
RaverieDefineEnum(RigidBodyDynamicState);
RaverieDefineEnum(CastFilterState);
RaverieDefineEnum(PhysicsEffectType);
RaverieDefineEnum(PhysicsSolverPositionCorrection);
RaverieDefineEnum(ConstraintPositionCorrection);
RaverieDefineEnum(PhysicsSolverType);
RaverieDefineEnum(PhysicsSolverSubType);
RaverieDefineEnum(PhysicsIslandType);
RaverieDefineEnum(PhysicsIslandPreProcessingMode);
RaverieDefineEnum(PhysicsContactTangentTypes);
RaverieDefineEnum(JointFrameOfReference);
RaverieDefineEnum(AxisDirection);
RaverieDefineEnum(PhysicsEffectInterpolationType);
RaverieDefineEnum(PhysicsEffectEndCondition);
RaverieDefineEnum(Mode2DStates);
RaverieDefineEnum(CapsuleScalingMode);
RaverieDefineEnum(CollisionFilterCollisionFlags);
RaverieDefineEnum(CollisionFilterBlockType);
RaverieDefineEnum(SpringDebugDrawMode);
RaverieDefineEnum(SpringDebugDrawType);
RaverieDefineEnum(SpringSortOrder);

// Bind the joint types special because they're generated using the #define
// #include trick
RaverieDefineExternalBaseType(JointTypes::Enum, TypeCopyMode::ValueType, builder, type)
{
  RaverieFullBindEnum(builder, type, SpecialType::Enumeration);
  // Add all of the joint types
  for (size_t i = 0; i < JointTypes::Size; ++i)
  {
    RaverieFullBindEnumValue(builder, type, i, JointTypes::Names[i]);
  }
}

RaverieDefineStaticLibrary(PhysicsLibrary)
{
  builder.CreatableInScriptDefault = false;

  // Ranges
  RaverieInitializeRangeAs(ContactPointRange, "ContactPointRange");
  RaverieInitializeRange(ContactRange);
  RaverieInitializeRange(JointRange);
  RaverieInitializeRange(CastResultsRange);
  RaverieInitializeRangeAs(CastResults::range, "CastResultsArrayRange");
  RaverieInitializeRange(SweepResultRange);
  RaverieInitializeRangeAs(PhysicsMeshVertexData::RangeType, "PhysicsMeshVertexRange");
  RaverieInitializeRangeAs(PhysicsMeshIndexData::RangeType, "PhysicsMeshIndexRange");

  // Enums
  RaverieInitializeEnum(RigidBodyDynamicState);
  RaverieInitializeEnum(CastFilterState);
  RaverieInitializeEnum(PhysicsEffectType);
  RaverieInitializeEnum(PhysicsSolverPositionCorrection);
  RaverieInitializeEnum(ConstraintPositionCorrection);
  RaverieInitializeEnum(PhysicsSolverType);
  RaverieInitializeEnum(PhysicsSolverSubType);
  RaverieInitializeEnum(PhysicsIslandType);
  RaverieInitializeEnum(PhysicsIslandPreProcessingMode);
  RaverieInitializeEnum(PhysicsContactTangentTypes);
  RaverieInitializeEnum(JointFrameOfReference);
  RaverieInitializeEnum(AxisDirection);
  RaverieInitializeEnum(PhysicsEffectInterpolationType);
  RaverieInitializeEnum(PhysicsEffectEndCondition);
  RaverieInitializeEnum(Mode2DStates);
  RaverieInitializeEnum(CapsuleScalingMode);
  RaverieInitializeEnum(CollisionFilterCollisionFlags);
  RaverieInitializeEnum(CollisionFilterBlockType);
  RaverieInitializeEnum(SpringDebugDrawMode);
  RaverieInitializeEnum(SpringDebugDrawType);
  RaverieInitializeEnum(SpringSortOrder);
  RaverieInitializeEnum(JointTypes);

  // Meta Components
  RaverieInitializeType(CollisionFilterMetaComposition);
  RaverieInitializeType(PhysicsSolverConfigMetaComposition);
  // Events
  RaverieInitializeType(BaseCollisionEvent);
  RaverieInitializeType(CollisionEvent);
  RaverieInitializeType(CollisionGroupEvent);
  RaverieInitializeType(CustomJointEvent);
  RaverieInitializeType(JointEvent);
  RaverieInitializeType(CustomPhysicsEffectEvent);
  RaverieInitializeType(CastFilterEvent);
  RaverieInitializeType(PreSolveEvent);

  RaverieInitializeType(PhysicsEngine);
  RaverieInitializeType(PhysicsSpace);
  RaverieInitializeType(RigidBody);
  RaverieInitializeType(Region);
  RaverieInitializeType(MassOverride);

  // Colliders
  RaverieInitializeType(Collider);
  RaverieInitializeType(BoxCollider);
  RaverieInitializeType(CapsuleCollider);
  RaverieInitializeType(ConvexMeshCollider);
  RaverieInitializeType(CylinderCollider);
  RaverieInitializeType(EllipsoidCollider);
  RaverieInitializeType(HeightMapCollider);
  RaverieInitializeType(MeshCollider);
  RaverieInitializeType(MultiConvexMeshCollider);
  RaverieInitializeType(SphereCollider);

  // PhysicsEffects
  RaverieInitializeType(PhysicsEffect);
  RaverieInitializeType(BasicDirectionEffect);
  RaverieInitializeType(ForceEffect);
  RaverieInitializeType(GravityEffect);
  RaverieInitializeType(BasicPointEffect);
  RaverieInitializeType(PointForceEffect);
  RaverieInitializeType(PointGravityEffect);
  RaverieInitializeType(BuoyancyEffect);
  RaverieInitializeType(DragEffect);
  RaverieInitializeType(FlowEffect);
  RaverieInitializeType(IgnoreSpaceEffects);
  RaverieInitializeType(ThrustEffect);
  RaverieInitializeType(TorqueEffect);
  RaverieInitializeType(VortexEffect);
  RaverieInitializeType(WindEffect);
  RaverieInitializeType(CustomPhysicsEffect);

  // Joints
  RaverieInitializeType(CustomConstraintInfo);
  RaverieInitializeType(JointSpring);
  RaverieInitializeType(JointLimit);
  RaverieInitializeType(JointMotor);
  RaverieInitializeType(JointDebugDrawConfig);
  RaverieInitializeType(JointConfigOverride);
  RaverieInitializeType(Joint);
  RaverieInitializeType(CustomJoint);
  RaverieInitializeType(ConstraintConfigBlock);
  // Joints and JointBlocks
#define JointType(jointType)                                                                                                                                                                           \
  RaverieInitializeType(jointType);                                                                                                                                                                    \
  RaverieInitializeType(jointType##Block);
#include "JointList.hpp"
#undef JointType
  RaverieInitializeType(ContactBlock);

  // FilterBlocks
  RaverieInitializeType(CollisionFilterBlock);
  RaverieInitializeType(CollisionStartBlock);
  RaverieInitializeType(CollisionPersistedBlock);
  RaverieInitializeType(CollisionEndBlock);
  RaverieInitializeType(PreSolveBlock);

  // Resources
  RaverieInitializeType(PhysicsMaterial);
  RaverieInitializeType(GenericPhysicsMesh);
  RaverieInitializeType(ConvexMesh);
  RaverieInitializeType(MultiConvexMesh);
  RaverieInitializeType(PhysicsMesh);
  RaverieInitializeType(CollisionFilter);
  RaverieInitializeType(CollisionGroup);
  RaverieInitializeType(CollisionTable);
  RaverieInitializeType(PhysicsSolverConfig);
  // Resource Helpers
  RaverieInitializeType(PhysicsMeshVertexData);
  RaverieInitializeType(PhysicsMeshIndexData);
  RaverieInitializeType(MultiConvexMeshVertexData);
  RaverieInitializeType(MultiConvexMeshIndexData);
  RaverieInitializeType(SubConvexMesh);
  RaverieInitializeType(MultiConvexMeshSubMeshData);
  RaverieInitializeRangeAs(MultiConvexMeshVertexData::RangeType, "MultiConvexMeshVertexRange");
  RaverieInitializeRangeAs(MultiConvexMeshIndexData::RangeType, "MultiConvexMeshIndexRange");
  RaverieInitializeRangeAs(MultiConvexMeshSubMeshData::RangeType, "MultiConvexMeshSubMeshRange");

  // Casting
  RaverieInitializeType(BaseCastFilter);
  RaverieInitializeType(CastFilter);
  RaverieInitializeType(CastResult);
  RaverieInitializeType(CastResults);
  RaverieInitializeType(SweepResult);

  // Misc
  RaverieInitializeType(PhysicsCar);
  RaverieInitializeTypeAs(PhysicsCar::CarWheelRef, "CarWheelRef");
  RaverieInitializeTypeAs(PhysicsCar::CarWheelArray, "CarWheelArray");
  RaverieInitializeType(PhysicsCarWheel);
  RaverieInitializeType(CustomCollisionEventTracker);

  RaverieInitializeType(JointCreator);
  RaverieInitializeType(DynamicMotor);
  RaverieInitializeType(PhysicsRaycastProvider);
  RaverieInitializeTypeAs(ContactPoint, "ContactPoint");
  RaverieInitializeType(ContactGraphEdge);
  RaverieInitializeType(JointGraphEdge);

  // Not ready for consumption yet, but I want to test it with dev config
  // METAREFACTOR they should always be initialized, but hidden with an
  // attribute
  // RaverieInitializeType(SpringSystem);
  // RaverieInitializeType(DecorativeCloth);
  // RaverieInitializeType(DecorativeRope);
  // RaverieInitializeType(GjkDebug);
  // RaverieInitializeType(MeshDebug);
  // RaverieInitializeType(TimeOfImpactDebug);
  // RaverieInitializeType(ColliderInspector);

  EngineLibraryExtensions::AddNativeExtensions(builder);
}

void PhysicsLibrary::Initialize()
{
  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());

  // Create the material manager.
  InitializeResourceManager(PhysicsMaterialManager);
  InitializeResourceManager(PhysicsSolverConfigManager);
  InitializeResourceManager(PhysicsMeshManager);
  InitializeResourceManager(CollisionGroupManager);
  InitializeResourceManager(CollisionTableManager);
  InitializeResourceManager(ConvexMeshManager);
  InitializeResourceManager(MultiConvexMeshManager);
}

void PhysicsLibrary::Shutdown()
{
  GetLibrary()->ClearComponents();
}

} // namespace Raverie
