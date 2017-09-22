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
    OutputBufferSizeThreaded(0),
    BufferBaseSize(0),
    BufferLargeSize(0),
    WriteBufferThreaded(0),
    ReadBufferThreaded(0),
    LowLatency(true),
    MixedBufferIndex(0)
  {

    // Null first buffer so we can check if they've been created yet
    OutputBuffersThreaded[0] = nullptr;

    PaUtil_InitializeRingBuffer(&InputRingBuffer, sizeof(float), InputBufferSize, InputBuffer);
    PaUtil_FlushRingBuffer(&InputRingBuffer);
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
    Counter.WaitAndDecrement();
  }

  //************************************************************************************************
  void AudioInputOutput::ResetOutput(Zero::Status& status)
  {
    // Shut down and re-initialize 
    ShutDownOutput(status);
    InitializeOutput(status);
    if (status.Succeeded())
      StartOutputStream(status);
  }

  //************************************************************************************************
  void AudioInputOutput::ResetInput(Zero::Status& status)
  {
    // Shut down and re-initialize 
    ShutDownInput(status);
    InitializeInput(status);
    if (status.Succeeded())
      StartOutputStream(status);
  }

  //************************************************************************************************
  unsigned AudioInputOutput::GetBufferSize(unsigned sampleRate)
  {
    unsigned size = BufferSizeStartValue;
    float multiplier = SmallBufferMultiplier;
    if (!LowLatency)
      multiplier = LargeBufferMultiplier;

    while (size < (unsigned)(multiplier * sampleRate))
      size += BufferSizeStartValue;

    return size;
  }

  //************************************************************************************************
  void AudioInputOutput::GetInputData(Zero::Array<float>& buffer, unsigned howManySamples)
  {
    unsigned samplesAvailable = PaUtil_GetRingBufferReadAvailable(&InputRingBuffer);

    if (samplesAvailable < howManySamples)
      howManySamples = samplesAvailable;
    
    buffer.Resize(howManySamples);

    unsigned samplesRead = PaUtil_ReadRingBuffer(&InputRingBuffer, buffer.Data(), howManySamples);

    if (samplesRead != howManySamples)
      buffer.Resize(samplesRead);
  }

  //************************************************************************************************
  void AudioInputOutput::InitializeOutputBuffers()
  {
    unsigned outputSampleRate = GetOutputSampleRate();

    BufferBaseSize = BufferSizeStartValue;
    while (BufferBaseSize < (unsigned)(SmallBufferMultiplier * outputSampleRate))
      BufferBaseSize += BufferSizeStartValue;

    BufferLargeSize = BufferSizeStartValue * 4;
    while (BufferLargeSize < (unsigned)(LargeBufferMultiplier * outputSampleRate))
      BufferLargeSize += BufferSizeStartValue;

    if (LowLatency)
      SetUpOutputBuffers(BufferBaseSize * GetOutputChannels());
    else
      SetUpOutputBuffers(BufferLargeSize * GetOutputChannels());
  }

  //************************************************************************************************
  void AudioInputOutput::SetUpOutputBuffers(const unsigned size)
  {
    OutputBufferSizeThreaded = size;

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
  void AudioInputOutput::SetLatency(bool lowLatency)
  {
    if (lowLatency == LowLatency)
      return;

    LowLatency = lowLatency;

    Zero::Status status;
    ShutDownOutput(status);

    if (LowLatency)
      SetUpOutputBuffers(BufferBaseSize * GetOutputChannels());
    else
      SetUpOutputBuffers(BufferLargeSize * GetOutputChannels());

    InitializeOutput(status);
  }

  //************************************************************************************************
  void AudioInputOutput::GetMixedOutputSamples(float* outputBuffer, const unsigned howManySamples)
  {
    float* mixedBuffer = OutputBuffersThreaded[ReadBufferThreaded];

    unsigned samples = howManySamples;

    // Make sure we don't go past the end of the buffer
    if (samples > OutputBufferSizeThreaded - MixedBufferIndex)
      samples = OutputBufferSizeThreaded - MixedBufferIndex;

    // Copy data into output buffer
    memcpy(outputBuffer, mixedBuffer + MixedBufferIndex, samples * sizeof(float));

    // Advance the buffer position index
    MixedBufferIndex += samples;

    // Check if we reached the end of the buffer
    if (MixedBufferIndex >= OutputBufferSizeThreaded)
    {
      // Increment the buffer read number, wrapping if necessary
      ReadBufferThreaded = (ReadBufferThreaded + 1) % NumOutputBuffers;
      // Reset the index
      MixedBufferIndex = 0;

      // Check if we need to add more samples to the output buffer
      if (samples < howManySamples)
      {
        unsigned previousSamples = samples;
        samples = howManySamples - previousSamples;
        mixedBuffer = OutputBuffersThreaded[ReadBufferThreaded];
        memcpy(outputBuffer + previousSamples, mixedBuffer, samples * sizeof(float));
        MixedBufferIndex += samples;
      }
    }

    if (OutputBufferSizeThreaded - MixedBufferIndex <= howManySamples)
      // Signal the mix thread to mix another buffer
      Counter.Increment();
  }

  //************************************************************************************************
  void AudioInputOutput::SaveInputSamples(const float* inputBuffer, unsigned howManySamples)
  {
    PaUtil_WriteRingBuffer(&InputRingBuffer, inputBuffer, howManySamples);
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
    OutputStream(nullptr),
    OutputSampleRate(0),
    InputStream(nullptr),
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
  void AudioIOPortAudio::InitializeAPI(Zero::Status& status)
  {
    OutputParameters->device = -1;
    InputParameters->device = -1;

    // Initialize PortAudio and check for error
    PaError result = Pa_Initialize();
    if (result != paNoError)
    {
      SetStatusAndLog(status, Zero::String::Format(
        "Unable to initialize PortAudio: %s", Pa_GetErrorText(result)));
      return;
    }
    ZPrint("PortAudio initialized\n");

    // Get number of devices
    int numDevices = Pa_GetDeviceCount();
    // Port Audio error condition
    if (numDevices < 0)
    {
      SetStatusAndLog(status, Zero::String::Format(
        "Error getting audio devices: Pa_CountDevices returned 0x%x", numDevices));
      return;
    }
    // No audio devices, can't do anything
    else if (numDevices == 0)
    {
      SetStatusAndLog(status, "No audio output devices found");
      ZPrint("Audio initialization unsuccessful\n");
      return;
    }

    int numAPIs = Pa_GetHostApiCount();
    if (numAPIs == 0)
    {
      SetStatusAndLog(status, "No audio APIs found");
      ZPrint("Audio initialization unsuccessful\n");
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
      SetStatusAndLog(status, "No supported audio APIs found.");
      ZPrint("Audio initialization unsuccessful\n");
      return;
    }

    // Get API info
    ApiInfo = Pa_GetHostApiInfo(apiIndex);
  }

  //************************************************************************************************
  void AudioIOPortAudio::InitializeOutput(Zero::Status& status)
  {
    // Make sure API is already initialized
    if (!ApiInfo)
    {
      InitializeAPI(status);

      if (status.Failed())
      {
        SetStatusAndLog(status,
          "Audio API was not successfully initialized, unable to initialize output");
        return;
      }
    }

    // Set up output parameters
    OutputParameters->device = ApiInfo->defaultOutputDevice;    // Default output device
    if (OutputParameters->device == paNoDevice)
    {
      SetStatusAndLog(status, "No default audio output device found.");
      ZPrint("Audio initialization unsuccessful\n");
      return;
    }
    OutputParameters->sampleFormat = paFloat32;    // 32 bit floating point output

                                                   // Get the device info
    const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo(OutputParameters->device);

    OutputParameters->suggestedLatency = deviceInfo->defaultLowOutputLatency;
    OutputSampleRate = (unsigned)deviceInfo->defaultSampleRate;

    // Set the variables that depend on the number of output channels
    OutputParameters->channelCount = deviceInfo->maxOutputChannels;

    // Create the string with data on the audio setup
    Zero::StringBuilder messageString;
    messageString << ApiInfo->name << " API, " << deviceInfo->name << ", ";
    messageString << OutputParameters->channelCount << " output channels, ";
    messageString << gAudioSystem->SystemChannelsThreaded << " mix channels, ";
    messageString << OutputParameters->suggestedLatency << " latency, ";
    messageString << OutputSampleRate << " sample rate";

    ZPrint("API             : %s\n", deviceInfo->name);
    ZPrint("Output channels : %d\n", OutputParameters->channelCount);
    ZPrint("Mix channels    : %d\n", gAudioSystem->SystemChannelsThreaded);
    ZPrint("Latency         : %f\n", OutputParameters->suggestedLatency);
    ZPrint("Sample rate     : %d\n", OutputSampleRate);

    // Check device settings
    PaError result = Pa_IsFormatSupported(nullptr, OutputParameters, (double)OutputSampleRate);
    if (result != paFormatIsSupported)
    {
      // Parameters were not supported - set error string and return
      Zero::StringBuilder errorString;
      errorString << "Audio settings not supported by default output device. ";
      errorString << messageString.ToString();
      status.SetFailed(messageString.ToString());
      ZPrint("Settings not supported by default output device\n Audio initialization unsuccessful\n");
      return;
    }

    // Set the message string
    status.Message = messageString.ToString();

    InitializeOutputBuffers();
  }

  //************************************************************************************************
  void AudioIOPortAudio::InitializeInput(Zero::Status& status)
  {
    // Make sure API is already initialized
    if (!ApiInfo)
    {
      SetStatusAndLog(status,
        "Audio API was not successfully initialized, unable to initialize input");
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
        ZPrint("Audio input format is not supported\n");
        InputParameters->device = -1;
      }
      else
      {

        ZPrint("Successfully initialized audio input\n");
      }
    }
  }

  //************************************************************************************************
  void AudioIOPortAudio::ShutDownAPI(Zero::Status& status)
  {
    // Terminate PortAudio and check for error
    PaError result = Pa_Terminate();
    if (result != paNoError)
    {
      SetStatusAndLog(status, Zero::String::Format(
        "Error terminating PortAudio: %s", Pa_GetErrorText(result)));
      return;
    }

    ZPrint("Terminated PortAudio\n");
  }

  //************************************************************************************************
  void AudioIOPortAudio::ShutDownOutput(Zero::Status& status)
  {
    if (OutputStream)
      StopOutputStream(status);

    ShutDownAPI(status);
  }

  //************************************************************************************************
  void AudioIOPortAudio::ShutDownInput(Zero::Status& status)
  {
    if (InputStream)
      StopInputStream(status);
  }

  //************************************************************************************************
  unsigned AudioIOPortAudio::GetOutputChannels()
  {
    return OutputParameters->channelCount;
  }

  //************************************************************************************************
  unsigned AudioIOPortAudio::GetOutputSampleRate()
  {
    return OutputSampleRate;
  }

  //************************************************************************************************
  unsigned AudioIOPortAudio::GetInputChannels()
  {
    return InputParameters->channelCount;
  }

  //************************************************************************************************
  unsigned AudioIOPortAudio::GetInputSampleRate()
  {
    return InputSampleRate;
  }

  //************************************************************************************************
  bool AudioIOPortAudio::IsOutputStreamOpen()
  {
    return OutputStream != nullptr;
  }

  //************************************************************************************************
  bool AudioIOPortAudio::IsInputStreamOpen()
  {
    return InputStream != nullptr;
  }

  //************************************************************************************************
  void AudioIOPortAudio::StartOutputStream(Zero::Status& status)
  {
    // No current output device
    if (OutputParameters->device < 0)
    {
      status.SetFailed("No audio output device, can't start output stream");
      ZPrint(status.Message.c_str());
      ZPrint("\n");
      return;
    }

    // Open a PortAudio stream
    PaError result = Pa_OpenStream(
      &OutputStream,
      nullptr,                          // No input
      OutputParameters,
      OutputSampleRate,
      paFramesPerBufferUnspecified,     // Use variable optimal number of frames per callback
      paClipOff,                        // Won't check for clipping
      PACallback,
      this);
    if (result != paNoError)
    {
      status.SetFailed(Zero::String::Format("Error opening audio output stream: %s", 
        Pa_GetErrorText(result)));
      ZPrint(status.Message.c_str());
      ZPrint("\n");
      return;
    }

    // If opened successfully, start the stream
    result = Pa_StartStream(OutputStream);
    if (result != paNoError)
    {
      status.SetFailed(Zero::String::Format("Error starting audio output stream : %s", 
        Pa_GetErrorText(result)));
      ZPrint(status.Message.c_str());
      ZPrint("\n");
      return;
    }

    ZPrint("Audio output stream started\n");
  }

  //************************************************************************************************
  void AudioIOPortAudio::StopOutputStream(Zero::Status& status)
  {
    if (!OutputStream)
      return;

    // Close the stream
    PaError result = Pa_CloseStream(OutputStream);
    if (result != paNoError)
    {
      status.SetFailed(Zero::String::Format("Error closing audio output stream: %s", 
        Pa_GetErrorText(result)));
      ZPrint(status.Message.c_str());
      ZPrint("\n");
      return;
    }

    OutputStream = nullptr;

    ZPrint("Audio output stream stopped\n");
  }

  //************************************************************************************************
  void AudioIOPortAudio::StartInputStream(Zero::Status& status)
  {
    // No current input device
    if (InputParameters->device < 0)
    {
      status.SetFailed("No audio input device, can't start input stream");
      ZPrint(status.Message.c_str());
      ZPrint("\n");
      return;
    }

    // Open PortAudio stream
    PaError result = Pa_OpenStream(
      &InputStream,
      InputParameters,
      nullptr,            // no output
      InputSampleRate,
      0,                  // Use variable optimal number of frames per callback
      0,
      PACallback,
      this);
    if (result != paNoError)
    {
      status.SetFailed(Zero::String::Format("Error opening audio input stream: %s", 
        Pa_GetErrorText(result)));
      ZPrint(status.Message.c_str());
      ZPrint("\n");
      return;
    }

    // If opened successfully, start the stream
    result = Pa_StartStream(InputStream);
    if (result != paNoError)
    {
      status.SetFailed(Zero::String::Format("Error starting audio input stream : %s", 
        Pa_GetErrorText(result)));
      ZPrint(status.Message.c_str());
      ZPrint("\n");
      return;
    }

    ZPrint("Audio input stream started\n");
  }

  //************************************************************************************************
  void AudioIOPortAudio::StopInputStream(Zero::Status& status)
  {
    if (!InputStream)
      return;

    // Close the stream
    PaError result = Pa_CloseStream(InputStream);
    if (result != paNoError)
    {
      status.SetFailed(Zero::String::Format("Error closing audio output stream: %s", 
        Pa_GetErrorText(result)));
      ZPrint(status.Message.c_str());
      ZPrint("\n");
      return;
    }

    InputStream = nullptr;

    ZPrint("Audio input stream stopped\n");
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
}
