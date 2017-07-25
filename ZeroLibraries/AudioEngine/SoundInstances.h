///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef INSTANCES_H
#define INSTANCES_H

namespace Audio
{
  class TagObject;
  class ThreadedVolumeModifier;
  class SoundInstanceNodeData;

  //---------------------------------------------------------------------------- Sound Instance Node

  class SoundInstanceNode : public SimpleCollapseNode
  {
  public:
    SoundInstanceNode(Zero::Status& status, Zero::StringParam name, const unsigned ID, 
      SoundAssetNode* parentAsset, const bool looping, const bool startPaused, 
      ExternalNodeInterface* extInt, const bool isThreaded = false);

    bool GetPaused() { return Paused; }
    // Pauses the output of the instance
    void Pause();
    // Resumes output
    void Resume();
    // Stops the instance (will delete this object when finished if no external data)
    void Stop();
    // Returns true if instance is currently looping
    bool GetLooping();
    // Sets whether this instance should loop
    void SetLooping(const bool loop);
    // Returns the time in seconds at which the instance will start playing
    float GetStartTime();
    // Sets the time in seconds from the beginning of the file at which playback should start
    void SetStartTime(const float startTime);
    // Returns the time in seconds at which the instance will stop playing
    float GetEndTime();
    // Sets the time in seconds from the beginning of the file at which the instance should stop
    void SetEndTime(const float seconds);
    // The time in seconds from the beginning of the file to jump back to when looping
    float GetLoopStartTime();
    // Sets the time to jump back to when starting another loop
    void SetLoopStartTime(const float time);
    // The time in seconds from the beginning of the file to stop when looping
    float GetLoopEndTime();
    // Sets the time to stop and jump back when looping
    void SetLoopEndTime(const float time);
    // The number of seconds to cross-fade after the loop end time 
    float GetLoopTailTime();
    // Sets the number of seconds to use for cross-fading after the loop end time
    void SetLoopTailTime(const float time);
    // If true, the loop tail will be cross-faded. If false, it will be added
    // but will fade out smoothly.
    bool GetCrossFadeTail();
    // Sets whether the loop tail cross-fades or is added and fades.
    void SetCrossFadeTail(bool crossFade);
    // Sets the volume of the instance over a specified time in seconds
    void SetVolume(const float newVolume, float timeToChange);
    // Returns the current volume of the instance
    float GetVolume();
    // Sets the pitch change of the instance (in cents) over a specified time in seconds
    void SetPitch(const int pitchCents, const float timeToChange);
    // Returns the current pitch change, in cents
    int GetPitch();
    // Returns true if the instance is currently playing
    bool IsPlaying();
    // Changes the playback position to a specified time in seconds
    void JumpTo(const float seconds);
    // Returns the current (approximate) playback time
    float GetTime();
    // Returns the current beats per minute
    float GetBeatsPerMinute();
    // Sets the beats per minute
    void SetBeatsPerMinute(const float bpm);
    // Sets the time signature
    void SetTimeSignature(const int beats, const int noteType);
    // Returns the time at which a notification will be sent
    float GetCustomNotifyTime();
    // Sets a time (in seconds, from file beginning) at which the instance will send a notification
    void SetCustomNotifyTime(const float time);

  private:
    ~SoundInstanceNode();
    bool GetOutputSamples(Zero::Array<float>* outputBuffer, const unsigned numberOfChannels,
      ListenerNode* listener, const bool firstRequest) override;
    void AddAttenuatedOutputToTag(TagObject* tag);

    typedef Zero::Array<TagObject*> TagListType;
    // List of tags that the instance is currently associated with
    TagListType TagList;

    bool Virtual;

    // The asset to use for sample data
    SoundAssetNode* Asset;
    // The data used by a threaded instance for playback
    SoundInstanceNodeData* Data;
    // The (approximate) current time position in the file
    double CurrentTime;
    // If true, sound will loop instead of stopping. 
    bool Looping;
    // If true, sound instance is paused. 
    bool Paused;
    // Current volume of this sound. 
    float Volume;
    // Current pitch shift
    float PitchFactor;
    // If true, the sound has finished playing. 
    bool Finished;
    // Number of seconds per music beat
    float SecondsPerBeat;
    // The time position in the file on the previous mix
    double PreviousTime;
    // Number of music beats per music bar
    int BeatsPerBar;
    // Accumulates beats to know when we hit another bar
    int BeatsPerBarCount;
    // Type of note used for the beat (4 = quarter note, etc.)
    int BeatNoteType;
    // Tracks accumulated time for eighth notes
    double AccumulatedTime;
    // Number of seconds for each eighth note
    float SecondsPerEighth;
    // Number of eighth notes per music beat
    int EighthsPerBeat;
    // Accumulates number of eighth notes (reset on each bar)
    int EighthNoteCount;
    // The time at which the instance will start playing
    float StartTime;
    // The time at which the instance will stop playing
    float EndTime;
    // The time to jump back to when looping
    float LoopStartTime;
    // The time to stop and jump back when looping
    float LoopEndTime;
    // The time after the LoopEndTime to use for cross-fading
    float LoopTailTime;
    // If true, the loop tail will be cross-faded rather than just faded out
    bool CrossFadeTail;
    // Time for custom notification
    float NotifyTime;
    // Tracks whether the custom notification has been sent
    bool CustomNotifySent;

    Equalizer* EqualizerFilter;
    DynamicsProcessor* CompressorFilter;
    const Zero::Array<float>* CompressorSideChainInput;

    // Gets the next frame of sample data. Can only be accessed sequentially.
    void GetNextFrame(FrameData &data);
    // Skips forward the specified number of frames. Handles looping and interpolations.
    void SkipForward(const unsigned howManyFrames);
    // Checks for reaching end of file, handling looping or stopping
    void CheckForEnd();
    // Sends notification and removes instance from any associated tags. 
    void FinishedCleanUp();
    // Check for whether the total volume is lower than the minimum
    bool BelowMinimumVolume(unsigned frames);
    // Adds the cross-fade frames to the current frame
    // Pass in 0 for resampleFactor if not resampling
    void ApplyCrossFade(FrameData& frame, float resampleFactor);
    // Increases the size of the cross-fading buffer
    void IncreaseCrossFadeBuffer(unsigned channels, unsigned sampleIndex);
    // Returns a volume modifier from the array
    ThreadedVolumeModifier* GetAvailableVolumeMod();
    // Removes this instance from all tags it is associated with
    void RemoveFromAllTags();
    // Handle music beat notifications
    void MusicNotifications();
    // Resets the music beat tracking to the current location in the file
    void ResetMusicBeats();

    void DisconnectThisAndAllInputs() override;

    friend class TagObject;
  };

}

#endif