///////////////////////////////////////////////////////////////////////////////
/// 
/// Authors: Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

struct PhysicsCar;
struct TransformProxyObjectEvent;

struct ConstraintInfo;

DeclareBitField7(CarWheelFlags, IsDriveWheel, IsInContact, InEditor, ChildedWheel, IsSliding, Is2DWheel, Active);

/// A wheel for a high speed physics based car. Each wheel contains properties to describe
/// how to interact with the world (eg. spring forces, friction, etc...).
struct PhysicsCarWheel: public Component
{
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  PhysicsCarWheel();

  // Component Interface
  void Serialize(Serializer& stream) override;
  void OnAllObjectsCreated(CogInitializer& initializer) override;
  void OnDestroy(uint flags) override;
  void DebugDraw() override;
  void TransformUpdate(TransformUpdateInfo& info) override;
  void AttachTo(AttachmentInfo& info) override;
  void Detached(AttachmentInfo& info) override;
  // Helper for AttachTo/Detached
  void FixChildState(Cog* parent);

  // Helpers
  PhysicsCar* GetCarBody();
  void UpdateCachedCarBody();
  void OnCogPathCogChanged(Event* e);
  void UpdateCarBodyConnections();
  /// The path to the car that this is a wheel for.
  CogPath GetPhysicsCarPath();
  void SetPhysicsCarPath(CogPath& carPath);

  void UpdateLocalPointOnCar(bool forcedUpdate = false);
  void UpdatePreRotationOnCar(bool forcedUpdate = false);
  void UpdateTransformRelativeToParent();
  real CalculateRotationalVelocity(Vec3Param dir, Vec3Param relativeVelocity);
  Vec3 GetRelativeVelocity();
  real CalculateImpulse(Vec3Param dir, Vec3Param relativeVelocity);

  /// Determines what directions the forward, axle, and spring are on the wheel.
  /// The forward direction is taken from the x-axis of the basis.
  /// Likewise the axle is from the y-axis and the spring is from the z-axis.
  /// Use ToQuaternion to construct this.
  Quat GetWorldWheelBasis();
  void SetWorldWheelBasis(QuatParam worldBasis);

  // Internal updates
  void UpdateWheelData();
  void CastWheelPosition();
  void CalculateSpringForce();
  void ApplySpringForce(real dt);
  void BeginIteration();
  real InternalFrictionCalculation(Vec3Param dir, ConstraintInfo& info);
  void SolveFrictionImpulse(real dt);
  void FinishedIteration(real dt);
  void UpdateWheelTransform(real dt);

  // Properties

  /// Should this wheel calculate forces for the current PhysicsCar.
  bool GetActive();
  void SetActive(bool active);
  /// The local position on the car body that the wheel starts at (raycasts from).
  Vec3 GetWheelLocalStartPosition();
  void SetWheelLocalStartPosition(Vec3Param localStartPosition);
  /// How much this wheel steers. [0, 1] where 1 is the max steering of the car.
  real GetSteerFactor();
  void SetSteerFactor(real steerFactor);
  /// The max force that this wheel can exert to break.
  real GetMaxBrakeStrength();
  void SetMaxBrakeStrength(real maxStrength);
  /// Used to rotate the wheel before taking into account it's transform.
  /// Typically used to rotate a cylinder to align with a model.
  Quat GetPreRotation();
  void SetPreRotation(QuatParam quat);
  /// The radius of the wheel.
  real GetRadius();
  void SetRadius(real radius);
  /// The minimum length of the spring of the wheel. If a wheel cast hits an object
  /// at a time before min but after start then the wheel will still collide with
  /// this object but the spring forces and visuals will be at the min spring length.
  /// This is useful for putting the starting raycast position inside of the object
  /// so as to avoid tunneling but still having the wheel only visually display where it should.
  real GetSpringMinLength();
  void SetSpringMinLength(real value);
  /// The t value to start the raycast at. This t value is 0 at the wheel position
  /// and travels in the direction of the wheel spring direction. This value is used
  /// to modify where the raycast actually starts relative to the start position.
  real GetSpringStartLength();
  void SetSpringStartLength(real value);
  /// The maximum length of the spring of the wheel. If the wheel hits something further
  /// away than this length (plus the wheel radius) then that object will be ignored.
  real GetSpringMaxLength();
  void SetSpringMaxLength(real value);
  /// The maximum force that the wheel's spring can exert.
  real GetMaxSpringForce();
  void SetMaxSpringForce(real maxForce);
  /// If the wheel is currently in contact with an object.
  bool GetIsInContact();
  void SetIsInContact(bool stae);
  /// If the wheel is currently sliding. This means that the wheel is slipping
  /// from spinning too fast (using dynamic friction instead of static).
  bool GetIsSliding();
  void SetIsSliding(bool state);
  /// Drive wheels turn when the car body has gas pressed.
  bool GetIsDriveWheel();
  void SetIsDriveWheel(bool state);
  /// Does this wheel only operate in 2D? Ignores the side friction axis.
  bool GetIs2DWheel();
  void SetIs2DWheel(bool state);

  /// The current rotation of the wheel in radians about it's axle.
  real GetRotation();
  /// The current rotational velocity of the wheel about it's axle.
  real GetRotationalVelocity();
  /// A coefficient from 0 to 1 that represents how much grip the wheel has.
  real GetGrip();
  /// The normal impulse (spring force) being exerted by the wheel.
  real GetNormalImpulse();
  /// The forward impulse (drive force) being exerted by the wheel.
  real GetForwardImpulse();
  /// The side impulse (side friction) being exerted by the wheel.
  real GetSideImpulse();
  /// The current length of the spring.
  real GetSpringLength();
  /// The object that this wheel is currently in contact with.
  Cog* GetContactedObject();
  /// The point in world space where the wheel is currently in contact.
  /// Will be the zero vector if there is no contact.
  Vec3 GetContactPoint();
  /// The normal of the surface where the wheel is currently in contact.
  /// Will be the zero vector if there is no contact.
  Vec3 GetContactNormal();

  /// The current axis of the wheel's axle in world space.
  Vec3 GetWorldAxleAxis();
  /// The current axis of the wheel's forward in world space.
  Vec3 GetWorldForwardAxis();
  /// The current world-space spring axis of the wheel.
  Vec3 GetWorldSpringAxis();
  /// The velocity of the center of the wheel while taking into account the car's velocity.
  Vec3 GetWorldLinearVelocity();
  /// The axis that represents the world angular velocity of the wheel.
  Vec3 GetWorldAngularVelocity();
  

  real mSteerFactor;
  /// Used to alter the direction this wheel turns when the motor receives power.
  /// Typically set to 1 or -1. Useful to cause a wheel to temporarily rotate backwards
  /// without having to change its basis.
  real mDriveFactor;
  real mMaxBrakeStrength;
  real mRadius;
  /// The frequency at which the spring of this wheel oscillates per second
  real mFrequencyHz;
  /// The damping ratio when the spring is compressing (0: no damping, 1 critical damping)
  real mDampingCompressionRatio;
  /// The damping ratio when the spring is relaxing (0: no damping, 1 critical damping)
  real mDampingRelaxationRatio;
  Vec3 mWheelLocalStartPosition;
  Vec3 mBodyWheelSpringDir;
  Vec3 mBodyWheelForwardDir;
  Vec3 mBodyWheelAxleDir;
  real mSpringStartLength;
  real mSpringMaxLength;
  real mSpringMinLength;
  /// The rest length of the spring.
  real mSpringRestLength;
  real mMaxSpringForce;
  Quat mPreRotation;
  real mCurrRotation;

  //working
  Vec3 mWorldWheelStartPosition;
  Vec3 mWorldWheelSpringDir;
  Vec3 mWorldWheelForwardDir;
  Vec3 mWorldWheelAxleDir;
  Vec3 mWorldContactPosition;
  Vec3 mWorldContactNormal;
  Vec3 mWorldForwardTangent;
  Vec3 mWorldAxleTangent;

  Collider* mContactedObject;
  real mRotationalVelocity;
  real mSpringForce;
  real mOldSpringLength;
  real mSpringLength;
  CogPath mPhysicsCarPath;

  /// The max distance that a spring can compress in one frame.
  real mMaxSpringCompressionDistance;
  /// The max distance that a spring can relax in one frame.
  real mMaxSpringRelaxationDistance;

  BitField<CarWheelFlags::Enum> mFlags;

  PhysicsCar* mCarBody;
  
  /// Used with Coulomb's friction to determine when the wheel will start slipping
  /// in the forward direction. (i.e. the friction is bound by muS * Fnormal).
  real mForwardStaticFriction;
  /// Determines the force applied in the forward direction when the wheel is
  /// in dynamic friction and therefore slipping. (i.e. force = muK * Fnormal)
  real mForwardDynamicFriction;
  /// Same as ForwardStaticFriction, but in the side direction. Forward and side
  /// friction are separated since the forward direction is rolling friction
  /// and the side direction is sliding friction.
  real mSideStaticFriction;
  /// Same as ForwardDynamicFriction, but in the side direction.
  /// See SideStaticFriction for a why these are separated.
  real mSideDynamicFriction;
  /// Artificially increases the grip of the car (where 2 is twice the grip).
  /// The total grip scalar is computed as CarGripScalar * WheelGripScalar
  /// so the total car can be easily tweaked while allowing individual wheel tweaks.
  real mGripScalar;

  /// The accumulated forward impulse from driving/braking.
  /// Used to help in solving the constraints for friction.
  real mTotalForwardImpulse;
  /// The accumulated side impulse from fixing drifting or sliding.
  real mTotalSideImpulse;
  /// The final grip of the wheel in the forward direction (used to compute the actual grip).
  real mForwardGrip;
  /// The final grip of the wheel in the side direction (used to compute the actual grip).
  real mSideGrip;
};

}//namespace Zero
