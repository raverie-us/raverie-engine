///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.h"

namespace Audio
{
  //----------------------------------------------------------------------------- Audio IO Interface

  //************************************************************************************************
  AudioIOInterface::AudioIOInterface() :
    mOutputStreamLatency(LatencyValues::LowLatency),
    MixedOutputBuffer(nullptr)
  {
    memset(OutputBufferSizePerLatency, 0, sizeof(unsigned) * LatencyValues::Count);

    InputRingBuffer.Initialize(sizeof(float), InputBufferSize, InputBuffer);
  }

  //************************************************************************************************
  AudioIOInterface::~AudioIOInterface()
  {
    if (MixedOutputBuffer)
      delete[] MixedOutputBuffer;
  }

  //************************************************************************************************
  bool AudioIOInterface::Initialize()
  {
    // Return false if the API is not initialized successfully
    if (AudioIO.InitializeAPI(&mApiErrorMessage) != StreamStatus::Initialized)
      return false;

    // Initialize output stream
    StreamInfo& outputInfo = StreamInfoList[StreamTypes::Output];
    outputInfo.mStatus = AudioIO.InitializeStream(StreamTypes::Output, &outputInfo.mErrorMessage);
    outputInfo.mChannels = AudioIO.GetStreamChannels(StreamTypes::Output);
    outputInfo.mSampleRate = AudioIO.GetStreamSampleRate(StreamTypes::Output);

    // Initialize and set up the buffer used for mixed output
    InitializeOutputBuffers();

    // Initialize input stream
    StreamInfo& inputInfo = StreamInfoList[StreamTypes::Input];
    inputInfo.mStatus = AudioIO.InitializeStream(StreamTypes::Input, &inputInfo.mErrorMessage);
    inputInfo.mChannels = AudioIO.GetStreamChannels(StreamTypes::Input);
    inputInfo.mSampleRate = AudioIO.GetStreamSampleRate(StreamTypes::Input);

    return true;
  }

  //************************************************************************************************
  void AudioIOInterface::ShutDown()
  {
    AudioIO.ShutDownAPI();
  }

  //************************************************************************************************
  bool AudioIOInterface::StartStreams(bool startOutput, bool startInput)
  {
    if (!startOutput && !startInput)
      return false;

    bool returnValue = true;

    // Start output stream if requested
    if (startOutput)
    {
      // Start the stream, passing in the IOCallback function and this object as the data
      StreamInfoList[StreamTypes::Output].mStatus = AudioIO.StartStream(StreamTypes::Output, 
        &StreamInfoList[StreamTypes::Output].mErrorMessage, IOCallback, this);

      // Check if it was started successfully
      if (StreamInfoList[StreamTypes::Output].mStatus != StreamStatus::Started)
        returnValue = false;
    }

    // Start input stream if requested
    if (startInput)
    {
      // Start the stream, passing in the IOCallback function and this object as the data
      StreamInfoList[StreamTypes::Input].mStatus = AudioIO.StartStream(StreamTypes::Input, 
        &StreamInfoList[StreamTypes::Input].mErrorMessage, IOCallback, this);

      // Check if it was started successfully
      if (StreamInfoList[StreamTypes::Input].mStatus != StreamStatus::Started)
        returnValue = false;
    }

    return returnValue;
  }

  //************************************************************************************************
  bool AudioIOInterface::StopStreams(bool stopOutput, bool stopInput)
  {
    bool returnValue = true;

    // Stop output stream if requested
    if (stopOutput)
    {
      StreamInfoList[StreamTypes::Output].mStatus = AudioIO.StopStream(StreamTypes::Output,
        &StreamInfoList[StreamTypes::Output].mErrorMessage);

      // Check if it was stopped successfully
      if (StreamInfoList[StreamTypes::Output].mStatus != StreamStatus::Stopped)
        returnValue = false;
    }

    // Stop input stream if requested
    if (stopInput)
    {
      StreamInfoList[StreamTypes::Input].mStatus = AudioIO.StopStream(StreamTypes::Input,
        &StreamInfoList[StreamTypes::Input].mErrorMessage);

      // Check if it was stopped successfully
      if (StreamInfoList[StreamTypes::Input].mStatus != StreamStatus::Stopped)
        returnValue = false;
    }

    return returnValue;
  }

  //************************************************************************************************
  Audio::StreamStatus::Enum AudioIOInterface::GetStreamStatus(StreamTypes::Enum whichStream)
  {
    return StreamInfoList[whichStream].mStatus;
  }

  //************************************************************************************************
  const Zero::String& AudioIOInterface::GetStreamErrorMessage(StreamTypes::Enum whichStream)
  {
    return StreamInfoList[whichStream].mErrorMessage;
  }

  //************************************************************************************************
  const Zero::String& AudioIOInterface::GetSystemErrorMessage()
  {
    return mApiErrorMessage;
  }

  //************************************************************************************************
  unsigned AudioIOInterface::GetStreamChannels(StreamTypes::Enum whichStream)
  {
    return StreamInfoList[whichStream].mChannels;
  }

  //************************************************************************************************
  unsigned AudioIOInterface::GetStreamSampleRate(StreamTypes::Enum whichStream)
  {
    return StreamInfoList[whichStream].mSampleRate;
  }

  //************************************************************************************************
  void AudioIOInterface::WaitUntilOutputNeededThreaded()
  {
    MixThreadSemaphore.WaitAndDecrement();
  }

  //************************************************************************************************
  void AudioIOInterface::GetInputDataThreaded(Zero::Array<float>& buffer, unsigned howManySamples)
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
  void AudioIOInterface::SetOutputLatency(LatencyValues::Enum newLatency)
  {
    // If the setting is the same, don't do anything
    if (newLatency == mOutputStreamLatency)
      return;

    // Set the latency variable
    mOutputStreamLatency = newLatency;
    // Shut down the output stream so we can change the buffer size
    AudioIO.StopStream(StreamTypes::Output, nullptr);
    // Set up the output buffer with the current latency setting
    SetUpOutputBuffers();
    // Restart the audio output
    AudioIO.InitializeStream(StreamTypes::Output, nullptr);
    StreamInfoList[StreamTypes::Output].mStatus = AudioIO.StartStream(StreamTypes::Output,
      &StreamInfoList[StreamTypes::Output].mErrorMessage, IOCallback, this);
  }

  //************************************************************************************************
  void AudioIOInterface::InitializeOutputBuffers()
  {
    // Save the output sample rate and audio channels
    unsigned outputSampleRate = AudioIO.GetStreamSampleRate(StreamTypes::Output);
    unsigned outputChannels = AudioIO.GetStreamChannels(StreamTypes::Output);

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
  void AudioIOInterface::SetUpOutputBuffers()
  {
    // If the buffer already exists, delete it
    if (MixedOutputBuffer)
      delete[] MixedOutputBuffer;

    // Create the buffer at the appropriate size
    MixedOutputBuffer = new float[OutputBufferSizePerLatency[mOutputStreamLatency]];
    // Set up the OutputRingBuffer with the new buffer
    OutputRingBuffer.Initialize(sizeof(float), OutputBufferSizePerLatency[mOutputStreamLatency], 
      MixedOutputBuffer);
  }

  //************************************************************************************************
  void AudioIOInterface::GetMixedOutputSamples(float* outputBuffer, const unsigned frames)
  {
    // Note: remember this function is called asynchronously from the audio device thread

    // Save the number of samples available to read
    unsigned available = OutputRingBuffer.GetReadAvailable();

    // Make sure we don't try to read more samples than are available
    unsigned samplesNeeded = frames * StreamInfoList[StreamTypes::Output].mChannels;
    unsigned samples = samplesNeeded;
    if (samples > available)
      samples = available;

    // Copy the samples from the OutputRingBuffer
    OutputRingBuffer.Read((void*)outputBuffer, samples);

    // If there weren't enough available, set the rest to 0
    if (samples < samplesNeeded)
      memset(outputBuffer + samples, 0, samplesNeeded - samples);

    // Signal the semaphore for the mix thread
    MixThreadSemaphore.Increment();
  }

  //************************************************************************************************
  void AudioIOInterface::SaveInputSamples(const float* inputBuffer, unsigned frames)
  {
    // Note: remember this function is called asynchronously from the audio device thread

    InputRingBuffer.Write(inputBuffer, frames * StreamInfoList[StreamTypes::Input].mChannels);
  }

  //------------------------------------------------------------------------------------------------

  //************************************************************************************************
  void IOCallback(float* outputBuffer, float* inputBuffer, unsigned framesPerBuffer, void* data)
  {
    if (outputBuffer)
      ((AudioIOInterface*)data)->GetMixedOutputSamples(outputBuffer, framesPerBuffer);
    if (inputBuffer)
      ((AudioIOInterface*)data)->SaveInputSamples(inputBuffer, framesPerBuffer);
  }


}
