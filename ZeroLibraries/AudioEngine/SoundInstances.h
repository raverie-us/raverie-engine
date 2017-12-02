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

  //------------------------------------------------------------------------------ Cross Fade Object

  class AudioFadeObject
  {
  public:
    AudioFadeObject();

    void StartFade(float startingVolume, unsigned startingIndex, unsigned fadeFrames, 
      SoundAsset* asset, bool crossFade);
    void ApplyFade(float* buffer, unsigned howManyFrames);
    void GetMoreSamples();

    // If true, sound is currently fading
    bool mFading;
    // The current frame index of the fade
    unsigned mFrameIndex;
    // The asset frame at which the fade started
    unsigned mStartFrame;
    // If true, the audio will be cross-faded rather than just faded out
    // (new audio will fade in proportionally to the audio fading out)
    bool mCrossFade;
    // The asset associated with this sound
    SoundAsset* mAsset;
    // Used to interpolate cross-fading volumes. 
    InterpolatingObject VolumeInterpolator;
    // The samples to use for cross-fading
    Zero::Array<float> FadeSamples;
    // Number of frames to use for default fade
    unsigned mDefaultFrames;
  };

  //---------------------------------------------------------------------- Music Notification Object

  class MusicNotificationObject
  {
  public:
    MusicNotificationObject() :
      mSecondsPerBeat(0),
      mBeatsPerBar(0),
      mBeatsCount(0),
      mBeatNoteType(0),
      mSecondsPerEighth(0),
      mEighthsPerBeat(0),
      mEighthNoteCount(0),
      mTotalEighths(0)
    {}

    void ProcessAndNotify(float currentTime, SoundNode* siblingNode);
    void ResetBeats(float currentTime, SoundNode* siblingNode);

    // Number of seconds per music beat
    float mSecondsPerBeat;
    // Number of music beats per music bar
    int mBeatsPerBar;
    // Accumulates beats to know when we hit another bar
    int mBeatsCount;
    // Type of note used for the beat (4 = quarter note, etc.)
    int mBeatNoteType;
    // Number of seconds for each eighth note
    float mSecondsPerEighth;
    // Number of eighth notes per music beat
    int mEighthsPerBeat;
    // Accumulates number of eighth notes (reset on each bar)
    int mEighthNoteCount;
    // Tracks the total number of eighth notes from the beginning of the music
    unsigned mTotalEighths;
  };

  //---------------------------------------------------------------------------- Sound Instance Node

  class SoundInstanceNode : public SimpleCollapseNode
  {
  public:
    SoundInstanceNode(Zero::Status& status, Zero::StringParam name, const unsigned ID, 
      SoundAsset* parentAsset, const bool looping, const bool startPaused, 
      ExternalNodeInterface* extInt, const bool isThreaded = false);

    // Returns true if the instance is currently paused.
    bool GetPaused();
    // Pass in true to pause a currently playing instance, then false to resume playing it.
    void SetPaused(bool isPaused);
    // Stops the instance (will delete this object when finished if no external data).
    void Stop();
    // Returns true if instance is currently looping.
    bool GetLooping();
    // Sets whether this instance should loop.
    void SetLooping(const bool loop);
    // Returns the time in seconds at which the instance will start playing.
    float GetStartTime();
    // Sets the time in seconds from the beginning of the file at which playback should start.
    void SetStartTime(const float startTime);
    // Returns the time in seconds at which the instance will stop playing.
    float GetEndTime();
    // Sets the time in seconds from the beginning of the file at which the instance should stop.
    void SetEndTime(const float seconds);
    // The time in seconds from the beginning of the file to jump back to when looping.
    float GetLoopStartTime();
    // Sets the time to jump back to when starting another loop.
    void SetLoopStartTime(const float time);
    // The time in seconds from the beginning of the file to stop when looping.
    float GetLoopEndTime();
    // Sets the time to stop and jump back when looping.
    void SetLoopEndTime(const float time);
    // The number of seconds to cross-fade after the loop end time.
    float GetLoopTailTime();
    // Sets the number of seconds to use for cross-fading after the loop end time.
    void SetLoopTailTime(const float time);
    // If true, the loop tail will be cross-faded. If false, it will be added
    // but will fade out smoothly.
    bool GetCrossFadeTail();
    // Sets whether the loop tail cross-fades or is added and fades.
    void SetCrossFadeTail(bool crossFade);
    // Returns the current volume of the instance.
    float GetVolume();
    // Sets the volume of the instance over a specified time in seconds.
    void SetVolume(const float newVolume, float timeToChange);
    // Returns the current pitch change, in cents.
    int GetPitch();
    // Sets the pitch change of the instance (in cents) over a specified time in seconds.
    void SetPitch(const int pitchCents, const float timeToChange);
    // Returns true if the instance is currently playing.
    bool IsPlaying();
    // Changes the playback position to a specified time in seconds.
    void JumpTo(const float seconds);
    // Returns the current (approximate) playback time.
    float GetTime();
    // Returns the current beats per minute.
    float GetBeatsPerMinute();
    // Sets the beats per minute.
    void SetBeatsPerMinute(const float bpm);
    // Sets the time signature.
    void SetTimeSignature(const int beats, const int noteType);
    // Returns the time at which a notification will be sent.
    float GetCustomNotifyTime();
    // Sets a time (in seconds, from file beginning) at which the instance will send a notification.
    void SetCustomNotifyTime(const float time);

  private:
    ~SoundInstanceNode();
    bool GetOutputSamples(Zero::Array<float>* outputBuffer, const unsigned numberOfChannels,
      ListenerNode* listener, const bool firstRequest) override;
    // Adds the audio data from the mix to the tag, taking into account volume modifications
    void AddAttenuatedOutputToTag(TagObject* tag);
    // Fills the InputSamples buffer with the specified number of audio frames
    void FillBuffer(unsigned outputFrames, unsigned outputChannels);
    // Resets back to the loop start point
    void Loop();
    // Translates the audio data in the array to the specified output channels, and puts the data
    // back into the array
    static void TranslateChannels(Zero::Array<float>& inputSamples, const unsigned inputFrames,
      const unsigned inputChannels, const unsigned outputChannels);
    // Sends notification and removes instance from any associated tags.
    void FinishedCleanUp();
    // Check for whether the total volume is lower than the minimum.
    bool BelowMinimumVolume(unsigned frames);
    // Returns a volume modifier from the array.
    ThreadedVolumeModifier* GetAvailableVolumeMod();
    // Removes this instance from all tags it is associated with.
    void RemoveFromAllTags();
    // Handle music beat notifications.
    void MusicNotifications();
    // Resets the music beat tracking to the current location in the file.
    void ResetMusicBeats();
    // Calls the disconnect function on the base class and then calls FinishedCleanUp on 
    // the threaded node.
    void DisconnectThisAndAllInputs() override;

    typedef Zero::Array<TagObject*> TagListType;
    // List of tags that the instance is currently associated with.
    TagListType TagList;
    // Equalizer filter controlled by tags.
    Equalizer* TagEqualizer;
    // Compressor filter controlled by tags.
    DynamicsProcessor* TagCompressor;
    // Input from another tag used by the TagCompressor filter.
    const Zero::Array<float>* TagCompressorInput;

    // Handles fading the audio when looping or jumping.
    AudioFadeObject Fade;
    // Handles sending music notifications to the external object.
    MusicNotificationObject MusicNotify;
    // Handles pitch changes.
    PitchChangeHandler Pitch;
    // The asset to use for audio sample data.
    SoundAsset* Asset;

    // The (approximate) current time position in the file.
    double mCurrentTime;
    // If true, sound will loop instead of stopping.
    bool mLooping;
    // If true, sound instance is paused.
    bool mPaused;
    // Current volume of this sound.
    float mVolume;
    // If true, the sound has finished playing.
    bool mFinished;
    // The time at which the instance will start playing.
    float mStartTime;
    // The time at which the instance will stop playing.
    float mEndTime;
    // The time to jump back to when looping.
    float mLoopStartTime;
    // The time to stop and jump back when looping.
    float mLoopEndTime;
    // The time after the LoopEndTime to use for cross-fading.
    float mLoopTailTime;
    // If true, the loop tail will cross-fade (new audio will fade in as the tail fades out).
    bool mCrossFadeTail;
    // Time for custom notification.
    float mNotifyTime;
    // Tracks whether the custom notification has been sent.
    bool mCustomNotifySent;
    // The current factor by which the pitch is being changed.
    float mPitchFactor;

    // ** Threaded object variables **

    // Index of the current audio frame. 
    unsigned mFrameIndex;
    // If true, sound is ramping volume down to zero to pause. 
    bool mPausing;
    // If true, sound is ramping volume down to zero to stop. 
    bool mStopping;
    // Counts number of frames until pausing or stopping. 
    unsigned mStopFrameCount;
    // Number of frames to wait until pausing or stopping. 
    unsigned mStopFramesToWait;
    // If true, volume is being interpolated. 
    bool mInterpolatingVolume;
    // If true, currently changing pitch. 
    bool mPitchShifting;
    // The frame the playback should start at.
    unsigned mStartFrame;
    // The frame this instance's playback should stop at.
    unsigned mEndFrame;
    // The frame to jump back to when looping.
    unsigned mLoopStartFrame;
    // The frame to stop and jump back when looping.
    unsigned mLoopEndFrame;
    // The frames after the LoopEndTime to use for fading.
    unsigned mLoopTailFrames;
    // The time, in seconds, per audio frame.
    double mTimeIncrement;
    // The time position in the file on the previous mix.
    double mPreviousTime;
    // Used to control volume modifications while pausing.
    ThreadedVolumeModifier *PausingModifier;
    // Used to interpolate from one volume to another. 
    InterpolatingObject VolumeInterpolator;
    // Volume adjustments, used by the instance and by tags.
    Zero::Array<ThreadedVolumeModifier*> VolumeModList;

    bool mVirtual;

    friend class TagObject;
  };

}

#endif
