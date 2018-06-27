///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Andrea Ellinger
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Zero
{

//-------------------------------------------------------------------------------------- Sound Asset

class SoundAsset : public ReferenceCountedObject
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  SoundAsset(const String& assetName, bool streaming);
  virtual ~SoundAsset();

  // Appends the specified number of samples to the array, starting at the specified frame index.
  virtual void AppendSamplesThreaded(BufferType* buffer, const unsigned frameIndex,
    unsigned numberOfSamples, unsigned instanceID) = 0;
  // Resets a streaming file back to the beginning.
  virtual void ResetStreamingFile(unsigned instanceID) {}
  // Called when a non-threaded sound instance is created. 
  void AddInstance(unsigned instanceID);
  // Called when a non-threaded sound instance is deleted. 
  void RemoveInstance(unsigned instanceID);

  // If true, this is a streaming asset.
  const bool mStreaming;
  // The length of the audio file, in seconds.
  float mFileLength;
  // The number of channels in the audio data.
  unsigned mChannels;
  // The number of audio frames in the audio data.
  unsigned mFrameCount;
  // The name of the asset
  const String mName;

private:
  // Number of existing references from sound instances.
  unsigned mInstanceReferenceCount;

  // Called from AddInstance.
  virtual void OnAddInstanceThreaded(unsigned instanceID) {}
  // Called from RemoveInstance.
  virtual void OnRemoveInstanceThreaded(unsigned instanceID) {}

};

//------------------------------------------------------------------------- Decompressed Sound Asset

class DecompressedSoundAsset : public SoundAsset
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  DecompressedSoundAsset(Status& status, const String& fileName, const String& assetName);

  // Appends the specified number of samples to the array, starting at the specified frame index.
  void AppendSamplesThreaded(BufferType* buffer, const unsigned frameIndex, unsigned numberOfSamples,
    unsigned instanceID) override;
  // Called by the decoder to pass off decoded samples
  void DecodingCallback(DecodedPacket* packet);

private:
  // The decoder object used by the asset
  DecompressedDecoder mDecoder;
  // The buffer of decoded samples.
  BufferType mSamples;
  // The number of decoded samples currently available
  volatile unsigned mSamplesAvailableShared;
};

//---------------------------------------------------------------------- Streaming Data Per Instance

class StreamingDataPerInstance
{
public:
  // The file object must be already open, and will not be closed by this decoder
  StreamingDataPerInstance(Status& status, File* inputFile, ThreadLock* lock, unsigned channels, 
    unsigned frames, unsigned instanceID);
  // The input data buffer must already exist, and will not be deleted by this decoder
  StreamingDataPerInstance(Status& status, byte* inputData, unsigned dataSize, unsigned channels, 
    unsigned frames, unsigned instanceID);

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

  Link<StreamingDataPerInstance> link;
};

//---------------------------------------------------------------------------- Streaming Sound Asset

class StreamingSoundAsset : public SoundAsset
{
public:
  ZilchDeclareType(TypeCopyMode::ReferenceType);

  StreamingSoundAsset(Status& status, const String& fileName, AudioFileLoadType::Enum loadType,
    const String& assetName);
  ~StreamingSoundAsset();

  // Appends the specified number of samples to the array, starting at the specified frame index.
  void AppendSamplesThreaded(BufferType* buffer, const unsigned frameIndex, unsigned numberOfSamples,
    unsigned instanceID) override;
  // Resets a streaming file back to the beginning.
  void ResetStreamingFile(unsigned instanceID) override;
  // Adds data for a new instance. Does not check for duplicates.
  void OnAddInstanceThreaded(unsigned instanceID) override;
  // Removes data for a specific instance.
  void OnRemoveInstanceThreaded(unsigned instanceID) override;

private:
  // Looks for a specific instance ID in the data list. Returns null if not found.
  StreamingDataPerInstance* GetInstanceData(unsigned instanceID);
  // Copies available decoded samples into the provided buffer
  void CopySamplesIntoBuffer(float* outputBuffer, unsigned sampleIndex, unsigned samplesRequested,
    StreamingDataPerInstance* data);

  // Decoded data per instance
  InList<StreamingDataPerInstance> mDataPerInstanceList;
  // If streaming from file, the file object to keep open
  File mInputFile;
  // The name of the file
  String mFileName;
  // If streaming from memory, the data read in from the file
  Array<byte> mInputFileData;
  // Used to lock when reading from the input file
  ThreadLock mLock;
};

} // namespace Zero
