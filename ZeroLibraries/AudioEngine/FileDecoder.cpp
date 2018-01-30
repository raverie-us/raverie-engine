///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.h"
#include "stb_vorbis.h"
#include "opus.h"

namespace Audio
{
  //--------------------------------------------------------------------------------- Decoded Packet

  //************************************************************************************************
  DecodedPacket::DecodedPacket(unsigned bufferSize) :
    Samples(bufferSize)
  {

  }

  //************************************************************************************************
  DecodedPacket::DecodedPacket(const DecodedPacket& copy) :
    Samples(Zero::MoveReference<Zero::Array<float>>(const_cast<DecodedPacket&>(copy).Samples))
  {

  }

  //************************************************************************************************
  DecodedPacket& DecodedPacket::operator=(const DecodedPacket& other)
  {
    Samples = Zero::MoveReference<Zero::Array<float>>(const_cast<DecodedPacket&>(other).Samples);
    return *this;
  }

  //----------------------------------------------------------------------------------- File Decoder

  //************************************************************************************************
  FileDecoder::FileDecoder(Zero::Status& status, const Zero::String& fileName, const bool streaming, 
      SoundAssetFromFile* asset) :
    DecodingTaskCount(0),
    ParentAlive(this),
    InputFileData(nullptr),
    DataIndex(0),
    Streaming(streaming),
    FileName(fileName)
  {
    // The constructor happens on the game thread

    // Open the input file
    Zero::File inputFile;
    inputFile.Open(fileName, Zero::FileMode::Read, Zero::FileAccessPattern::Sequential);
    // If not open, set the failed message and return
    if (!inputFile.IsOpen())
    {
      status.SetFailed(Zero::String::Format("Unable to open audio file %s", fileName.c_str()));
      return;
    }

    // Get the size of the file
    long long size = inputFile.CurrentFileSize();
    // Check for an invalid size
    if (size < sizeof(FileHeader))
    {
      status.SetFailed(Zero::String::Format("Unable to read from audio file %s", fileName.c_str()));
      return;
    }

    // Save the file size 
    DataSize = (unsigned)size;

    // If not streaming, create a buffer for all data and read it in
    if (!Streaming)
    {
      InputFileData = new byte[DataSize];
      inputFile.Read(status, InputFileData, DataSize);
    }
    // Otherwise create a buffer for the maximum packet size and read in the file header
    else
    {
      InputFileData = new byte[FileEncoder::MaxPacketSize];
      inputFile.Read(status, InputFileData, sizeof(FileHeader));
    }

    // If the read failed, delete the buffer and return
    if (status.Failed())
    {
      delete[] InputFileData;
      InputFileData = nullptr;
      return;
    }

    // Read the file header from the input data
    FileHeader header;
    memcpy(&header, InputFileData, sizeof(header));
    // Move the index forward
    DataIndex += sizeof(header);

    // If this isn't the right type of file, set the failed message, delete the buffer, and return
    if (header.Name[0] != 'Z' || header.Name[1] != 'E')
    {
      status.SetFailed(Zero::String::Format("Audio file %s is an incorrect format", fileName.c_str()));
      delete[] InputFileData;
      InputFileData = nullptr;
      return;
    }

    // Set the data variables
    SamplesPerChannel = header.SamplesPerChannel;
    Channels = header.Channels;

    // Create a decoder for each channel
    int error;
    for (short i = 0; i < Channels; ++i)
    {
      Decoders[i] = opus_decoder_create(AudioSystemInternal::SystemSampleRate, 1, &error);

      // If there was an error creating the decoder, set the failed message and delete the buffer
      if (error < 0)
      {
        status.SetFailed(Zero::String::Format("Error creating audio decoder: %s", opus_strerror(error)));

        delete[] InputFileData;
        InputFileData = nullptr;
        return;
      }
    }
  }

  //************************************************************************************************
  FileDecoder::~FileDecoder()
  {
    // Delete any existing decoded packets
    DecodedPacket packet;
    while (DecodedPacketQueue.Read(packet))
    {

    }

    // Destroy any alive decoders
    for (short i = 0; i < Channels; ++i)
    {
      if (Decoders[i])
        opus_decoder_destroy(Decoders[i]);
    }

    // If there is data in the buffer, delete it
    if (InputFileData)
      delete[] InputFileData;
  }

  //************************************************************************************************
  void FileDecoder::AddDecodingTask()
  {
    AtomicIncrement32(&DecodingTaskCount);

    // Add the decoding task
    gAudioSystem->AddDecodingTask(Zero::CreateFunctor(&FileDecoder::DecodePacket, this));
  }

  //************************************************************************************************
  void FileDecoder::DecodePacket()
  {
    // Remember that this function happens on the decoding thread
    
    // If no data, can't do anything
    if (!InputFileData || DataIndex >= DataSize || (Streaming && !InputFile.IsOpen()))
    {
      // If the pointer is null, this object should be deleted 
      // (sets ParentAlive to null if it's currently null, returns original value which would be null)
      if (AtomicCompareExchangePointer(&ParentAlive, nullptr, nullptr) == nullptr)
        delete this;

      AtomicDecrement32(&DecodingTaskCount);

      return;
    }

    int frames;
    Zero::Status status;

    // Get a packet for each channel
    for (short i = 0; i < Channels; ++i)
    {
      PacketHeader packHead;

      // Read in the packet header
      if (!Streaming)
        memcpy(&packHead, InputFileData + DataIndex, sizeof(packHead));
      else
        InputFile.Read(status, (byte*)&packHead, sizeof(packHead));

      // Move the data index forward
      DataIndex += sizeof(packHead);

      ErrorIf(DataIndex + packHead.Size > DataSize);

      // If not a valid packet, set frames to zero
      if (packHead.Size == 0 || (packHead.Name[0] != 'p' || packHead.Name[1] != 'a'))
        frames = 0;
      else
      {
        // If not streaming, decode the packet from the buffer
        if (!Streaming)
          frames = opus_decode_float(Decoders[i], InputFileData + DataIndex, packHead.Size,
            DecodedPackets[i], FileEncoder::PacketFrames, 0);
        // Otherwise read in the data from the file before decoding
        else
        {
          InputFile.Read(status, InputFileData, packHead.Size);

          frames = opus_decode_float(Decoders[i], InputFileData, packHead.Size,
            DecodedPackets[i], FileEncoder::PacketFrames, 0);
        }
      }

      ErrorIf(frames < 0, opus_strerror(frames));

      // Move the data index forward
      DataIndex += packHead.Size;
    }

    // Check if we're not streaming
    if (!Streaming)
    {
      // If we've reached the end of the file, delete the data
      if (DataIndex >= DataSize)
      {
        delete[] InputFileData;
        InputFileData = nullptr;
      }
      // Otherwise add another task to continue decoding the file
      else 
        gAudioSystem->AddDecodingTask(Zero::CreateFunctor(&FileDecoder::DecodePacket, this));
    }

    // If the pointer is null, this object should be deleted 
    // (sets ParentAlive to null if it's currently null, returns original value which would be null)
    if (AtomicCompareExchangePointer(&ParentAlive, nullptr, nullptr) == nullptr)
      delete this;

    // Add the decoded packets to the queue. Must happen last so that all actions are completed
    // before the packet is sent to the SoundAsset.
    QueueDecodedPackets(frames);

    AtomicDecrement32(&DecodingTaskCount);
  }

  //************************************************************************************************
  bool FileDecoder::StreamIsOpen()
  {
    return InputFile.IsOpen();
  }

  //************************************************************************************************
  void FileDecoder::ResetStream()
  {
    if (!InputFile.IsOpen())
      return;

    // Wait for all existing tasks to be done
    while (AtomicCompareExchange32(&DecodingTaskCount, 0, 0) != 0)
    {

    }

    // Remove any current packets from the queue
    DecodedPacket packet;
    while (DecodedPacketQueue.Read(packet))
    {

    }

    // Set the file to the start of the data
    InputFile.Seek(sizeof(FileHeader));
    // Set the index
    DataIndex = sizeof(FileHeader);

    // Destroy the current decoders (since they rely on history for decoding, they can't 
    // continue from the beginning of the file)
    for (short i = 0; i < Channels; ++i)
      opus_decoder_destroy(Decoders[i]);

    // Create new decoders
    int error;
    for (short i = 0; i < Channels; ++i)
      Decoders[i] = opus_decoder_create(AudioSystemInternal::SystemSampleRate, 1, &error);

    AddDecodingTask();
  }

  //************************************************************************************************
  void FileDecoder::CloseStream()
  {
    if (InputFile.IsOpen())
      InputFile.Close();


    DecodedPacket packet;
    while (DecodedPacketQueue.Read(packet))
    {

    }
  }

  //************************************************************************************************
  void FileDecoder::OpenStream()
  {
    InputFile.Open(FileName, Zero::FileMode::Read, Zero::FileAccessPattern::Sequential);

    ResetStream();
  }

  //************************************************************************************************
  void FileDecoder::QueueDecodedPackets(unsigned numberOfFrames)
  {
    // Create the DecodedPacket object
    DecodedPacket newPacket(numberOfFrames * Channels);

    // Step through each frame of samples
    for (unsigned frame = 0, index = 0; frame < numberOfFrames; ++frame)
    {
      // Copy the sample from each channel to the interleaved sample buffer
      for (short channel = 0; channel < Channels; ++channel, ++index)
      {
        float sample = DecodedPackets[channel][frame];
        newPacket.Samples[index] = sample;

        // Samples should be between [-1, +1] but it's possible
        // encoding caused the sample to jump beyond 1
        ErrorIf(sample < -2.0f || sample > 2.0f);
      }
    }

    // Add the DecodedPacket object to the queue
    DecodedPacketQueue.Write(newPacket);
  }

  //--------------------------------------------------------------------------------- Packet Decoder

  //************************************************************************************************
  PacketDecoder::~PacketDecoder()
  {
    if (Decoder)
      opus_decoder_destroy(Decoder);
  }

  //************************************************************************************************
  void PacketDecoder::InitializeDecoder()
  {
    if (Decoder)
      opus_decoder_destroy(Decoder);

    int error;
    Decoder = opus_decoder_create(AudioSystemInternal::SystemSampleRate, PacketEncoder::Channels, &error);
  }

  //************************************************************************************************
  void PacketDecoder::DecodePacket(const byte* packetData, const unsigned dataSize, 
    float*& decodedData, unsigned& numberOfSamples)
  {
    ReturnIf(!Decoder, , "Tried to decode packet without initializing decoder");

    decodedData = new float[PacketEncoder::PacketFrames];
    numberOfSamples = opus_decode_float(Decoder, packetData, dataSize, decodedData, 
      PacketEncoder::PacketFrames, 0);
  }

}