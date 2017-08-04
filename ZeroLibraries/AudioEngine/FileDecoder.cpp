///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.h"
#include "stb_vorbis.h"

namespace Audio
{
  //----------------------------------------------------------------------------------- File Decoder

  //************************************************************************************************
  FileDecoder::FileDecoder(Zero::Status& status, const Zero::String& fileName, const bool streaming) :
    Streaming(streaming),
    DecodingCheck(this)
  {
    InputFile.Open(fileName, Zero::FileMode::Read, Zero::FileAccessPattern::Sequential);
    if (!InputFile.IsOpen())
    {
      status.SetFailed(Zero::String::Format("Unable to open audio file %s", fileName.c_str()));
      return;
    }

    InputFile.Read(status, (byte*)&Header, sizeof(Header));
    if (status.Failed())
      return;

    ErrorIf(Header.Name[0] != 'Z');

    int error;
    for (short i = 0; i < Header.Channels; ++i)
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

  void FileDecoder::DecodeNextPacket()
  {
    // Remember that this function happens on the decoding thread

    // If the file isn't open, can't do anything
    // TODO also could be from memory instead of a file
    if (!InputFile.IsOpen())
    {
      if (AtomicCheckEqualityPointer(DecodingCheck, nullptr))
        delete this;

      return;
    }

    Zero::Status status;
    PacketHeader packHead;
    int frames;

    // Get a packet for each channel
    for (short i = 0; i < Header.Channels; ++i)
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

          // If the DecodingCheck pointer is still this object, set it to NULL
          if (AtomicCompareExchangePointer((void*)DecodingCheck, nullptr, (void*)this)))))))

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
      frames = opus_decode_float(Decoders[packHead.Channel], PacketBuffer, packHead.Size, DecodedPackets[i], FrameSize, 0);

      ErrorIf(frames < 0, Zero::String::Format("Opus error: %s", opus_strerror(frames)).c_str());
    }

    // Create the DecodedPacket object
    DecodedPacket* newPacket = new DecodedPacket();
    // Set the number of frames
    newPacket->FrameCount = frames;
    // Create the buffer for the samples
    newPacket->Samples = new float[newPacket->FrameCount * Header.Channels];

    // Step through each frame of samples
    for (unsigned frame = 0, index = 0; frame < newPacket->FrameCount; ++frame)
    {
      // Copy this sample from the decoded packets to the DecodedPacket sample buffer
      for (short channel = 0; channel < Header.Channels; ++channel, ++index)
        newPacket->Samples[index] = DecodedPackets[channel][frame];
    }

    // Add the DecodedPacket object to the queue
    DecodedPacketQueue.Write(newPacket);

    // If the pointer is now NULL, the asset has been destroyed
    if (AtomicCheckEqualityPointer(DecodingCheck, nullptr))
      delete this;
  }

}