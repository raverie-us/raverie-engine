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
    OutputStreamLatency(LowLatency),
    MixedOutputBuffer(nullptr)
  {
    memset(OutputBufferSizePerLatency, 0, sizeof(unsigned) * NumLatencyValues);

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
  void AudioInputOutput::SetOutputLatency(LatencyValues newLatency)
  {
    // If the setting is the same, don't do anything
    if (newLatency == OutputStreamLatency)
      return;

    // Set the latency variable
    OutputStreamLatency = newLatency;
    // Shut down the output stream so we can change the buffer size
    ShutDownStream(OutputStream);
    // Set up the output buffer with the current latency setting
    SetUpOutputBuffers();
    // Restart the audio output
    InitializeStream(OutputStream);
    StartStream(OutputStream);
  }

  //************************************************************************************************
  const Audio::StreamInfo& AudioInputOutput::GetStreamInfo(StreamType whichStream)
  {
    return StreamInfoList[whichStream];
  }

  //************************************************************************************************
  void AudioInputOutput::InitializeOutputBuffers()
  {
    // Save the output sample rate and audio channels
    unsigned outputSampleRate = GetStreamSampleRate(OutputStream);
    unsigned outputChannels = GetStreamChannels(OutputStream);

    // Start at the BufferSizeStartValue
    unsigned size = BufferSizeStartValue;
    // We need to get close to a value that accounts for the sample rate and channels
    float checkValue = (float)BufferSizeMultiplier * outputSampleRate * outputChannels;
    // Continue multiplying by 2 until we get close enough (buffer must be multiple of 2)
    while (!IsWithinLimit((float)size, checkValue, 1000.0f))
      size *= 2;

    OutputBufferSizePerLatency[LowLatency] = size;
    OutputBufferSizePerLatency[HighLatency] = size * 4;

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
    PaOutputStream(nullptr),
    OutputSampleRate(0),
    PaInputStream(nullptr),
    InputSampleRate(0),
    ApiInfo(nullptr)
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
  StreamStatus AudioIOPortAudio::InitializeAPI()
  {
    OutputParameters->device = -1;
    InputParameters->device = -1;

    // Initialize PortAudio and check for error
    PaError result = Pa_Initialize();
    if (result != paNoError)
    {
      LogAudioIoError(Zero::String::Format("Unable to initialize PortAudio: %s", 
        Pa_GetErrorText(result)), &LastErrorMessage);
      return ApiProblem;
    }
    ZPrint("PortAudio initialized\n");

    // Get number of devices
    int numDevices = Pa_GetDeviceCount();
    // Port Audio error condition
    if (numDevices < 0)
    {
      LastErrorMessage = Zero::String::Format(
        "Error getting audio devices: Pa_CountDevices returned 0x%x", numDevices);
      return ApiProblem;
    }
    // No audio devices, can't do anything
    else if (numDevices == 0)
    {
      LastErrorMessage = "No audio output devices found";
      ZPrint("Audio initialization unsuccessful: no output devices\n");
      return Uninitialized;
    }

    int numAPIs = Pa_GetHostApiCount();
    if (numAPIs == 0)
    {
      LastErrorMessage = "No audio APIs found";
      ZPrint("Audio initialization unsuccessful: no audio APIs found\n");
      return ApiProblem;
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
      return ApiProblem;
    }

    // Get API info
    ApiInfo = Pa_GetHostApiInfo(apiIndex);

    return Initialized;
  }

  //************************************************************************************************
  StreamStatus AudioIOPortAudio::InitializeStream(StreamType whichStream)
  {
    if (whichStream == InputStream)
      InitializeInput();
    else if (whichStream == OutputStream)
      InitializeOutput();

    return StreamInfoList[whichStream].Status;
  }

  //************************************************************************************************
  StreamStatus AudioIOPortAudio::StartStream(StreamType whichStream)
  {
    StreamInfo& info = StreamInfoList[whichStream];
    PaStreamParameters* parameters = OutputParameters;
    if (whichStream == InputStream)
      parameters = InputParameters;

    if (parameters->device < 0)
    {
      LogAudioIoError("No audio device, can't start stream", &info.ErrorMessage);
      info.Status = DeviceProblem;
      return DeviceProblem;
    }

    PaError result;
    // Open a PortAudio stream
    if (whichStream == OutputStream)
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
      info.Status = DeviceProblem;
      return DeviceProblem;
    }

    // If opened successfully, start the stream
    if (whichStream == OutputStream)
      result = Pa_StartStream(PaOutputStream);
    else
      result = Pa_StartStream(PaInputStream);
    if (result != paNoError)
    {
      LogAudioIoError(Zero::String::Format("Error starting audio stream : %s",
        Pa_GetErrorText(result)), &info.ErrorMessage);
      info.Status = DeviceProblem;
      return DeviceProblem;
    }

    if (whichStream == OutputStream)
      ZPrint("Audio output stream started\n");
    else
      ZPrint("Audio input stream started\n");
      
    info.Status = Started;
    return Started;

  }

  //************************************************************************************************
  StreamStatus AudioIOPortAudio::StopStream(StreamType whichStream)
  {
    StreamInfo& info = StreamInfoList[whichStream];

    if (whichStream == OutputStream)
    {
      if (!PaOutputStream)
        return Uninitialized;

      // Close the stream
      PaError result = Pa_CloseStream(PaOutputStream);
      if (result != paNoError)
      {
        LogAudioIoError(Zero::String::Format("Error closing audio output stream: %s",
          Pa_GetErrorText(result)), &info.ErrorMessage);
        info.Status = DeviceProblem;
        return DeviceProblem;
      }

      PaOutputStream = nullptr;

      ZPrint("Audio output stream stopped\n");
      info.Status = Stopped;
      return Stopped;
    }
    else if (whichStream == InputStream)
    {
      if (!PaInputStream)
        return Uninitialized;

      // Close the stream
      PaError result = Pa_CloseStream(PaInputStream);
      if (result != paNoError)
      {
        LogAudioIoError(Zero::String::Format("Error closing audio input stream: %s",
          Pa_GetErrorText(result)), &info.ErrorMessage);
        info.Status = DeviceProblem;
        return DeviceProblem;
      }

      PaInputStream = nullptr;

      ZPrint("Audio input stream stopped\n");
      info.Status = Stopped;
      return Stopped;
    }

    return Uninitialized;
  }

  //************************************************************************************************
  Audio::StreamStatus AudioIOPortAudio::ShutDownStream(StreamType whichStream)
  {
    StopStream(whichStream);

    return Stopped;
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
  unsigned AudioIOPortAudio::GetStreamChannels(StreamType whichStream)
  {
    if (whichStream == OutputStream)
      return OutputParameters->channelCount;
    else
      return InputParameters->channelCount;
  }

  //************************************************************************************************
  unsigned AudioIOPortAudio::GetStreamSampleRate(StreamType whichStream)
  {
    if (whichStream == OutputStream)
      return OutputSampleRate;
    else
      return InputSampleRate;
  }

  //************************************************************************************************
  bool AudioIOPortAudio::IsStreamStarted(StreamType whichStream)
  {
    if (whichStream == OutputStream)
      return PaOutputStream != nullptr;
    else
      return PaInputStream != nullptr;
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
      StreamInfoList[InputStream].ErrorMessage = 
        "Audio API was not successfully initialized, unable to initialize input";
      StreamInfoList[InputStream].Status = Uninitialized;
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
        StreamInfoList[InputStream].ErrorMessage = "Audio input format is not supported";
        ZPrint("Audio input format is not supported\n");
        InputParameters->device = -1;
        StreamInfoList[InputStream].Status = DeviceProblem;
      }
      else
      {
        ZPrint("Successfully initialized audio input\n");
        StreamInfoList[InputStream].Status = Initialized;
      }
    }
    else
    {
      StreamInfoList[InputStream].ErrorMessage = "No default audio input device.";
      StreamInfoList[InputStream].Status = DeviceProblem;
    }
  }

  //************************************************************************************************
  void AudioIOPortAudio::InitializeOutput()
  {
    // Make sure API is already initialized
    if (!ApiInfo)
    {
      StreamStatus status = InitializeAPI();

      if (status != Initialized)
      {
        StreamInfoList[OutputStream].ErrorMessage = 
          "Audio API was not successfully initialized, unable to initialize output";
        StreamInfoList[OutputStream].Status = Uninitialized;
        return;
      }
    }

    // Set up output parameters
    OutputParameters->device = ApiInfo->defaultOutputDevice;    // Default output device
    if (OutputParameters->device == paNoDevice)
    {
      StreamInfoList[OutputStream].ErrorMessage = "No default audio output device found.";
      ZPrint("Audio initialization unsuccessful: no default audio device\n");
      StreamInfoList[OutputStream].Status = DeviceProblem;
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
      StreamInfoList[OutputStream].Status = DeviceProblem;
      return;
    }

    ZPrint("Successfully initialized audio output\n");
    StreamInfoList[OutputStream].Status = Initialized;

    InitializeOutputBuffers();
  }

}
