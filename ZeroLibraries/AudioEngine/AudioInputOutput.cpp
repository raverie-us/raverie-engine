///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.h"

namespace Audio
{
  //************************************************************************************************
  static int PACallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
  {
    if (outputBuffer)
    {
      // Cast data passed through stream to our structure
      CallbackDataObject *data = (CallbackDataObject*)userData;

      float* buffer = data->GetReadBuffer();

      // Determine number of samples to copy from read buffer to output buffer
      unsigned size = framesPerBuffer * data->Channels;
      // Make sure we don't go past the end of the buffer
      if (size > data->BufferSize - data->Index)
        size = data->BufferSize - data->Index;
      // Copy data into output buffer
      memcpy(outputBuffer, buffer + data->Index, size * sizeof(float));
      // Advance the buffer position index
      data->Index += size;

      // If we reached the end of the buffer, increment buffer index and signal the mix thread
      if (data->Index >= data->BufferSize)
      {
        data->ReadBufferThreaded = (data->ReadBufferThreaded + 1) % AudioInputOutput::NumOutputBuffers;

        data->Index = 0;
        data->Counter.Increment();
      }

      return paContinue;
    }
    //else if (inputBuffer)
    //{
    //  PaUtil_WriteRingBuffer(&gAudioSystem->AudioIO.InputRingBuffer, inputBuffer, 
    //    framesPerBuffer * gAudioSystem->AudioIO.InputParameters.channelCount);

    //  return paContinue;
    //}

    return paAbort;
  }

  //--------------------------------------------------------------------------- Callback Data Object

  //************************************************************************************************
  float* CallbackDataObject::GetReadBuffer()
  {
    return gAudioSystem->AudioIO.OutputBuffersThreaded[ReadBufferThreaded];
  }

  //----------------------------------------------------------------------------- Audio Input Output

  //************************************************************************************************
  AudioInputOutput::AudioInputOutput() :
    Stream(nullptr),
    InputStream(nullptr),
    WriteBufferThreaded(0),
    Initialized(false),
    OutputChannelsThreaded(0),
    OutputBufferSizeThreaded(0),
    LowLatency(true)
  {
    // Null out all parameters
    memset(&OutputParameters, 0, sizeof(PaStreamParameters));
    memset(&InputParameters, 0, sizeof(PaStreamParameters));

    // Null first buffer so we can check if they've been created yet
    OutputBuffersThreaded[0] = nullptr;
  }

  //************************************************************************************************
  AudioInputOutput::~AudioInputOutput()
  {
    // Delete output buffers
    if (OutputBuffersThreaded[0])
    {
      for (unsigned i = 0; i < NumOutputBuffers; ++i)
        delete OutputBuffersThreaded[i];
    }
  }

  //************************************************************************************************
  float* AudioInputOutput::GetOutputBuffer()
  {
    return OutputBuffersThreaded[WriteBufferThreaded];
  }

  //************************************************************************************************
  void AudioInputOutput::FinishedMixingBuffer()
  {
    WriteBufferThreaded = (WriteBufferThreaded + 1) % NumOutputBuffers;
  }

  //************************************************************************************************
  void AudioInputOutput::WaitUntilOutputNeeded()
  {
    CallbackDataThreaded.Counter.WaitAndDecrement();
  }

  //************************************************************************************************
  void AudioInputOutput::Initialize(Zero::Status& status)
  {
    Initialized = true;

    // Initialize PortAudio and check for error
    PaError result = Pa_Initialize();
    if (result != paNoError)
    {
      ZPrint("Failed to initialize PortAudio: %s\n", Pa_GetErrorText(result));
      status.SetFailed(Zero::String::Format("PortAudio error: %s", Pa_GetErrorText(result)));
      return;
    }
    ZPrint("PortAudio initialized\n");

    // Get number of devices
    int numDevices = Pa_GetDeviceCount();
    // Port Audio error condition
    if (numDevices < 0)
    {
      ZPrint("Error getting audio devices: Pa_CountDevices returned 0x%x\n Audio initialization failed\n", 
        numDevices);
      status.SetFailed(Zero::String::Format("PortAudio error: Pa_CountDevices returned 0x%x", 
        numDevices));
      return;
    }
    // No audio devices, can't do anything
    else if (numDevices == 0)
    {
      ZPrint("No audio output devices found\n Audio initialization failed\n");
      status.SetFailed("No audio output devices found");
      return;
    }

    int numAPIs = Pa_GetHostApiCount();
    if (numAPIs == 0)
    {
      ZPrint("No audio APIs found\n Audio initialization failed\n");
      status.SetFailed("No audio APIs found.");
      return;
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
      ZPrint("No supported audio APIs found\n Audio initialization failed\n");
      status.SetFailed("No supported audio APIs found.");
      return;
    }
    
    // Get API info
    const PaHostApiInfo *apiInfo = Pa_GetHostApiInfo(apiIndex);

    // Set up output parameters
    OutputParameters.device = apiInfo->defaultOutputDevice;    // Default output device
    if (OutputParameters.device == paNoDevice)
    {
      ZPrint("No default audio output device found\n Audio initialization failed\n");
      status.SetFailed("No default audio output device found.");
      return;
    }
    OutputParameters.sampleFormat = paFloat32;    // 32 bit floating point output

    // Get the device info
    const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo(OutputParameters.device);

    OutputParameters.suggestedLatency = deviceInfo->defaultLowOutputLatency;
    gAudioSystem->SystemSampleRate = (unsigned)deviceInfo->defaultSampleRate;

    // Set the variables that depend on the number of output channels
    OutputParameters.channelCount = deviceInfo->maxOutputChannels;
    OutputChannelsThreaded = deviceInfo->maxOutputChannels;
    CallbackDataThreaded.Channels = OutputChannelsThreaded;

    // Create the string with data on the audio setup
    Zero::StringBuilder messageString;
    messageString << apiInfo->name << " API, " << deviceInfo->name << ", ";
    messageString << OutputChannelsThreaded << " output channels, ";
    messageString << gAudioSystem->SystemChannelsThreaded << " mix channels, ";
    messageString << OutputParameters.suggestedLatency << " latency, ";
    messageString << gAudioSystem->SystemSampleRate << " sample rate";

    ZPrint("API             : %s\n", deviceInfo->name);
    ZPrint("Output channels : %d\n", OutputChannelsThreaded);
    ZPrint("Mix channels    : %d\n", gAudioSystem->SystemChannelsThreaded);
    ZPrint("Latency         : %f\n", OutputParameters.suggestedLatency);
    ZPrint("Sample rate     : %d\n", gAudioSystem->SystemSampleRate);

    // Check device settings
    result = Pa_IsFormatSupported(NULL, &OutputParameters, (double)gAudioSystem->SystemSampleRate);
    if (result != paFormatIsSupported)
    {
      // Parameters were not supported - set error string and return
      Zero::StringBuilder errorString;
      errorString << "Audio settings not supported by default output device. ";
      errorString << messageString.ToString();
      status.SetFailed(messageString.ToString());
      ZPrint("Settings not supported by default output device\n Audio initialization failed\n");
      return;
    }

    // Set the message string
    status.Message = messageString.ToString();

    RestartStream(true, status);

    //InputParameters.device = apiInfo->defaultInputDevice;
    //InputParameters.sampleFormat = OutputParameters.sampleFormat;
    //const PaDeviceInfo* inputDeviceInfo = Pa_GetDeviceInfo(InputParameters.device);
    //InputParameters.suggestedLatency = inputDeviceInfo->defaultLowInputLatency;
    //InputParameters.channelCount = inputDeviceInfo->maxInputChannels;
    //InputSampleRate = (unsigned)inputDeviceInfo->defaultSampleRate;

    //result = Pa_IsFormatSupported(&InputParameters, NULL, (double)InputSampleRate);
    //if (result != paFormatIsSupported)
    //{
    //  Error("Audio input format is not supported");
    //}

    //PaUtil_InitializeRingBuffer(&InputRingBuffer, sizeof(float), InputBufferSize, InputBuffer);
    //PaUtil_FlushRingBuffer(&InputRingBuffer);

  }

  //************************************************************************************************
  void AudioInputOutput::ShutDown(Zero::Status& status)
  {
    PaError result;

    StopPAStream(status);
    //StopInputStream();

    // Terminate PortAudio and check for error
    result = Pa_Terminate();
    if (result != paNoError)
    {
      status.SetFailed(Zero::String::Format("PortAudio error: %s", Pa_GetErrorText(result)));
    }
  }

  //************************************************************************************************
  void AudioInputOutput::Reset()
  {
    Zero::Status status;

    // Shut down and re-initialize Port Audio
    ShutDown(status);
    Initialize(status);
  }

  //************************************************************************************************
  void AudioInputOutput::StopStream()
  {
    Zero::Status status;
    StopPAStream(status);
  }

  //************************************************************************************************
  void AudioInputOutput::StartStream()
  {
    if (!Initialized)
      return;

    Zero::Status status;
    StartPAStream(status);

    //StartInputStream();
  }

  //************************************************************************************************
  bool AudioInputOutput::IsStreamOpen()
  {
    return Stream != nullptr;
  }

  //************************************************************************************************
  void AudioInputOutput::StartInputStream()
  {
    PaError result = Pa_OpenStream(
      &InputStream,
      &InputParameters,
      NULL,
      InputSampleRate,
      CallbackFrameSizeThreaded,
      0,
      PACallback,
      &CallbackDataThreaded);
    if (result != paNoError)
      Error("Error opening input stream");

    result = Pa_StartStream(InputStream);
    if (result != paNoError)
      Error("Error starting input stream");
  }

  //************************************************************************************************
  void AudioInputOutput::StopInputStream()
  {
    if (!InputStream)
      return;

    PaError result = Pa_CloseStream(InputStream);
    if (result != paNoError)
      Error("Error closing input stream");

    InputStream = nullptr;
  }

  //************************************************************************************************
  void AudioInputOutput::RestartStream(const bool lowLatency, Zero::Status& status)
  {
    LowLatency = lowLatency;

    if (Stream)
    {
      ShutDown(status);

      if (lowLatency)
        SetLatency(BufferBaseSize);
      else
        SetLatency(BufferLargeSize);

      Initialize(status);
    }
    else
    {
      SetLatency(BufferBaseSize);

      StartPAStream(status);
    }
  }

  //************************************************************************************************
  unsigned AudioInputOutput::GetBaseBufferSize()
  {
    if (LowLatency)
      return BufferBaseSize;
    else
      return BufferLargeSize;
  }

  //************************************************************************************************
  void AudioInputOutput::StartPAStream(Zero::Status& status)
  {
    // Open a PA stream
    PaError result = Pa_OpenStream(
      &Stream,
      NULL, // No input
      &OutputParameters,
      gAudioSystem->SystemSampleRate,
      CallbackFrameSizeThreaded,
      paClipOff,  // Won't check for clipping
      PACallback,
      &CallbackDataThreaded);

    // Start the stream
    result = Pa_StartStream(Stream);
    if (result != paNoError)
    {
      ZPrint("Error starting output stream: %s\n", Pa_GetErrorText(result));
      status.SetFailed(Zero::String::Format("PortAudio error: %s", Pa_GetErrorText(result)));
    }

    ZPrint("Output stream started\n");
  }

  //************************************************************************************************
  void AudioInputOutput::StopPAStream(Zero::Status& status)
  {
    if (!Stream)
      return;

    // Close the stream
    PaError result = Pa_CloseStream(Stream);
    if (result != paNoError)
    {
      status.SetFailed(Zero::String::Format("PortAudio error: %s", Pa_GetErrorText(result)));
    }

    Stream = nullptr;
  }

  //************************************************************************************************
  void AudioInputOutput::SetUpBuffers(const unsigned size)
  {
    OutputBufferSizeThreaded = size;
    CallbackDataThreaded.BufferSize = OutputBufferSizeThreaded;

    // Check if there are existing buffers
    if (OutputBuffersThreaded[0])
    {
      // Delete existing buffers
      for (int i = 0; i < NumOutputBuffers; ++i)
        delete OutputBuffersThreaded[i];
    }

    // Create the output buffers and set them to zero 
    for (int i = 0; i < NumOutputBuffers; ++i)
    {
      OutputBuffersThreaded[i] = new float[size];
      memset(OutputBuffersThreaded[i], 0, size * sizeof(float));
    }

  }

  //************************************************************************************************
  void AudioInputOutput::SetLatency(const unsigned baseSize)
  {
    CallbackFrameSizeThreaded = baseSize / 2;
    SetUpBuffers(baseSize * OutputChannelsThreaded);
  }

  //-------------------------------------------------------------------------- Microphone Input Node

  //************************************************************************************************
  bool MicrophoneInputNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels, 
    ListenerNode* listener, const bool firstRequest)
  {
    if (PaUtil_GetRingBufferReadAvailable(&gAudioSystem->AudioIO.InputRingBuffer) == 0)
      return false;

    PaUtil_ReadRingBuffer(&gAudioSystem->AudioIO.InputRingBuffer, outputBuffer->Data(), 
      outputBuffer->Size());

    return true;
  }

}