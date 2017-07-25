///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2014-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// What kind of position correction should be applied for any constraint that is set to Inherit.
/// <param name="Baumgarte">Use a baumgarte penalty force.</param>
/// <param name="PostStabilization">Directly fix position errors via translation.</param>
DeclareEnum2(PhysicsSolverPositionCorrection, Baumgarte, PostStabilization);
/// What kind of position correction behavior is desired for constraint or constraint type.
/// <param name="Baumgarte">Use a baumgarte penalty force.</param>
/// <param name="PostStabilization">Directly fix position errors via translation.</param>
/// <param name="Inherit">Use the global position correction method.</param>
DeclareEnum3(ConstraintPositionCorrection, Baumgarte, PostStabilization, Inherit);
/// What kind of solver technique to use for position correction. Mainly for testing.
DeclareEnum2(PhysicsSolverSubType, BasicSolving, BlockSolving);
/// How to compute the tangents for a contact point. Mainly for testing.
DeclareEnum3(PhysicsContactTangentTypes, OrthonormalTangents, VelocityTangents, RandomTangents);

//-------------------------------------------------------------------ConstraintConfigBlock
/// A block of information for solving a joint (or constraint) type.
/// This is used to configure how one joint is solved independently of another joint.
struct ConstraintConfigBlock : public SafeId32Object
{
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  ConstraintConfigBlock();

  //-------------------------------------------------------------------Object Interface
  void Serialize(Serializer& serializer);

  //-------------------------------------------------------------------Properties
  /// The amount of error allowed before position correction takes effect.
  real GetSlop();
  void SetSlop(real slop);
  /// The exponential constant for correcting linear error with a penalty impulse.
  real GetLinearBaumgarte();
  void SetLinearBaumgarte(real linearBaumgarte);
  /// The exponential constant for correcting angular error with a penalty impulse.
  real GetAngularBaumgarte();
  void SetAngularBaumgarte(real angularBaumgarte);
  /// The max amount of error that can be corrected by the
  /// linear portion of any constraint in one frame (only for PostStabilization).
  real GetLinearErrorCorrection();
  void SetLinearErrorCorrection(real maxError);
  /// The max amount of error that can be corrected by the
  /// angular portion of any constraint in one frame (only for PostStabilization).
  real GetAngularErrorCorrection();
  void SetAngularErrorCorrection(real maxError);
  /// What method should be used to fix errors in joints.
  ConstraintPositionCorrection::Enum GetPositionCorrectionType();
  void SetPositionCorrectionType(ConstraintPositionCorrection::Enum correctionType);

  //-------------------------------------------------------------------Internal
  /// Reset the default values of this constraint block
  void ResetDefaultValues();

  /// Used to denote what kind of joint this block is for.
  /// This corresponds to the joint's Joint::StaticGetJointType().
  uint mJointId;

  real mSlop;
  real mLinearBaumgarte;
  real mAngularBaumgarte;
  real mLinearErrorCorrection;
  real mAngularErrorCorrection;
  ConstraintPositionCorrection::Enum mPositionCorrectionType;
};

////////////////////////////////////////////////////////////////////////////////////////////
/// Create block types for each kind of constraint, mostly to just give them names in the ui
////////////////////////////////////////////////////////////////////////////////////////////

/// The block type for a contact constraint
struct ContactBlock : public ConstraintConfigBlock
{
  ZilchDeclareType(TypeCopyMode::ReferenceType);
  ContactBlock() {mJointId = Zero::JointEnums::JointCount;}
};

/// Use the define/include trick to create a named block for each joint type
#define JointType(jointType)                                           \
  struct jointType##Block : public ConstraintConfigBlock               \
  {                                                                    \
    ZilchDeclareType(TypeCopyMode::ReferenceType);                     \
    jointType##Block() {mJointId = Zero::JointEnums::jointType##Type;} \
  };
#include "Physics/Joints/JointList.hpp"
#undef JointType


//-------------------------------------------------------------------PhysicsSolverConfig
/// Defines various configuration values used by physics to solve constraints.
/// This resource defines a tiered set of properties that can be overridden global or per constraint type.
class PhysicsSolverConfig : public DataResource
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  PhysicsSolverConfig();
  ~PhysicsSolverConfig();

  //-------------------------------------------------------------------DataResource Interface
  void Serialize(Serializer& stream) override;
  void Initialize() override;
  HandleOf<Resource> Clone() override;
  HandleOf<PhysicsSolverConfig> RuntimeClone();
  void ResourceModified() override;

  //-------------------------------------------------------------------Properties
  /// The number of iterations used in the constraint solver. Affects how stiff joints will be.
  uint GetSolverIterationCount();
  void SetSolverIterationCount(uint count);
  /// The number of iterations used for position correction (if position correction is used).
  uint GetPositionIterationCount();
  void SetPositionIterationCount(uint count);
  /// To prevent numerical issues, restitution is only applied if the
  /// relative velocity between the two objects is above this value.
  real GetVelocityRestitutionThreshold();
  void SetVelocityRestitutionThreshold(real threshold);

  /// The kind of solver used. For the most part this is
  /// internal and should only affect performance.
  PhysicsSolverType::Enum GetSolverType() const;
  void SetSolverType(PhysicsSolverType::Enum solverType);
  /// What method should be used to fix errors in joints. Baumgarte fixes errors by
  /// adding extra velocity but results in a more spongy behavior. Post Stabilization
  /// fixes errors by directly modifying position but can behave worse in unsolvable configurations.
  PhysicsSolverPositionCorrection::Enum GetPositionCorrectionType();
  void SetPositionCorrectionType(PhysicsSolverPositionCorrection::Enum correctionType);
  /// What kind of solver to use for post stabilization. Mostly for testing.
  PhysicsSolverSubType::Enum GetSubCorrectionType();
  void SetSubCorrectionType(PhysicsSolverSubType::Enum subType);

  //-------------------------------------------------------------------Internal
  ConstraintConfigBlock& GetContactBlock();
  void RebuildConstraintBlockValues();
  void CopyTo(PhysicsSolverConfig* destination);

  // Composition interface
  uint GetSize() const;
  HandleOf<ConstraintConfigBlock> GetBlockAt(uint index);
  HandleOf<ConstraintConfigBlock> GetById(BoundType* typeId);
  void Add(const HandleOf<ConstraintConfigBlock>& block, int index);
  bool Remove(const HandleOf<ConstraintConfigBlock>& block);

  uint mSolverIterationCount;
  uint mPositionIterationCount;
  real mVelocityRestitutionThreshold;
  /// Should warm starting be performed? This should always be true. Exposed property for debugging.
  bool mWarmStart;
  /// Should contact caching be performed? This should always be true. Exposed property for debugging.
  bool mCacheContacts;
  PhysicsContactTangentTypes::Enum mTangentType;
  PhysicsSolverType::Enum mSolverType;
  PhysicsSolverPositionCorrection::Enum mPositionCorrectionType;
  PhysicsSolverSubType::Enum mSubType;

  Array<ConstraintConfigBlock*> mBlocks;

  Array<ConstraintConfigBlock> mJointBlocks;
  ConstraintConfigBlock mContactBlock;
};

typedef SimpleResourceFactory<PhysicsSolverConfig, ConstraintConfigBlock> PhysicsSolverConfigMetaComposition;

//-------------------------------------------------------------------PhysicsSolverConfigManager
class PhysicsSolverConfigManager : public ResourceManager
{
public:
  DeclareResourceManager(PhysicsSolverConfigManager, PhysicsSolverConfig);

  PhysicsSolverConfigManager(BoundType* resourceType);
};

}//namespace Zero
