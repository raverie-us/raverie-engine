///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

namespace Events
{
ZilchDefineEvent(CollectSpecializationConstants);
}//namespace Events

 //-------------------------------------------------------------------SpecializationConstantEvent
ZilchDefineType(SpecializationConstantEvent, builder, type)
{
}

SpecializationConstantEvent::SpecializationConstantEvent()
{
  mSpecializationConstantPass = nullptr;
  mInputReflectionData = nullptr;
}

int SpecializationConstantEvent::GetFirstId(StringParam name)
{
  int id = mInputReflectionData->mSpecializationConstants.FindValue(name, -1);
  return id;
}

//-------------------------------------------------------------------SpirVSpecializationConstantPass
SpirVSpecializationConstantPass::SpirVSpecializationConstantPass()
{
  mFreezeAllConstants = true;
}

bool SpirVSpecializationConstantPass::RunTranslationPass(ShaderTranslationPassResult& inputData, ShaderTranslationPassResult& outputData)
{
  // By default, all of the reflection data is the same as the input stage
  outputData.mReflectionData = inputData.mReflectionData;
  ShaderStageInterfaceReflection& stageReflection = outputData.mReflectionData;

  spvtools::Optimizer optimizer((spv_target_env)mTargetEnv);
  String flag = String::Format("--set-spec-const-default-value=%d:%s", 1, "1,2,3");
  optimizer.RegisterPassFromFlag(flag.c_str());

  // Query for any constants unique to this pass (such as fragment properties)
  SpecializationConstantEvent toSend;
  toSend.mSpecializationConstantPass = this;
  toSend.mInputReflectionData = &inputData.mReflectionData;
  EventSend(this, Events::CollectSpecializationConstants, &toSend);
  SetSpecializationValues(optimizer, stageReflection, toSend.mSpecializationOverridesById);

  if(mFreezeAllConstants)
  {
    optimizer.RegisterPassFromFlag("--freeze-spec-const");
    // Freezing constants means that these are no longer valid specialization
    // constants to set so we need to clear them from reflection.
    stageReflection.mSpecializationConstants.Clear();
  }

  bool success = RunOptimizer(optimizer, inputData.mByteStream, outputData.mByteStream);
  return success;
}

void SpirVSpecializationConstantPass::SetSpecializationValues(spvtools::Optimizer& optimizer, ShaderStageInterfaceReflection& stageReflection, HashMap<int, String>& overrides)
{
  AutoDeclare(range, overrides.All());
  for(; !range.Empty(); range.PopFront())
  {
    AutoDeclareReference(pair, range.Front());
    int specConstantId = pair.first;
    String specConstantValue = pair.second;

    // If the id exists, set the default value (has to be by string at the moment)
    String flag = String::Format("--set-spec-const-default-value=%d:%s", specConstantId, specConstantValue.c_str());
    optimizer.RegisterPassFromFlag(flag.c_str());
  }
}

}//namespace Zero
