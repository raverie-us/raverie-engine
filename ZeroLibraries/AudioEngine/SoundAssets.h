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
    // Returns true if this asset is streaming.
    virtual bool GetStreaming() { return false; }
    // Returns the number of audio channels.
    virtual unsigned GetChannels() = 0;
    // Returns the length of the audio file, in seconds
    virtual float GetLengthOfFile() = 0;

  protected:
    virtual ~SoundAsset();

    // Pointer to the threaded asset (will be NULL on the threaded object).
    SoundAsset* ThreadedAsset;
    // True if this is the threaded asset.
    const bool Threaded;

  private:
    // Called when a non-threaded sound instance is created. 
    void AddReference(unsigned instanceID);
    // Called when a non-threaded sound instance is deleted. If 0 references and no external
    // interface, the asset will be deleted.
    void ReleaseReference(unsigned instanceID);

    // Appends the specified number of samples to the array, starting at the specified frame index.
    virtual void AppendSamples(BufferType* buffer, const unsigned frameIndex,
      unsigned numberOfSamples, unsigned instanceID) = 0;
    // The total number of audio frames in this asset's data.
    virtual unsigned GetNumberOfFrames() = 0;
    // Resets a streaming file back to the beginning.
    virtual void ResetStreamingFile(unsigned instanceID) {}
    // Called from AddReference.
    virtual void AddInstance(unsigned instanceID) {}
    // Called from ReleaseReference.
    virtual void RemoveInstance(unsigned instanceID) {}

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

  namespace FileLoadType { enum Enum { Auto, StreamedFromFile, StreamedFromMemory, Decompressed }; }

  //----------------------------------------------------------------------- Decompressed Sound Asset

  class DecompressedSoundAsset : public SoundAsset
  {
  public:
    DecompressedSoundAsset(Zero::Status& status, const Zero::String& fileName,
      ExternalNodeInterface* externalInterface, const bool isThreaded = false);

    // Appends the specified number of samples to the array, starting at the specified frame index.
    void AppendSamples(BufferType* buffer, const unsigned frameIndex, unsigned numberOfSamples,
      unsigned instanceID) override;
    // The total number of audio frames in this asset's data.
    unsigned GetNumberOfFrames() override;
    // Returns the number of channels in the audio data.
    unsigned GetChannels() override;
    // Returns the length of the audio file, in seconds.
    float GetLengthOfFile() override;
    // Called by the decoder to pass off decoded samples
    void DecodingCallback(DecodedPacket* packet);

  private:
    ~DecompressedSoundAsset();

    // The length of the audio file, in seconds.
    float mFileLength;
    // The number of channels in the audio data.
    unsigned mChannels;
    // The number of audio frames in the audio data.
    unsigned mFrameCount;
    // The decoder object used by the threaded asset
    DecompressedDecoder* mDecoder;
    // The buffer of decoded samples.
    BufferType mSamples;
    // The number of decoded samples currently available
    volatile unsigned mSamplesAvailableShared;
  };

  //-------------------------------------------------------------------- Streaming Data Per Instance

  class StreamingDataPerInstance
  {
  public:
    // The file object must be already open, and will not be closed by this decoder
    StreamingDataPerInstance(Zero::Status& status, Zero::File* inputFile, Zero::ThreadLock* lock, 
      unsigned channels, unsigned frames, unsigned instanceID);
    // The input data buffer must already exist, and will not be deleted by this decoder
    StreamingDataPerInstance(Zero::Status& status, byte* inputData, unsigned dataSize,
      unsigned channels, unsigned frames, unsigned instanceID);

    // Resets the data to start streaming from the beginning of the file
    void Reset();
    // Called by the decoder to pass off decoded samples
    void DecodingCallback(DecodedPacket* packet);

    // The current buffer of decoded samples
    BufferType mSamples;
    // Keeps track of previously played samples to translate indexes
    unsigned mPreviousSamples;
    // The decoder object
    StreamingDecoder mDecoder;
    // The ID of the instance associated with this data
    unsigned mInstanceID;
    // The list of decoded packets to process
    LockFreeQueue<DecodedPacket> mDecodedPacketQueue;
    
    Zero::Link<StreamingDataPerInstance> link;
  };

  //-------------------------------------------------------------------------- Streaming Sound Asset

  class StreamingSoundAsset : public SoundAsset
  {
  public:
    StreamingSoundAsset(Zero::Status& status, const Zero::String& fileName, FileLoadType::Enum loadType,
      ExternalNodeInterface* externalInterface, const bool isThreaded = false);

    // Appends the specified number of samples to the array, starting at the specified frame index.
    void AppendSamples(BufferType* buffer, const unsigned frameIndex, unsigned numberOfSamples,
      unsigned instanceID) override;
    // The total number of audio frames in this asset's data.
    unsigned GetNumberOfFrames() override;
    // Returns true if this asset is streaming.
    bool GetStreaming() override;
    // Resets a streaming file back to the beginning.
    void ResetStreamingFile(unsigned instanceID) override;
    // Returns the number of channels in the audio data.
    unsigned GetChannels() override;
    // Returns the length of the audio file, in seconds.
    float GetLengthOfFile() override;
    // Adds data for a new instance. Does not check for duplicates.
    void AddInstance(unsigned instanceID) override;
    // Removes data for a specific instance.
    void RemoveInstance(unsigned instanceID) override;

  private:
    ~StreamingSoundAsset();

    // Looks for a specific instance ID in the data list. Returns null if not found.
    StreamingDataPerInstance* GetInstanceData(unsigned instanceID);
    // Copies available decoded samples into the provided buffer
    void CopySamplesIntoBuffer(float* outputBuffer, unsigned sampleIndex, unsigned samplesRequested,
      StreamingDataPerInstance* data);

    // The length of the audio file, in seconds.
    float mFileLength;
    // The number of channels in the audio data.
    unsigned mChannels;
    // The number of audio frames in the audio data.
    unsigned mFrameCount;
    // Decoded data per instance
    Zero::InList<StreamingDataPerInstance> mDataPerInstanceList;
    // If streaming from file, the file object to keep open
    Zero::File mInputFile;
    // The name of the file
    Zero::String mFileName;
    // If streaming from memory, the data read in from the file
    Zero::Array<byte> mInputFileData;
    // Used to lock when reading from the input file
    Zero::ThreadLock mLock;
  };

  //--------------------------------------------------------------------- Generated Wave Sound Asset

  class GeneratedWaveSoundAsset : public SoundAsset
  {
  public:
    GeneratedWaveSoundAsset(const OscillatorTypes::Enum waveType, const float frequency, 
      ExternalNodeInterface *extInt, const bool isThreaded = false);

    // Appends the specified number of samples to the array, starting at the specified frame index.
    void AppendSamples(BufferType* buffer, const unsigned frameIndex, unsigned numberOfSamples,
      unsigned instanceID) override;
    // Returns the number of channels in the audio data
    unsigned GetChannels() override { return 1; }
    // The total number of audio frames in this asset's data
    unsigned GetNumberOfFrames() override;
    // Returns the length of the audio file, in seconds.
    float GetLengthOfFile() override;
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