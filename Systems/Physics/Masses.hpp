///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2010-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

namespace Physics
{

///A structure to wrap the internal storage details of mass. This hides the
///fact that mass is a Vec3 to deal with axis locking and makes future changes
///painless (hopefully).
struct Mass
{
  Mass();
  void SetInvMass(real invMass);
  void SetAxisLock(bool state, uint axis);

  Vec3 Apply(Vec3Param vector) const;
  Vec3 ApplyInverted(Vec3Param vector) const;

  real GetScalarInvMass() const;
  /// Primarily a feature for sse to get the vector to load
  Vec3 GetInvMasses() const;

  void Check();

private:
  Vec3 mInvMasses;
  real mInvMass;
};

///A structure to wrap the internal storage details of inertia. This should
///make the future change of the model inertia being a Vec3
///(diagonalized Mat3) painless.

struct Inertia
{
  Inertia();

  void SetInvTensorModel(Mat3Param invTensorM);
  void ClearTensors();
  void SetTensorsToIdentity();

  Vec3 Apply(Vec3Param vector) const;
  Vec3 ApplyInverted(Vec3Param vector) const;

  Mat3 GetInvModelTensor() const;
  Mat3 GetInvWorldTensor() const;

  void ComputeWorldTensor(Mat3Param rotation);
  void LockLocalAxis(uint axis);
  void WorldLock2D();

private:
  Mat3 mInvInertiaTensorM;
  Mat3 mInvInertiaTensorW;
};

}//namespace Physics

}//namespace Zero
