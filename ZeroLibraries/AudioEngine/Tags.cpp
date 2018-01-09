///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.h"

namespace Audio
{
  //------------------------------------------------------------------------------------- Tag Object

  //************************************************************************************************
  TagObject::TagObject(bool isThreaded) : 
    mThreadedTag(nullptr),
    mIsThreaded(isThreaded),
    mPaused(false),
    mInstanceLimit(0),
    mMixVersion(gAudioSystem->MixVersionNumber - 1),
    mVolume(1.0f),
    mModifyingVolume(false),
    mUseEqualizer(false),
    CompressorObject(nullptr),
    mUseCompressor(false),
    mCompressorInputTag(nullptr),
    EqualizerSettings(nullptr)
  {
    if (!isThreaded)
    {
      mThreadedTag = new TagObject(true);
      gAudioSystem->AddTag(this, false);
    }
    else
    {
      CompressorObject = new DynamicsProcessor();
      EqualizerSettings = new Equalizer();

      // Constructor happens on main thread, so need to send a message to add to threaded list
      gAudioSystem->AddTask(Zero::CreateFunctor(&AudioSystemInternal::AddTag, gAudioSystem, this, true));
    }
  }

  //************************************************************************************************
  TagObject::~TagObject()
  {
    if (!mIsThreaded)
    {
      // Delete the threaded tag
      if (mThreadedTag)
        delete mThreadedTag;

      // Remove this tag from all non-threaded instances
      forRange(SoundInstanceNode* instance, mSoundInstanceList.All())
        instance->TagList.EraseValue(this);
    }
    else
    {
      // Remove this tag from all threaded instances
      forRange(InstanceDataMapType::pair mapPair, DataPerInstance.All())
      {
        SoundInstanceNode* instance = mapPair.first;
        InstanceData* data = mapPair.second;

        // If currently modifying volume, interpolate to avoid sudden changes
        if (mModifyingVolume && data->mVolumeModifier)
        {
          data->mVolumeModifier->Reset(data->mVolumeModifier->GetCurrentVolume(), 1.0f, 
            AudioSystemInternal::PropertyChangeFrames, AudioSystemInternal::PropertyChangeFrames);
          data->mVolumeModifier = nullptr;
        }

        // Remove this tag from the instance's list
        instance->TagList.EraseValue(this);

        delete data;
      }

      if (CompressorObject)
        delete CompressorObject;
      if (EqualizerSettings)
        delete EqualizerSettings;
    }

    // Remove this tag from the audio system's list
    gAudioSystem->RemoveTag(this, mIsThreaded);
  }

  //************************************************************************************************
  void TagObject::UpdateForMix(unsigned howManyFrames, unsigned channels)
  {
    if (mMixVersion == gAudioSystem->MixVersionNumber)
      return;

    // Check if we are using the compressor
    if (mUseCompressor)
    {
      BufferType* compressorInput;

      // If we are not using another tag for the compressor input, get the output from this tag
      if (!mCompressorInputTag)
        compressorInput = GetTotalInstanceOutput(howManyFrames, channels);
      // Otherwise get the output from the other tag
      else
        compressorInput = mCompressorInputTag->GetTotalInstanceOutput(howManyFrames, channels);

      // Reset the buffer of volume modifiers
      mCompressorVolumes.Clear();

      // Check if there was output from the tag
      if (!compressorInput->Empty())
      {
        // Reset the volume buffer to all 1.0 (this will become volume multipliers when 
        // processed by the compressor filter)
        mCompressorVolumes.Resize(compressorInput->Size(), 1.0f);

        // Run the compressor filter, using the tag output as the envelope input
        CompressorObject->ProcessBuffer(mCompressorVolumes.Data(), compressorInput->Data(),
          mCompressorVolumes.Data(), channels, mCompressorVolumes.Size());
      }
    }

    mMixVersion = gAudioSystem->MixVersionNumber;
  }

  //************************************************************************************************
  void TagObject::ProcessInstance(BufferType* instanceOutput, unsigned channels, SoundInstanceNode* instance)
  {
    // If this is the first instance for this mix, update
    if (mMixVersion != gAudioSystem->MixVersionNumber)
      UpdateForMix(instanceOutput->Size() / channels, channels);

    // Check if we are using the equalizer
    if (mUseEqualizer)
    {
      InstanceData* data = DataPerInstance.FindValue(instance, nullptr);
      ErrorIf(!data, "InstanceData was not created in tag's map");

      // If the equalizer filter does not exist for this instance, create it
      if (!data->mEqualizer)
        data->mEqualizer = new Equalizer(*EqualizerSettings);

      // Create a temporary buffer for the equalizer output
      BufferType processedOutput(instanceOutput->Size());

      // Apply the filter to all samples
      data->mEqualizer->ProcessBuffer(instanceOutput->Data(), processedOutput.Data(), channels,
        instanceOutput->Size());

      // Move the equalizer output into the instanceOutput buffer
      instanceOutput->Swap(processedOutput);
    }

    // Check if we are using the compressor and there are volume values
    if (mUseCompressor && !mCompressorVolumes.Empty())
    {
      // Apply the corresponding compressor volume to each sample
      int i = 0;
      forRange(float& sample, instanceOutput->All())
        sample *= mCompressorVolumes[i++];
    }
  }

  //************************************************************************************************
  void TagObject::RemoveTag()
  {
    // If not threaded
    if (!mIsThreaded && mThreadedTag)
    {
        // Tell the threaded tag to remove itself
        gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::RemoveTag, mThreadedTag));
        mThreadedTag = nullptr;
    }

    gAudioSystem->DelayDeleteTag(this, mIsThreaded);
  }

  //************************************************************************************************
  void TagObject::AddInstance(SoundInstanceNode* instance)
  {
    // Make sure the instance isn't null
    if (!instance)
      return;

    if (!mIsThreaded)
    {
      // If already tagged, do nothing
      if (mSoundInstanceList.Contains(instance))
        return;

      // Add the instance to the tag's list
      mSoundInstanceList.PushBack(instance);
      // Add the tag to the instance's list
      instance->TagList.PushBack(this);

      // If the tag is currently paused, pause the instance
      if (mPaused)
        instance->SetPaused(true);

      // Notify the threaded tag to add the threaded instance
      if (mThreadedTag)
        gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::AddInstance, mThreadedTag, 
          (SoundInstanceNode*)instance->GetSiblingNode()));

      if (ExternalInterface)
        ExternalInterface->SendAudioEvent(AudioEventTypes::TagAddedInstance, (void*)nullptr);
    }
    else
    {
      // If already playing max instances, mark this one as virtual
      //if (InstanceLimit > 0 && InstanceVolumeMap.Size() >= InstanceLimit)
      //  instance->mVirtual = true;

      // Add the tag to the instance's list
      instance->TagList.PushBack(this);

      // Add a new data object to the map
      InstanceData* data = new InstanceData();
      DataPerInstance[instance] = data;

      // If modifying volume, create the modifier
      if (mModifyingVolume)
      {
        data->mVolumeModifier = instance->GetAvailableVolumeMod();
        data->mVolumeModifier->Reset(1.0f, mVolume, AudioSystemInternal::PropertyChangeFrames, 0);
      }

      // If using equalizer, create it and set the settings
      if (mUseEqualizer)
        data->mEqualizer = new Equalizer(*EqualizerSettings);
    }
  }

  //************************************************************************************************
  void TagObject::RemoveInstance(SoundInstanceNode* instance)
  {
    // Make sure the instance isn't null
    if (!instance)
      return;

    if (!mIsThreaded)
    {
      // First check if the instance has been added to this tag
      if (!mSoundInstanceList.Contains(instance))
        return;

      // Remove the instance from this tag's list
      mSoundInstanceList.EraseValue(instance);

      // Notify the threaded tag to remove the threaded instance
      if (mThreadedTag)
        gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::RemoveInstance, mThreadedTag, 
          (SoundInstanceNode*)instance->GetSiblingNode()));

      // If there are no more tagged instances, send a notification
      if (ExternalInterface && mSoundInstanceList.Empty())
        ExternalInterface->SendAudioEvent(AudioEventTypes::TagIsUnreferenced, (void*)nullptr);
    }
    else
    {
      // Find this instance's data in the map
      InstanceData* data = DataPerInstance.FindValue(instance, nullptr);
      if (data)
      {
        // If we are modifying the volume, interpolate back to 1.0
        if (mModifyingVolume && data->mVolumeModifier)
        {
          data->mVolumeModifier->Reset(data->mVolumeModifier->GetCurrentVolume(), 1.0f,
            AudioSystemInternal::PropertyChangeFrames, AudioSystemInternal::PropertyChangeFrames);
          data->mVolumeModifier = nullptr;
        }

        // Delete the data object
        delete data;

        // Remove the instance from the map
        DataPerInstance.Erase(instance);
      }
    }

    // Remove this tag from the instance's list
    instance->TagList.EraseValue(this);
  }

  //************************************************************************************************
  void TagObject::SetVolume(const float volume, const float time)
  {
    // Set the current volume variable
    mVolume = volume;

    if (!mIsThreaded)
    {
      // Notify the threaded tag to change its volume
      if (mThreadedTag)
        gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::SetVolume, mThreadedTag, volume, time));
    }
    else
    {
      mModifyingVolume = true;

      // Determine how many frames to change the volume over, keeping a minimum value
      unsigned frames = Math::Max((unsigned)(time * AudioSystemInternal::SystemSampleRate),
        AudioSystemInternal::PropertyChangeFrames);

      // Set the volume modifier for each tagged instance
      forRange(InstanceDataMapType::pair mapPair, DataPerInstance.All())
      {
        SoundInstanceNode* instance = mapPair.first;
        InstanceData* data = mapPair.second;

        if (!data->mVolumeModifier)
          data->mVolumeModifier = instance->GetAvailableVolumeMod();
        
        data->mVolumeModifier->Reset(data->mVolumeModifier->GetCurrentVolume(), volume, frames, 0);
      }
    }
  }

  //************************************************************************************************
  float TagObject::GetVolume()
  {
    return mVolume;
  }

  //************************************************************************************************
  void TagObject::PauseInstances()
  {
    if (!mIsThreaded)
    {
      mPaused = true;

      // Pause all instances in the list
      forRange(SoundInstanceNode* instance, mSoundInstanceList.All())
        instance->SetPaused(true);
    }
  }

  //************************************************************************************************
  void TagObject::ResumeInstances()
  {
    if (!mIsThreaded)
    {
      mPaused = false;

      // Resume all instances in the list
      forRange(SoundInstanceNode* instance, mSoundInstanceList.All())
        instance->SetPaused(false);
    }
  }

  //************************************************************************************************
  void TagObject::StopInstances()
  {
    if (!mIsThreaded)
    {
      // Stop all instances in the list
      forRange(SoundInstanceNode* instance, mSoundInstanceList.All())
        instance->Stop();
    }
  }

  //************************************************************************************************
  bool TagObject::GetPaused()
  {
    return mPaused;
  }

  //************************************************************************************************
  int TagObject::GetNumberOfInstances()
  {
    return mSoundInstanceList.Size();
  }

  //************************************************************************************************
  const Zero::Array<SoundInstanceNode*>* Audio::TagObject::GetInstances()
  {
    return &mSoundInstanceList;
  }

  //************************************************************************************************
  bool TagObject::GetUseEqualizer()
  {
    return mUseEqualizer;
  }

  //************************************************************************************************
  void TagObject::SetUseEqualizer(const bool useEQ)
  {
    mUseEqualizer = useEQ;

    if (!mIsThreaded)
    {
      if (mThreadedTag)
        gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::SetUseEqualizer, mThreadedTag, useEQ));
    }
  }

  //************************************************************************************************
  float TagObject::GetBelow80HzGain()
  {
    return EqualizerSettings->GetBelow80HzGain();
  }

  //************************************************************************************************
  void TagObject::SetBelow80HzGain(const float gain)
  {
    if (!mIsThreaded)
    {
      if (mThreadedTag)
        gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::SetBelow80HzGain, mThreadedTag, gain));
    }
    else 
    {
      EqualizerSettings->SetBelow80HzGain(gain);

      // Set the value on existing equalizers. If new ones are created they will copy settings.
      forRange(InstanceData* data, DataPerInstance.Values())
        data->mEqualizer->SetBelow80HzGain(gain);
    }
  }

  //************************************************************************************************
  float TagObject::Get150HzGain()
  {
    return EqualizerSettings->Get150HzGain();
  }

  //************************************************************************************************
  void TagObject::Set150HzGain(const float gain)
  {
    if (!mIsThreaded)
    {
      if (mThreadedTag)
        gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::Set150HzGain, mThreadedTag, gain));
    }
    else 
    {
      EqualizerSettings->Set150HzGain(gain);

      // Set the value on existing equalizers. If new ones are created they will copy settings.
      forRange(InstanceData* data, DataPerInstance.Values())
        data->mEqualizer->Set150HzGain(gain);
    }
  }

  //************************************************************************************************
  float TagObject::Get600HzGain()
  {
    return EqualizerSettings->Get600HzGain();
  }

  //************************************************************************************************
  void TagObject::Set600HzGain(const float gain)
  {
    if (!mIsThreaded)
    {
      if (mThreadedTag)
        gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::Set600HzGain, mThreadedTag, gain));
    }
    else 
    {
      EqualizerSettings->Set600HzGain(gain);

      // Set the value on existing equalizers. If new ones are created they will copy settings.
      forRange(InstanceData* data, DataPerInstance.Values())
        data->mEqualizer->Set600HzGain(gain);
    }
  }

  //************************************************************************************************
  float TagObject::Get2500HzGain()
  {
    return EqualizerSettings->Get2500HzGain();
  }

  //************************************************************************************************
  void TagObject::Set2500HzGain(const float gain)
  {
    if (!mIsThreaded)
    {
      if (mThreadedTag)
        gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::Set2500HzGain, mThreadedTag, gain));
    }
    else 
    {
      EqualizerSettings->Set2500HzGain(gain);

      // Set the value on existing equalizers. If new ones are created they will copy settings.
      forRange(InstanceData* data, DataPerInstance.Values())
        data->mEqualizer->Set2500HzGain(gain);
    }
  }

  //************************************************************************************************
  float TagObject::GetAbove5000HzGain()
  {
    return EqualizerSettings->GetAbove5000HzGain();
  }

  //************************************************************************************************
  void TagObject::SetAbove5000HzGain(const float gain)
  {
    if (!mIsThreaded)
    {
      if (mThreadedTag)
        gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::SetAbove5000HzGain, mThreadedTag, gain));
    }
    else
    {
      EqualizerSettings->SetAbove5000HzGain(gain);

      // Set the value on existing equalizers. If new ones are created they will copy settings.
      forRange(InstanceData* data, DataPerInstance.Values())
        data->mEqualizer->SetAbove5000HzGain(gain);
    }
  }

  //************************************************************************************************
  void TagObject::InterpolateAllBands(GainValues* values, const float timeToInterpolate)
  {
    if (!mIsThreaded)
    {
      if (mThreadedTag)
        gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::InterpolateAllBands, mThreadedTag, 
          values, timeToInterpolate));
    }
    else 
    {
      EqualizerSettings->InterpolateBands(values->mBelow80Hz, values->mAt150Hz, values->mAt600Hz,
        values->mAt2500Hz, values->mAbove5000Hz, timeToInterpolate);

      // Set the value on existing equalizers. If new ones are created they will copy settings.
      forRange(InstanceData* data, DataPerInstance.Values())
        data->mEqualizer->InterpolateBands(values->mBelow80Hz, values->mAt150Hz, values->mAt600Hz,
          values->mAt2500Hz, values->mAbove5000Hz, timeToInterpolate);

      delete values;
    }
  }

  //************************************************************************************************
  bool TagObject::GetUseCompressor()
  {
    return mUseCompressor;
  }

  //************************************************************************************************
  void TagObject::SetUseCompressor(const bool useCompressor)
  {
    mUseCompressor = useCompressor;

    if (!mIsThreaded)
    {
      if (mThreadedTag)
        gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::SetUseCompressor, mThreadedTag,
          useCompressor));
    }
  }

  //************************************************************************************************
  float TagObject::GetCompressorThreshold()
  {
    return CompressorObject->GetThreshold();
  }

  //************************************************************************************************
  void TagObject::SetCompressorThreshold(const float value)
  {
    if (!mIsThreaded)
    {
      if (mThreadedTag)
        gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::SetCompressorThreshold, mThreadedTag, value));
    }
    else 
    {
      CompressorObject->SetThreshold(value);
    }
  }

  //************************************************************************************************
  float TagObject::GetCompressorAttackMSec()
  {
    return CompressorObject->GetAttackMSec();
  }

  //************************************************************************************************
  void TagObject::SetCompressorAttackMSec(const float value)
  {
    if (!mIsThreaded)
    {
      if (mThreadedTag)
        gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::SetCompressorAttackMSec, mThreadedTag, value));
    }
    else
    {
      CompressorObject->SetAttackMSec(value);
    }
  }

  //************************************************************************************************
  float TagObject::GetCompressorReleaseMSec()
  {
    return CompressorObject->GetReleaseMSec();
  }

  //************************************************************************************************
  void TagObject::SetCompressorReleaseMsec(const float value)
  {
    if (!mIsThreaded)
    {
      if (mThreadedTag)
        gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::SetCompressorReleaseMsec, mThreadedTag, value));
    }
    else 
    {
      CompressorObject->SetReleaseMSec(value);
    }
  }

  //************************************************************************************************
  float TagObject::GetCompressorRatio()
  {
    return CompressorObject->GetRatio();
  }

  //************************************************************************************************
  void TagObject::SetCompressorRatio(const float value)
  {
    if (!mIsThreaded)
    {
      if (mThreadedTag)
        gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::SetCompressorRatio, mThreadedTag, value));
    }
    else 
    {
      CompressorObject->SetRatio(value);
    }
  }

  //************************************************************************************************
  float TagObject::GetCompressorKneeWidth()
  {
    return CompressorObject->GetKneeWidth();
  }

  //************************************************************************************************
  void TagObject::SetCompressorKneeWidth(const float value)
  {
    if (!mIsThreaded)
    {
      if (mThreadedTag)
        gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::SetCompressorKneeWidth, mThreadedTag, value));
    }
    else 
    {
      CompressorObject->SetKneeWidth(value);
    }
  }

  //************************************************************************************************
  unsigned TagObject::GetInstanceLimit()
  {
    return mInstanceLimit;
  }

  //************************************************************************************************
  void TagObject::SetInstanceLimit(const float limit)
  {
    mInstanceLimit = (unsigned)limit;

    if (!mIsThreaded)
    {
      if (mThreadedTag)
        gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::SetInstanceLimit, mThreadedTag, limit));
    }
  }

  //************************************************************************************************
  void TagObject::SetCompressorInputTag(TagObject* tag)
  {
    if (!mIsThreaded)
    {
      if (mThreadedTag)
      {
        if (tag && tag->mThreadedTag)
          gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::SetCompressorInputTag, mThreadedTag,
            tag->mThreadedTag));
        else
          gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::SetCompressorInputTag, mThreadedTag,
          (TagObject*)nullptr));
      }
    }
    else
    {
      mCompressorInputTag = tag;
    }
  }

  //************************************************************************************************
  BufferType* TagObject::GetTotalInstanceOutput(unsigned howManyFrames, unsigned channels)
  {
    // Create a temporary buffer to get output from each instance
    BufferType instanceBuffer(howManyFrames * channels);
    // Resize the total output buffer
    mTotalInstanceOutput.Resize(howManyFrames * channels);
    // Set all samples to zero
    memset(mTotalInstanceOutput.Data(), 0, sizeof(float) * mTotalInstanceOutput.Size());

    forRange(InstanceDataMapType::pair mapPair, DataPerInstance.All())
    {
      SoundInstanceNode* instance = mapPair.first;

      // Check if the instance has valid output
      if (instance->GetOutputForThisMix(&instanceBuffer, channels))
      {
        // Get the instance's attenuated volume (SoundEmitters, VolumeNodes, etc.)
        float attenuatedVolume = instance->GetAttenuationThisMix();
        // Use the size of either the total output or instance output, whichever is smaller
        unsigned limit = Math::Min(mTotalInstanceOutput.Size(), instanceBuffer.Size());

        // Add the instance output into the total output, adjusting with tag volume
        // and attenuated instance volume
        for (unsigned i = 0; i < limit; ++i)
          mTotalInstanceOutput[i] += instanceBuffer[i] * attenuatedVolume * mVolume;
      }
    }

    return &mTotalInstanceOutput;
  }

  //************************************************************************************************
  void TagObject::RemoveInstanceFromLists(SoundInstanceNode* instance)
  {
    if (!mIsThreaded)
    {
      mSoundInstanceList.EraseValue(instance);
    }
    else
    {
      InstanceData* data = DataPerInstance.FindValue(instance, nullptr);
      if (data)
      {
        delete data;
        DataPerInstance.Erase(instance);
      }
    }
  }

  //************************************************************************************************
  TagObject::InstanceData::InstanceData() : 
    mVolumeModifier(nullptr), 
    mEqualizer(nullptr)
  {

  }

  //************************************************************************************************
  TagObject::InstanceData::~InstanceData()
  {
    if (mEqualizer) 
      delete mEqualizer;
    if (mVolumeModifier) 
      mVolumeModifier->Active = false;
  }
}
