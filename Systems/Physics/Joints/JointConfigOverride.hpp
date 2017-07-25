///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2012-2016, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

struct JointNode;

/// Overrides a joint's configuration values of slop, linear/angular Baumgarte, and
/// linear/angular error correction. Slop is the amount of error allowed before position
/// correction takes effect. Baumgarte is used to correct error with a penalty impulse.
/// Baumgarte is split into linear and angular portions because of stability.
/// Error correction is only used when the joint is solved with post stabilization.
struct JointConfigOverride : public Component
{
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  JointConfigOverride();
  virtual ~JointConfigOverride();

  // Component Interface
  void Serialize(Serializer& stream) override;
  void Initialize(CogInitializer& initializer) override;

  /// The amount of error allowed before position correction takes effect.
  real GetSlop() const;
  void SetSlop(real slop);
  
  /// The exponential constant for correcting linear error with a penalty impulse.
  real GetLinearBaumgarte() const;
  void SetLinearBaumgarte(real linearBaumgarte);

  /// The exponential constant for correcting angular error with a penalty impulse.
  real GetAngularBaumgarte() const;
  void SetAngularBaumgarte(real angularBaumgarte);

  /// The max amount of error that can be corrected by the
  /// linear portion of any constraint in one frame (only for PostStabilization).
  real GetLinearErrorCorrection();
  void SetLinearErrorCorrection(real maxError);

  /// The max amount of error that can be corrected by the
  /// angular portion of any constraint in one frame (only for PostStabilization).
  real GetAngularErrorCorrection();
  void SetAngularErrorCorrection(real maxError);
  
  /// The kind of position correction that this joint should use.
  ConstraintPositionCorrection::Enum GetPositionCorrectionType() const;
  void SetPositionCorrectionType(ConstraintPositionCorrection::Enum correctionType);

  real mSlop;
  real mLinearBaumgarte;
  real mAngularBaumgarte;
  real mLinearErrorCorrection;
  real mAngularErrorCorrection;
  ConstraintPositionCorrection::Enum mPositionCorrectionType;

  JointNode* mNode;
};

}//namespace Zero
