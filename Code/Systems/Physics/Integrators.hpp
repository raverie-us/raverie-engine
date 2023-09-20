// MIT Licensed (see LICENSE.md).
#pragma once

namespace Raverie
{

class RigidBody;

namespace Physics
{

DeclareEnum4(IntegrationMethods, Euler, Verlet, Rk2, Rk4);

// Integration is put in a struct so that it is easier to friend these functions
struct Integration
{
  static void IntegrateVelocity(RigidBody* body, real dt);
  static void IntegrateEulerVelocity(RigidBody* body, real dt);

  static void IntegratePosition(RigidBody* body, real dt);
  static void IntegrateEulerPosition(RigidBody* body, real dt);

  static void Integrate(RigidBody* body, real dt);
  static void IntegrateEuler(RigidBody* body, real dt);
  static void IntegrateVerlet(RigidBody* body, real dt);
  static void IntegrateRk2(RigidBody* body, real dt);
  static void IntegrateRk2Velocity(RigidBody* body, real dt);
  static void IntegrateRk2Position(RigidBody* body, real dt);

  static Vec3 VelocityApproximation(Vec3Param startPosition, Vec3Param endPosition, real dt);
  static Vec3 AngularVelocityApproximation(QuatParam startRotation, QuatParam endRotation, real dt);
  static Vec3 AngularVelocityApproximation(Mat3Param startRotation, Mat3Param endRotation, real dt);
};

} // namespace Physics

} // namespace Raverie
