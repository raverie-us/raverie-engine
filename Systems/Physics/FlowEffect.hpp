///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

DeclareBitField2(FlowFlags, LocalForce, AttractToFlowCenter);

/// Applies a force to make an object move at a target speed in a given direction.
/// This can also be used to pull the object towards the center of the flow (the
/// axis in the flow direction centered at the effect). Used to model a river
/// or a tractor beam.
class FlowEffect : public PhysicsEffect
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  FlowEffect();

  // Component Interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& data) override;
  void DebugDraw() override;

  // Physics Effect Interface
  void PreCalculate(real dt) override;
  void ApplyEffect(RigidBody* obj, real dt) override;

  void UpdateFlowInformation();

  // Properties

  /// Determines if the flow direction is in the local space of the object.
  bool GetLocalForce() const;
  void SetLocalForce(bool state);
  /// Determines if the flow field will attract objects towards the center of
  /// the field. This can be used to create a tractor beam effect that will
  /// keep the object inside of the flow field.
  bool GetAttractToFlowCenter();
  void SetAttractToFlowCenter(bool state);
  /// The target speed of objects in the flow field.
  real GetFlowSpeed();
  void SetFlowSpeed(real speed);
  /// The max force that can be used to reach the target flow speed.
  real GetMaxFlowForce();
  void SetMaxFlowForce(real strength);
  /// The direction that the field is flowing.
  /// This can be defined in world or local space.
  Vec3 GetFlowDirection();
  void SetFlowDirection(Vec3Param dir);
  /// The direction that the field is flowing in world space.
  Vec3 GetWorldFlowDirection();
  /// The target speed for an object to be pulled towards the center of the flow.
  real GetAttractSpeed();
  void SetAttractSpeed(real speed);
  /// The max force that can be used to reach the target attract speed.
  real GetMaxAttractForce();
  void SetMaxAttractForce(real strength);

private:
  BitField<FlowFlags::Enum> mForceStates;

  Vec3 mFlowDirection;
  real mFlowSpeed;
  real mMaxFlowForce;
  real mMaxAttractForce;
  real mAttractSpeed;

  // Cached world values
  Vec3 mWorldFlowDirection;
  Vec3 mWorldFlowCenter;
};

}//namespace Zero
