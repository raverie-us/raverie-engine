// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"

namespace Raverie
{

namespace Events
{
RaverieDefineEvent(CollectSpecializationConstants);
} // namespace Events

RaverieDefineType(SpecializationConstantEvent, builder, type)
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

SpirVSpecializationConstantPass::SpirVSpecializationConstantPass()
{
  mFreezeAllConstants = true;
}

bool SpirVSpecializationConstantPass::RunTranslationPass(ShaderTranslationPassResult& inputData, ShaderTranslationPassResult& outputData)
{
  mErrorLog.Clear();

  // By default, all of the reflection data is the same as the input stage
  outputData.mReflectionData = inputData.mReflectionData;

  Array<String> flags;
  // Get the flags for specializations
  GetSpecializationFlags(flags, inputData.mReflectionData, outputData.mReflectionData);
  bool success = true;
  // Only run the optimizer if there's specialization constant flags.
  if (!flags.Empty())
    success = RunOptimizer(SPV_OPTIMIZER_NO_PASS, flags, inputData.mByteStream, outputData.mByteStream);
  else
    // Otherwise, just copy over the stream data
    outputData.mByteStream.Load(inputData.mByteStream.Data(), inputData.mByteStream.ByteCount());

  return success;
}

void SpirVSpecializationConstantPass::GetSpecializationFlags(Array<String>& outFlags, ShaderStageInterfaceReflection& inputStageReflection, ShaderStageInterfaceReflection& outputStageReflection)
{
  // Query for any constants unique to this pass (such as fragment properties)
  SpecializationConstantEvent toSend;
  toSend.mSpecializationConstantPass = this;
  toSend.mInputReflectionData = &inputStageReflection;
  EventSend(this, Events::CollectSpecializationConstants, &toSend);

  // Convert these flags into a better working format
  SetSpecializationValues(outFlags, toSend.mSpecializationOverridesById);
  // If requested, freeze all constants (optimizer only supports all or nothing)
  if (mFreezeAllConstants)
  {
    outFlags.PushBack("--freeze-spec-const");
    // Freezing constants means that these are no longer valid specialization
    // constants to set so we need to clear them from reflection.
    outputStageReflection.mSpecializationConstants.Clear();
  }
}

void SpirVSpecializationConstantPass::SetSpecializationValues(Array<String>& outFlags, HashMap<int, String>& overrides)
{
  AutoDeclare(range, overrides.All());
  for (; !range.Empty(); range.PopFront())
  {
    AutoDeclareReference(pair, range.Front());
    int specConstantId = pair.first;
    String specConstantValue = pair.second;

    // If the id exists, set the default value (has to be by string at the
    // moment)
    String flag = String::Format("--set-spec-const-default-value=%d:%s", specConstantId, specConstantValue.c_str());
    outFlags.PushBack(flag);
  }
}

} // namespace Raverie
