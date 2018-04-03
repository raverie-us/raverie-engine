///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.h"

namespace Audio
{
  //----------------------------------------------------------------------------- Audio Input Output

  //************************************************************************************************
  AudioInputOutput::AudioInputOutput() :
    OutputStreamLatency(LatencyValues::LowLatency),
    MixedOutputBuffer(nullptr)
  {
    memset(OutputBufferSizePerLatency, 0, sizeof(unsigned) * LatencyValues::Count);

    InputRingBuffer.Initialize(sizeof(float), InputBufferSize, InputBuffer);
  }

  //************************************************************************************************
  AudioInputOutput::~AudioInputOutput()
  {
    if (MixedOutputBuffer)
      delete[] MixedOutputBuffer;
  }

  //************************************************************************************************
  void AudioInputOutput::WaitUntilOutputNeededThreaded()
  {
    MixThreadSemaphore.WaitAndDecrement();
  }

  //************************************************************************************************
  void AudioInputOutput::GetInputDataThreaded(Zero::Array<float>& buffer, unsigned howManySamples)
  {
    unsigned samplesAvailable = InputRingBuffer.GetReadAvailable();

    if (samplesAvailable < howManySamples)
      howManySamples = samplesAvailable;
    
    buffer.Resize(howManySamples);

    unsigned samplesRead = InputRingBuffer.Read(buffer.Data(), howManySamples);

    if (samplesRead != howManySamples)
      buffer.Resize(samplesRead);
  }

  //************************************************************************************************
  void AudioInputOutput::SetOutputLatency(LatencyValues::Enum newLatency)
  {
    // If the setting is the same, don't do anything
    if (newLatency == OutputStreamLatency)
      return;

    // Set the latency variable
    OutputStreamLatency = newLatency;
    // Shut down the output stream so we can change the buffer size
    ShutDownStream(StreamTypes::Output);
    // Set up the output buffer with the current latency setting
    SetUpOutputBuffers();
    // Restart the audio output
    InitializeStream(StreamTypes::Output);
    StartStream(StreamTypes::Output);
  }

  //************************************************************************************************
  const Audio::StreamInfo& AudioInputOutput::GetStreamInfo(StreamTypes::Enum whichStream)
  {
    return StreamInfoList[whichStream];
  }

  //************************************************************************************************
  void AudioInputOutput::InitializeOutputBuffers()
  {
    // Save the output sample rate and audio channels
    unsigned outputSampleRate = GetStreamSampleRate(StreamTypes::Output);
    unsigned outputChannels = GetStreamChannels(StreamTypes::Output);

    // Start at the BufferSizeStartValue
    unsigned size = BufferSizeStartValue;
    // We need to get close to a value that accounts for the sample rate and channels
    float checkValue = (float)BufferSizeMultiplier * outputSampleRate * outputChannels;
    // Continue multiplying by 2 until we get close enough (buffer must be multiple of 2)
    while (size < checkValue)
      size *= 2;

    OutputBufferSizePerLatency[LatencyValues::LowLatency] = size;
    OutputBufferSizePerLatency[LatencyValues::HighLatency] = size * 4;

    SetUpOutputBuffers();
  }

  //************************************************************************************************
  void AudioInputOutput::SetUpOutputBuffers()
  {
    // If the buffer already exists, delete it
    if (MixedOutputBuffer)
      delete[] MixedOutputBuffer;

    // Create the buffer at the appropriate size
    MixedOutputBuffer = new float[OutputBufferSizePerLatency[OutputStreamLatency]];
    // Set up the OutputRingBuffer with the new buffer
    OutputRingBuffer.Initialize(sizeof(float), OutputBufferSizePerLatency[OutputStreamLatency], 
      MixedOutputBuffer);
  }

  //************************************************************************************************
  void AudioInputOutput::GetMixedOutputSamples(float* outputBuffer, const unsigned howManySamples)
  {
    // Save the number of samples available to read
    unsigned available = OutputRingBuffer.GetReadAvailable();

    // Make sure we don't try to read more samples than are available
    unsigned samples = howManySamples;
    if (samples > available)
      samples = available;

    // Copy the samples from the OutputRingBuffer
    OutputRingBuffer.Read((void*)outputBuffer, samples);

    // If there weren't enough available, set the rest to 0
    if (samples < howManySamples)
      memset(outputBuffer + samples, 0, howManySamples - samples);

    // Signal the semaphore for the mix thread
    MixThreadSemaphore.Increment();
  }

  //************************************************************************************************
  void AudioInputOutput::SaveInputSamples(const float* inputBuffer, unsigned howManySamples)
  {
    InputRingBuffer.Write(inputBuffer, howManySamples);
  }
  
}
