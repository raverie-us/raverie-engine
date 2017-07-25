///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Physics
{

Mass::Mass()
{
  mInvMass = real(1.0);
  mInvMasses.Splat(mInvMass);
}

void Mass::SetInvMass(real invMass)
{
  mInvMass = invMass;
  mInvMasses = Vec3(mInvMass, mInvMass, mInvMass);
}

void Mass::SetAxisLock(bool state, uint axis)
{
  mInvMasses = Vec3(mInvMass, mInvMass, mInvMass);
  if(state)
    mInvMasses[axis] = 0;
}

Vec3 Mass::Apply(Vec3Param vector) const
{
  return vector * mInvMasses;
}

Vec3 Mass::ApplyInverted(Vec3Param vector) const
{
  Vec3 result = vector;
  if(mInvMasses[0] != real(0.0))
    result[0] /= mInvMasses[0];
  if(mInvMasses[1] != real(0.0))
    result[1] /= mInvMasses[1];
  if(mInvMasses[2] != real(0.0))
    result[2] /= mInvMasses[2];
  return result;
}

real Mass::GetScalarInvMass() const
{
  return mInvMass;
}

Vec3 Mass::GetInvMasses() const
{
  return mInvMasses;
}

void Mass::Check()
{
  ErrorIf(mInvMass != mInvMasses[0], "This should never happen");
}

Inertia::Inertia()
{
  mInvInertiaTensorM.ZeroOut();
  mInvInertiaTensorW.ZeroOut();
}

void Inertia::SetInvTensorModel(Mat3Param invTensorM)
{
  mInvInertiaTensorM = invTensorM;
}

void Inertia::ClearTensors()
{
  mInvInertiaTensorM.ZeroOut();
  mInvInertiaTensorW.ZeroOut();
}

void Inertia::SetTensorsToIdentity()
{
  mInvInertiaTensorM.SetIdentity();
  mInvInertiaTensorW.SetIdentity();
}

Vec3 Inertia::Apply(Vec3Param vector) const
{
  return Math::Transform(mInvInertiaTensorW, vector);
}

Vec3 Inertia::ApplyInverted(Vec3Param vector) const
{
  Mat3 inertiaTensorW = mInvInertiaTensorW;

  // Try to invert the inverse world inertia tensor to get the world inertia tensor
  bool inverted = inertiaTensorW.SafeInvert();
  // If we succeed then just transform the input vector
  if(inverted)
    return Math::Transform(inertiaTensorW, vector);

  // Otherwise there's a chance we can still compute the result vector.
  // Instead of computing a pseudo inverse though we can use an LCP solver to approximate the result of (I^-1 * v) directly.
  inertiaTensorW = mInvInertiaTensorW;
  Vec3 input = vector;
  Vec3 result = Vec3::cZero;

  // Solve the LCP up to a max of 7 iterations
  IndexDim3Policy policy;
  SimpleErrorCallback errCallback;
  Math::GaussSeidelSolver solver;
  solver.mMaxIterations = 7;
  solver.Solve(inertiaTensorW, input, result, policy, errCallback);
  if(errCallback.mSuccessfullySolved)
    return result;

  // If we failed to solve then just return the zero vector?
  return Vec3::cZero;
}

Mat3 Inertia::GetInvModelTensor() const
{
  return mInvInertiaTensorM;
}

Mat3 Inertia::GetInvWorldTensor() const
{
  return mInvInertiaTensorW;
}

void Inertia::ComputeWorldTensor(Mat3Param rotation)
{
  mInvInertiaTensorW = rotation * mInvInertiaTensorM * rotation.Transposed();
}

void Inertia::LockLocalAxis(uint axis)
{
  mInvInertiaTensorM.SetBasis(axis, Vec3::cZero);
}

void Inertia::WorldLock2D()
{
  mInvInertiaTensorW.SetCross(0, Vec3::cZero);
  mInvInertiaTensorW.SetCross(1, Vec3::cZero);
}

}//namespace Physics

}//namespace Zero
