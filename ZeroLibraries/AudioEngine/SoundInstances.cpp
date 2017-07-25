///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.h"

namespace Audio
{
  //----------------------------------------------------------------------- Sound Instance Node Data

  class SoundInstanceNodeData
  {
  public:
    SoundInstanceNodeData() :
      PausingModifier(NULL),
      ResampleIndex(0),
      FrameIndex(0),
      CrossFading(false),
      CrossFadeFrameIndex(0),
      CrossFadeStartFrame(0),
      CrossFadeResampleIndex(0),
      PitchShifting(false),
      ResampleFactor(0.0f),
      FrameCount(0),
      Stopping(false),
      Resampling(false),
      Pausing(false),
      InterpolatingVolume(false),
      InterpolatingPitch(false),
      TimeIncrement(0),
      StartFrame(0),
      EndFrame(0),
      LoopStartFrame(0),
      LoopEndFrame(0),
      LoopTailFrames(0)
    {
      CrossFadeVolumeInterpolator.SetCurve(SquaredCurveType);
    }
    ~SoundInstanceNodeData();

    // If true, sound is being resampled. 
    bool Resampling;
    // If resampling, the non-integer index. 
    double ResampleIndex;
    // If resampling, the amount to advance the index by. 
    float ResampleFactor;
    // Index of the current audio frame. 
    unsigned FrameIndex;
    // If true, sound is cross-fading for seamless looping. 
    bool CrossFading;
    // The sample index for cross-fading
    unsigned CrossFadeFrameIndex;
    // The samples to use for cross-fading
    Zero::Array<float> CrossFadeSamples;
    // The frame at which the cross-fading starting
    unsigned CrossFadeStartFrame;
    // Amount of time to cross-fade when looping. 
    const float CrossFadeTime = 0.1f;
    // Used for the cross-fade index when resampling or pitch shifting
    double CrossFadeResampleIndex;
    // Used to interpolate cross-fading volumes. 
    InterpolatingObject CrossFadeVolumeInterpolator;
    // If true, sound is ramping volume down to zero to pause. 
    bool Pausing;
    // If true, sound is ramping volume down to zero to stop. 
    bool Stopping;
    // Counts number of frames until pausing or stopping. 
    unsigned FrameCount;
    // Number of frames to wait until pausing or stopping. 
    unsigned FramesToWait;
    // If true, volume is being interpolated. 
    bool InterpolatingVolume;
    // Used to interpolate from one volume to another. 
    InterpolatingObject VolumeValues;
    // If true, pitch is being interpolated. 
    bool InterpolatingPitch;
    // Used to interpolate between pitches. 
    InterpolatingObject PitchValues;
    // If true, currently changing pitch. 
    bool PitchShifting;
    // The frame the playback should start at
    unsigned StartFrame;
    // The frame this instance's playback should stop at
    unsigned EndFrame;
    // The frame to jump back to when looping
    unsigned LoopStartFrame;
    // The frame to stop and jump back when looping
    unsigned LoopEndFrame;
    // The frames after the LoopEndTime to use for cross-fading
    unsigned LoopTailFrames;
    // Used to control volume modifications while pausing
    ThreadedVolumeModifier *PausingModifier;
    // The time, in seconds, per audio frame
    double TimeIncrement;

    // Volume adjustments, used by the instance and by tags
    Zero::Array<ThreadedVolumeModifier*> VolumeModList;
  };

  //************************************************************************************************
  SoundInstanceNodeData::~SoundInstanceNodeData()
  {
    forRange(ThreadedVolumeModifier* mod, VolumeModList.All())
      delete mod;
  }

  //---------------------------------------------------------------------------- Sound Instance Node

  //************************************************************************************************
  SoundInstanceNode::SoundInstanceNode(Zero::Status& status, Zero::StringParam name, const unsigned ID,
      SoundAssetNode* parentAsset, const bool looping, const bool startPaused, 
      ExternalNodeInterface* extInt, const bool isThreaded) :
    SimpleCollapseNode(status, name, ID, extInt, false, true, isThreaded), 
    Asset(parentAsset), 
    Volume(0.8f), 
    Finished(false), 
    Looping(looping), 
    Paused(startPaused), 
    Data(NULL), 
    PitchFactor(1.0f),
    CurrentTime(0),
    SecondsPerBeat(0),
    PreviousTime(0),
    BeatsPerBar(0),
    BeatsPerBarCount(0),
    BeatNoteType(0),
    AccumulatedTime(0),
    SecondsPerEighth(0),
    EighthsPerBeat(0),
    EighthNoteCount(0),
    StartTime(0),
    EndTime((float)parentAsset->GetNumberOfFrames() / (float)parentAsset->GetSampleRate()),
    LoopStartTime(0),
    LoopEndTime((float)parentAsset->GetNumberOfFrames() / (float)parentAsset->GetSampleRate()),
    LoopTailTime(0),
    NotifyTime(0),
    CustomNotifySent(false),
    Virtual(false),
    EqualizerFilter(nullptr),
    CompressorFilter(nullptr),
    CompressorSideChainInput(nullptr),
    CrossFadeTail(false)
  {
    if (!Threaded)
    {
      if (parentAsset->AddReference())
      {
        SetSiblingNodes(new SoundInstanceNode(status, name, ID, parentAsset->ThreadedAsset, looping, 
          startPaused, nullptr, true), status);
      }
      // Couldn't attach to this asset
      else
      {
        status.SetFailed(Zero::String::Format(
          "Unable to play instance from asset %s. Usually caused by trying to play an already playing streaming file.", 
          Asset->Name.c_str()));
        Asset = nullptr;
      }
    }
    else
    {
      Data = new SoundInstanceNodeData;

      Data->TimeIncrement = 1.0 / (double)Asset->GetSampleRate();
      Data->FrameIndex = 0;
      Data->ResampleIndex = Data->FrameIndex;
      Data->EndFrame = Asset->GetNumberOfFrames() - 1;

      // If the asset's sample rate is different from the system, store the resample factor
      if (Asset->GetSampleRate() != gAudioSystem->SystemSampleRate)
      {
        Data->ResampleFactor = (float)Asset->GetSampleRate() / (float)(gAudioSystem->SystemSampleRate);
        Data->Resampling = true;
      }

      // Set volume ramp if not paused
      if (!Paused)
      {
        ThreadedVolumeModifier *volumeMod = GetAvailableVolumeMod();
        if (volumeMod)
          volumeMod->Reset(0.0f, 1.0f, 0.02f, 0.0f, 0.02f, 0.0f);

        ResetMusicBeats();
      }

    }
  }

  //************************************************************************************************
  SoundInstanceNode::~SoundInstanceNode()
  {
    if (Threaded)
    {
      // Remove this instance from threaded tags
      forRange(TagObject* tag, TagList.All())
        tag->InstanceVolumeMap.Erase(this);

      delete Data;
      if (EqualizerFilter)
        delete EqualizerFilter;
      if (CompressorFilter)
        delete CompressorFilter;
    }
    else
    {
      // Remove this instance from non-threaded tags
      forRange(TagObject* tag, TagList.All())
        tag->Instances.EraseValue(this);

      if (Asset)
        Asset->ReleaseReference();
    }
  }

  //************************************************************************************************
  void SoundInstanceNode::Pause()
  {
    if (!Threaded)
    {
      Paused = true;
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&SoundInstanceNode::Pause, (SoundInstanceNode*)GetSiblingNode()));
    }
    else
    {
      if (!Data->Pausing && !Paused)
      {
        Data->Pausing = true;
        Data->FrameCount = 0;
        Data->FramesToWait = (unsigned)(0.021f * gAudioSystem->SystemSampleRate);
        ThreadedVolumeModifier* mod = GetAvailableVolumeMod();
        if (mod)
        {
          mod->Reset(1.0f, 0.0f, 0.02f, 0.0f, 0.0f, 0.0f);
          Data->PausingModifier = mod;
        }
      }
    }
  }

  //************************************************************************************************
  void SoundInstanceNode::Resume()
  {
    if (!Threaded)
    {
      Paused = false;
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&SoundInstanceNode::Resume, 
          (SoundInstanceNode*)GetSiblingNode()));
    }
    else
    {
      Paused = false;
      Data->Pausing = false;
      ThreadedVolumeModifier* mod = GetAvailableVolumeMod();
      if (mod)
        mod->Reset(0.0f, 1.0f, 0.02f, 0.0f, 0.02f, 0.0f);

      if (CurrentTime == 0)
        ResetMusicBeats();
    }
  }

  //************************************************************************************************
  void SoundInstanceNode::Stop()
  {
    if (!Threaded)
    {
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&SoundInstanceNode::Stop, 
          (SoundInstanceNode*)GetSiblingNode()));
    }
    else
    {
      Data->Stopping = true;
      Data->FrameCount = 0;
      Data->FramesToWait = (unsigned)(0.025f * gAudioSystem->SystemSampleRate);
      ThreadedVolumeModifier* mod = GetAvailableVolumeMod();
      if (mod)
        mod->Reset(1.0f, 0.0f, 0.02f, 0.0f, 0.0f, 0.0f);
    }
  }

  //************************************************************************************************
  bool SoundInstanceNode::GetLooping()
  {
    return Looping;
  }

  //************************************************************************************************
  void SoundInstanceNode::SetLooping(const bool loop)
  {
    Looping = loop;

    if (!Threaded && GetSiblingNode())
      gAudioSystem->AddTask(Zero::CreateFunctor(&SoundInstanceNode::SetLooping, 
        (SoundInstanceNode*)GetSiblingNode(), loop));
  }

  //************************************************************************************************
  float SoundInstanceNode::GetStartTime()
  {
    return StartTime;
  }

  //************************************************************************************************
  void SoundInstanceNode::SetStartTime(const float startTime)
  {
    if (!Threaded)
    {
      StartTime = startTime;
      if (StartTime < 0.0f || StartTime * Asset->GetSampleRate() >= Asset->GetNumberOfFrames())
        StartTime = 0.0f;

      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&SoundInstanceNode::SetStartTime,
          (SoundInstanceNode*)GetSiblingNode(), StartTime));
    }
    else
    {
      Data->StartFrame = (unsigned)(startTime * Asset->GetSampleRate());

      if (Data->FrameIndex < Data->StartFrame)
        Data->FrameIndex = Data->StartFrame;
    }
  }

  //************************************************************************************************
  float SoundInstanceNode::GetEndTime()
  {
    return EndTime;
  }

  //************************************************************************************************
  void SoundInstanceNode::SetEndTime(const float endTime)
  {
    if (!Threaded)
    {
      EndTime = endTime;
      if (EndTime < 0.0f)
        EndTime = 0.0f;
      else if (EndTime * Asset->GetSampleRate() >= Asset->GetNumberOfFrames())
        EndTime = (float)Asset->GetNumberOfFrames() / (float)Asset->GetSampleRate();

      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&SoundInstanceNode::SetEndTime,
        (SoundInstanceNode*)GetSiblingNode(), EndTime));
    }
    else
    {
      Data->EndFrame = (unsigned)(endTime * Asset->GetSampleRate());
      if (Data->EndFrame >= Asset->GetNumberOfFrames())
        Data->EndFrame = Asset->GetNumberOfFrames() - 1;
    }
  }

  //************************************************************************************************
  float SoundInstanceNode::GetLoopStartTime()
  {
    return LoopStartTime;
  }

  //************************************************************************************************
  void SoundInstanceNode::SetLoopStartTime(const float time)
  {
    if (!Threaded)
    {
      LoopStartTime = time;
      if (LoopStartTime < 0.0f || LoopStartTime * Asset->GetSampleRate() >= Asset->GetNumberOfFrames())
        LoopStartTime = 0.0f;

      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&SoundInstanceNode::SetLoopStartTime,
        (SoundInstanceNode*)GetSiblingNode(), LoopStartTime));
    }
    else
    {
      Data->LoopStartFrame = (unsigned)(time * Asset->GetSampleRate());
    }
  }

  //************************************************************************************************
  float SoundInstanceNode::GetLoopEndTime()
  {
    return LoopEndTime;
  }

  //************************************************************************************************
  void SoundInstanceNode::SetLoopEndTime(const float time)
  {
    if (!Threaded)
    {
      LoopEndTime = time;
      if (LoopEndTime < 0.0f)
        LoopEndTime = 0.0f;
      else if (LoopEndTime * Asset->GetSampleRate() >= Asset->GetNumberOfFrames())
        LoopEndTime = (float)Asset->GetNumberOfFrames() / (float)Asset->GetSampleRate();

      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&SoundInstanceNode::SetLoopEndTime,
        (SoundInstanceNode*)GetSiblingNode(), LoopEndTime));
    }
    else
    {
      Data->LoopEndFrame = (unsigned)(time * Asset->GetSampleRate());
    }
  }

  //************************************************************************************************
  float SoundInstanceNode::GetLoopTailTime()
  {
    return LoopTailTime;
  }

  //************************************************************************************************
  void SoundInstanceNode::SetLoopTailTime(const float time)
  {
    if (!Threaded)
    {
      LoopTailTime = time;
      if (LoopTailTime < 0.0f)
        LoopTailTime = 0.0f;

      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&SoundInstanceNode::SetLoopTailTime,
        (SoundInstanceNode*)GetSiblingNode(), LoopTailTime));
    }
    else
    {
      Data->LoopTailFrames = (unsigned)(time * Asset->GetSampleRate());
    }
  }

  //************************************************************************************************
  bool SoundInstanceNode::GetCrossFadeTail()
  {
    return CrossFadeTail;
  }

  //************************************************************************************************
  void SoundInstanceNode::SetCrossFadeTail(bool crossFade)
  {
    CrossFadeTail = crossFade;

    if (!Threaded && GetSiblingNode())
      gAudioSystem->AddTask(Zero::CreateFunctor(&SoundInstanceNode::SetCrossFadeTail,
        (SoundInstanceNode*)GetSiblingNode(), crossFade));
  }

  //************************************************************************************************
  void SoundInstanceNode::SetVolume(const float newVolume, float time)
  {
    if (!Threaded)
    {
      // If not interpolating, paused, or the volume is close, set the volume directly
      if (time == 0 || Paused || IsWithinLimit(newVolume, Volume, 0.01f))
        Volume = newVolume;

      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&SoundInstanceNode::SetVolume,
          (SoundInstanceNode*)GetSiblingNode(), newVolume, time));
    }
    else
    {
      // If paused or the volume is close, just set the volume directly
      if (Paused || IsWithinLimit(newVolume, Volume, 0.01f))
      {
        Data->InterpolatingVolume = false;
        Volume = newVolume;
      }
      else
      {
        Data->InterpolatingVolume = true;

        // Make sure there's a minimum interpolation time
        if (time < 0.02f)
          time = 0.02f;

        Data->VolumeValues.SetValues(Volume, newVolume, (unsigned)(time * gAudioSystem->SystemSampleRate));
      }
    }
  }

  //************************************************************************************************
  float SoundInstanceNode::GetVolume()
  {
    return Volume;
  }

  //************************************************************************************************
  void SoundInstanceNode::SetPitch(const int pitchCents, const float time)
  {
    if (!Threaded)
    {
      // If not interpolating, set the pitch value
      if (time == 0)
        PitchFactor = Math::Pow(2.0f, pitchCents / 1200.0f);
      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&SoundInstanceNode::SetPitch,
        (SoundInstanceNode*)GetSiblingNode(), pitchCents, time));
    }
    else
    {
      // Check for no pitch shift and no interpolation
      if (pitchCents == 0 && time == 0)
      {
        Data->PitchShifting = false;
        PitchFactor = 1.0f;
      }
      else
      {
        if (!Data->PitchShifting && !Data->Resampling)
          Data->ResampleIndex = Data->FrameIndex;

        Data->PitchShifting = true;

        if (time > 0)
        {
          Data->InterpolatingPitch = true;
          Data->PitchValues.SetValues(PitchFactor, Math::Pow(2.0f, pitchCents / 1200.0f),
            (unsigned)(time * gAudioSystem->SystemSampleRate));
        }
        else
          PitchFactor = Math::Pow(2.0f, pitchCents / 1200.0f);
      }
    }
  }

  //************************************************************************************************
  int SoundInstanceNode::GetPitch()
  {
    if (PitchFactor == 0)
      return 0;
    else
      return (int)(1200 * Math::Log2(PitchFactor));
  }

  //************************************************************************************************
  bool SoundInstanceNode::IsPlaying()
  {
    return !Finished;
  }

  //************************************************************************************************
  void SoundInstanceNode::JumpTo(const float seconds)
  {
    if (!Threaded)
    {
      // Can't jump when using a streaming asset
      if (Asset->GetStreaming())
        return;

      if (GetSiblingNode())
        gAudioSystem->AddTask(Zero::CreateFunctor(&SoundInstanceNode::JumpTo,
        (SoundInstanceNode*)GetSiblingNode(), seconds));
    }
    else
    {
      // Only need to cross-fade if we're not at the beginning of the file and volume is audible
      if (Data->FrameIndex > Data->StartFrame)
      {
        Data->CrossFading = true;
        Data->CrossFadeFrameIndex = 0;
        Data->CrossFadeResampleIndex = 0;
        Data->CrossFadeSamples.Resize((unsigned)(Data->CrossFadeTime * gAudioSystem->SystemSampleRate));
        Data->CrossFadeVolumeInterpolator.SetValues(1.0f, 0.0f, Data->CrossFadeSamples.Size());
        Asset->GetBuffer(Data->CrossFadeSamples.Data(), Data->FrameIndex, Data->CrossFadeSamples.Size());
      }

      Data->FrameIndex = (unsigned)(seconds * Asset->GetSampleRate());
      if (Data->FrameIndex > Data->EndFrame)
        Data->FrameIndex = Data->EndFrame;
      else if (Data->FrameIndex < Data->StartFrame)
        Data->FrameIndex = Data->StartFrame;

      Data->ResampleIndex = Data->FrameIndex;

      CurrentTime = Data->FrameIndex * Data->TimeIncrement;
      ResetMusicBeats();
    }
  }

  //************************************************************************************************
  float SoundInstanceNode::GetTime()
  {
    return (float)CurrentTime;
  }

  //************************************************************************************************
  float SoundInstanceNode::GetBeatsPerMinute()
  {
    return 60.0f / SecondsPerBeat;
  }

  //************************************************************************************************
  void SoundInstanceNode::SetBeatsPerMinute(const float bpm)
  {
    SecondsPerBeat = 60.0f / bpm;

    SecondsPerEighth = SecondsPerBeat * BeatNoteType / 8.0f;

    if (!Threaded && GetSiblingNode())
      gAudioSystem->AddTask(Zero::CreateFunctor(&SoundInstanceNode::SetBeatsPerMinute,
      (SoundInstanceNode*)GetSiblingNode(), bpm));
  }

  //************************************************************************************************
  void SoundInstanceNode::SetTimeSignature(const int beats, const int noteType)
  {
    BeatsPerBar = beats;
    BeatNoteType = noteType;

    SecondsPerEighth = SecondsPerBeat * BeatNoteType / 8.0f;
    EighthsPerBeat = 8 / BeatNoteType;

    if (!Threaded && GetSiblingNode())
      gAudioSystem->AddTask(Zero::CreateFunctor(&SoundInstanceNode::SetTimeSignature,
      (SoundInstanceNode*)GetSiblingNode(), beats, noteType));
  }

  //************************************************************************************************
  float SoundInstanceNode::GetCustomNotifyTime()
  {
    return NotifyTime;
  }

  //************************************************************************************************
  void SoundInstanceNode::SetCustomNotifyTime(const float time)
  {
    NotifyTime = time;
    CustomNotifySent = false;

    if (!Threaded && GetSiblingNode())
      gAudioSystem->AddTask(Zero::CreateFunctor(&SoundInstanceNode::SetCustomNotifyTime,
      (SoundInstanceNode*)GetSiblingNode(), time));
  }

  //************************************************************************************************
  bool SoundInstanceNode::GetOutputSamples(Zero::Array<float>* outputBuffer, 
    const unsigned numberOfChannels, ListenerNode* listener, const bool firstRequest)
  {
    if (!Threaded)
      return false;

    if (Finished || Paused)
      return false;

    unsigned bufferSize = outputBuffer->Size();
    unsigned bufferFrames = bufferSize / numberOfChannels;

    if (Virtual)
    {
      SkipForward(bufferFrames);
      return false;
    }

    // If volume is lower than minimum volume threshold, skip forward
    if (BelowMinimumVolume(bufferFrames))
    {
      SkipForward(bufferFrames);
      return false;
    }

    FrameData data;
    data.HowManyChannels = numberOfChannels;
    InputSamples.Resize(bufferSize);

    // Fill buffer with audio data, adjusted with instance volume
    for (unsigned i = 0; i < bufferSize; i += numberOfChannels)
    {
      GetNextFrame(data);

      // If interpolating volume, get new value
      if (Data->InterpolatingVolume)
      {
        Volume = Data->VolumeValues.NextValue();

        Data->InterpolatingVolume = !Data->VolumeValues.Finished(GetSiblingNode());

        if (!Data->InterpolatingVolume && GetSiblingNode())
          // Update the non-threaded instance with the final volume
          gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundInstanceNode::Volume,
            (SoundInstanceNode*)GetSiblingNode(), Volume));
      }

      // Add all frames to the buffer, adjusting with volume
      for (unsigned j = 0; j < gAudioSystem->SystemChannelsThreaded; ++j)
        InputSamples[i + j] = data.Samples[j] * Volume;
    }

    // Apply modifications
    forRange (ThreadedVolumeModifier* modifier, Data->VolumeModList.All())
    {
      if (modifier->Active)
        modifier->ApplyModification(InputSamples.Data(), bufferSize);
    }

    // Check if we should apply the equalizer filter
    if (EqualizerFilter)
    {
      // Process samples into the output buffer
      EqualizerFilter->ProcessBuffer(InputSamples.Data(), outputBuffer->Data(), numberOfChannels, bufferSize);
      // Move back to the input buffer
      InputSamples.Swap(*outputBuffer);
    }

    // Check if we should apply the compressor filter
    if (CompressorFilter)
    {
      // Process samples into the output buffer
      if (!CompressorSideChainInput)
        CompressorFilter->ProcessBuffer(InputSamples.Data(), InputSamples.Data(), outputBuffer->Data(),
          numberOfChannels, bufferSize);
      else
        CompressorFilter->ProcessBuffer(InputSamples.Data(), CompressorSideChainInput->Data(), 
          outputBuffer->Data(), numberOfChannels, bufferSize);
    }
    else
    {
      // Move from input buffer to output buffer
      InputSamples.Swap(*outputBuffer);
    }

    if (Paused && Data->PausingModifier)
    {
      Data->PausingModifier->Active = false;
      Data->PausingModifier = nullptr;
    }

    if (GetSiblingNode())
    {
      // Set the time variable on the non-threaded node
      gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundInstanceNode::CurrentTime, 
        (SoundInstanceNode*)GetSiblingNode(), CurrentTime));

      // Update the volume on the non-threaded instance
      if (Data->InterpolatingVolume)
        gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundInstanceNode::Volume, 
            (SoundInstanceNode*)GetSiblingNode(), Volume));

      // Update the pitch on the non-threaded instance
      if (Data->InterpolatingPitch)
        gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundInstanceNode::PitchFactor, 
            (SoundInstanceNode*)GetSiblingNode(), PitchFactor));
    }

    return true;
  }

  //************************************************************************************************
  void SoundInstanceNode::AddAttenuatedOutputToTag(TagObject* tag)
  {
    if (!ValidOutputLastMix)
      return;

    float volume(1.0);
    forRange(SoundNode* node, GetOutputs()->All())
    {
      float value = node->GetAttenuatedVolume();
      if (value > 0)
        volume *= value;
    }

    tag->TotalOutput.Resize(MixedOutput.Size());

    for (unsigned i = 0; i < tag->TotalOutput.Size(); ++i)
    {
      tag->TotalOutput[i] += MixedOutput[i] * volume;
    }
  }

  //************************************************************************************************
  void SoundInstanceNode::GetNextFrame(FrameData &frameData)
  {
    // If paused, just fill in zeros for sample values
    if (Paused)
    {
      for (unsigned i = 0; i < frameData.HowManyChannels; ++i)
        frameData.Samples[i] = 0.0f;

      return;
    }

    // If finished, just fill in zeros for sample values
    if (Finished)
    {
      for (unsigned i = 0; i < frameData.HowManyChannels; ++i)
        frameData.Samples[i] = 0.0f;

      return;
    }

    FrameData thisFrame = Asset->GetFrame(Data->FrameIndex);

    // Not resampling or pitch shifting
    if (!Data->Resampling && !Data->PitchShifting)
    {
      ++Data->FrameIndex;

      if (Data->CrossFading)
        ApplyCrossFade(frameData, 0.0f);
    }
    else
    {
      float factor;

      // Not resampling, are pitch shifting
      if (!Data->Resampling && Data->PitchShifting)
        factor = PitchFactor;
      // Resampling & pitch shifting
      else if (Data->Resampling && Data->PitchShifting)
        factor = PitchFactor * Data->ResampleFactor;
      // Resampling, not pitch shifting
      else
        factor = Data->ResampleFactor;

      // Add the interpolated value to the samples
      FrameData nextFrame = Asset->GetFrame(Data->FrameIndex + 1);
      for (unsigned i = 0; i < thisFrame.HowManyChannels; ++i)
        thisFrame.Samples[i] += (float)((nextFrame.Samples[i] - thisFrame.Samples[i]) 
          * (Data->ResampleIndex - Data->FrameIndex));

      // Advance the indexes
      Data->ResampleIndex += factor;
      Data->FrameIndex = (unsigned)Data->ResampleIndex;

      // Check for handling interpolating pitch
      if (Data->InterpolatingPitch)
      {
        PitchFactor = Data->PitchValues.NextValue();

        Data->InterpolatingPitch = !Data->PitchValues.Finished(GetSiblingNode());

        if (!Data->InterpolatingPitch && GetSiblingNode())
          // Update the non-threaded object with the final pitch
          gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundInstanceNode::PitchFactor,
            (SoundInstanceNode*)GetSiblingNode(), PitchFactor));
      }

      // Check if we need to handle cross-fading
      if (Data->CrossFading)
        ApplyCrossFade(thisFrame, factor);
    }

    // Check for reaching the end of the file or the loop end point
    CheckForEnd();

    // Check for pausing (stop when inaudible)
    if (Data->Pausing)
    {
      ++Data->FrameCount;
      if (Data->FrameCount >= Data->FramesToWait)
      {
        Paused = true;
        Data->Pausing = false;
      }
    }
    // Check for stopping (stop when inaudible)
    else if (Data->Stopping)
    {
      ++Data->FrameCount;
      if (Data->FrameCount >= Data->FramesToWait)
        FinishedCleanUp();
    }

    // Channels match, can just copy
    if (thisFrame.HowManyChannels == frameData.HowManyChannels)
      memcpy(frameData.Samples, thisFrame.Samples, sizeof(float) * thisFrame.HowManyChannels);
    // Otherwise adjust channels to match output
    else
    {
      AudioFrame frame(thisFrame.Samples, thisFrame.HowManyChannels);
      frame.TranslateChannels(frameData.HowManyChannels);
      memcpy(frameData.Samples, frame.Samples, sizeof(float) * frameData.HowManyChannels);
    }

    // Advance the current time
    CurrentTime = Data->FrameIndex * Data->TimeIncrement;

    MusicNotifications();
  }

  //************************************************************************************************
  void SoundInstanceNode::SkipForward(const unsigned howManyFrames)
  {
    // Not resampling or pitch shifting
    if (!Data->Resampling && !Data->PitchShifting)
    {
      Data->FrameIndex += howManyFrames;

      if (Data->CrossFading)
      {
        Data->CrossFadeFrameIndex += howManyFrames;

        if (Data->CrossFadeFrameIndex >= Data->CrossFadeVolumeInterpolator.GetTotalFrames()
          || Data->CrossFadeStartFrame + Data->CrossFadeFrameIndex >= Data->EndFrame)
          Data->CrossFading = false;
        else
          Data->CrossFadeSamples.Resize(Data->CrossFadeFrameIndex * Asset->GetChannels());
      }
    }
    else
    {
      double totalAdvance(0.0);

      if (!Data->InterpolatingPitch)
      {
        // Not resampling, are pitch shifting
        if (!Data->Resampling && Data->PitchShifting)
          totalAdvance = PitchFactor * howManyFrames;
        // Resampling & pitch shifting
        else if (Data->Resampling && Data->PitchShifting)
          totalAdvance = Data->ResampleFactor * PitchFactor * howManyFrames;
        // Resampling, not pitch shifting
        else
          totalAdvance = Data->ResampleFactor * howManyFrames;
      }
      else
      {
        // When interpolating need to go through every frame
        for (unsigned i = 0; i < howManyFrames; ++i)
        {
          float factor;

          // Not resampling, are pitch shifting
          if (!Data->Resampling)
            factor = PitchFactor;
          // Resampling & pitch shifting
          else
            factor = Data->ResampleFactor * PitchFactor;

          totalAdvance += factor;
          PitchFactor = Data->PitchValues.NextValue();
        }

        Data->InterpolatingPitch = !Data->PitchValues.Finished(GetSiblingNode());

        if (!Data->InterpolatingPitch && GetSiblingNode())
          // Update the non-threaded object with the final pitch
          gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundInstanceNode::PitchFactor,
          (SoundInstanceNode*)GetSiblingNode(), PitchFactor));
      }

      Data->ResampleIndex += totalAdvance;
      Data->FrameIndex = (unsigned)Data->ResampleIndex;

      if (Data->CrossFading)
      {
        Data->CrossFadeResampleIndex += totalAdvance;
        Data->CrossFadeFrameIndex = (unsigned)Data->CrossFadeResampleIndex;

        if (Data->CrossFadeFrameIndex >= Data->CrossFadeVolumeInterpolator.GetTotalFrames()
          || Data->CrossFadeStartFrame + Data->CrossFadeFrameIndex >= Data->EndFrame)
          Data->CrossFading = false;
        else
          Data->CrossFadeSamples.Resize(Data->CrossFadeFrameIndex * Asset->GetChannels());
      }
    }

    // Check for reaching the end of the file or the loop end point
    CheckForEnd();

    if (Data->Pausing)
    {
      Data->FrameCount += howManyFrames;
      if (Data->FrameCount >= Data->FramesToWait)
      {
        Paused = true;
        Data->Pausing = false;
      }
    }
    else if (Data->Stopping)
    {
      Data->FrameCount += howManyFrames;
      if (Data->FrameCount >= Data->FramesToWait)
        FinishedCleanUp();
    }

    CurrentTime = Data->FrameIndex * Data->TimeIncrement;

    MusicNotifications();

    if (Data->InterpolatingVolume)
    {
      Data->VolumeValues.JumpForward(howManyFrames);
      Data->InterpolatingVolume = !Data->VolumeValues.Finished(GetSiblingNode());
    }

    if (Paused && Data->PausingModifier)
    {
      Data->PausingModifier->Active = false;
      Data->PausingModifier = NULL;
    }

    // Set the time variable on the non-threaded node
    if (GetSiblingNode())
      gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundInstanceNode::CurrentTime, 
          (SoundInstanceNode*)GetSiblingNode(), CurrentTime));

    if (Virtual)
    {
      // Check all tags to see if we can un-virtualize
      for (TagListType::range tags = TagList.All(); !tags.Empty(); tags.PopFront())
      {
        if (!tags.Front()->CanInstanceUnVirtualize(this))
          return;
      }

      Virtual = false;
      Data->InterpolatingVolume = true;
      Data->VolumeValues.SetValues(0.0f, Volume, (unsigned)(0.1f * gAudioSystem->SystemSampleRate));
    }
  }

  //************************************************************************************************
  void SoundInstanceNode::CheckForEnd()
  {
    // If we reached the end 
    if (Data->FrameIndex >= Data->EndFrame)
    {
      // Looping, reset to beginning
      if (Looping)
      {
        Data->FrameIndex = Data->LoopStartFrame;
        Data->ResampleIndex = (float)Data->FrameIndex;

        CurrentTime = Data->FrameIndex * Data->TimeIncrement;
        ResetMusicBeats();

        if (GetSiblingNode())
          gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundNode::SendEventToExternalData,
              GetSiblingNode(), Notify_InstanceLooped, (void*)nullptr));
      }
      // Done, add to list to remove
      else
      {
        FinishedCleanUp();
      }
    }
    // If looping & reached loop end
    else if (Looping && Data->LoopEndFrame > 0 && Data->FrameIndex >= Data->LoopEndFrame)
    {
      // Reset variables
      Data->CrossFading = true;
      Data->CrossFadeStartFrame = Data->FrameIndex;
      Data->FrameIndex = Data->LoopStartFrame;
      Data->ResampleIndex = (double)Data->FrameIndex;
      Data->CrossFadeFrameIndex = 0;
      Data->CrossFadeResampleIndex = 0;
      CurrentTime = Data->FrameIndex * Data->TimeIncrement;
      ResetMusicBeats();

      // Determine the number of frames for the cross-fade
      unsigned fadeSize = (unsigned)(Data->CrossFadeTime * gAudioSystem->SystemSampleRate);
      if (Data->LoopTailFrames > 0 && !Asset->GetStreaming())
        fadeSize = Data->LoopTailFrames;

      // Make sure we don't go past the end of the file
      if (Data->FrameIndex + fadeSize > Data->EndFrame)
        fadeSize -= Data->FrameIndex + fadeSize - Data->EndFrame;

      // Reset the volume interpolator with the number of frames
      Data->CrossFadeVolumeInterpolator.SetValues(1.0f, 0.0f, fadeSize);

      // We can only get a second of samples at a time
      if (fadeSize > gAudioSystem->SystemSampleRate)
        fadeSize = gAudioSystem->SystemSampleRate;

      // Adjust to the number of samples
      fadeSize *= Asset->GetChannels();
      // Set the size of the cross-fade sample buffer
      Data->CrossFadeSamples.Resize(fadeSize);
      // Get the samples from the asset
      Asset->GetBuffer(Data->CrossFadeSamples.Data(), Data->CrossFadeStartFrame, fadeSize);
      
      // If streaming, reset to the beginning of the file
      if (Asset->GetStreaming())
        Asset->ResetStreamingFile();

      if (GetSiblingNode())
        gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundNode::SendEventToExternalData,
            GetSiblingNode(), Notify_InstanceLooped, (void*)nullptr));
    }

  }

  //************************************************************************************************
  void SoundInstanceNode::FinishedCleanUp()
  {
    if (Threaded)
    {
      if (!Finished)
      {
        Finished = true;

        // Send notification that this instance is finished
        if (GetSiblingNode())
        {
          gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundNode::SendEventToExternalData,
            GetSiblingNode(), Notify_InstanceFinished, (void*)nullptr));
          gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundInstanceNode::FinishedCleanUp,
            (SoundInstanceNode*)GetSiblingNode()));
          gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundInstanceNode::RemoveFromAllTags,
            (SoundInstanceNode*)GetSiblingNode()));
        }
      }
    }
    else
    {
      if (!Finished)
      {
        Finished = true;

        // If no external interface, can delete
        if (!GetExternalInterface())
          DisconnectOnlyThis();
      }
    }
  }

  //************************************************************************************************
  bool SoundInstanceNode::BelowMinimumVolume(unsigned frames)
  {
    // Determine overall volume at the beginning and end of the mix
    float volume1(Volume);
    float volume2(Volume);

    // If interpolating volume, get the volume at the end of the mix
    if (Data->InterpolatingVolume)
      volume2 = Data->VolumeValues.ValueAtIndex(Data->FrameIndex + frames);

    // Adjust with all volume modifiers
    forRange(ThreadedVolumeModifier* modifier, Data->VolumeModList.All())
    {
      if (modifier->Active)
      {
        volume1 *= modifier->GetCurrentVolume();
        volume2 *= modifier->GetFutureVolume(frames);
      }
    }

    // Return true if both volumes lower than minimum volume threshold
    if (volume1 < gAudioSystem->MinimumVolumeThresholdThreaded 
      && volume2 < gAudioSystem->MinimumVolumeThresholdThreaded)
      return true;
    else
      return false;
  }

  //************************************************************************************************
  void SoundInstanceNode::ApplyCrossFade(FrameData& frame, float resampleFactor)
  {
    unsigned sampleIndex = Data->CrossFadeFrameIndex * frame.HowManyChannels;

    // Check if we need more samples
    if (sampleIndex >= Data->CrossFadeSamples.Size())
      IncreaseCrossFadeBuffer(frame.HowManyChannels, sampleIndex);
    
    // Get the current cross-fading volume
    float volume = Data->CrossFadeVolumeInterpolator.ValueAtIndex(Data->CrossFadeFrameIndex);
    for (unsigned i = 0; i < frame.HowManyChannels; ++i, ++sampleIndex)
    {
      // If cross-fading, apply the other volume to the current sample (goes from 0-1)
      if (CrossFadeTail)
        frame.Samples[i] *= 1.0f - volume;

      ErrorIf(sampleIndex >= Data->CrossFadeSamples.Size(), 
        "Audio Engine: Index is greater than cross-fade sample buffer");

      // Apply the volume to the cross-fade sample and add to current sample (goes from 1-0)
      frame.Samples[i] += Data->CrossFadeSamples[sampleIndex] * volume;
    }

    // If not resampling, just advance the index
    if (resampleFactor == 0.0f)
    {
      ++Data->CrossFadeFrameIndex;
    }
    else
    {

      // Check if we need more samples
      if (sampleIndex >= Data->CrossFadeSamples.Size())
        IncreaseCrossFadeBuffer(frame.HowManyChannels, sampleIndex);

      // If the next frame is within the limits, add the interpolated amount to the samples
      if (Data->CrossFadeFrameIndex + 1 < Data->EndFrame && Data->CrossFadeFrameIndex + 1 
        < Data->CrossFadeVolumeInterpolator.GetTotalFrames())
      {
        float volume = Data->CrossFadeVolumeInterpolator.ValueAtIndex(Data->CrossFadeFrameIndex + 1);

        ErrorIf(sampleIndex >= Data->CrossFadeSamples.Size(), 
          "Audio Engine: Index is greater than cross-fade buffer size");

        for (unsigned i = 0; i < frame.HowManyChannels; ++i, ++sampleIndex)
          frame.Samples[i] += (float)((Data->CrossFadeSamples[sampleIndex] 
            - Data->CrossFadeSamples[sampleIndex - frame.HowManyChannels])
            * (Data->CrossFadeResampleIndex - Data->CrossFadeFrameIndex));
      }

      // Advance the cross-fading indexes
      Data->CrossFadeResampleIndex += resampleFactor;
      Data->CrossFadeFrameIndex = (unsigned)Data->CrossFadeResampleIndex;
    }

    // Check if cross-fading is done or if we would run past the end of the data
    if (Data->CrossFadeFrameIndex >= Data->CrossFadeVolumeInterpolator.GetTotalFrames()
      || Data->CrossFadeStartFrame + Data->CrossFadeFrameIndex >= Data->EndFrame)
      Data->CrossFading = false;

    ErrorIf(Data->CrossFadeFrameIndex * frame.HowManyChannels > Data->CrossFadeSamples.Size(), 
      "Audio Engine: Index is greater than cross-fade buffer size");
  }

  //************************************************************************************************
  void SoundInstanceNode::IncreaseCrossFadeBuffer(unsigned channels, unsigned sampleIndex)
  {
    // Get a second of new samples
    unsigned newSamples = gAudioSystem->SystemSampleRate * channels;

    // If that's too many, adjust the size
    if (sampleIndex + newSamples > Data->CrossFadeVolumeInterpolator.GetTotalFrames() * channels)
      newSamples -= sampleIndex + newSamples - (Data->CrossFadeVolumeInterpolator.GetTotalFrames() 
        * channels);

    ErrorIf(sampleIndex > Data->CrossFadeSamples.Size(), 
      "Audio Engine: Index is greater than cross-fade buffer size");

    // Increase size of buffer
    Data->CrossFadeSamples.Resize(Data->CrossFadeSamples.Size() + newSamples);

    // Get samples from asset
    Asset->GetBuffer(Data->CrossFadeSamples.Data() + sampleIndex, Data->CrossFadeStartFrame 
      + newSamples / channels, newSamples);
  }

  //************************************************************************************************
  ThreadedVolumeModifier* SoundInstanceNode::GetAvailableVolumeMod()
  {
    for (unsigned i = 0; i < Data->VolumeModList.Size(); ++i)
    {
      if (!Data->VolumeModList[i]->Active)
      {
        Data->VolumeModList[i]->Reset(1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
        return Data->VolumeModList[i];
      }
    }

    Data->VolumeModList.PushBack(new ThreadedVolumeModifier);
    return Data->VolumeModList.Back();
  }

  //************************************************************************************************
  void SoundInstanceNode::RemoveFromAllTags()
  {
    if (!Threaded)
    {
      while (!TagList.Empty())
        TagList.Back()->RemoveInstance(this);
    }
  }

  //************************************************************************************************
  void SoundInstanceNode::MusicNotifications()
  {
    // If there is a custom notification time set and we've hit that time
    if (NotifyTime > 0 && !CustomNotifySent && CurrentTime >= NotifyTime)
    {
      CustomNotifySent = true;

      // Send notification
      if (GetSiblingNode())
        gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundNode::SendEventToExternalData, 
            GetSiblingNode(), Notify_MusicCustomTime, (void*)nullptr));
    }

    // Track music beats if necessary
    if (SecondsPerEighth > 0)
    {
      double timePassed = CurrentTime - PreviousTime;
      PreviousTime = CurrentTime;
      if (timePassed <= 0.0f)
        timePassed = 0.00000001;

      AccumulatedTime += timePassed;

      // Check for eighth notes
      if (AccumulatedTime >= SecondsPerEighth)
      {
        AccumulatedTime = 0;

        // Increase eighth note count
        ++EighthNoteCount;

        // Check for beat
        if (EighthNoteCount % EighthsPerBeat == 0)
        {
          ++BeatsPerBarCount;

          // Check for new bar
          if (BeatsPerBarCount == BeatsPerBar)
          {
            EighthNoteCount = 0;
            BeatsPerBarCount = 0;

            // Send notification
            if (GetSiblingNode())
              gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundNode::SendEventToExternalData,
                  GetSiblingNode(), Notify_MusicBar, (void*)nullptr));
          }

          // Send notification
          if (GetSiblingNode())
            gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundNode::SendEventToExternalData, 
                GetSiblingNode(), Notify_MusicBeat, (void*)nullptr));
        }

        // Send notification
        if (GetSiblingNode())
          gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundNode::SendEventToExternalData,
              GetSiblingNode(), Notify_MusicEighthNote, (void*)nullptr));

        // Check for quarter note
        if (EighthNoteCount % 2 == 0)
        {
          // Send notification
          if (GetSiblingNode())
            gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundNode::SendEventToExternalData, 
                GetSiblingNode(), Notify_MusicQuarterNote, (void*)nullptr));
        }

        // Check for half note
        if (EighthNoteCount % 4 == 0)
        {
          // Send notification
          if (GetSiblingNode())
            gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundNode::SendEventToExternalData,
                GetSiblingNode(), Notify_MusicHalfNote, (void*)nullptr));
        }

        // Check for whole note
        if (EighthNoteCount % 8 == 0)
        {
          // Send notification
          if (GetSiblingNode())
            gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundNode::SendEventToExternalData, 
                GetSiblingNode(), Notify_MusicWholeNote, (void*)nullptr));
        }
      }
    }
  }

  //************************************************************************************************
  void SoundInstanceNode::ResetMusicBeats()
  {
    if (SecondsPerEighth == 0)
      return;

    PreviousTime = CurrentTime;

    float timePerMeasure = SecondsPerBeat * BeatsPerBar;
    float measuresSoFar = (float)CurrentTime / timePerMeasure;
    float timeSinceLastBar = (float)CurrentTime - ((int)measuresSoFar * timePerMeasure);

    // If we are at or very close to the beginning of a measure, reset everything
    if (timeSinceLastBar < 0.00001f)
    {
      AccumulatedTime = 0;
      BeatsPerBarCount = 0;
      EighthNoteCount = 0;

      if (GetSiblingNode())
      {
        gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundNode::SendEventToExternalData,
          GetSiblingNode(), Notify_MusicBar, (void*)nullptr));
        gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundNode::SendEventToExternalData, 
          GetSiblingNode(), Notify_MusicBeat, (void*)nullptr));
        gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundNode::SendEventToExternalData, 
          GetSiblingNode(), Notify_MusicWholeNote, (void*)nullptr));
        gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundNode::SendEventToExternalData,
          GetSiblingNode(), Notify_MusicEighthNote, (void*)nullptr));
        gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundNode::SendEventToExternalData, 
          GetSiblingNode(), Notify_MusicQuarterNote, (void*)nullptr));
        gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&SoundNode::SendEventToExternalData, 
          GetSiblingNode(), Notify_MusicHalfNote, (void*)nullptr));
      }
    }
    else
    {
      float eighthNotes = timeSinceLastBar / SecondsPerEighth;
      
      // Haven't hit first eighth note yet
      if (eighthNotes < 1.0f)
      {
        AccumulatedTime = timeSinceLastBar;
        BeatsPerBarCount = 0;
        EighthNoteCount = 0;
      }
      else
      {
        EighthNoteCount = (int)eighthNotes;
        AccumulatedTime = eighthNotes - EighthNoteCount;
        BeatsPerBarCount = EighthNoteCount / EighthsPerBeat;
      }
    }
  }

  //************************************************************************************************
  void SoundInstanceNode::DisconnectThisAndAllInputs()
  {
    SoundNode::DisconnectThisAndAllInputs();

    if (GetSiblingNode())
    {
      gAudioSystem->AddTask(Zero::CreateFunctor(&SoundInstanceNode::FinishedCleanUp, 
        ((SoundInstanceNode*)GetSiblingNode())));
    }
  }
}