// MIT Licensed (see LICENSE.md).

#include "Precompiled.hpp"
#include "Foundation/Platform/PlatformCommunication.hpp"

namespace Raverie
{

const float cAudioBufferSizeMultiplier = 0.05;

AudioIOInterface::AudioIOInterface() : MixedOutputBuffer(nullptr), InputBuffer(nullptr), mOutputStreamLatency(AudioLatency::Low)
{
  memset(OutputBufferSizePerLatency, 0, sizeof(unsigned) * AudioLatency::Size);
}

AudioIOInterface::~AudioIOInterface()
{
  if (MixedOutputBuffer)
    delete[] MixedOutputBuffer;
  if (InputBuffer)
    delete[] InputBuffer;
}

bool AudioIOInterface::Initialize(bool initOutput, bool initInput)
{
  bool returnValue = true;

  // Initialize output stream
  if (initOutput && StreamInfoList[StreamTypes::Output].mStatus != StreamStatus::Initialized)
  {
    StreamInfo& outputInfo = StreamInfoList[StreamTypes::Output];
    outputInfo.mChannels = cAudioChannels;
    outputInfo.mSampleRate = cAudioSampleRate;

    InitializeOutputBuffers();
  }

  // Initialize input stream
  if (initInput && StreamInfoList[StreamTypes::Input].mStatus != StreamStatus::Initialized)
  {
    StreamInfo& inputInfo = StreamInfoList[StreamTypes::Input];
    inputInfo.mChannels = cAudioChannels;
    inputInfo.mSampleRate = cAudioSampleRate;

    InitializeInputBuffers();
  }

  return returnValue;
}

bool AudioIOInterface::StartStreams(bool startOutput, bool startInput)
{
  if (!startOutput && !startInput)
    return false;

  // Start output stream if requested
  if (startOutput)
  {
    StreamInfoList[StreamTypes::Output].mStatus = StreamStatus::Started;
  }

  // Start input stream if requested
  if (startInput)
  {
    StreamInfoList[StreamTypes::Input].mStatus = StreamStatus::Started;
  }

  return true;
}

bool AudioIOInterface::StopStreams(bool stopOutput, bool stopInput)
{
  // Stop output stream if requested
  if (stopOutput)
  {
    StreamInfoList[StreamTypes::Output].mStatus = StreamStatus::Stopped;
  }

  // Stop input stream if requested
  if (stopInput)
  {
    StreamInfoList[StreamTypes::Input].mStatus = StreamStatus::Stopped;
  }

  return true;
}

StreamStatus::Enum AudioIOInterface::GetStreamStatus(StreamTypes::Enum whichStream)
{
  return StreamInfoList[whichStream].mStatus;
}

const String& AudioIOInterface::GetStreamErrorMessage(StreamTypes::Enum whichStream)
{
  return StreamInfoList[whichStream].mErrorMessage;
}

const String& AudioIOInterface::GetSystemErrorMessage()
{
  return mApiErrorMessage;
}

unsigned AudioIOInterface::GetStreamChannels(StreamTypes::Enum whichStream)
{
  return StreamInfoList[whichStream].mChannels;
}

unsigned AudioIOInterface::GetStreamSampleRate(StreamTypes::Enum whichStream)
{
  return StreamInfoList[whichStream].mSampleRate;
}

void AudioIOInterface::WaitUntilOutputNeededThreaded()
{
  MixThreadSemaphore.WaitAndDecrement();
}

void AudioIOInterface::GetInputDataThreaded(Array<float>& buffer, unsigned howManySamples)
{
  unsigned samplesAvailable = InputRingBuffer.GetReadAvailable();

  if (samplesAvailable < howManySamples)
    howManySamples = samplesAvailable;

  buffer.Resize(howManySamples);

  unsigned samplesRead = InputRingBuffer.Read(buffer.Data(), howManySamples);

  if (samplesRead != howManySamples)
    buffer.Resize(samplesRead);
}

void AudioIOInterface::SetOutputLatencyThreaded(AudioLatency::Enum newLatency)
{
  // If the setting is the same, don't do anything
  if (newLatency == mOutputStreamLatency)
    return;

  // Set the latency variable
  mOutputStreamLatency = newLatency;
  // Set up the output buffer with the current latency setting
  InitializeRingBuffer(OutputRingBuffer, MixedOutputBuffer, OutputBufferSizePerLatency[mOutputStreamLatency]);
}

void AudioIOInterface::InitializeOutputBuffers()
{
  unsigned size = GetBufferSize(cAudioSampleRate, cAudioChannels);

  OutputBufferSizePerLatency[AudioLatency::Low] = size;
  OutputBufferSizePerLatency[AudioLatency::High] = size * 4;

  InitializeRingBuffer(OutputRingBuffer, MixedOutputBuffer, OutputBufferSizePerLatency[mOutputStreamLatency]);
}

void AudioIOInterface::InitializeInputBuffers()
{
  unsigned size = GetBufferSize(cAudioSampleRate, cAudioChannels);

  InitializeRingBuffer(InputRingBuffer, InputBuffer, size * 2);
}

unsigned AudioIOInterface::GetBufferSize(unsigned sampleRate, unsigned channels)
{
  // Start at the BufferSizeStartValue
  unsigned size = BufferSizeStartValue;
  // We need to get close to a value that accounts for the sample rate and
  // channels
  float checkValue = cAudioBufferSizeMultiplier * sampleRate * channels;
  // Continue multiplying by 2 until we get close enough (buffer must be
  // multiple of 2)
  while (size < checkValue)
    size *= 2;

  return size;
}

void AudioIOInterface::InitializeRingBuffer(RingBuffer& ringBuffer, float* buffer, unsigned size)
{
  // If the buffer already exists, delete it
  if (buffer)
    delete[] buffer;

  // Create the buffer
  buffer = new float[size];
  // Initialize the RingBuffer with the new buffer
  ringBuffer.Initialize(sizeof(float), size, buffer);
}

void AudioIOInterface::GetMixedOutputSamples(float* outputBuffer, const unsigned frames)
{
  // Note: remember this function is called asynchronously from the audio device
  // thread

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
  if (samples < samplesNeeded) {
    memset(outputBuffer + samples, 0, (samplesNeeded - samples) * sizeof(float));
    ZPrint("Missed samples (engine) %d\n", samplesNeeded - samples);
  }

  // Signal the semaphore for the mix thread
  MixThreadSemaphore.Increment();
}

void AudioIOInterface::SaveInputSamples(const float* inputBuffer, unsigned frames)
{
  // Note: remember this function is called asynchronously from the audio device
  // thread

  InputRingBuffer.Write(inputBuffer, frames * StreamInfoList[StreamTypes::Input].mChannels);
}

const float* RaverieExportNamed(ExportAudioOutput)(size_t framesRequested) {
  static Array<float> sTemporaryBuffer;
  size_t samplesRequested = framesRequested * cAudioChannels;
  if (sTemporaryBuffer.Size() < samplesRequested) {
    sTemporaryBuffer.Resize(samplesRequested);
  }
  Z::gEngine->has(SoundSystem)->Mixer.AudioIO.GetMixedOutputSamples(sTemporaryBuffer.Data(), framesRequested);
  return sTemporaryBuffer.Data();
}

void RaverieExportNamed(ExportAudioInput)(const float* samplesPerChannel, size_t frames) {
  Z::gEngine->has(SoundSystem)->Mixer.AudioIO.SaveInputSamples(samplesPerChannel, frames);
}

} // namespace Raverie
