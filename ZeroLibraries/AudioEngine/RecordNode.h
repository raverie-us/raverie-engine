///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef RECORDNODE_H
#define RECORDNODE_H

namespace Audio
{
  //------------------------------------------------------------------------------------ Record Node

  class RecordNode : public SimpleCollapseNode
  {
  public:
    RecordNode(Zero::Status& status, Zero::StringParam name, unsigned ID,
      ExternalNodeInterface* extInt, bool isThreaded = false);

    // Returns the name currently set for the output file
    Zero::StringParam GetFileName() { return FileName; }
    // Sets the name to use for the output file
    void SetFileName(Zero::StringParam fileName);
    // Starts recording data to the file
    void StartRecording();
    // Stops recording and closes the file
    void StopRecording();
    // Returns true if recording is paused
    bool GetPaused();
    // Pauses recording temporarily (must still call StopRecording when finished)
    void SetPaused(const bool paused);
    // Gets whether data should constantly write to disk or be saved and written when StopRecording is called
    bool GetStreamToDisk();
    // Sets whether data should constantly write to disk or be saved and written when StopRecording is called
    void SetStreamToDisk(const bool streamToDisk);

  private:
    ~RecordNode();
    bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
      ListenerNode* listener, const bool firstRequest) override;
    void WriteBuffer(Zero::Array<float>* buffer, unsigned numberOfChannels);

    Zero::String FileName;
    Zero::File FileStream;
    unsigned SamplesRecorded;
    float MaxValue;
    bool Recording;
    bool Paused;
    unsigned Channels;
    bool Streaming;
    Zero::Array<float> SavedSamples;

    struct WavHeader
    {
      char riff_chunk[4];
      unsigned chunk_size;
      char wave_fmt[4];
      char fmt_chunk[4];
      unsigned fmt_chunk_size;
      unsigned short audio_format;
      unsigned short number_of_channels;
      unsigned sampling_rate;
      unsigned bytes_per_second;
      unsigned short bytes_per_sample;
      unsigned short bits_per_sample;
      char data_chunk[4];
      unsigned data_chunk_size;
    };
  };
}

#endif
