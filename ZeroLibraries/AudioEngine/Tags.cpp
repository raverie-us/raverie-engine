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
    CurrentVolume(1.0f), 
    ModifyingVolume(false), 
    ThreadedTag(NULL), 
    IsThreaded(isThreaded),
    Paused(false),
    LowPassGain(1.0f),
    HighPassGain(1.0f),
    Band1Gain(1.0f),
    Band2Gain(1.0f),
    Band3Gain(1.0f),
    CompressorThresholdDB(0),
    CompressorAttackMSec(20),
    CompressorReleaseMSec(1000),
    CompressorRatio(1),
    CompressorKneeWidth(0),
    UseEqualizer(false),
    UseCompressor(false),
    ExternalInterface(nullptr),
    InstanceLimit(0),
    CompressorInputTag(nullptr)
  {
    if (!isThreaded)
    {
      ThreadedTag = new TagObject(true);
      gAudioSystem->AddTag(this, false);
    }
    else
    {
      // Constructor happens on main thread, so need to send a message to add to threaded list
      gAudioSystem->AddTask(Zero::CreateFunctor(&AudioSystemInternal::AddTag, gAudioSystem, this, true));

      TotalOutput.Resize(gAudioSystem->AudioIO.OutputBufferSizeThreaded, 0);
    }
  }

  //************************************************************************************************
  TagObject::~TagObject()
  {
    // If threaded, remove from all threaded instances
    if (IsThreaded)
    {
      RemoveFromAllInstances();
    }
    else
    {
      // Delete the threaded tag
      if (ThreadedTag)
        delete ThreadedTag;

      // Remove this tag from all non-threaded instances
      forRange (SoundInstanceNode* node, Instances.All())
        node->TagList.EraseValue(this);
    }

    gAudioSystem->RemoveTag(this, IsThreaded);
  }

  //************************************************************************************************
  void TagObject::RemoveTag()
  {
    // If not threaded
    if (!IsThreaded)
    {
      // Check if the threaded tag exists
      if (ThreadedTag)
      {
        // Tell the threaded tag to remove itself
        gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::RemoveTag, ThreadedTag));
        ThreadedTag = nullptr;
      }
    }
    // If threaded
    else 
    {
      // Remove this tag from all threaded instances
      RemoveFromAllInstances();
    }

    gAudioSystem->DelayDeleteTag(this, IsThreaded);
  }

  //************************************************************************************************
  void TagObject::AddInstance(SoundInstanceNode* instance)
  {
    // Make sure the instance isn't null
    if (!instance)
      return;

    if (!IsThreaded)
    {
      // If already tagged, do nothing
      if (Instances.Contains(instance))
        return;

      // If the tag is currently paused, pause the instance
      if (Paused)
        instance->Pause();

      // Add the instance to the tag's list
      Instances.PushBack(instance);
      // Add the tag to the instance's list
      instance->TagList.PushBack(this);

      // Notify the threaded tag to add the threaded instance
      if (ThreadedTag)
        gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::AddInstance, ThreadedTag, 
          (SoundInstanceNode*)instance->GetSiblingNode()));

      if (ExternalInterface)
        ExternalInterface->SendAudioEvent(Notify_TagAddedInstance, (void*)nullptr);
    }
    else
    {
      // If already playing max instances, mark this one as virtual
      if (InstanceLimit > 0 && InstanceVolumeMap.Size() >= InstanceLimit)
        instance->Virtual = true;

      // Add this tag to the instance's list
      instance->TagList.PushBack(this);

      // If currently modifying volume, add modification to the instance
      if (ModifyingVolume)
      {
        // If this instance's volume was not already in the map, get a new modifier
        if (!InstanceVolumeMap[instance])
          InstanceVolumeMap[instance] = instance->GetAvailableVolumeMod();

        // If there is a volume modifier, set the volume
        if (InstanceVolumeMap[instance])
          InstanceVolumeMap[instance]->Reset(1.0f, CurrentVolume, 0.02f, 0.0f, 0.0f, 0.0f);
      }
      else
        InstanceVolumeMap[instance] = NULL;

      if (UseEqualizer)
        instance->EqualizerFilter = new Equalizer(LowPassGain, Band1Gain, Band2Gain, Band3Gain, HighPassGain);

      if (UseCompressor)
        instance->CompressorFilter = new DynamicsProcessor(0, CompressorThresholdDB, CompressorAttackMSec,
          CompressorReleaseMSec, CompressorRatio, 0, CompressorKneeWidth, Audio::DynamicsProcessor::Compressor);
    }
  }

  //************************************************************************************************
  void TagObject::RemoveInstance(SoundInstanceNode* instance)
  {
    // Make sure the instance isn't null
    if (!instance)
      return;

    if (!IsThreaded)
    {
      // Notify the threaded tag to remove the threaded instance
      if (ThreadedTag)
        gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::RemoveInstance, ThreadedTag, 
          (SoundInstanceNode*)instance->GetSiblingNode()));

      // Remove the instance from this tag's list
      Instances.EraseValue(instance);
      // Remove this tag from the instance's list
      instance->TagList.EraseValue(this);

      if (ExternalInterface && Instances.Empty())
        ExternalInterface->SendAudioEvent(Notify_TagIsUnreferenced, (void*)nullptr);
    }
    else
    {
      // If not in the map, do nothing
      if (!InstanceVolumeMap.FindPointer(instance))
        return;

      SetInstanceDataOnRemove(instance, InstanceVolumeMap.FindValue(instance, nullptr));

      // Remove the instance from the map of volume modifiers
      InstanceVolumeMap.Erase(instance);
    }
  }

  //************************************************************************************************
  void TagObject::SetVolume(const float volume, const float time)
  {
    // Set the current volume variable
    CurrentVolume = volume;

    if (!IsThreaded)
    {
      // Notify the threaded tag to change its volume
      if (ThreadedTag)
        gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::SetVolume, ThreadedTag, volume, time));
    }
    else
    {
      ModifyingVolume = true;

      // Walk through all instances
      InstanceVolumeMapType::range instances = InstanceVolumeMap.All();
      while (!instances.Empty())
      {
        SoundInstanceNode* instanceNode = instances.Front().first;

        // If there is no volume modifier, create one
        if (!instances.Front().second)
          InstanceVolumeMap[instanceNode] = instanceNode->GetAvailableVolumeMod();

        // If the modifier exists, set its values
        ThreadedVolumeModifier* volumeMod = InstanceVolumeMap[instanceNode];
        if (volumeMod)
          volumeMod->Reset(volumeMod->GetCurrentVolume(), volume, time, 0.0f, 0.0f, 0.0f);

        instances.PopFront();
      }
    }
  }

  //************************************************************************************************
  float TagObject::GetVolume()
  {
    return CurrentVolume;
  }

  //************************************************************************************************
  void TagObject::PauseInstances()
  {
    Paused = true;

    if (!IsThreaded)
    {
      // Notify the threaded tag to pause its instances
      if (ThreadedTag)
        gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::PauseInstances, ThreadedTag));
    }
    else
    {
      // Pause all instances in the volume modifier map
      InstanceVolumeMapType::range instances = InstanceVolumeMap.All();
      while (!instances.Empty())
      {
        instances.Front().first->Pause();

        instances.PopFront();
      }
    }
  }

  //************************************************************************************************
  void TagObject::ResumeInstances()
  {
    Paused = false;

    if (!IsThreaded)
    {
      // Notify the threaded tag to resume its instances
      if (ThreadedTag)
        gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::ResumeInstances, ThreadedTag));
    }
    else
    {
      // Resume all instances in the volume modifier map
      InstanceVolumeMapType::range instances = InstanceVolumeMap.All();
      while (!instances.Empty())
      {
        instances.Front().first->Resume();

        instances.PopFront();
      }
    }
  }

  //************************************************************************************************
  void TagObject::StopInstances()
  {
    if (!IsThreaded)
    {
      // Notify the threaded tag to stop its instances
      if (ThreadedTag)
        gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::StopInstances, ThreadedTag));
    }
    else
    {
      // Stop all instances in the volume modifier map
      InstanceVolumeMapType::range instances = InstanceVolumeMap.All();
      while (!instances.Empty())
      {
        instances.Front().first->Stop();

        instances.PopFront();
      }
    }
  }

  //************************************************************************************************
  bool TagObject::GetPaused()
  {
    return Paused;
  }

  //************************************************************************************************
  int TagObject::GetNumberOfInstances()
  {
    return Instances.Size();
  }

  //************************************************************************************************
  const Zero::Array<SoundInstanceNode*>* Audio::TagObject::GetInstances()
  {
    return &Instances;
  }

  //************************************************************************************************
  bool TagObject::GetUseEqualizer()
  {
    return UseEqualizer;
  }

  //************************************************************************************************
  void TagObject::SetUseEqualizer(const bool useEQ)
  {
    if (!IsThreaded)
    {
      UseEqualizer = useEQ;

      if (ThreadedTag)
        gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::SetUseEqualizer, ThreadedTag, useEQ));
    }
    else
    {
      // Should use equalizer and currently aren't
      if (useEQ && !UseEqualizer)
      {
        UseEqualizer = true;

        // Go through all instances
        for (InstanceVolumeMapType::range instances = InstanceVolumeMap.All();
          !instances.Empty(); instances.PopFront())
        {
          SoundInstanceNode* instance = instances.Front().first;

          // If there isn't an equalizer (there shouldn't be), add one
          if (!instance->EqualizerFilter)
            instance->EqualizerFilter = new Equalizer(LowPassGain, Band1Gain, Band2Gain, Band3Gain, 
              HighPassGain);
        }
      }
      // Should stop using equalizer and currently are using
      else if (!useEQ && UseEqualizer)
      {
        UseEqualizer = false;

        // Go through all instances
        for (InstanceVolumeMapType::range instances = InstanceVolumeMap.All();
          !instances.Empty(); instances.PopFront())
        {
          SoundInstanceNode* instance = instances.Front().first;

          // If there is an equalizer (there should be), delete it
          if (instance->EqualizerFilter)
          {
            delete instance->EqualizerFilter;
            instance->EqualizerFilter = nullptr;
          }
        }
      }
    }
  }

  //************************************************************************************************
  float TagObject::GetBelow80HzGain()
  {
    return LowPassGain;
  }

  //************************************************************************************************
  void TagObject::SetBelow80HzGain(const float gain)
  {
    LowPassGain = gain;

    if (!IsThreaded)
    {
      if (ThreadedTag)
        gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::SetBelow80HzGain, ThreadedTag, gain));
    }
    else if (UseEqualizer)
    {
      for (InstanceVolumeMapType::range instances = InstanceVolumeMap.All();
        !instances.Empty(); instances.PopFront())
      {
        SoundInstanceNode* instance = instances.Front().first;

        if (!instance->EqualizerFilter)
          instance->EqualizerFilter = new Equalizer();

        instance->EqualizerFilter->SetBelow80HzGain(gain);
      }
    }
  }

  //************************************************************************************************
  float TagObject::Get150HzGain()
  {
    return Band1Gain;
  }

  //************************************************************************************************
  void TagObject::Set150HzGain(const float gain)
  {
    Band1Gain = gain;

    if (!IsThreaded)
    {
      if (ThreadedTag)
        gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::Set150HzGain, ThreadedTag, gain));
    }
    else if (UseEqualizer)
    {
      for (InstanceVolumeMapType::range instances = InstanceVolumeMap.All();
        !instances.Empty(); instances.PopFront())
      {
        SoundInstanceNode* instance = instances.Front().first;

        if (!instance->EqualizerFilter)
          instance->EqualizerFilter = new Equalizer();

        instance->EqualizerFilter->Set150HzGain(gain);
      }
    }
  }

  //************************************************************************************************
  float TagObject::Get600HzGain()
  {
    return Band2Gain;
  }

  //************************************************************************************************
  void TagObject::Set600HzGain(const float gain)
  {
    Band2Gain = gain;

    if (!IsThreaded)
    {
      if (ThreadedTag)
        gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::Set600HzGain, ThreadedTag, gain));
    }
    else if (UseEqualizer)
    {
      for (InstanceVolumeMapType::range instances = InstanceVolumeMap.All();
        !instances.Empty(); instances.PopFront())
      {
        SoundInstanceNode* instance = instances.Front().first;

        if (!instance->EqualizerFilter)
          instance->EqualizerFilter = new Equalizer();

        instance->EqualizerFilter->Set600HzGain(gain);
      }
    }
  }

  //************************************************************************************************
  float TagObject::Get2500HzGain()
  {
    return Band3Gain;
  }

  //************************************************************************************************
  void TagObject::Set2500HzGain(const float gain)
  {
    Band3Gain = gain;

    if (!IsThreaded)
    {
      if (ThreadedTag)
        gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::Set2500HzGain, ThreadedTag, gain));
    }
    else if (UseEqualizer)
    {
      for (InstanceVolumeMapType::range instances = InstanceVolumeMap.All();
        !instances.Empty(); instances.PopFront())
      {
        SoundInstanceNode* instance = instances.Front().first;

        if (!instance->EqualizerFilter)
          instance->EqualizerFilter = new Equalizer();

        instance->EqualizerFilter->Set2500HzGain(gain);
      }
    }
  }

  //************************************************************************************************
  float TagObject::GetAbove5000HzGain()
  {
    return HighPassGain;
  }

  //************************************************************************************************
  void TagObject::SetAbove5000HzGain(const float gain)
  {
    HighPassGain = gain;

    if (!IsThreaded)
    {
      if (ThreadedTag)
        gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::SetAbove5000HzGain, ThreadedTag, gain));
    }
    else if (UseEqualizer)
    {
      for (InstanceVolumeMapType::range instances = InstanceVolumeMap.All();
        !instances.Empty(); instances.PopFront())
      {
        SoundInstanceNode* instance = instances.Front().first;

        if (!instance->EqualizerFilter)
          instance->EqualizerFilter = new Equalizer();

        instance->EqualizerFilter->SetAbove5000HzGain(gain);
      }
    }
  }

  //************************************************************************************************
  void TagObject::InterpolateAllBands(GainValues* values, const float timeToInterpolate)
  {
    LowPassGain = values->Below80Hz;
    Band1Gain = values->At150Hz;
    Band2Gain = values->At600Hz;
    Band3Gain = values->At2500Hz;
    HighPassGain = values->Above5000Hz;

    if (!IsThreaded)
    {
      if (ThreadedTag)
        gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::InterpolateAllBands, ThreadedTag, values, timeToInterpolate));
    }
    else if (UseEqualizer)
    {
      for (InstanceVolumeMapType::range instances = InstanceVolumeMap.All();
        !instances.Empty(); instances.PopFront())
      {
        SoundInstanceNode* instance = instances.Front().first;

        if (!instance->EqualizerFilter)
          instance->EqualizerFilter = new Equalizer();

        instance->EqualizerFilter->InterpolateBands(values->Below80Hz, values->At150Hz, values->At600Hz,
          values->At2500Hz, values->Above5000Hz, timeToInterpolate);
      }

      delete values;
    }
  }

  //************************************************************************************************
  bool TagObject::GetUseCompressor()
  {
    return UseCompressor;
  }

  //************************************************************************************************
  void TagObject::SetUseCompressor(const bool useCompressor)
  {
    if (!IsThreaded)
    {
      UseCompressor = UseCompressor;

      if (ThreadedTag)
        gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::SetUseCompressor, ThreadedTag, useCompressor));
    }
    else
    {
      // Should use compressor and currently aren't
      if (useCompressor && !UseCompressor)
      {
        UseCompressor = true;

        // Go through all instances
        for (InstanceVolumeMapType::range instances = InstanceVolumeMap.All();
          !instances.Empty(); instances.PopFront())
        {
          SoundInstanceNode* instance = instances.Front().first;

          // If there isn't a compressor (there shouldn't be), add one
          if (!instance->CompressorFilter)
            instance->CompressorFilter = new DynamicsProcessor(0, CompressorThresholdDB, CompressorAttackMSec,
              CompressorReleaseMSec, CompressorRatio, 0, CompressorKneeWidth, Audio::DynamicsProcessor::Compressor);
        }
      }
      // Should stop using compressor and currently are using
      else if (!useCompressor && UseCompressor)
      {
        UseCompressor = false;

        // Go through all instances
        for (InstanceVolumeMapType::range instances = InstanceVolumeMap.All();
          !instances.Empty(); instances.PopFront())
        {
          SoundInstanceNode* instance = instances.Front().first;

          // If there is a compressor (there should be), delete it
          if (instance->CompressorFilter)
          {
            delete instance->CompressorFilter;
            instance->CompressorFilter = nullptr;
          }
        }
      }
    }
  }

  //************************************************************************************************
  float TagObject::GetCompressorThreshold()
  {
    return CompressorThresholdDB;
  }

  //************************************************************************************************
  void TagObject::SetCompressorThreshold(const float value)
  {
    CompressorThresholdDB = value;

    if (!IsThreaded)
    {
      if (ThreadedTag)
        gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::SetCompressorThreshold, ThreadedTag, value));
    }
    else if (UseCompressor)
    {
      for (InstanceVolumeMapType::range instances = InstanceVolumeMap.All();
        !instances.Empty(); instances.PopFront())
      {
        SoundInstanceNode* instance = instances.Front().first;

        if (!instance->CompressorFilter)
          instance->CompressorFilter = new DynamicsProcessor();

        instance->CompressorFilter->SetThreshold(value);
      }
    }
  }

  //************************************************************************************************
  float TagObject::GetCompressorAttackMSec()
  {
    return CompressorAttackMSec;
  }

  //************************************************************************************************
  void TagObject::SetCompressorAttackMSec(const float value)
  {
    CompressorAttackMSec = value;

    if (!IsThreaded)
    {
      if (ThreadedTag)
        gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::SetCompressorAttackMSec, ThreadedTag, value));
    }
    else if (UseCompressor)
    {
      for (InstanceVolumeMapType::range instances = InstanceVolumeMap.All();
        !instances.Empty(); instances.PopFront())
      {
        SoundInstanceNode* instance = instances.Front().first;

        if (!instance->CompressorFilter)
          instance->CompressorFilter = new DynamicsProcessor();

        instance->CompressorFilter->SetAttackMSec(value);
      }
    }
  }

  //************************************************************************************************
  float TagObject::GetCompressorReleaseMSec()
  {
    return CompressorReleaseMSec;
  }

  //************************************************************************************************
  void TagObject::SetCompressorReleaseMsec(const float value)
  {
    CompressorReleaseMSec = value;

    if (!IsThreaded)
    {
      if (ThreadedTag)
        gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::SetCompressorReleaseMsec, ThreadedTag, value));
    }
    else if (UseCompressor)
    {
      for (InstanceVolumeMapType::range instances = InstanceVolumeMap.All();
        !instances.Empty(); instances.PopFront())
      {
        SoundInstanceNode* instance = instances.Front().first;

        if (!instance->CompressorFilter)
          instance->CompressorFilter = new DynamicsProcessor();

        instance->CompressorFilter->SetReleaseMsec(value);
      }
    }
  }

  //************************************************************************************************
  float TagObject::GetCompressorRatio()
  {
    return CompressorRatio;
  }

  //************************************************************************************************
  void TagObject::SetCompressorRatio(const float value)
  {
    CompressorRatio = value;

    if (!IsThreaded)
    {
      if (ThreadedTag)
        gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::SetCompressorRatio, ThreadedTag, value));
    }
    else if (UseCompressor)
    {
      for (InstanceVolumeMapType::range instances = InstanceVolumeMap.All();
        !instances.Empty(); instances.PopFront())
      {
        SoundInstanceNode* instance = instances.Front().first;

        if (!instance->CompressorFilter)
          instance->CompressorFilter = new DynamicsProcessor();

        instance->CompressorFilter->SetRatio(value);
      }
    }
  }

  //************************************************************************************************
  float TagObject::GetCompressorKneeWidth()
  {
    return CompressorKneeWidth;
  }

  //************************************************************************************************
  void TagObject::SetCompressorKneeWidth(const float value)
  {
    CompressorKneeWidth = value;

    if (!IsThreaded)
    {
      if (ThreadedTag)
        gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::SetCompressorKneeWidth, ThreadedTag, value));
    }
    else if (UseCompressor)
    {
      for (InstanceVolumeMapType::range instances = InstanceVolumeMap.All();
        !instances.Empty(); instances.PopFront())
      {
        SoundInstanceNode* instance = instances.Front().first;

        if (!instance->CompressorFilter)
          instance->CompressorFilter = new DynamicsProcessor();

        instance->CompressorFilter->SetKneeWidth(value);
      }
    }
  }

  //************************************************************************************************
  unsigned TagObject::GetInstanceLimit()
  {
    return InstanceLimit;
  }

  //************************************************************************************************
  void TagObject::SetInstanceLimit(const float limit)
  {
    InstanceLimit = (unsigned)limit;

    if (!IsThreaded)
    {
      if (ThreadedTag)
        gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::SetInstanceLimit, ThreadedTag, limit));
    }
  }

  //************************************************************************************************
  void TagObject::SetCompressorInputTag(TagObject* tag)
  {
    if (!IsThreaded)
    {
      if (ThreadedTag)
      {
        if (tag && tag->ThreadedTag)
          gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::SetCompressorInputTag, ThreadedTag, 
            tag->ThreadedTag));
        else
          gAudioSystem->AddTask(Zero::CreateFunctor(&TagObject::SetCompressorInputTag, ThreadedTag, 
            (TagObject*)nullptr));
      }
    }
    else
    {
      CompressorInputTag = tag;
    }
  }

  //************************************************************************************************
  void TagObject::UpdateCompressorInput()
  {
    if (CompressorInputTag && UseCompressor && !InstanceVolumeMap.Empty())
    {
      const Zero::Array<float>* buffer = CompressorInputTag->GetTagOutput();

      for (InstanceVolumeMapType::range pairs = InstanceVolumeMap.All(); !pairs.Empty(); pairs.PopFront())
      {
        pairs.Front().first->CompressorSideChainInput = buffer;
      }
    }
  }

  //************************************************************************************************
  const Zero::Array<float>* TagObject::GetTagOutput()
  {
    memset(TotalOutput.Data(), 0, sizeof(float) * TotalOutput.Size());

    for (InstanceVolumeMapType::range instances = InstanceVolumeMap.All(); !instances.Empty(); instances.PopFront())
    {
      instances.Front().first->AddAttenuatedOutputToTag(this);
    }

    return &TotalOutput;
  }

  //************************************************************************************************
  void TagObject::RemoveFromAllInstances()
  {
    // Make sure this is the threaded tag
    if (IsThreaded)
    {
      // Step through all instances
      InstanceVolumeMapType::range instances = InstanceVolumeMap.All();
      while (!instances.Empty())
      {
        SetInstanceDataOnRemove(instances.Front().first, instances.Front().second);

        instances.PopFront();
      }

      InstanceVolumeMap.Clear();
    }
  }

  //************************************************************************************************
  void TagObject::SetInstanceDataOnRemove(SoundInstanceNode* instance, ThreadedVolumeModifier* modifier)
  {
    // Remove this tag from the instance's list
    instance->TagList.EraseValue(this);

    // If there is a volume modifier, deactivate it
    if (modifier)
    {
      // If currently modifying volume, ramp volume back to 1
      if (ModifyingVolume && modifier->Active)
        modifier->Reset(modifier->GetCurrentVolume(), 1.0f, 0.05f, 0.0f, 0.05f, 0.0f);
      // Otherwise, make sure it's set to not active
      else
        modifier->Active = false;
    }
  }

  //************************************************************************************************
  bool TagObject::CanInstanceUnVirtualize(SoundInstanceNode* instance)
  {
    // If instances are below limit, then yes
    if (InstanceLimit == 0 || InstanceVolumeMap.Size() < InstanceLimit)
      return true;

    int overLimit = InstanceVolumeMap.Size() - InstanceLimit;
    int count = 0;
    // Count how many instances (other than the one that called this function) are virtual
    for (InstanceVolumeMapType::range instances = InstanceVolumeMap.All(); !instances.Empty(); instances.PopFront())
    {
      SoundInstanceNode* thisInstance = instances.Front().first;
      if (thisInstance->Virtual && thisInstance != instance)
        ++count;

      if (count > overLimit)
        return true;
    }

    if (count >= overLimit)
      return true;
    else
      return false;
  }
}