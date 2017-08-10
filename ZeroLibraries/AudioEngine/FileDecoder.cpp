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
  //----------------------------------------------------------------------------------- File Decoder

  //************************************************************************************************
  FileDecoder::FileDecoder(Zero::Status& status, const Zero::String& fileName, const bool streaming, 
      SoundAssetFromFile* asset) :
    Streaming(streaming),
    Decoding(this),
    Asset(asset),
    InputFileData(nullptr),
    DataIndex(0),
    FileName(fileName)
  {
    Zero::File inputFile;
    inputFile.Open(fileName, Zero::FileMode::Read, Zero::FileAccessPattern::Sequential);
    if (!inputFile.IsOpen())
    {
      status.SetFailed(Zero::String::Format("Unable to open audio file %s", fileName.c_str()));
      return;
    }

    long long size = inputFile.CurrentFileSize();
    if (size < sizeof(FileHeader))
    {
      status.SetFailed(Zero::String::Format("Unable to read from audio file %s", fileName.c_str()));
      return;
    }

    DataSize = (unsigned)size;

    if (!Streaming)
    {
      InputFileData = new byte[DataSize];
      inputFile.Read(status, InputFileData, DataSize);
    }
    else
    {
      InputFileData = new byte[MaxPacketSize];
      inputFile.Read(status, InputFileData, sizeof(FileHeader));
    }

    if (status.Failed())
    {
      delete[] InputFileData;
      InputFileData = nullptr;
      return;
    }

    FileHeader header;
    memcpy(&header, InputFileData, sizeof(header));
    DataIndex += sizeof(header);

    if (header.Name[0] != 'Z' || header.Name[1] != 'E')
    {
      status.SetFailed(Zero::String::Format("Audio file %s is an incorrect format", fileName.c_str()));
      delete[] InputFileData;
      InputFileData = nullptr;
      return;
    }

    SamplesPerChannel = header.SamplesPerChannel;
    Channels = header.Channels;

    int error;
    for (short i = 0; i < Channels; ++i)
    {
      Decoders[i] = opus_decoder_create(48000, 1, &error);
      if (error < 0)
      {
        status.SetFailed(Zero::String::Format("Error creating audio decoder: %s", opus_strerror(error)));

        // TODO some sort of failed state so it doesn't try to do anything else with this asset

        delete[] InputFileData;
        InputFileData = nullptr;
        return;
      }
    }

    // TODO Need to not do this on the game thread!!
    gAudioSystem->AddDecodingTask(Zero::CreateFunctor(&FileDecoder::DecodeNextPacket, this));
  }

  //************************************************************************************************
  FileDecoder::~FileDecoder()
  {
    for (short i = 0; i < Channels; ++i)
      opus_decoder_destroy(Decoders[i]);

    if (InputFileData)
      delete[] InputFileData;
  }

  //************************************************************************************************
  void FileDecoder::DecodeNextPacket()
  {
    // Remember that this function happens on the decoding thread

    // If no data, can't do anything
    if (!InputFileData || DataIndex >= DataSize || (Streaming && !InputFile.IsOpen()))
    {
      // If the asset is null, should delete
      if (AtomicCheckEqualityPointer(Asset, nullptr))
        delete this;

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

      if (packHead.Size == 0 || (packHead.Name[0] != 'p' || packHead.Name[1] != 'a'))
        frames = 0;
      else
      {
        if (!Streaming)
          frames = opus_decode_float(Decoders[i], InputFileData + DataIndex, packHead.Size,
            DecodedPackets[i], FrameSize, 0);
        else
        {
          InputFile.Read(status, InputFileData, packHead.Size);

          frames = opus_decode_float(Decoders[i], InputFileData, packHead.Size,
            DecodedPackets[i], FrameSize, 0);
        }
      }

      ErrorIf(frames < FrameSize);

      // Move the data index forward
      DataIndex += packHead.Size;

      ErrorIf(DataIndex >= DataSize);
    }

    // Add the decoded packets to the queue
    QueueDecodedPackets(frames);

    // If we've reached the end of the file, delete the data and stop decoding
    if (DataIndex >= DataSize && !Streaming)
    {
      delete[] InputFileData;
      InputFileData = nullptr;

      // Set the decoding marker to null
      AtomicSetPointer((void**)&Decoding, (void*)nullptr);
    }

    // If the asset is null, should delete
    if (AtomicCheckEqualityPointer(Asset, nullptr))
      delete this;
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

    InputFile.Seek(sizeof(FileHeader));
    DataIndex = sizeof(FileHeader);

    // TODO read in buffer
  }

  //************************************************************************************************
  void FileDecoder::CloseStream()
  {
    if (InputFile.IsOpen())
      InputFile.Close();
  }

  //************************************************************************************************
  void FileDecoder::OpenStream()
  {
    InputFile.Open(FileName, Zero::FileMode::Read, Zero::FileAccessPattern::Sequential);

    ResetStream();
  }

  //************************************************************************************************
  void FileDecoder::QueueDecodedPackets(int numberOfFrames)
  {
    // Create the DecodedPacket object
    DecodedPacket* newPacket = new DecodedPacket();
    // Set the number of frames
    newPacket->FrameCount = numberOfFrames;
    // Create the buffer for the samples
    newPacket->Samples = new float[newPacket->FrameCount * Channels];

    // Step through each frame of samples
    for (unsigned frame = 0, index = 0; frame < newPacket->FrameCount; ++frame)
    {
      // Copy this sample from the decoded packets to the DecodedPacket sample buffer
      for (short channel = 0; channel < Channels; ++channel, ++index)
      {
        newPacket->Samples[index] = DecodedPackets[channel][frame];

        ErrorIf(newPacket->Samples[index] < -1.0f || newPacket->Samples[index] > 1.0f);
      }
    }

    // Add the DecodedPacket object to the queue
    DecodedPacketQueue.Write(newPacket);
  }

}