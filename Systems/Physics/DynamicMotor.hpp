///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys
/// Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Forward Declarations
class PhysicsSpace;
class Transform;
class RigidBody;
struct JointMotor;
typedef Physics::RelativeVelocityJoint RelativeVelocityJoint;

namespace Physics
{
  struct LinearAxisJoint;
}//namespace Physics

typedef Physics::LinearAxisJoint LinearAxisJoint;

/// Controls an object's movement using joints. This allows creating a physics
/// based character controller that reacts to physics (joints, forces, collisions, etc...).
/// The motor controls relative velocity with respect to a target object frame.
class DynamicMotor : public Component
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  DynamicMotor();
  virtual ~DynamicMotor();

  // Component Interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  RelativeVelocityJoint* CreateJoint();
  RelativeVelocityJoint* GetJoint();

  /// Compute relative velocity with respect to the world. Used to signify that an absolute world speed is desired.
  void SetReferenceFrameToWorld();
  /// Compute the relative velocity with respect to a target object. Used to control movement on moving platforms.
  void SetReferenceFrameToObject(Cog* object);

  /// Attempts to move the body in the given direction.
  void MoveInDirection(Vec3Param direction, Vec3Param up);

  /// Should physics restrict the movement of this object?
  bool GetActive() const;
  void SetActive(bool active);

  /// What is the max impulse allowed for controlling movement.
  float GetMaxMoveImpulse();
  void SetMaxMoveImpulse(float val);

private:
  PhysicsSpace* mSpace;
  Transform* mTransform;
  RigidBody* mBody;

  float mMaxMoveImpulse;
  /// Whether or not the motors are active.
  bool mActive;
  /// Safe handle to the joint's cog. We must fetch the joint from
  /// this as someone may delete it out from under us.
  CogId mVelJointCog;
};

}//namespace Zero
