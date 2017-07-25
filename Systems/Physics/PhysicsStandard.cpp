///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Trevor Sundberg
/// Copyright 2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

// Ranges
ZilchDefineRange(CastResultsRange);
ZilchDefineRange(ContactPointRange);
ZilchDefineRange(SweepResultRange);
ZilchDefineRange(CastResults::range);
ZilchDefineRange(ContactRange);
ZilchDefineRange(JointRange);
ZilchDefineRange(PhysicsMeshVertexData::RangeType);
ZilchDefineRange(PhysicsMeshIndexData::RangeType);
ZilchDefineRange(MultiConvexMeshVertexData::RangeType);
ZilchDefineRange(MultiConvexMeshIndexData::RangeType);
ZilchDefineRange(MultiConvexMeshSubMeshData::RangeType);

// Enums
ZilchDefineEnum(RigidBodyDynamicState);
ZilchDefineEnum(CastFilterState);
ZilchDefineEnum(PhysicsEffectType);
ZilchDefineEnum(PhysicsSolverPositionCorrection);
ZilchDefineEnum(ConstraintPositionCorrection);
ZilchDefineEnum(PhysicsSolverType);
ZilchDefineEnum(PhysicsSolverSubType);
ZilchDefineEnum(PhysicsIslandType);
ZilchDefineEnum(PhysicsIslandPreProcessingMode);
ZilchDefineEnum(PhysicsContactTangentTypes);
ZilchDefineEnum(JointFrameOfReference);
ZilchDefineEnum(AxisDirection);
ZilchDefineEnum(PhysicsEffectInterpolationType);
ZilchDefineEnum(PhysicsEffectEndCondition);
ZilchDefineEnum(Mode2DStates);
ZilchDefineEnum(CapsuleScalingMode);
ZilchDefineEnum(CollisionFilterCollisionFlags);
ZilchDefineEnum(CollisionFilterBlockType);
ZilchDefineEnum(SpringDebugDrawMode);
ZilchDefineEnum(SpringDebugDrawType);
ZilchDefineEnum(SpringSortOrder);

// Bind the joint types special because they're generated using the #define #include trick
ZilchDefineExternalBaseType(JointTypes::Enum, TypeCopyMode::ValueType, builder, type)
{
  ZilchFullBindEnum(builder, type, SpecialType::Enumeration); 
  // Add all of the joint types
  for(size_t i = 0; i < JointTypes::Size; ++i)
  {
    ZilchFullBindEnumValue(builder, type, i, JointTypes::Names[i]);
  }
}

//**************************************************************************************************
ZilchDefineStaticLibrary(PhysicsLibrary)
{
  builder.CreatableInScriptDefault = false;

  // Ranges
  ZilchInitializeRangeAs(ContactPointRange, "ContactPointRange");
  ZilchInitializeRange(ContactRange);
  ZilchInitializeRange(JointRange);
  ZilchInitializeRange(CastResultsRange);
  ZilchInitializeRangeAs(CastResults::range, "CastResultsArrayRange");
  ZilchInitializeRange(SweepResultRange);
  ZilchInitializeRangeAs(PhysicsMeshVertexData::RangeType, "PhysicsMeshVertexRange");
  ZilchInitializeRangeAs(PhysicsMeshIndexData::RangeType, "PhysicsMeshIndexRange");

  // Enums
  ZilchInitializeEnum(RigidBodyDynamicState);
  ZilchInitializeEnum(CastFilterState);
  ZilchInitializeEnum(PhysicsEffectType);
  ZilchInitializeEnum(PhysicsSolverPositionCorrection);
  ZilchInitializeEnum(ConstraintPositionCorrection);
  ZilchInitializeEnum(PhysicsSolverType);
  ZilchInitializeEnum(PhysicsSolverSubType);
  ZilchInitializeEnum(PhysicsIslandType);
  ZilchInitializeEnum(PhysicsIslandPreProcessingMode);
  ZilchInitializeEnum(PhysicsContactTangentTypes);
  ZilchInitializeEnum(JointFrameOfReference);
  ZilchInitializeEnum(AxisDirection);
  ZilchInitializeEnum(PhysicsEffectInterpolationType);
  ZilchInitializeEnum(PhysicsEffectEndCondition);
  ZilchInitializeEnum(Mode2DStates);
  ZilchInitializeEnum(CapsuleScalingMode);
  ZilchInitializeEnum(CollisionFilterCollisionFlags);
  ZilchInitializeEnum(CollisionFilterBlockType);
  ZilchInitializeEnum(SpringDebugDrawMode);
  ZilchInitializeEnum(SpringDebugDrawType);
  ZilchInitializeEnum(SpringSortOrder);
  ZilchInitializeEnum(JointTypes);

  // Meta Components
  ZilchInitializeType(CollisionFilterMetaComposition);
  ZilchInitializeType(PhysicsSolverConfigMetaComposition);
  // Events
  ZilchInitializeType(BaseCollisionEvent);
  ZilchInitializeType(CollisionEvent);
  ZilchInitializeType(CollisionGroupEvent);
  ZilchInitializeType(CustomJointEvent);
  ZilchInitializeType(JointEvent);
  ZilchInitializeType(CustomPhysicsEffectEvent);
  ZilchInitializeType(CastFilterEvent);
  ZilchInitializeType(PreSolveEvent);


  ZilchInitializeType(PhysicsEngine);
  ZilchInitializeType(PhysicsSpace);
  ZilchInitializeType(RigidBody);
  ZilchInitializeType(Region);
  ZilchInitializeType(MassOverride);

  // Colliders
  ZilchInitializeType(Collider);
  ZilchInitializeType(BoxCollider);
  ZilchInitializeType(CapsuleCollider);
  ZilchInitializeType(ConvexMeshCollider);
  ZilchInitializeType(CylinderCollider);
  ZilchInitializeType(EllipsoidCollider);
  ZilchInitializeType(HeightMapCollider);
  ZilchInitializeType(MeshCollider);
  ZilchInitializeType(MultiConvexMeshCollider);
  ZilchInitializeType(SphereCollider);

  // PhysicsEffects
  ZilchInitializeType(PhysicsEffect);
  ZilchInitializeType(BasicDirectionEffect);
  ZilchInitializeType(ForceEffect);
  ZilchInitializeType(GravityEffect);
  ZilchInitializeType(BasicPointEffect);
  ZilchInitializeType(PointForceEffect);
  ZilchInitializeType(PointGravityEffect);
  ZilchInitializeType(BuoyancyEffect);
  ZilchInitializeType(DragEffect);
  ZilchInitializeType(FlowEffect);
  ZilchInitializeType(IgnoreSpaceEffects);
  ZilchInitializeType(ThrustEffect);
  ZilchInitializeType(TorqueEffect);
  ZilchInitializeType(VortexEffect);
  ZilchInitializeType(WindEffect);
  ZilchInitializeType(CustomPhysicsEffect);

  // Joints
  ZilchInitializeType(CustomConstraintInfo);
  ZilchInitializeType(JointSpring);
  ZilchInitializeType(JointLimit);
  ZilchInitializeType(JointMotor);
  ZilchInitializeType(JointDebugDrawConfig);
  ZilchInitializeType(JointConfigOverride);
  ZilchInitializeType(Joint);
  ZilchInitializeType(CustomJoint);
  ZilchInitializeType(ConstraintConfigBlock);
  // Joints and JointBlocks
#define JointType(jointType)              \
  ZilchInitializeType(jointType);         \
  ZilchInitializeType(jointType##Block);
#include "Physics/Joints/JointList.hpp"
#undef JointType
  ZilchInitializeType(ContactBlock);

  // FilterBlocks
  ZilchInitializeType(CollisionFilterBlock);
  ZilchInitializeType(CollisionStartBlock);
  ZilchInitializeType(CollisionPersistedBlock);
  ZilchInitializeType(CollisionEndBlock);
  ZilchInitializeType(PreSolveBlock);

  // Resources
  ZilchInitializeType(PhysicsMaterial);
  ZilchInitializeType(GenericPhysicsMesh);
  ZilchInitializeType(ConvexMesh);
  ZilchInitializeType(MultiConvexMesh);
  ZilchInitializeType(PhysicsMesh);
  ZilchInitializeType(CollisionFilter);
  ZilchInitializeType(CollisionGroup);
  ZilchInitializeType(CollisionTable);
  ZilchInitializeType(PhysicsSolverConfig);
  // Resource Helpers
  ZilchInitializeType(PhysicsMeshVertexData);
  ZilchInitializeType(PhysicsMeshIndexData);
  ZilchInitializeType(MultiConvexMeshVertexData);
  ZilchInitializeType(MultiConvexMeshIndexData);
  ZilchInitializeType(SubConvexMesh);
  ZilchInitializeType(MultiConvexMeshSubMeshData);
  ZilchInitializeRangeAs(MultiConvexMeshVertexData::RangeType, "MultiConvexMeshVertexRange");
  ZilchInitializeRangeAs(MultiConvexMeshIndexData::RangeType, "MultiConvexMeshIndexRange");
  ZilchInitializeRangeAs(MultiConvexMeshSubMeshData::RangeType, "MultiConvexMeshSubMeshRange");
  
  // Casting
  ZilchInitializeType(BaseCastFilter);
  ZilchInitializeType(CastFilter);
  ZilchInitializeType(CastResult);
  ZilchInitializeType(CastResults);
  ZilchInitializeType(SweepResult);

  // Misc
  ZilchInitializeType(PhysicsCar);
  ZilchInitializeTypeAs(PhysicsCar::CarWheelRef, "CarWheelRef");
  ZilchInitializeTypeAs(PhysicsCar::CarWheelArray, "CarWheelArray");
  ZilchInitializeType(PhysicsCarWheel);
  ZilchInitializeType(CustomCollisionEventTracker);

  ZilchInitializeType(JointCreator);
  ZilchInitializeType(DynamicMotor);
  ZilchInitializeType(PhysicsRaycastProvider);
  ZilchInitializeTypeAs(ContactPoint, "ContactPoint");
  ZilchInitializeType(ContactGraphEdge);
  ZilchInitializeType(JointGraphEdge);

  // Not ready for consumption yet, but I want to test it with dev config
  // METAREFACTOR they should always be initialized, but hidden with an attribute
  if(Z::gEngine->GetConfigCog()->has(Zero::DeveloperConfig))
  {
    ZilchInitializeType(SpringSystem);
    ZilchInitializeType(DecorativeCloth);
    ZilchInitializeType(DecorativeRope);
    ZilchInitializeType(GjkDebug);
    ZilchInitializeType(MeshDebug);
    ZilchInitializeType(TimeOfImpactDebug);
    ZilchInitializeType(ColliderInspector);
  }

  EngineLibraryExtensions::AddNativeExtensions(builder);
}

//**************************************************************************************************
void PhysicsLibrary::Initialize()
{
  BuildStaticLibrary();
  MetaDatabase::GetInstance()->AddNativeLibrary(GetLibrary());

  //Create the material manager.
  InitializeResourceManager(PhysicsMaterialManager);
  InitializeResourceManager(PhysicsSolverConfigManager);
  InitializeResourceManager(PhysicsMeshManager);
  InitializeResourceManager(CollisionGroupManager);
  InitializeResourceManager(CollisionTableManager);
  InitializeResourceManager(ConvexMeshManager);
  InitializeResourceManager(MultiConvexMeshManager);
}

//**************************************************************************************************
void PhysicsLibrary::Shutdown()
{
  GetLibrary()->ClearComponents();
}

}//namespace Zero
