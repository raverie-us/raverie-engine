///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.h"
#include "portaudio.h"

namespace Audio
{
  //----------------------------------------------------------------------------- Audio Input Output

  //************************************************************************************************
  AudioInputOutput::AudioInputOutput() :
    MixedOutputBuffer(nullptr),
    OutputStreamLatency(LatencyValues::LowLatency)
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
  
  //------------------------------------------------------------- Audio Input Output using PortAudio

  //************************************************************************************************
  static int PACallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
  {
    return ((AudioIOPortAudio*)userData)->HandleCallback(inputBuffer, outputBuffer, framesPerBuffer);
  }

  //************************************************************************************************
  AudioIOPortAudio::AudioIOPortAudio() :
    ApiInfo(nullptr),
    PaOutputStream(nullptr),
    OutputSampleRate(0),
    PaInputStream(nullptr),
    InputSampleRate(0)
  {
    OutputParameters = new PaStreamParameters();
    InputParameters = new PaStreamParameters();

    // Null out all parameters
    memset(OutputParameters, 0, sizeof(PaStreamParameters));
    memset(InputParameters, 0, sizeof(PaStreamParameters));
  }

  //************************************************************************************************
  AudioIOPortAudio::~AudioIOPortAudio()
  {
    delete OutputParameters;
    delete InputParameters;
  }

  //************************************************************************************************
  StreamStatus::Enum AudioIOPortAudio::InitializeAPI()
  {
    OutputParameters->device = -1;
    InputParameters->device = -1;

    // Initialize PortAudio and check for error
    PaError result = Pa_Initialize();
    if (result != paNoError)
    {
      LogAudioIoError(Zero::String::Format("Unable to initialize PortAudio: %s", 
        Pa_GetErrorText(result)), &LastErrorMessage);
      return StreamStatus::ApiProblem;
    }
    ZPrint("PortAudio initialized\n");

    // Get number of devices
    int numDevices = Pa_GetDeviceCount();
    // Port Audio error condition
    if (numDevices < 0)
    {
      LastErrorMessage = Zero::String::Format(
        "Error getting audio devices: Pa_CountDevices returned 0x%x", numDevices);
      return StreamStatus::ApiProblem;
    }
    // No audio devices, can't do anything
    else if (numDevices == 0)
    {
      LastErrorMessage = "No audio output devices found";
      ZPrint("Audio initialization unsuccessful: no output devices\n");
      return StreamStatus::Uninitialized;
    }

    int numAPIs = Pa_GetHostApiCount();
    if (numAPIs == 0)
    {
      LastErrorMessage = "No audio APIs found";
      ZPrint("Audio initialization unsuccessful: no audio APIs found\n");
      return StreamStatus::ApiProblem;
    }

    PaHostApiIndex apiIndex = -1;
    // Check for WASAPI
    for (int i = 0; i < numAPIs; ++i)
    {
      const PaHostApiInfo *apiInfo = Pa_GetHostApiInfo(i);
      if (apiInfo->type == paWASAPI)
      {
        apiIndex = i;
        break;
      }
    }
    // No WASAPI, check for DirectSound
    if (apiIndex < 0)
    {
      for (int i = 0; i < numAPIs; ++i)
      {
        const PaHostApiInfo *apiInfo = Pa_GetHostApiInfo(i);
        if (apiInfo->type == paDirectSound)
        {
          apiIndex = i;
          break;
        }
      }
    }

    if (apiIndex < 0)
    {
      LastErrorMessage = "No supported audio APIs found.";
      ZPrint("Audio initialization unsuccessful: no supported API found\n");
      return StreamStatus::ApiProblem;
    }

    // Get API info
    ApiInfo = Pa_GetHostApiInfo(apiIndex);

    return StreamStatus::Initialized;
  }

  //************************************************************************************************
  StreamStatus::Enum AudioIOPortAudio::InitializeStream(StreamTypes::Enum whichStream)
  {
    if (whichStream == StreamTypes::Input)
      InitializeInput();
    else if (whichStream == StreamTypes::Output)
      InitializeOutput();

    return StreamInfoList[whichStream].Status;
  }

  //************************************************************************************************
  StreamStatus::Enum AudioIOPortAudio::StartStream(StreamTypes::Enum whichStream)
  {
    StreamInfo& info = StreamInfoList[whichStream];
    PaStreamParameters* parameters = OutputParameters;
    if (whichStream == StreamTypes::Input)
      parameters = InputParameters;

    if (parameters->device < 0)
    {
      LogAudioIoError("No audio device, can't start stream", &info.ErrorMessage);
      info.Status = StreamStatus::DeviceProblem;
      return StreamStatus::DeviceProblem;
    }

    PaError result;
    // Open a PortAudio stream
    if (whichStream == StreamTypes::Output)
    {
      result = Pa_OpenStream(
        &PaOutputStream,
        nullptr,                          // No input
        OutputParameters,
        OutputSampleRate,
        paFramesPerBufferUnspecified,     // Use variable optimal number of frames per callback
        paClipOff,                        // Won't check for clipping
        PACallback,
        this);
    }
    else
    {
      result = Pa_OpenStream(
        &PaInputStream,
        InputParameters,
        nullptr,            // no output
        InputSampleRate,
        0,                  // Use variable optimal number of frames per callback
        0,
        PACallback,
        this);
    }
    if (result != paNoError)
    {
      LogAudioIoError(Zero::String::Format("Error opening audio stream: %s",
        Pa_GetErrorText(result)), &info.ErrorMessage);
      info.Status = StreamStatus::DeviceProblem;
      return StreamStatus::DeviceProblem;
    }

    // If opened successfully, start the stream
    if (whichStream == StreamTypes::Output)
      result = Pa_StartStream(PaOutputStream);
    else
      result = Pa_StartStream(PaInputStream);
    if (result != paNoError)
    {
      LogAudioIoError(Zero::String::Format("Error starting audio stream : %s",
        Pa_GetErrorText(result)), &info.ErrorMessage);
      info.Status = StreamStatus::DeviceProblem;
      return StreamStatus::DeviceProblem;
    }

    if (whichStream == StreamTypes::Output)
      ZPrint("Audio output stream started\n");
    else
      ZPrint("Audio input stream started\n");
      
    info.Status = StreamStatus::Started;
    return StreamStatus::Started;

  }

  //************************************************************************************************
  StreamStatus::Enum AudioIOPortAudio::StopStream(StreamTypes::Enum whichStream)
  {
    StreamInfo& info = StreamInfoList[whichStream];

    if (whichStream == StreamTypes::Output)
    {
      if (!PaOutputStream)
        return StreamStatus::Uninitialized;

      // Close the stream
      PaError result = Pa_CloseStream(PaOutputStream);
      if (result != paNoError)
      {
        LogAudioIoError(Zero::String::Format("Error closing audio output stream: %s",
          Pa_GetErrorText(result)), &info.ErrorMessage);
        info.Status = StreamStatus::DeviceProblem;
        return StreamStatus::DeviceProblem;
      }

      PaOutputStream = nullptr;

      ZPrint("Audio output stream stopped\n");
      info.Status = StreamStatus::Stopped;
      return StreamStatus::Stopped;
    }
    else if (whichStream == StreamTypes::Input)
    {
      if (!PaInputStream)
        return StreamStatus::Uninitialized;

      // Close the stream
      PaError result = Pa_CloseStream(PaInputStream);
      if (result != paNoError)
      {
        LogAudioIoError(Zero::String::Format("Error closing audio input stream: %s",
          Pa_GetErrorText(result)), &info.ErrorMessage);
        info.Status = StreamStatus::DeviceProblem;
        return StreamStatus::DeviceProblem;
      }

      PaInputStream = nullptr;

      ZPrint("Audio input stream stopped\n");
      info.Status = StreamStatus::Stopped;
      return StreamStatus::Stopped;
    }

    return StreamStatus::Uninitialized;
  }

  //************************************************************************************************
  Audio::StreamStatus::Enum AudioIOPortAudio::ShutDownStream(StreamTypes::Enum whichStream)
  {
    StopStream(whichStream);

    return StreamStatus::Stopped;
  }

  //************************************************************************************************
  void AudioIOPortAudio::ShutDownAPI()
  {
    // Terminate PortAudio and check for error
    PaError result = Pa_Terminate();
    if (result != paNoError)
    {
      LastErrorMessage = Zero::String::Format("Error terminating PortAudio: %s", Pa_GetErrorText(result));
      return;
    }

    ZPrint("Terminated PortAudio\n");
  }

  //************************************************************************************************
  unsigned AudioIOPortAudio::GetStreamChannels(StreamTypes::Enum whichStream)
  {
    if (whichStream == StreamTypes::Output)
      return OutputParameters->channelCount;
    else
      return InputParameters->channelCount;
  }

  //************************************************************************************************
  unsigned AudioIOPortAudio::GetStreamSampleRate(StreamTypes::Enum whichStream)
  {
    if (whichStream == StreamTypes::Output)
      return OutputSampleRate;
    else
      return InputSampleRate;
  }

  //************************************************************************************************
  int AudioIOPortAudio::HandleCallback(const void *inputBuffer, void *outputBuffer, 
     unsigned long framesPerBuffer)
  {
    if (outputBuffer)
    {
      GetMixedOutputSamples((float*)outputBuffer, framesPerBuffer * OutputParameters->channelCount);

      return paContinue;
    }
    else if (inputBuffer)
    {
      SaveInputSamples((const float*)inputBuffer, framesPerBuffer * InputParameters->channelCount);

      return paContinue;
    }

    return paAbort;
  }

  //************************************************************************************************
  void AudioIOPortAudio::InitializeInput()
  {
    // Make sure API is already initialized
    if (!ApiInfo)
    {
      StreamInfoList[StreamTypes::Input].ErrorMessage =
        "Audio API was not successfully initialized, unable to initialize input";
      StreamInfoList[StreamTypes::Input].Status = StreamStatus::Uninitialized;
      return;
    }

    // Check if there is an input device
    InputParameters->device = ApiInfo->defaultInputDevice;
    if (InputParameters->device >= 0)
    {
      InputParameters->sampleFormat = OutputParameters->sampleFormat;
      const PaDeviceInfo* inputDeviceInfo = Pa_GetDeviceInfo(InputParameters->device);
      InputParameters->suggestedLatency = inputDeviceInfo->defaultLowInputLatency;
      InputParameters->channelCount = inputDeviceInfo->maxInputChannels;
      InputSampleRate = (unsigned)inputDeviceInfo->defaultSampleRate;

      PaError result = Pa_IsFormatSupported(InputParameters, nullptr, (double)InputSampleRate);
      if (result != paFormatIsSupported)
      {
        StreamInfoList[StreamTypes::Input].ErrorMessage = "Audio input format is not supported";
        ZPrint("Audio input format is not supported\n");
        InputParameters->device = -1;
        StreamInfoList[StreamTypes::Input].Status = StreamStatus::DeviceProblem;
      }
      else
      {
        ZPrint("Successfully initialized audio input\n");
        StreamInfoList[StreamTypes::Input].Status = StreamStatus::Initialized;
      }
    }
    else
    {
      StreamInfoList[StreamTypes::Input].ErrorMessage = "No default audio input device.";
      StreamInfoList[StreamTypes::Input].Status = StreamStatus::DeviceProblem;
    }
  }

  //************************************************************************************************
  void AudioIOPortAudio::InitializeOutput()
  {
    // Make sure API is already initialized
    if (!ApiInfo)
    {
      StreamStatus::Enum status = InitializeAPI();

      if (status != StreamStatus::Initialized)
      {
        StreamInfoList[StreamTypes::Output].ErrorMessage = 
          "Audio API was not successfully initialized, unable to initialize output";
        StreamInfoList[StreamTypes::Output].Status = StreamStatus::Uninitialized;
        return;
      }
    }

    // Set up output parameters
    OutputParameters->device = ApiInfo->defaultOutputDevice;    // Default output device
    if (OutputParameters->device == paNoDevice)
    {
      StreamInfoList[StreamTypes::Output].ErrorMessage = "No default audio output device found.";
      ZPrint("Audio initialization unsuccessful: no default audio device\n");
      StreamInfoList[StreamTypes::Output].Status = StreamStatus::DeviceProblem;
      return;
    }
    OutputParameters->sampleFormat = paFloat32;    // 32 bit floating point output

    // Get the device info
    const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo(OutputParameters->device);

    OutputParameters->suggestedLatency = deviceInfo->defaultLowOutputLatency;
    OutputSampleRate = (unsigned)deviceInfo->defaultSampleRate;

    // Set the variables that depend on the number of output channels
    OutputParameters->channelCount = deviceInfo->maxOutputChannels;

    // Check device settings
    PaError result = Pa_IsFormatSupported(nullptr, OutputParameters, (double)OutputSampleRate);
    if (result != paFormatIsSupported)
    {
      // Parameters were not supported
      LastErrorMessage = "Audio settings not supported by default output device. ";
      ZPrint("Settings not supported by default output device\n Audio initialization unsuccessful\n");
      StreamInfoList[StreamTypes::Output].Status = StreamStatus::DeviceProblem;
      return;
    }

    ZPrint("Successfully initialized audio output\n");
    StreamInfoList[StreamTypes::Output].Status = StreamStatus::Initialized;

    InitializeOutputBuffers();
  }

}
