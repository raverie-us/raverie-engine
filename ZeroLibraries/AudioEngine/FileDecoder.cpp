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
    Asset(asset)
  {
    InputFile.Open(fileName, Zero::FileMode::Read, Zero::FileAccessPattern::Sequential);
    if (!InputFile.IsOpen())
    {
      status.SetFailed(Zero::String::Format("Unable to open audio file %s", fileName.c_str()));
      return;
    }

    FileHeader header;

    InputFile.Read(status, (byte*)&header, sizeof(header));
    if (status.Failed())
      return;

    if (header.Name[0] != 'Z' || header.Name[1] != 'E')
    {
      status.SetFailed(Zero::String::Format("Audio file %s is an incorrect format", fileName.c_str()));
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
  }

  //************************************************************************************************
  void FileDecoder::DecodeNextPacket()
  {
    // Remember that this function happens on the decoding thread

    // If the file isn't open, can't do anything
    // TODO also could be from memory instead of a file
    if (!InputFile.IsOpen())
    {
      // If the asset is null, should delete
      if (AtomicCheckEqualityPointer(Asset, nullptr))
        delete this;

      return;
    }

    Zero::Status status;
    PacketHeader packHead;
    int frames;

    // Get a packet for each channel
    for (short i = 0; i < Channels; ++i)
    {
      // Read in the packet header
      InputFile.Read(status, (byte*)&packHead, sizeof(packHead));
      // Check if the read failed
      if (status.Failed())
      {
        // If not streaming, close the file and return
        if (!Streaming)
        {
          InputFile.Close();

          // Set the decoding marker to null
          AtomicSetPointer((void**)&Decoding, (void*)nullptr);

          return;
        }
        // Otherwise, reset to the beginning of the file and try again
        // TODO change this when streaming implemented?
        else
        {
          InputFile.Seek(sizeof(FileHeader), Zero::FileOrigin::Begin);
          InputFile.Read(status, (byte*)&packHead, sizeof(packHead));
          if (status.Failed())
          {
            InputFile.Close();
            AtomicSetPointer((void**)&Decoding, (void*)nullptr);
            return;
          }
        }
      }

      ErrorIf(packHead.Name[0] != 'p' || packHead.Name[1] != 'a');
      ErrorIf(packHead.Size > MaxPacketSize);
      ErrorIf(packHead.Channel != i);

      // Read in the packet from the file
      InputFile.Read(status, PacketBuffer, packHead.Size);

      // Decode the packet
      frames = opus_decode_float(Decoders[packHead.Channel], PacketBuffer, packHead.Size, 
        DecodedPackets[i], FrameSize, 0);

      ErrorIf(frames < 0, Zero::String::Format("Opus error: %s", opus_strerror(frames)).c_str());
    }

    // Create the DecodedPacket object
    DecodedPacket* newPacket = new DecodedPacket();
    // Set the number of frames
    newPacket->FrameCount = frames;
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

    // If the asset is null, should delete
    if (AtomicCheckEqualityPointer(Asset, nullptr))
      delete this;
  }

}