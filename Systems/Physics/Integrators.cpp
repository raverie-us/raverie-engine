///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Claeys, Joshua Davis, Benjamin Strukus
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Physics
{

Vec3 Solve33(Mat3Param J, Vec3Param f)
{
  Mat3 invJ = J.SafeInverted();
  return Math::Transform(invJ, f);
}

// deltaW2 = -dt * I^-1 (Cross(w_2, I * w_2))
// I * (w_2 - w_1) + dt * Cross(w_2, I2 * w_2)
// J = I_body + dt [skew(w_body) * I_body - skew(I_body * w_body)]

// Solve gyroscopic adds in commonly ignored rotational terms using a implicit integration of a
// portion of angular velocity. See Erin Catto's 2015 GDC presentation for details on the topic.
Vec3 SolveGyroscopic(RigidBody* body, float dt)
{
  Quat rotation = body->mRotationQuat;
  Mat3 invInertia = body->mInvInertia.GetInvModelTensor();
  real invInertiaDeterminant = invInertia.Determinant();
  // If the body can't be rotated then don't we can't do anything so return
  if(invInertiaDeterminant == 0)
    return Vec3::cZero;

  Mat3 inertiaBody = invInertia.Inverted();
  Vec3 angularVelocity = body->mAngularVelocity;

  // Convert to body coordinates
  Quat invRotation = rotation.Inverted();
  Vec3 bodyOmega = Math::Multiply(invRotation, angularVelocity);
  // Compute the residual vector
  Vec3 residual = dt * Math::Cross(bodyOmega, Math::Transform(inertiaBody, bodyOmega));
  // Compute the jacobian
  Mat3 term1 = Math::Multiply(Math::SkewSymmetric(bodyOmega), inertiaBody);
  Mat3 term2 = Math::SkewSymmetric(Math::Transform(inertiaBody, bodyOmega));
  Mat3 jacobian = inertiaBody + dt * (term1 - term2);

  // Single Newton-Raphson update
  Vec3 solved = Solve33(jacobian, residual);
  bodyOmega = bodyOmega - solved;
  // Back to world coordinates
  Vec3 omega2 = Math::Multiply(rotation, bodyOmega);
  // Need to return delta w_2
  return omega2 - angularVelocity;
}

void Integration::Integrate(RigidBody* body, real dt)
{
  IntegrateEuler(body, dt);
}

void Integration::IntegrateVelocity(RigidBody* body, real dt)
{
  //deal with the case of applying velocity outside of 2d mode that isn't
  //handled by the mass being zeroed. The one issue with this is that if
  //any hook is ever provided to alter velocity during physic's frame, then
  //this will not catch it and might cause issues. As this is not allowed
  //at the moment, this solution is more elegant and easier to deal with.
  if(body->mState.IsSet(RigidBodyStates::Mode2D))
  {
    body->mVelocity.z = real(0.0);
    body->mAngularVelocity.x = real(0.0);
    body->mAngularVelocity.y = real(0.0);
  }

  //IntegrateEulerVelocity(body, dt);
  IntegrateRk2Velocity(body, dt);

  //ErrorIf(body->mAngularVelocity.Length() > real(2000.0), "Spinning too fast");
  //ErrorIf(!body->mVelocity.Valid(), "Velocity vector is invalid.");
  //ErrorIf(!body->mAngularVelocity.Valid(), "Angular velocity vector is invalid.");
}

void Integration::IntegrateEulerVelocity(RigidBody* body, real dt)
{
  //Set old values
  body->mVelocityOld = body->mVelocity;
  body->mAngularVelocityOld = body->mAngularVelocity;

  //Integrate velocity and position
  body->mVelocity += body->mInvMass.Apply(body->mForceAccumulator * dt);
  
  // Use superposition rule to split integration into an explicit and implicit step
  Vec3 explicitW = body->mInvInertia.Apply(body->mTorqueAccumulator) * dt;
  Vec3 implicitW = SolveGyroscopic(body, dt);
  body->mAngularVelocity += explicitW + implicitW;
}

void Integration::IntegratePosition(RigidBody* body, real dt)
{
  //IntegrateEulerPosition(body,dt);
  IntegrateRk2Position(body, dt);

//  ErrorIf(!body->mPosition.Valid(), "Position vector is invalid.");
//  ErrorIf(!body->mOrientation.Valid(), "Orientation matrix is invalid.");

  body->GenerateIntegrationUpdate();
}

void Integration::IntegrateEulerPosition(RigidBody* body, real dt)
{
  //Set old values
  //body->mPositionOld = body->mPosition;

  //Integrate velocity and position
  body->UpdateCenterMass(body->mVelocity * dt);

  //Integrate orientation
  Vec3Ref AngVel = body->mAngularVelocity;

  /*Collider* collider = body->GetCollider();
  Mat3 OrientationMat = body->mOrientationMat;

  Mat3 angVelMatrix = SkewSymmetric(AngVel);
  OrientationMat = OrientationMat + Math::Concat(angVelMatrix, OrientationMat) * dt;
  OrientationMat.Orthonormalize();
  body->mOrientationMat = OrientationMat;
  collider->mOrientationMtx = OrientationMat;*/

  Quat Orientation = body->GetWorldRotationQuat();
  Quat Qw(AngVel.x, AngVel.y, AngVel.z, real(0.0));
  Orientation = (Qw * Orientation) * real(0.5) * dt;
  body->UpdateOrientation(Orientation);
}

void Integration::IntegrateEuler(RigidBody* body, real dt)
{
  IntegrateEulerVelocity(body, dt);
  IntegrateEulerPosition(body, dt);
}

void Integration::IntegrateVerlet(RigidBody* body, real dt)
{

}

void Integration::IntegrateRk2(RigidBody* body, real dt)
{

}

void Integration::IntegrateRk2Velocity(RigidBody* body, real dt)
{
  //Set old values
  body->mVelocityOld = body->mVelocity;
  body->mAngularVelocityOld = body->mAngularVelocity;

  //Integrate velocity and position
  body->mVelocity = Math::MultiplyAdd(body->mVelocity, body->mInvMass.Apply(body->mForceAccumulator), dt);
  // Use superposition rule to split integration into an explicit and implicit step
  Vec3 explicitW = body->mInvInertia.Apply(body->mTorqueAccumulator) * dt;
  Vec3 implicitW = SolveGyroscopic(body, dt);
  body->mAngularVelocity += explicitW + implicitW;

  //clamp to max velocity values to avoid bad floating point values (exceptions in particular)
  real maxVel = body->mSpace->mMaxVelocity;
  body->mVelocity = Math::Clamped(body->mVelocity, -maxVel, maxVel);
  body->mAngularVelocity = Math::Clamped(body->mAngularVelocity, -maxVel, maxVel);
}

void Integration::IntegrateRk2Position(RigidBody* body, real dt)
{
  Vec3 newVelocity = body->mVelocity;
       newVelocity = Math::MultiplyAdd(newVelocity, body->mInvMass.Apply(body->mForceAccumulator), dt * real(.5));
       newVelocity *= dt;
  Vec3 newRotation = body->mAngularVelocity;

  body->UpdateCenterMass(newVelocity);

  /*Collider* collider = body->GetCollider();
  Mat3 orientationMatrix = body->mOrientationMat;

  Mat3 angVelMatrix = SkewSymmetric(newRotation);
  orientationMatrix += Math::Concat(angVelMatrix, orientationMatrix) * dt;
  orientationMatrix.Orthonormalize();
  body->mOrientationMat = orientationMatrix;
  collider->mOrientationMtx = orientationMatrix;*/

  Quat Orientation = body->GetWorldRotationQuat();
  Quat Qw(newRotation.x, newRotation.y, newRotation.z, real(0.0));
  Orientation = (Qw * Orientation) * real(0.5) * dt;
  body->UpdateOrientation(Orientation);
}

Vec3 Integration::VelocityApproximation(Vec3Param startPosition, Vec3Param endPosition, real dt)
{
  if(dt == real(0.0))
    return Vec3::cZero;

  real framerate = real(1.0) / dt;
  return (endPosition - startPosition) * framerate;
}

Vec3 Integration::AngularVelocityApproximation(QuatParam startRotation, QuatParam endRotation, real dt)
{
  if(dt == real(0.0))
    return Vec3::cZero;

  real framerate = real(1.0) / dt;

  Quat orientationDelta = endRotation * startRotation.Conjugated();
  return Vec3(orientationDelta.V3()) * 2 * framerate;
}

Vec3 Integration::AngularVelocityApproximation(Mat3Param startRotation, Mat3Param endRotation, real dt)
{
  if(dt == real(0.0))
    return Vec3::cZero;

  real framerate = real(1.0) / dt;

  Mat3 identity;
  identity.SetIdentity();
  Mat3 skew = endRotation * startRotation.Transposed() - identity;
  return Vec3(skew.m21, skew.m02, skew.m10) * framerate;
}

}//namespace Physics

}//namespace Zero
