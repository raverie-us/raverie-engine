///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

struct PhysicsCarWheel;
class PhysicsSpace;
class Collider;
class RigidBody;

DeclareBitField3(CarFlags, DebugDraw, InEditor, OnAllObjectsCreatedCalled);

/// A controller for a high speed physics based car. The car is controlled with a steer,
/// gas, and brake scalar. The car will raycast wheel positions
/// to try to keep the wheels on the ground and then apply friction and normal
/// forces to propel the car.
struct PhysicsCar : public Component
{
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  PhysicsCar();

  // Component Interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;
  void OnAllObjectsCreated(CogInitializer& initializer) override;
  void OnDestroy(uint flags) override;
  /// Debug draws all of the wheels associated with this car.
  void DebugDraw() override;
  void TransformUpdate(TransformUpdateInfo& info) override;
  
  /// Add the PhysicsCarWheel to be controlled by this car.
  void AddWheelCog(Cog* wheelCog);
  /// Remove the PhysicsCarWheel from being controlled by this car.
  void RemoveWheelCog(Cog* wheelCog);
  void ClearWheels();

  // Internal updates
  void Update(real dt);
  void UpdatePositions(real dt);

  void CacheActiveWheels();
  void UpdateWheelData();
  void CastWheelPositions();
  void CalculateSpringForces();
  void ApplySpringForces(real dt);
  void BeginFrictionCalculations(real dt);
  void CalculateFrictionImpulses(real dt);
  void EndFrictionCalculations(real dt);
  void UpdateWheelTransforms(real dt);

  // Properties

  /// Whether or not the car should debug draw.
  bool GetDebugDraw();
  void SetDebugDraw(bool state);
  /// How much the wheel is being steered. This is measured in
  /// radians about this object's y-axis
  real GetSteer();
  void SetSteer(real steer);
  /// How much the gas is being pressed [-1,1] where -1 is full reverse.
  real GetGas();
  void SetGas(real gas);
  /// How much the brake is being pressed [0,1] where 1 is full brake.
  real GetBrake();
  void SetBrake(real brake);

  /// The number of wheels currently in contact with an object.
  uint NumberOfWheelsInContact();

  /// Helper class for referencing a PhysicsCarWheel. Contains a few helper functions
  /// and helps with binding to the property grid (currently disabled).
  struct CarWheelRef : public Object
  {
    ZilchDeclareType(TypeCopyMode::ReferenceType);

    Cog* GetCog();
    PhysicsCarWheel* GetCarWheel();

    CogId mWheelId;
  };

  /// An array interface to the cog paths of wheels that this car uses. This array is read-only.
  struct CarWheelArray : public SafeId32Object
  {
    ZilchDeclareType(TypeCopyMode::ReferenceType);

    CarWheelArray();
    /// Get the cog for a wheel by index.
    Cog* Get(int index);
    /// How many wheels this car owns.
    int GetCount();

    PhysicsCar* mCarBody;
  };

  Array<CarWheelRef> mWheelRefs;
  Array<PhysicsCarWheel*> mActiveWheels;
  /// Read-only array of wheels belonging to this car.
  CarWheelArray mWheelCogs;

  Link<PhysicsCar> SpaceLink;
  
  /// Coefficient used to apply the side friction force closer to the car's center
  /// of mass. 1 applies the force at the wheel position, 0 applies the force at the
  /// point along the contact normal closest to the center of mass.
  real mWheelFrictionSideRollCoef;
  /// Coefficient used to apply the forward friction force closer to the car's center
  /// of mass. 1 applies the force at the wheel position, 0 applies the force at the
  /// point along the contact normal closest to the center of mass.
  real mWheelFrictionFrontRollCoef;

  real mSteerInput;
  real mGasInput;
  real mBrakeInput;
  BitField<CarFlags::Enum> mFlags;

  Collider* mCollider;
  RigidBody* mBody;
  PhysicsSpace* mSpace;

  Math::DecomposedMatrix4 mWorldTransform;

  /// The desired maximum speed of the car. Similar to a speed governor.
  real mMaxSpeed;
  /// The maximum torque the engine can apply to try to reach the max speed.
  real mMaxTorque;
  /// Artificially increases the grip of the car (where 2 is twice the grip).
  /// The total grip scalar is computed as CarGripScalar * WheelGripScalar
  /// so the total car can be easily tweaked while allowing individual wheel tweaks.
  real mGripScalar;
  /// Prevents the car from entering dynamic friction when applying brakes.
  /// If the brake would start to skid, the brake force is clamped to the
  /// max amount that will not slip.
  bool mAntiLockBrakes;
  /// Governs the max torque that the engine can apply. This is used to keep
  /// the wheels from spinning out (slipping) when too high of a torque is applied.
  /// If the tires would slip, the engine will apply the maximum
  /// torque for the tires to not slip.
  bool mTorqueGovernor;
  /// Whether or not the car will run any logic at all. If this is false wheels
  /// will not work, they will not behave as springs, drive, or anything else.
  bool mActive;
};

}//namespace Zero
