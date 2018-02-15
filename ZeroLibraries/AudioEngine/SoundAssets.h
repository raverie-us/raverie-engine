///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef ASSETS_H
#define ASSETS_H

struct OpusDecoder;

namespace Audio
{
  //------------------------------------------------------------------------------------ Sound Asset

  // Base class for all sound assets
  class SoundAsset
  {
  public:
    SoundAsset(ExternalNodeInterface* externalInterface, const bool threaded);

    // Sets the external interface pointer. If set to NULL and there are no references, asset will be deleted.
    void SetExternalInterface(ExternalNodeInterface* externalInterface);
    // Returns true if there are sound instances currently active.
    bool IsPlaying();
    // Asset name is stored so that it can be retrieved from a SoundInstance.
    Zero::String mName;

  protected:
    virtual ~SoundAsset();

    // Pointer to the threaded asset (will be NULL on the threaded object).
    SoundAsset* ThreadedAsset;
    // True if this is the threaded asset.
    const bool Threaded;

  private:
    // Called when a non-threaded sound instance is created. Returns false if the instance
    // can't be attached to this asset (such as already streaming one instance).
    bool AddReference();
    // Called when a non-threaded sound instance is deleted. If 0 references and no external
    // interface, the asset will be deleted.
    void ReleaseReference();

    // Appends the specified number of samples to the array, starting at the specified frame index.
    virtual void AppendSamples(BufferType* buffer, const unsigned frameIndex,
      unsigned numberOfSamples) = 0;
    // Returns the number of audio channels.
    virtual unsigned GetChannels() = 0;
    // The total number of audio frames in this asset's data.
    virtual unsigned GetNumberOfFrames() = 0;
    // Returns true if this asset is streaming.
    virtual bool GetStreaming() = 0;
    // Returns true if another instance can be added to this asset. Called from AddReference.
    virtual bool OkayToAddInstance() { return true; }
    // Called from ReleaseReference.
    virtual void RemoveInstance() {}
    // Resets a streaming file back to the beginning.
    virtual void ResetStreamingFile() {}

    // Pointer to the external interface object for this asset.
    ExternalNodeInterface* ExternalData;
    // Number of existing references from sound instances.
    unsigned mReferenceCount;

    Zero::Link<SoundAsset> link;

    friend class SoundInstanceNode;
    friend class AudioFadeObject;
    friend class AudioSystemInternal;
    friend class GranularSynthNode;
  };

  //-------------------------------------------------------------------------- Sound Asset From File

  class FileDecoder;
  
  class SoundAssetFromFile : public SoundAsset
  {
  public:
    SoundAssetFromFile(Zero::Status& status, const Zero::String& fileName, const bool streaming,
      ExternalNodeInterface* externalInterface, const bool isThreaded = false);

    // Appends the specified number of samples to the array, starting at the specified frame index.
    void AppendSamples(BufferType* buffer, const unsigned frameIndex,
      unsigned numberOfSamples) override;
    // The total number of audio frames in this asset's data.
    unsigned GetNumberOfFrames() override;
    // Returns true if this asset is streaming.
    bool GetStreaming() override;
    // Resets a streaming file back to the beginning.
    void ResetStreamingFile() override;
    // Returns the length of the audio file, in seconds.
    float GetLengthOfFile();
    // Returns the number of channels in the audio data.
    unsigned GetChannels() override;
        
  private:
    ~SoundAssetFromFile();

    // Returns false if this is a streaming asset and there is already an instance associated with it.
    bool OkayToAddInstance() override;
    // Resets the HasStreamingInstance variable.
    void RemoveInstance() override;
    // Checks if there is a decoded packet to copy into the Samples buffer. Returns true if 
    // decoded samples were added and false if they were not.
    bool ProcessAvailableDecodedPacket();
    // Moves the NextStreamedSamples buffer into the Samples buffer and checks for more decoded samples.
    // Returns true if samples were moved into the Samples buffer and false if none were available.
    bool MoveBuffers();
    // Fills the buffer with the specified number of samples, starting at the specified index.
    // Will pad with zero if not enough samples available.
    void FillStreamingBuffer(float** buffer, unsigned* sampleIndex, unsigned* samplesNeeded);

    // If true, this is a streaming asset.
    bool mStreaming;
    // If true, the asset is streaming and has an instance associated with it.
    bool mHasStreamingInstance;
    // The length of the audio file, in seconds.
    float mFileLength;
    // The number of channels in the audio data.
    unsigned mChannels;
    // The number of audio frames in the audio data.
    unsigned mFrameCount;
    // Pointer to the decoder object used by this asset.
    FileDecoder* Decoder;
    // The buffer of decoded samples.
    BufferType Samples;
    // If streaming, keeps track of previous samples.
    unsigned mPreviousBufferSamples;
    // The next buffer of decoded streamed samples.
    BufferType NextStreamedSamples;
    // If true, the NextStreamedSamples buffer needs to be filled out.
    bool mNeedSecondBuffer;
  };

  //--------------------------------------------------------------------- Generated Wave Sound Asset

  class GeneratedWaveSoundAsset : public SoundAsset
  {
  public:
    GeneratedWaveSoundAsset(const OscillatorTypes::Enum waveType, const float frequency, 
      ExternalNodeInterface *extInt, const bool isThreaded = false);

    // Appends the specified number of samples to the array, starting at the specified frame index.
    void AppendSamples(BufferType* buffer, const unsigned frameIndex,
      unsigned numberOfSamples) override;
    // Returns the number of channels in the audio data
    unsigned GetChannels() override { return 1; }
    // The total number of audio frames in this asset's data
    unsigned GetNumberOfFrames() override;
    // Returns true if this asset is streaming
    bool GetStreaming() override;
    // Returns the current frequency of the generated wave
    float GetFrequency();
    // Sets the frequency of the generated wave, over a specified number of seconds
    void SetFrequency(const float frequency, const float time);
    // If a square wave is chosen, set the fraction (0 - 1.0) of the wave which will be positive
    void SetSquareWavePositiveFraction(float positiveFraction);

  private:
    ~GeneratedWaveSoundAsset();

    Oscillator* WaveData;
    // The current wave frequency
    float mFrequency;
    // Used to interpolate between two frequencies
    InterpolatingObject FrequencyInterpolator;
  };
}

#endif