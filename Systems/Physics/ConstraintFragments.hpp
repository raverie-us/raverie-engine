///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

struct JointLimit;
struct JointMotor;
struct Joint;

namespace Physics
{

struct GearJoint;
struct PulleyJoint;

///Computes the constraint value. This is not the error. Error has to be computed in a separate step because of limits.
void LinearAxisValue(WorldAnchorAtom& anchor, Vec3Param axis, ConstraintAtom& atom, real constant = real(0.0));
void AngularAxisValue(WorldAngleAtom& angle, Vec3Param axis, ConstraintAtom& atom, real constant = real(0.0));
void LimitedAngularAxisValue(MoleculeData& data, uint axis, ConstraintAtom& atom);
void GearAxisValue(GearJoint* joint, ConstraintAtom& atom, real constant);
void PulleyAxisValue(PulleyJoint* joint, ConstraintAtom& atom, real constant);
///Computes the error of the atom. If the limit is not nullptr then the atom is checked against
///the limit for computing error. Returns true if a limit was reached, false otherwise (so slop can be applied).
bool ComputeError(ConstraintAtom& atom, ImpulseLimitAtom& molLimit, JointLimit* limit, real constant, uint flag);
///Determines which atoms are active after being effect by the limit.
void ComputeActiveAtoms(ConstraintAtom* atoms, uint count, JointLimit* limit, uint& bitFlag);
///Computes the Jacobian for a linear axis.
void LinearAxisFragment(WorldAnchorAtom& anchor, Vec3Param axis, ConstraintMolecule& mol);
void LinearAxisFragment2d(WorldAnchorAtom& anchor, Vec3Param axis, ConstraintMolecule& mol);
void PrismaticAxisFragment2d(WorldAnchorAtom& anchor, Vec3Param axis, ConstraintMolecule& mol);
///Computes the Jacobian for an angular axis.
void AngularAxisFragment(WorldAngleAtom& angle, Vec3Param axis, ConstraintMolecule& mol);
void AngularAxisFragment2d(WorldAngleAtom& angle, ConstraintMolecule& mol);
///Computes the Jacobian for a gear joint.
void GearAxisFragment(GearJoint* joint, ConstraintMolecule& mol);
void PulleyAxisFragment(PulleyJoint* joint, ConstraintMolecule& mol);
///Fills out mass, impulse, error and limit on a fragment.
void ComputeMoleculeFragment(ImpulseLimitAtom& limit, ConstraintAtom& atom, ConstraintMolecule& mol, JointMass& masses);
///Create a fragment for a motor from the passed in constraint fragment.
void MotorFragment(ConstraintMolecule& constraintMol, ConstraintMolecule& motorMol, JointMotor& motor);
///Computes the bias and gamma for a rigid constraint.
void RigidConstraintFragment(real& molError, real& molBias, real& molGamma, real baumgarte);
void RigidConstraintFragment(ConstraintMolecule& mol, real baumgarte);
///Computes the bias and gamma for a soft constraint.
void SoftConstraintFragment(real& molMass, real& molError, real& molBias, real& molGamma, real springFrequencyHz, real springDampRatio, real baumgarte, real dt);
void SoftConstraintFragment(ConstraintMolecule& mol, SpringAtom& spring, real baumgarte, real dt);
///Special fragment for contacts that deals with restitution
void ContactNormalFragment(ConstraintMolecule& mol, real baumgarte, real restitutionBias);

///Compute lambda from the passed in fragment and velocities. Helper function.
real ComputeLambda(ConstraintMolecule& mol, JointVelocity& velocites);

}//namespace Physics

}//namespace Zero
