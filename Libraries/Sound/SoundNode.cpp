///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.hpp"

namespace Zero
{

using namespace AudioConstants;

namespace Events
{

DefineEvent(AudioInterpolationDone);
DefineEvent(SoundNodeDisconnected);
DefineEvent(SoundListenerRemoved);

} // namespace Events

//--------------------------------------------------------------------------------------- Sound Node

//**************************************************************************************************
ZilchDefineType(SoundNode, builder, type)
{
  ZeroBindDocumented();

  ZilchBindMethod(AddInputNode);
  ZilchBindMethod(InsertNodeAfter);
  ZilchBindMethod(InsertNodeBefore);
  ZilchBindMethod(ReplaceWith);
  ZilchBindMethod(RemoveInputNode);
  ZilchBindMethod(RemoveAllInputs);
  ZilchBindMethod(RemoveAllOutputs);
  ZilchBindMethod(RemoveAndAttachInputsToOutputs);
  ZilchBindGetterSetter(AutoCollapse);
  ZilchBindGetter(HasInputs);
  ZilchBindGetter(HasOutputs);
  ZilchBindGetter(InputCount);
  ZilchBindGetter(OutputCount);
  ZilchBindGetterSetter(BypassPercent)->AddAttribute(DeprecatedAttribute);
  ZilchBindGetterSetter(BypassValue);

  ZeroBindEvent(Events::AudioInterpolationDone, SoundEvent);
  ZeroBindEvent(Events::SoundNodeDisconnected, SoundEvent);
  ZeroBindEvent(Events::SoundListenerRemoved, SoundEvent);
}

//**************************************************************************************************
SoundNode::SoundNode(StringParam name, int ID, bool listenerDependent, bool generator) :
  cNodeID(ID),
  cName(name),
  mOkayToSaveThreaded(!listenerDependent),
  mInProcessThreaded(false),
  mMixedVersionThreaded(Z::gSound->Mixer.mMixVersionThreaded - 1),
  mNumMixedChannelsThreaded(0),
  mMixedListenerThreaded(nullptr),
  mWillCollapse(0),
  mValidOutputLastMix(false),
  mListenerDependentThreaded(listenerDependent),
  mBypassValue(0.0f),
  mGeneratorThreaded(generator)
{
  ConnectThisTo(&(Z::gSound->Mixer), Events::SoundListenerRemoved, RemoveListenerThreaded);
}

//**************************************************************************************************
SoundNode::~SoundNode()
{

}

//**************************************************************************************************
void SoundNode::AddInputNode(SoundNode* newNode)
{
  if (!newNode)
  {
    DoNotifyWarning("Incorrect SoundNode Operation", "Attempted to add SoundNode to null object");
    return;
  }

  if (newNode == this)
  {
    DoNotifyWarning("Incorrect SoundNode Operation", "Attempted to add SoundNode to itself as input");
    return;
  }

  HandleOf<SoundNode> handle(newNode);

  // Check if this node is already an input
  if (mInputs[AudioThreads::MainThread].Contains(handle))
    return;

  // Add new node to this node's inputs
  mInputs[AudioThreads::MainThread].PushBack(handle);
  // Add this node to the new node's outputs
  newNode->mOutputs[AudioThreads::MainThread].PushBack(this);

  Z::gSound->Mixer.AddTask(CreateFunctor(&SoundNode::AddInputNodeThreaded, this, handle), this);
}

//**************************************************************************************************
void SoundNode::RemoveInputNode(SoundNode* node)
{
  if (!node)
  {
    DoNotifyWarning("Incorrect SoundNode Operation", "Attempted to remove a null object from SoundNode inputs");
    return;
  }

  HandleOf<SoundNode> handle(node);

  // Remove node from input list
  if (!mInputs[AudioThreads::MainThread].EraseValue(handle))
    return;

  // Remove this node from the input node's output list
  node->mOutputs[AudioThreads::MainThread].EraseValue(HandleOf<SoundNode>(this));

  Z::gSound->Mixer.AddTask(CreateFunctor(&SoundNode::RemoveInputNodeThreaded, this, handle), this);

  // If there are no more inputs and this node should collapse, call the collapse function
  if (mInputs[AudioThreads::MainThread].Empty() && mWillCollapse.Get() == cTrue)
    CollapseNode();

  // If this node is now disconnected, send an event
  if (mInputs[AudioThreads::MainThread].Empty() && mOutputs[AudioThreads::MainThread].Empty())
  {
    SoundEvent event;
    DispatchEvent(Events::SoundNodeDisconnected, &event);
  }
}

//**************************************************************************************************
void SoundNode::InsertNodeBefore(SoundNode* node)
{
  if (!node)
  {
    DoNotifyWarning("Incorrect SoundNode Operation", "Attempted to add sound node to null object");
    return;
  }

  if (node == this)
  {
    DoNotifyWarning("Incorrect SoundNode Operation", "Attempted to insert SoundNode before itself");
    return;
  }

  // If there are no current inputs, just add the new node as input
  if (mInputs[AudioThreads::MainThread].Empty())
  {
    AddInputNode(node);
    return;
  }

  // Add all of this node's inputs to the new node and remove them from this node
  while (!mInputs[AudioThreads::MainThread].Empty())
  {
    node->AddInputNode(mInputs[AudioThreads::MainThread].Back());
    RemoveInputNode(mInputs[AudioThreads::MainThread].Back());
  }

  // Add new node as only input
  AddInputNode(node);
}

//**************************************************************************************************
void SoundNode::InsertNodeAfter(SoundNode* node)
{
  if (!node)
  {
    DoNotifyWarning("Incorrect SoundNode Operation", "Attempted to add sound node to null object");
    return;
  }

  if (node == this)
  {
    DoNotifyWarning("Incorrect SoundNode Operation", "Attempted to insert SoundNode after itself");
    return;
  }

  // If there are no current outputs, just add this to the new node
  if (mOutputs[AudioThreads::MainThread].Empty())
  {
    node->AddInputNode(this);
    return;
  }

  // Remove this node from the inputs of all output nodes and add new node
  while (!mOutputs[AudioThreads::MainThread].Empty())
  {
    mOutputs[AudioThreads::MainThread].Back()->AddInputNode(node);
    mOutputs[AudioThreads::MainThread].Back()->RemoveInputNode(this);
  }

  // Add new node as this node's only output
  node->AddInputNode(this);
}

//**************************************************************************************************
void SoundNode::ReplaceWith(SoundNode* node)
{
  if (!node)
  {
    DoNotifyWarning("Incorrect SoundNode Operation", "Attempted to replace sound node with null object");
    return;
  }

  if (node == this)
  {
    DoNotifyWarning("Incorrect SoundNode Operation", "Attempted to replace SoundNode with itself");
    return;
  }

  // Remove this node as an output from all inputs and add to replacement node
  while (!mInputs[AudioThreads::MainThread].Empty())
  {
    SoundNode* inputNode = mInputs[AudioThreads::MainThread].Back();
    // Check if the replacement node is the same as the input node
    if (node == inputNode)
      // Can't add to itself, just remove from Inputs
      mInputs[AudioThreads::MainThread].PopBack();
    else
    {
      RemoveInputNode(inputNode);
      if (node)
        node->AddInputNode(inputNode);
    }
  }

  // Remove this node as an input from all outputs and add replacement node
  while (!mOutputs[AudioThreads::MainThread].Empty())
  {
    SoundNode* outputNode = mOutputs[AudioThreads::MainThread].Back();
    // Check if the replacement node is the same as the output node
    if (node == outputNode)
      // Can't add to itself, just remove from Outputs
      mOutputs[AudioThreads::MainThread].PopBack();
    else
    {
      outputNode->RemoveInputNode(this);
      if (node)
        outputNode->AddInputNode(node);
    }
  }
}

//**************************************************************************************************
void SoundNode::RemoveAllInputs()
{
  // Remove this node as output for all inputs
  while (!mInputs[AudioThreads::MainThread].Empty())
    RemoveInputNode(mInputs[AudioThreads::MainThread].Back());
}

//**************************************************************************************************
void SoundNode::RemoveAllOutputs()
{
  // Remove this node as input for all outputs
  while (!mOutputs[AudioThreads::MainThread].Empty())
    mOutputs[AudioThreads::MainThread].Back()->RemoveInputNode(this);
}

//**************************************************************************************************
void SoundNode::RemoveAndAttachInputsToOutputs()
{
  // Add all inputs to all outputs
  forRange(SoundNode* input, mInputs[AudioThreads::MainThread].All())
  {
    forRange(SoundNode* output, mOutputs[AudioThreads::MainThread].All())
      output->AddInputNode(input);
  }

  RemoveAllOutputs();
  RemoveAllInputs();

  ErrorIf(!mInputs[AudioThreads::MainThread].Empty() || !mOutputs[AudioThreads::MainThread].Empty(), 
    "SoundNode was not fully disconnected");
}

//**************************************************************************************************
bool SoundNode::GetAutoCollapse()
{
  return mWillCollapse.Get() == cTrue;
}

//**************************************************************************************************
void SoundNode::SetAutoCollapse(bool willCollapse)
{
  if (willCollapse)
    mWillCollapse.Set(cTrue);
  else
    mWillCollapse.Set(cFalse);
}

//**************************************************************************************************
bool SoundNode::GetHasInputs()
{
  return !mInputs[AudioThreads::MainThread].Empty();
}

//**************************************************************************************************
bool SoundNode::GetHasOutputs()
{
  return !mOutputs[AudioThreads::MainThread].Empty();
}

//**************************************************************************************************
int SoundNode::GetInputCount()
{
  return mInputs[AudioThreads::MainThread].Size();
}

//**************************************************************************************************
int SoundNode::GetOutputCount()
{
  return mOutputs[AudioThreads::MainThread].Size();
}

//**************************************************************************************************
float SoundNode::GetBypassPercent()
{
  return mBypassValue.Get(AudioThreads::MainThread) * 100.0f;
}

//**************************************************************************************************
void SoundNode::SetBypassPercent(float percent)
{
  mBypassValue.Set(Math::Clamp(percent, 0.0f, 100.0f) / 100.0f, AudioThreads::MainThread);
}

//**************************************************************************************************
float SoundNode::GetBypassValue()
{
  return mBypassValue.Get(AudioThreads::MainThread);
}

//**************************************************************************************************
void SoundNode::SetBypassValue(float value)
{
  mBypassValue.Set(Math::Clamp(value, 0.0f, 1.0f), AudioThreads::MainThread);
}

//**************************************************************************************************
void SoundNode::DisconnectThisAndAllInputs()
{
  // Call this function on all input nodes (removes inputs)
  while (!mInputs[AudioThreads::MainThread].Empty())
    mInputs[AudioThreads::MainThread].Back()->DisconnectThisAndAllInputs();

  // Remove this node as input for all outputs
  while (!mOutputs[AudioThreads::MainThread].Empty())
    mOutputs[AudioThreads::MainThread].Back()->RemoveInputNode(this);

  ErrorIf(!mInputs[AudioThreads::MainThread].Empty() || !mOutputs[AudioThreads::MainThread].Empty(), 
    "SoundNode was not fully disconnected");
}

//**************************************************************************************************
void SoundNode::DispatchEventFromMixThread(const String eventID)
{
  SoundEvent event;
  DispatchEvent(eventID, &event);
}

//**************************************************************************************************
void SoundNode::WarningFromMixThread(const String title, const String message)
{
  DoNotifyWarning(title, message);
}

//**************************************************************************************************
const SoundNode::NodeListType* SoundNode::GetInputs(AudioThreads::Enum whichThread)
{
  return &mInputs[whichThread];
}

//**************************************************************************************************
const SoundNode::NodeListType* SoundNode::GetOutputs(AudioThreads::Enum whichThread)
{
  return &mOutputs[whichThread];
}

//**************************************************************************************************
bool SoundNode::HasAudibleOutput()
{
  return mValidOutputLastMix.Get() == cTrue;
}

//**************************************************************************************************
float SoundNode::GetVolumeChangeFromOutputsThreaded()
{
  float volume = 0.0f;
  forRange(SoundNode* output, mOutputs[AudioThreads::MixThread].All())
    volume += output->GetVolumeChangeFromOutputsThreaded();

  return volume;
}

//**************************************************************************************************
bool SoundNode::Evaluate(BufferType* outputBuffer, const unsigned numberOfChannels, ListenerNode* listener)
{
  bool hasOutput;

  // Check if this version has already been mixed
  if (Z::gSound->Mixer.mMixVersionThreaded == mMixedVersionThreaded)
  {
    // Check for errors 
    String message;
    if (mInProcessThreaded)
      message = String::Format("Loop in sound node structure, disconnected %s", cName.c_str());
    else if (outputBuffer->Size() != mMixedOutputThreaded.Size())
      message = String::Format("Mismatch in buffer size requests on sound node, disconnected %s", cName.c_str());
    else if (numberOfChannels != mNumMixedChannelsThreaded)
      message = String::Format("Mismatch in channel requests on sound node, disconnected %s", cName.c_str());
    if (!message.Empty())
    {
      String title = "Incorrect SoundNode Structure";
      Z::gSound->Mixer.AddTaskThreaded(CreateFunctor(&SoundNode::WarningFromMixThread, this,
        title, message), this);

      Z::gSound->Mixer.AddTaskThreaded(CreateFunctor(&SoundNode::RemoveAndAttachInputsToOutputs, 
        this), this);

      return false;
    }

    // If this node is saved, or if the listener matches the last mix, can simply copy data
    if (mOkayToSaveThreaded || listener == mMixedListenerThreaded)
    {
      // Copy mixed samples to output buffer if there is real data
      if (mValidOutputLastMix.Get() == cTrue)
        memcpy(outputBuffer->Data(), mMixedOutputThreaded.Data(), sizeof(float) * outputBuffer->Size());

      hasOutput = mValidOutputLastMix.Get() == cTrue;
    }
    // Otherwise, need to get output again
    else
    {
      mInProcessThreaded = true;
      mMixedListenerThreaded = listener;

      // Get output
      hasOutput = GetOutputSamples(&mMixedOutputThreaded, numberOfChannels, listener, false);

      if (mValidOutputLastMix.Get() == cFalse && hasOutput)
        mValidOutputLastMix.Set(cTrue);

      // Copy mixed samples to output buffer if there is real data
      if (hasOutput)
        memcpy(outputBuffer->Data(), mMixedOutputThreaded.Data(), sizeof(float) * outputBuffer->Size());

      mInProcessThreaded = false;
    }
  }
  // Hasn't been mixed yet
  else
  {
    mInProcessThreaded = true;
    mMixedVersionThreaded = Z::gSound->Mixer.mMixVersionThreaded;
    mNumMixedChannelsThreaded = numberOfChannels;
    mMixedListenerThreaded = listener;

    // Set mixed array to same size as output array
    mMixedOutputThreaded.Resize(outputBuffer->Size());

    // Check whether this node's data can be saved
    mOkayToSaveThreaded = !mListenerDependentThreaded;
    // If this node can be saved, check all its inputs to verify
    // (if any inputs can't be saved, neither can this node)
    if (mOkayToSaveThreaded)
    {
      forRange(SoundNode* input, mInputs[AudioThreads::MixThread].All())
      {
        if (!input->mOkayToSaveThreaded)
        {
          mOkayToSaveThreaded = false;
          break;
        }
      }
    }

    // Get output
    hasOutput = GetOutputSamples(&mMixedOutputThreaded, numberOfChannels, listener, true);

    ErrorIf(hasOutput && (mMixedOutputThreaded[0] > 10.0f || mMixedOutputThreaded[0] < -10.0f),
      "Audio data is outside of normal values");

    if (hasOutput)
      mValidOutputLastMix.Set(cTrue);
    else
      mValidOutputLastMix.Set(cFalse);

    // Copy mixed samples to output buffer if there is real data
    if (hasOutput)
    {
      ErrorIf(outputBuffer->Size() != mMixedOutputThreaded.Size(), "Buffer sizes do not match when evaluating sound node");
      memcpy(outputBuffer->Data(), mMixedOutputThreaded.Data(), sizeof(float) * mMixedOutputThreaded.Size());
    }

    // Mark as finished
    mInProcessThreaded = false;
  }

  return hasOutput;
}

//**************************************************************************************************
bool SoundNode::AccumulateInputSamples(const unsigned howManySamples, const unsigned numberOfChannels, 
  ListenerNode* listener)
{
  // No sources, do nothing
  if (mInputs[AudioThreads::MixThread].Empty())
    return false;

  BufferType tempBuffer(howManySamples);
  bool isThereInput(false);

  // Reset buffer
  mInputSamplesThreaded.Resize(howManySamples);

  // Get samples from all inputs
  forRange(SoundNode* input, mInputs[AudioThreads::MixThread].All())
  {
    // Check if this input has actual output data
    if (input->Evaluate(&tempBuffer, numberOfChannels, listener))
    {
      ErrorIf(tempBuffer[0] > 10.0f || tempBuffer[0] < -10.0f, "Audio data is outside of normal limits");

      // If this is the first input data, just swap the buffers
      if (!isThereInput)
      {
        isThereInput = true;
        mInputSamplesThreaded.Swap(tempBuffer);
      }
      // Otherwise add the new samples to the existing ones
      else
      {
        for (BufferRange myData = mInputSamplesThreaded.All(), newData = tempBuffer.All(); !myData.Empty();
          myData.PopFront(), newData.PopFront())
        {
          myData.Front() += newData.Front();
        }
      }
    }
  }

  return isThereInput;
}

//**************************************************************************************************
void SoundNode::AddBypassThreaded(BufferType* outputBuffer)
{
  // If some of the node's output should be bypassed, adjust the output buffer 
  // with a percentage of the input buffer
  float bypassValue = mBypassValue.Get(AudioThreads::MixThread);
  if (bypassValue > 0.0f)
  {
    for (BufferRange outputRange = outputBuffer->All(), inputRange = mInputSamplesThreaded.All();
      !outputRange.Empty(); outputRange.PopFront(), inputRange.PopFront())
    {
      outputRange.Front() = (inputRange.Front() * bypassValue)
        + (outputRange.Front() * (1.0f - bypassValue));
    }
  }
}

//**************************************************************************************************
void SoundNode::AddInputNodeThreaded(HandleOf<SoundNode> newNode)
{
  // Don't need to check for valid data, was already checked on main thread

  // Add new node to this node's inputs
  mInputs[AudioThreads::MixThread].PushBack(newNode);
  // Add this node to the new node's outputs
  newNode->mOutputs[AudioThreads::MixThread].PushBack(this);
}

//**************************************************************************************************
void SoundNode::RemoveInputNodeThreaded(HandleOf<SoundNode> node)
{
  // Don't need to check for valid data, was already checked on main thread

  // Remove node from input list
  mInputs[AudioThreads::MixThread].EraseValue(node);

  // Remove this node from the input node's output list
  node->mOutputs[AudioThreads::MixThread].EraseValue(HandleOf<SoundNode>(this));
}

//----------------------------------------------------------------------------- Simple Collapse Node

//**************************************************************************************************
ZilchDefineType(SimpleCollapseNode, builder, Type)
{

}

//**************************************************************************************************
void SimpleCollapseNode::CollapseNode()
{
  // If node has inputs, don't collapse
  if (GetHasInputs())
    return;

  // If there are no more inputs, remove this node from all outputs
  RemoveAllOutputs();
}

//-------------------------------------------------------------------------------------- Output Node

//**************************************************************************************************
ZilchDefineType(OutputNode, builder, Type)
{

}

//**************************************************************************************************
OutputNode::OutputNode() :
  SoundNode("FinalOutputNode", 0, false, false)
{

}

//**************************************************************************************************
float OutputNode::GetVolumeChangeFromOutputsThreaded()
{
  return 1.0f;
}

//**************************************************************************************************
bool OutputNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels, 
  ListenerNode* listener, const bool firstRequest)
{
  // Get input
  bool isThereOutput = AccumulateInputSamples(outputBuffer->Size(), numberOfChannels, nullptr);

  // Move input to output buffer
  if (isThereOutput)
    mInputSamplesThreaded.Swap(*outputBuffer);

  return isThereOutput;
}

//------------------------------------------------------------------------------------- Combine Node

//**************************************************************************************************
ZilchDefineType(CombineNode, builder, Type)
{

}

//**************************************************************************************************
CombineNode::CombineNode(Zero::StringParam name, unsigned ID) :
  SimpleCollapseNode(name, ID, false, false)
{

}

//**************************************************************************************************
bool CombineNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels, 
  ListenerNode* listener, const bool firstRequest)
{
  // Get input
  bool isThereOutput = AccumulateInputSamples(outputBuffer->Size(), numberOfChannels, listener);

  // Move input to output buffer
  if (isThereOutput)
    mInputSamplesThreaded.Swap(*outputBuffer);

  return isThereOutput;
}

//--------------------------------------------------------------------------- Combine And Pause Node

//**************************************************************************************************
ZilchDefineType(CombineAndPauseNode, builder, Type)
{

}

//**************************************************************************************************
CombineAndPauseNode::CombineAndPauseNode(Zero::StringParam name, unsigned ID) :
  SimpleCollapseNode(name, ID, false, false),
  mPaused(cFalse),
  mPausingThreaded(false),
  mMuted(cFalse),
  mMutingThreaded(false),
  mInterpolatingThreaded(false)
{

}

//**************************************************************************************************
bool CombineAndPauseNode::GetPaused()
{
  return mPaused.Get() == cTrue;
}

//**************************************************************************************************
void CombineAndPauseNode::SetPaused(const bool paused)
{
  Z::gSound->Mixer.AddTask(CreateFunctor(&CombineAndPauseNode::SetPausedThreaded, this, paused), this);
}

//************************************************************************************************
bool CombineAndPauseNode::GetMuted()
{
  return mMuted.Get() == cTrue;
}

//************************************************************************************************
void CombineAndPauseNode::SetMuted(bool muted)
{
  Z::gSound->Mixer.AddTask(CreateFunctor(&CombineAndPauseNode::SetMutedThreaded, this, muted), this);
}

//**************************************************************************************************
bool CombineAndPauseNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels, 
  ListenerNode* listener, const bool firstRequest)
{
  // Check if we are paused (don't need to process or return audio)
  if (mPaused.Get() == cTrue)
    return false;

  // Get input
  bool isThereOutput = AccumulateInputSamples(outputBuffer->Size(), numberOfChannels, listener);

  // Check if we are muted (need to process audio, don't need to return any)
  if (mMuted.Get() == cTrue)
    return false;

  // Move input to output buffer
  if (isThereOutput)
  {
    // Check if we're interpolating volume
    if (mInterpolatingThreaded)
    {
      // Apply volume adjustment
      float volume;
      BufferRange inputRange = mInputSamplesThreaded.All();
      for (unsigned i = 0; i < outputBuffer->Size(); i += numberOfChannels)
      {
        volume = VolumeInterpolator.NextValue();

        // Apply the volume multiplier to all samples
        for (unsigned j = 0; j < numberOfChannels; ++j, inputRange.PopFront())
          inputRange.Front() *= volume;
      }

      // Check if we're done interpolating
      if (VolumeInterpolator.Finished())
      {
        mInterpolatingThreaded = false;
        if (mPausingThreaded)
          mPaused.Set(cTrue);
        if (mMutingThreaded)
          mMuted.Set(cTrue);
      }
    }

    mInputSamplesThreaded.Swap(*outputBuffer);
  }

  return isThereOutput;
}

//**************************************************************************************************
void CombineAndPauseNode::SetPausedThreaded(const bool paused)
{
  // If we should pause and we are currently not paused and not pausing
  if (paused && mPaused.Get() == cFalse && !mPausingThreaded)
  {
    // If we are muted we don't need to interpolate volume
    if (mMuted.Get() == cTrue)
      mPaused.Set(cTrue);
    else
    {
      mPausingThreaded = true;

      // Only set the interpolator if we haven't already set it for muting
      if (!mMutingThreaded)
      {
        mInterpolatingThreaded = true;
        VolumeInterpolator.SetValues(1.0f, 0.0f, cPropertyChangeFrames);
      }
    }
  }
  // If we should un-pause and we are currently paused
  else if (!paused && (mPaused.Get() != cFalse || mPausingThreaded))
  {
    mPaused.Set(cFalse);
    mPausingThreaded = false;

    // If we are muted or muting we don't need to interpolate volume
    if (mMuted.Get() == cFalse && !mMutingThreaded)
    {
      mInterpolatingThreaded = true;
      VolumeInterpolator.SetValues(0.0f, 1.0f, cPropertyChangeFrames);
    }
  }
}

//**************************************************************************************************
void CombineAndPauseNode::SetMutedThreaded(const bool muted)
{
  // If we should mute and we are currently not muted and not muting
  if (muted && mMuted.Get() == cFalse && !mMutingThreaded)
  {
    // If we are paused we don't need to interpolate volume
    if (mPaused.Get() == cTrue)
      mMuted.Set(cTrue);
    else
    {
      mMutingThreaded = true;

      // Only set the interpolator if we haven't already set it for pausing
      if (!mPausingThreaded)
      {
        mInterpolatingThreaded = true;
        VolumeInterpolator.SetValues(1.0f, 0.0f, cPropertyChangeFrames);
      }
    }
  }
  // If we should un-mute and we are currently muted
  else if (!muted && mMuted.Get() == cTrue)
  {
    mMuted.Set(cFalse);
    mMutingThreaded = false;

    // If we are paused or pausing we don't need to interpolate volume
    if (mPaused.Get() == cFalse && !mPausingThreaded)
    {
      mInterpolatingThreaded = true;
      VolumeInterpolator.SetValues(0.0f, 1.0f, cPropertyChangeFrames);
    }
  }
}

} // namespace Zero
