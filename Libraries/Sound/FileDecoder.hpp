///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2018, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

struct OpusDecoder;

namespace Zero
{

//----------------------------------------------------------------------------------- Decoded Packet

class DecodedPacket
{
public:
  DecodedPacket() {}
  DecodedPacket(unsigned bufferSize);
  DecodedPacket(const DecodedPacket& copy);

  DecodedPacket& operator=(const DecodedPacket& other);

  // The buffer of samples for this channel
  BufferType mSamples;
};

//-------------------------------------------------------------------- Single Channel Packet Decoder

class SingleChannelPacketDecoder
{
public:
  SingleChannelPacketDecoder() : mDecoder(nullptr) {}
  ~SingleChannelPacketDecoder();

  // Initializes decoder for use with DecodePacket. 
  // If the decoder already exists, it will be destroyed and re-created.
  void InitializeDecoder();
  // Decodes a single packet of data and allocates a buffer for the decoded data.
  void DecodePacket(const byte* packetData, const unsigned dataSize, float*& decodedData,
    unsigned& numberOfSamples);

private:
  // Used for repeated calls to DecodePacket
  OpusDecoder* mDecoder;
};

//----------------------------------------------------------------------------------- Packet Decoder

class PacketDecoder
{
public:
  // Decodes the provided packet and returns the number of samples
  // Allocates memory for the decoded data
  static int DecodePacket(const byte* packetData, unsigned dataSize, OpusDecoder* decoder,
    float** decodedData);
  // Decodes the provided packet and returns the number of samples
  // Assumes the memory for the decoded data is already allocated
  static int DecodePacket(const byte* packetData, unsigned dataSize, OpusDecoder* decoder,
    float* decodedData);
  // Reads the header from the provided buffer and returns the size of the associated data,
  // or -1 if there is an error
  static int GetPacketDataSize(const byte* packetHeader);
  // Reads the header data of the file into the object and returns the size of the file's data
  static unsigned OpenAndReadHeader(Zero::Status& status, const String& fileName,
    File* file, FileHeader* header);
  // Creates the requested number of opus decoders, returns false if unsuccessful
  static bool CreateDecoders(Status& status, OpusDecoder** decoderArray, int howMany);
  // Destroys the requested number of opus decoders, setting the pointers to null
  static void DestroyDecoders(OpusDecoder** decoderArray, int howMany);
  // Fills in the provided buffer with the next packet data from a buffer of data. 
  // Returns -1 if getting packet fails or if the end of the data was reached.
  static int GetPacketFromMemory(byte* packetDataToWrite, const byte* inputData, unsigned inputDataSize,
    unsigned* dataIndex);
  // Fills in the provided buffer with the next packet data from a file.
  // Returns -1 if getting packet fails or if the end of the data was reached.
  static int GetPacketFromFile(byte* packetDataToWrite, File* inputFile, FilePosition* filePosition, 
    ThreadLock* lockObject);
};

//------------------------------------------------------------------------------------- File Decoder

typedef void(*FileDecoderCallback)(DecodedPacket*, void* data);

class AudioFileDecoder
{
public:
  AudioFileDecoder(int channels, unsigned samplesPerChannel, FileDecoderCallback callback, void* callbackData);
  virtual ~AudioFileDecoder();

  // Should only be called when starting the decoding thread 
  virtual void DecodingLoopThreaded() = 0;
  // Fills in the provided buffer with the next packet data. Returns -1 if getting packet
  // fails or if the end of the data was reached.
  virtual int GetNextPacket(byte* packetData) = 0;
  // Called to decode the next packet when the system is not threaded
  virtual void RunDecodingTask() = 0;
  // Requests the next chunk of decoded data
  void DecodeNextSection();

  // Number of channels of audio
  int mChannels;
  // Number of samples per channel in the audio data
  unsigned mSamplesPerChannel;

protected:
  // Decodes the next packet of data (assumed that this is called on a decoding thread)
  bool DecodePacketThreaded();
  // Starts up the decoding thread 
  void StartDecodingThread();
  // Stops and shuts down the decoding thread
  void StopDecodingThread();
  // Destroys the decoders
  virtual void ClearData();

  // The callback function that will be used to hand off decoded packets
  FileDecoderCallback mCallback;
  // The callback data to pass through
  void* mCallbackData;
  // Opus decoders for each channel
  OpusDecoder* mDecoders[AudioConstants::cMaxChannels];
  // Thread for decoding tasks
  Thread DecodeThread;
  // Tells the decoding thread it should shut down
  ThreadedInt mShutDownSignal;
  // Tells the decoding thread it should decode another packet
  Semaphore DecodingSemaphore;
};

//----------------------------------------------------------------------------- Decompressed Decoder

class DecompressedDecoder : public AudioFileDecoder
{
public:
  DecompressedDecoder(Zero::Status& status, const Zero::String& fileName, FileDecoderCallback callback,
    void* callbackData);
  ~DecompressedDecoder();

  // Should only be called when starting the decoding thread 
  void DecodingLoopThreaded() override;
  // Fills in the provided buffer with the next packet data. Returns -1 if getting packet
  // fails or if the end of the data was reached.
  int GetNextPacket(byte* packetData) override;
  // Called to decode the next packet when the system is not threaded
  void RunDecodingTask() override;

private:
  // Opens a file and reads in its data
  void OpenAndReadFile(Zero::Status& status, const Zero::String& fileName);
  // Destroys decoders and deletes input data
  void ClearData() override;

  // The data read in from the file 
  byte* mCompressedData;
  // The current read position for the compressed data
  unsigned mDataIndex;
  // The size of the compressed data
  unsigned mDataSize;
};

//-------------------------------------------------------------------------------- Streaming Decoder

class StreamingDecoder : public AudioFileDecoder
{
public:
  // The file object must be already open, and will not be closed by this decoder
  StreamingDecoder(Zero::Status& status, Zero::File* inputFile, Zero::ThreadLock* lock,
    unsigned channels, unsigned frames, FileDecoderCallback callback, void* callbackData);
  // The input data buffer must already exist, and will not be deleted by this decoder
  StreamingDecoder(Zero::Status& status, byte* inputData, unsigned dataSize, unsigned channels,
    unsigned frames, FileDecoderCallback callback, void* callbackData);

  // Should only be called when starting the decoding thread 
  void DecodingLoopThreaded() override;
  // Fills in the provided buffer with the next packet data. Returns -1 if getting packet
  // fails or if the end of the data was reached.
  int GetNextPacket(byte* packetData) override;
  // Called to decode the next packet when the system is not threaded
  void RunDecodingTask() override;
  // Resets streaming decoding to the beginning
  void Reset();

private:
  // The data read in from the file, if streaming from memory (will not be deleted)
  byte* mCompressedData;
  // The current read position for the compressed data, if streaming from memory
  unsigned mDataIndex;
  // The size of the compressed data, if streaming from memory
  unsigned mDataSize;

  // The file to read the data from (does not own this file and will not close it)
  File* mInputFile;
  // The current read position in the file, if streaming from file
  FilePosition mFilePosition;
  // Used to lock when reading from the file, if streaming from file
  ThreadLock* mLock;
};

} // namespace Zero
