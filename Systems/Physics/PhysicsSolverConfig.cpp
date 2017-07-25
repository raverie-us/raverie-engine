///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2014-2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

//-------------------------------------------------------------------ConstraintConfigBlock
ZilchDefineType(ConstraintConfigBlock, builder, type)
{
  ZeroBindDocumented();

  ZilchBindGetterSetterProperty(Slop);
  ZilchBindGetterSetterProperty(LinearBaumgarte);
  ZilchBindGetterSetterProperty(AngularBaumgarte);
  ZilchBindGetterSetterProperty(LinearErrorCorrection);
  ZilchBindGetterSetterProperty(AngularErrorCorrection);
  ZilchBindGetterSetterProperty(PositionCorrectionType);
}

ConstraintConfigBlock::ConstraintConfigBlock()
{
  mJointId = 0;
  ConstraintConfigBlock::ResetDefaultValues();
}

void ConstraintConfigBlock::Serialize(Serializer& stream)
{
  SerializeNameDefault(mSlop, real(.02f));
  SerializeNameDefault(mLinearBaumgarte, real(4.5f));
  SerializeNameDefault(mAngularBaumgarte, real(4.5f));
  SerializeNameDefault(mLinearErrorCorrection, real(0.2f));
  SerializeNameDefault(mAngularErrorCorrection, real(0.2f));
  SerializeEnumName(ConstraintPositionCorrection, mPositionCorrectionType);
}

real ConstraintConfigBlock::GetSlop()
{
  return mSlop;
}

void ConstraintConfigBlock::SetSlop(real slop)
{
  if(slop < 0)
  {
    DoNotifyWarning("Invalid value", "The slop value cannot be negative. Clamping value to be positive.");
    slop = 0;
  }
  mSlop = slop;
}

real ConstraintConfigBlock::GetLinearBaumgarte()
{
  return mLinearBaumgarte;
}

void ConstraintConfigBlock::SetLinearBaumgarte(real linearBaumgarte)
{
  real minValue = 0;
  real maxValue = 100;
  if(linearBaumgarte < minValue)
  {
    String msg = String::Format("LinearBaumgarte must be positive. Clamping to the range of [%g, %g]", minValue, maxValue);
    DoNotifyWarning("Invalid value", msg);
    linearBaumgarte = minValue;
  }
  else if(linearBaumgarte > maxValue)
  {
    String msg = String::Format("To prevent instabilities, LinearBaumgarte must be below %g. Clamping to the range of [%g, %g]", maxValue, minValue, maxValue);
    DoNotifyWarning("Invalid value", msg);
    linearBaumgarte = maxValue;
  }
  mLinearBaumgarte = linearBaumgarte;
}

real ConstraintConfigBlock::GetAngularBaumgarte()
{
  return mAngularBaumgarte;
}

void ConstraintConfigBlock::SetAngularBaumgarte(real angularBaumgarte)
{
  real minValue = 0;
  real maxValue = 100;
  if(angularBaumgarte < minValue)
  {
    String msg = String::Format("AngularBaumgarte must be positive. Clamping to the range of [%g, %g]", minValue, maxValue);
    DoNotifyWarning("Invalid value", msg);
    angularBaumgarte = minValue;
  }
  else if(angularBaumgarte > maxValue)
  {
    String msg = String::Format("To prevent instabilities, AngularBaumgarte must be below %g. Clamping to the range of [%g, %g]", maxValue, minValue, maxValue);
    DoNotifyWarning("Invalid value", msg);
    angularBaumgarte = maxValue;
  }
  mAngularBaumgarte = angularBaumgarte;
}

real ConstraintConfigBlock::GetLinearErrorCorrection()
{
  return mLinearErrorCorrection;
}

void ConstraintConfigBlock::SetLinearErrorCorrection(real maxError)
{
  if(maxError < 0)
  {
    DoNotifyWarning("Invalid Value", "LinearErrorCorrection must be positive. Clamping value to 0.");
    maxError = 0;
  }

  mLinearErrorCorrection = maxError;
}

real ConstraintConfigBlock::GetAngularErrorCorrection()
{
  return mAngularErrorCorrection;
}

void ConstraintConfigBlock::SetAngularErrorCorrection(real maxError)
{
  if(maxError < 0)
  {
    DoNotifyWarning("Invalid Value", "AngularErrorCorrection must be positive. Clamping value to 0.");
    maxError = 0;
  }

  mAngularErrorCorrection = maxError;
}

ConstraintPositionCorrection::Enum ConstraintConfigBlock::GetPositionCorrectionType()
{
  return mPositionCorrectionType;
}

void ConstraintConfigBlock::SetPositionCorrectionType(ConstraintPositionCorrection::Enum correctionType)
{
  // Deal with special joint types that can't be position corrected
  if(mJointId == JointEnums::RelativeVelocityJointType)
  {
    DoNotifyWarning("Cannot change correction type", "RelativeVelocityJoint is a velocity constraint hence it must use baumgarte correction");
    return;
  }

  // Validate a correct enum value
  if(correctionType >= ConstraintPositionCorrection::Size)
  {
    DoNotifyException("Invalid value", "CorrectionType must be set to a valid value from the ConstraintPositionCorrection enum");
    return;
  }
  mPositionCorrectionType = correctionType;
}

void ConstraintConfigBlock::ResetDefaultValues()
{
  // Set some default values that most constraint types use
  mSlop = real(.02f);
  mLinearBaumgarte = real(4.5f);
  mAngularBaumgarte = real(4.5f);
  mLinearErrorCorrection = real(0.2f);
  mAngularErrorCorrection = real(0.2f);
  mPositionCorrectionType = ConstraintPositionCorrection::Inherit;

  // Set joint specific values
  switch(mJointId)
  {
    case JointEnums::PositionJointType:
    {
      mLinearBaumgarte = real(15.3f);
      break;
    }
    case JointEnums::PrismaticJointType:
    {
      mLinearBaumgarte = real(1.0f);
      mSlop = real(0.1f);
      break;
    }
    case JointEnums::RelativeVelocityJointType:
    {
      mPositionCorrectionType = ConstraintPositionCorrection::Baumgarte;
      mLinearBaumgarte = real(1.0f);
      mSlop = real(0);
      break;
    }
    case JointEnums::StickJointType:
    {
      mLinearBaumgarte = real(0.4f);
      break;
    }
    case JointEnums::ManipulatorJointType:
    {
      mPositionCorrectionType = ConstraintPositionCorrection::Baumgarte;
      break;
    }
    case JointEnums::PhyGunJointType:
    {
      mPositionCorrectionType = ConstraintPositionCorrection::Baumgarte;
      break;
    }
    case JointEnums::RevoluteJointType:
    {
      mLinearBaumgarte = real(15.3f);
      mAngularBaumgarte = real(0.8f);
      break;
    }
    case JointEnums::FixedAngleJointType:
    {
      mSlop = real(0);
      break;
    }
    case JointEnums::UprightJointType:
    {
      mSlop = real(0);
      break;
    }
    case JointEnums::WeldJointType:
    {
      mSlop = real(0);
      break;
    }
    case JointEnums::WheelJointType:
    {
      mSlop = real(0);
      break;
    }
    case JointEnums::WheelJoint2dType:
    {
      mSlop = real(0);
      break;
    }
    case JointEnums::LinearAxisJointType:
    {
      mSlop = real(0);
      break;
    }
    // Currently this denotes a contact constraint
    case JointEnums::JointCount:
    {
      mSlop = real(.02f);
      mLinearBaumgarte = real(2.0f);
      break;
    }
  }
}

//-------------------------------------------------------------------ContactBlock
ZilchDefineType(ContactBlock, builder, type)
{
  ZeroBindComponent();
  type->HasOrAdd<::Zero::CogComponentMeta>(type);
  type->Add(new MetaSerialization());
  ZeroBindSetup(SetupMode::DefaultSerialization);
}

//-------------------------------------------------------------------Define each joint block type
#define JointType(jointType)                        \
  ZilchDefineType(jointType##Block, builder, type)  \
  {                                                 \
    ZeroBindComponent();                            \
    type->HasOrAdd<::Zero::CogComponentMeta>(type);    \
    type->Add(new MetaSerialization());             \
    ZeroBindSetup(SetupMode::DefaultSerialization); \
  }
#include "Physics/Joints/JointList.hpp"
#undef JointType

//-------------------------------------------------------------------Solver Config Factory
ZilchDefineType(PhysicsSolverConfigMetaComposition, builder, type)
{
}

//-------------------------------------------------------------------PhysicsSolverConfig
ZilchDefineType(PhysicsSolverConfig, builder, type)
{
  ZeroBindDocumented();
  ZilchBindDefaultDestructor();
  type->Add(new PhysicsSolverConfigMetaComposition(true));
  
  ZilchBindGetterSetterProperty(SolverIterationCount);
  ZilchBindGetterSetterProperty(PositionIterationCount);
  ZilchBindGetterSetterProperty(VelocityRestitutionThreshold);

  bool inDevConfig = Z::gEngine->GetConfigCog()->has(Zero::DeveloperConfig) != nullptr;
  // @JoshD: Hide for now so these won't confuse anyone
  // These properties are only for showing people how constraints handle
  // when you don't implement important features
  //if (inDevConfig)
  //{
  //  ZilchBindFieldProperty(mWarmStart);
  //  ZilchBindFieldProperty(mCacheContacts);
  //  ZilchBindGetterSetterProperty(SubCorrectionType);
  //}
  //else
  //{
  //  ZilchBindField(mWarmStart);
  //  ZilchBindField(mCacheContacts);
  //  ZilchBindGetterSetter(SubCorrectionType);
  //}
  //ZilchBindGetterSetterProperty(SolverType);

  ZilchBindGetterSetterProperty(PositionCorrectionType);
}

PhysicsSolverConfig::PhysicsSolverConfig()
{
  mTangentType = PhysicsContactTangentTypes::OrthonormalTangents;
  mPositionCorrectionType = PhysicsSolverPositionCorrection::Baumgarte;
  mSolverType = PhysicsSolverType::Basic;
  mSubType = PhysicsSolverSubType::BasicSolving;
}

PhysicsSolverConfig::~PhysicsSolverConfig()
{
  DeleteObjectsInContainer(mBlocks);
}

void PhysicsSolverConfig::Serialize(Serializer& stream)
{
  SerializeNameDefault(mSolverIterationCount, 15u);
  SerializeNameDefault(mPositionIterationCount, 3u);
  SerializeNameDefault(mVelocityRestitutionThreshold, real(3.0f));
  SerializeNameDefault(mWarmStart, true);
  SerializeNameDefault(mCacheContacts, true);

  SerializeEnumNameDefault(PhysicsContactTangentTypes, mTangentType, PhysicsContactTangentTypes::OrthonormalTangents);
  SerializeEnumNameDefault(PhysicsSolverPositionCorrection, mPositionCorrectionType, PhysicsSolverPositionCorrection::PostStabilization);
  SerializeEnumNameDefault(PhysicsSolverType, mSolverType, PhysicsSolverType::Basic);
  SerializeEnumNameDefault(PhysicsSolverSubType, mSubType, PhysicsSolverSubType::BlockSolving);

  // Serialize our composition of constraint config blocks
  BoundType* selfBoundType = this->ZilchGetDerivedType();
  PhysicsSolverConfigMetaComposition* factory = selfBoundType->Has<PhysicsSolverConfigMetaComposition>();
  factory->SerializeArray(stream, mBlocks);

  // Now that we've serialized all blocks we can cache the necessary run-time values
  RebuildConstraintBlockValues();
}

void PhysicsSolverConfig::Initialize()
{
  RebuildConstraintBlockValues();
}

void PhysicsSolverConfig::ResourceModified()
{
  RebuildConstraintBlockValues();
}

uint PhysicsSolverConfig::GetSolverIterationCount()
{
  return mSolverIterationCount;
}

void PhysicsSolverConfig::SetSolverIterationCount(uint count)
{
  bool wasClamped;
  mSolverIterationCount = Math::DebugClamp(count, 0u, 100u, wasClamped);

  if(wasClamped)
    DoNotifyWarning("Invalid Value", "Iteration count must be in the range [0, 100]");
}

uint PhysicsSolverConfig::GetPositionIterationCount()
{
  return mPositionIterationCount;
}

void PhysicsSolverConfig::SetPositionIterationCount(uint count)
{
  bool wasClamped;
  mPositionIterationCount = Math::DebugClamp(count, 0u, 100u, wasClamped);

  if(wasClamped)
    DoNotifyWarning("Invalid Value", "Iteration count must be in the range [0, 100]");
}

real PhysicsSolverConfig::GetVelocityRestitutionThreshold()
{
  return mVelocityRestitutionThreshold;
}

void PhysicsSolverConfig::SetVelocityRestitutionThreshold(real threshold)
{
  if(threshold < 0)
  {
    DoNotifyWarning("Invalid Value", "VelocityResolutionThreshold must be positive");
    threshold = 0;
  }

  mVelocityRestitutionThreshold = threshold;
}

PhysicsSolverType::Enum PhysicsSolverConfig::GetSolverType() const
{
  return mSolverType;
}

void PhysicsSolverConfig::SetSolverType(PhysicsSolverType::Enum solverType)
{
  if(solverType >= PhysicsSolverType::Size)
  {
    DoNotifyWarning("Invalid value", "SolverType must be set to a valid value from the SolverType enum");
    return;
  }
  mSolverType = solverType;
}

PhysicsSolverPositionCorrection::Enum PhysicsSolverConfig::GetPositionCorrectionType()
{
  return mPositionCorrectionType;
}

void PhysicsSolverConfig::SetPositionCorrectionType(PhysicsSolverPositionCorrection::Enum correctionType)
{
  if(correctionType >= PhysicsSolverPositionCorrection::Size)
  {
    DoNotifyWarning("Invalid value", "PositionCorrectionType must be set to a valid value from the PositionCorrection enum");
    return;
  }
  mPositionCorrectionType = correctionType;
}

PhysicsSolverSubType::Enum PhysicsSolverConfig::GetSubCorrectionType()
{
  return mSubType;
}

void PhysicsSolverConfig::SetSubCorrectionType(PhysicsSolverSubType::Enum subType)
{
  mSubType = subType;
}

ConstraintConfigBlock& PhysicsSolverConfig::GetContactBlock()
{
  return mContactBlock;
}

void PhysicsSolverConfig::RebuildConstraintBlockValues()
{
  // Reset all cached joint/contact blocks to their default values
  mJointBlocks.Resize(JointEnums::JointCount);
  for(size_t i = 0; i < mJointBlocks.Size(); ++i)
  {
    mJointBlocks[i].mJointId = i;
    mJointBlocks[i].ResetDefaultValues();
  }
  mContactBlock.mJointId = JointEnums::JointCount;
  mContactBlock.ResetDefaultValues();

  // For each user-defined block that we have, copy the block's values over the cached block's values.
  // We do this because we need configuration blocks for all constraint types even if the user doesn't
  // define them (and some have different default values).
  for(size_t i = 0; i < mBlocks.Size(); ++i)
  {
    if(mBlocks[i]->mJointId >= JointEnums::JointCount)
      mContactBlock = *mBlocks[i];
    else
      mJointBlocks[mBlocks[i]->mJointId] = *mBlocks[i];
  }
}

void PhysicsSolverConfig::CopyTo(PhysicsSolverConfig* destination)
{
  destination->mSolverIterationCount = mSolverIterationCount;
  destination->mPositionIterationCount = mPositionIterationCount;
  destination->mVelocityRestitutionThreshold = mVelocityRestitutionThreshold;
  destination->mWarmStart = mWarmStart;
  destination->mCacheContacts = mCacheContacts;
  destination->mTangentType = mTangentType;
  destination->mPositionCorrectionType = mPositionCorrectionType;
  destination->mSolverType = mSolverType;
  destination->mSubType = mSubType;

  // Clear the old blocks from our destination
  DeleteObjectsInContainer(destination->mBlocks);
  destination->mBlocks.Clear();
  // Clone all of our blocks into the destination
  BoundType* selfBoundType = this->ZilchGetDerivedType();
  PhysicsSolverConfigMetaComposition* factory = selfBoundType->Has<PhysicsSolverConfigMetaComposition>();
  for(size_t i = 0; i < mBlocks.Size(); ++i)
  {
    BoundType* blockMeta = ZilchVirtualTypeId(mBlocks[i]);
    ConstraintConfigBlock* newBlock = factory->MakeObject(blockMeta).Get<ConstraintConfigBlock*>();
    destination->mBlocks.PushBack(newBlock);
  }

  destination->mJointBlocks = mJointBlocks;
  destination->mContactBlock = mContactBlock;

  Array<ConstraintConfigBlock*> mBlocks;
}

uint PhysicsSolverConfig::GetSize() const
{
  return mBlocks.Size();
}

HandleOf<ConstraintConfigBlock> PhysicsSolverConfig::GetBlockAt(uint index)
{
  return mBlocks[index];
}

HandleOf<ConstraintConfigBlock> PhysicsSolverConfig::GetById(BoundType* typeId)
{
  // Find a block with the given typeId if it exists
  for(size_t i = 0; i < mBlocks.Size(); ++i)
  {
    ConstraintConfigBlock* block = mBlocks[i];
    if(ZilchVirtualTypeId(block) == typeId)
      return block;
  }

  return nullptr;
}

void PhysicsSolverConfig::Add(const HandleOf<ConstraintConfigBlock>& block, int index)
{
  // Set the newly created block's default values
  block->ResetDefaultValues();
  // Add it to our block list and then rebuild cached values
  mBlocks.PushBack(block);
  RebuildConstraintBlockValues();
}

bool PhysicsSolverConfig::Remove(const HandleOf<ConstraintConfigBlock>& block)
{
  // Try to find the index of this block
  size_t index = mBlocks.FindIndex(block);
  if(index >= mBlocks.Size())
    return false;

  // If we found the block then remove it. Also rebuild all cached values
  mBlocks.EraseAt(index);
  RebuildConstraintBlockValues();
  return true;
}

//-------------------------------------------------------------------PhysicsSolverConfigManager
ImplementResourceManager(PhysicsSolverConfigManager, PhysicsSolverConfig);

DefinePhysicsRuntimeClone(PhysicsSolverConfig);

PhysicsSolverConfigManager::PhysicsSolverConfigManager(BoundType* resourceType)
  : ResourceManager(resourceType)
{
  AddLoader("PhysicsSolverConfig", new TextDataFileLoader<PhysicsSolverConfigManager>());
  mCategory = "Physics";
  mCanAddFile = true;
  mOpenFileFilters.PushBack(FileDialogFilter("*.PhysicsSolverConfig.data"));
  DefaultResourceName = "Baumgarte";
  FallbackResourceName = "PostStabilization";
  mCanCreateNew = true;
  mCanDuplicate = true;
  mExtension = DataResourceExtension;
}

}//namespace Zero
