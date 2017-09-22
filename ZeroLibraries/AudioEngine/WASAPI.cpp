///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <mmreg.h>
#include <Audioclient.h>
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <functiondiscoverykeys.h>
#include <process.h>
#include <avrt.h>

// REFERENCE_TIME time units per second and per millisecond
#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

// TODO - check for AUDCLNT_E_DEVICE_INVALIDATED

namespace Audio
{
#define SAFE_CLOSE(h) if ((h) != nullptr) { CloseHandle((h)); (h) = nullptr; }
#define SAFE_RELEASE(object) if ((object) != nullptr) { (object)->Release(); (object) = nullptr; }
#define SAFE_FREE(object) if ((object) != nullptr) { CoTaskMemFree(object); (object) = nullptr; }
#define IF_FAILED_JUMP(result, label) if(FAILED(result)) { goto label;}


  typedef void WASAPICallbackType(float* outputBuffer, float* inputBuffer, const unsigned frameCount,
    void* userData);

  //------------------------------------------------------------------------------------- DeviceInfo

  class WasapiDeviceInfo
  {
    WasapiDeviceInfo() :
      AudioClient(nullptr),
      Device(nullptr),
      RenderClient(nullptr),
      CaptureClient(nullptr),
      Format(nullptr),
      WasapiEvent(nullptr),
      StopRequest(nullptr),
      ThreadExit(nullptr)
    {}
    ~WasapiDeviceInfo() { ReleaseData(); }

    void ReleaseData();
    void Initialize(Zero::Status& status, IMMDeviceEnumerator* enumerator, bool render);
    void ProcessingLoop();

    IAudioClient* AudioClient;
    IMMDevice* Device;
    IAudioRenderClient* RenderClient;
    IAudioCaptureClient* CaptureClient;
    tWAVEFORMATEX* Format;
    void* WasapiEvent;
    void* StopRequest;
    void* ThreadExit;
    unsigned BufferFrameCount;
    bool StreamOpen;
    static const unsigned NameLength = 512;
    char DeviceName[NameLength];
    WASAPICallbackType* ClientCallback;
    void* ClientData;

    friend class AudioIOWindows;
  };

  //************************************************************************************************
  void WasapiDeviceInfo::ReleaseData()
  {
    SAFE_RELEASE(AudioClient);
    SAFE_RELEASE(Device);
    SAFE_RELEASE(RenderClient);
    SAFE_RELEASE(CaptureClient);

    SAFE_FREE(Format);

    SAFE_CLOSE(WasapiEvent);
    SAFE_CLOSE(StopRequest);
    SAFE_CLOSE(ThreadExit);
  }

  //************************************************************************************************
  void WasapiDeviceInfo::Initialize(Zero::Status& status, IMMDeviceEnumerator* enumerator, bool render)
  {
    HRESULT result;

    // Get the default output device
    if (render)
      result = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &Device);
    else
      result = enumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &Device);
    if (FAILED(result))
    {
      SetStatusAndLog(status, Zero::String::Format(
        "Failed to create device when initializing audio API: %d", result));
      goto ErrorExit;
    }

    // Get the device state
    DWORD deviceState;
    result = Device->GetState(&deviceState);
    if (deviceState != DEVICE_STATE_ACTIVE)
    {
      SetStatusAndLog(status, Zero::String::Format(
        "Device not active when initializing audio API: %d", result));
      goto ErrorExit;
    }

    // Get access to the device properties
    IPropertyStore* property(nullptr);
    result = Device->OpenPropertyStore(STGM_READ, &property);
    if (FAILED(result))
    {
      SetStatusAndLog(status, Zero::String::Format(
        "Failed to access device property when initializing audio API: %d", result));
      goto ErrorExit;
    }

    // Get the FriendlyName property
    PROPVARIANT value;
    PropVariantInit(&value);
    result = property->GetValue(PKEY_Device_FriendlyName, &value);
    if (FAILED(result))
    {
      SetStatusAndLog(status, Zero::String::Format(
        "Failed to get device name when initializing audio API: %d", result));
      goto ErrorExit;
    }

    // Translate the name format
    if (value.pwszVal)
      WideCharToMultiByte(CP_UTF8, 0, value.pwszVal, (int)wcslen(value.pwszVal), DeviceName, 
        NameLength, 0, 0);

    // Create the audio client
    result = Device->Activate(IID_IAudioClient, CLSCTX_ALL, nullptr, (void**)&AudioClient);
    if (FAILED(result))
    {
      SetStatusAndLog(status, Zero::String::Format(
        "Failed to create client when initializing audio API: %d", result));
      goto ErrorExit;
    }

    // Get the format information
    result = AudioClient->GetMixFormat(&Format);
    if (FAILED(result))
    {
      SetStatusAndLog(status, Zero::String::Format(
        "Failed to get format when initializing audio API: %d", result));
      goto ErrorExit;
    }

    // Verify float32 output
    if (Format->wFormatTag != WAVE_FORMAT_EXTENSIBLE ||
      ((WAVEFORMATEXTENSIBLE*)Format)->SubFormat != KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)
    {
      // Shouldn't hit this unless something is really weird
      SetStatusAndLog(status, Zero::String::Format(
        "Incompatible format when initializing audio API: %d", result));
      goto ErrorExit;
    }

    // Initialize the audio client with the smallest acceptable buffer size
    result = AudioClient->Initialize(
      AUDCLNT_SHAREMODE_SHARED,            // Shared mode, not exclusive
      AUDCLNT_STREAMFLAGS_EVENTCALLBACK,   // Use event callback mode
      0,                                   // Pass in zero to get smallest buffer size
      0,                                   // Period must be 0 in shared mode
      Format,                              // Device format
      nullptr);                            // Audio session GUID value
    if (FAILED(result))
    {
      SetStatusAndLog(status, Zero::String::Format(
        "Failed to initialize client when initializing audio API: %d", result));
      goto ErrorExit;
    }

    // Get the actual buffer size
    result = AudioClient->GetBufferSize(&BufferFrameCount);
    if (FAILED(result))
    {
      SetStatusAndLog(status, Zero::String::Format(
        "Failed to get buffer size when initializing audio API: %d", result));
      goto ErrorExit;
    }

    // Create the render or capture client
    if (render)
      result = AudioClient->GetService(IID_IAudioRenderClient, (void**)&RenderClient);
    else
      result = AudioClient->GetService(IID_IAudioCaptureClient, (void**)&CaptureClient);
    if (FAILED(result))
    {
      if (render)
        SetStatusAndLog(status, Zero::String::Format(
          "Failed to get render client when initializing audio API: %d", result));
      else
        SetStatusAndLog(status, Zero::String::Format(
          "Failed to get capture client when initializing audio API: %d", result));
      goto ErrorExit;
    }

    // Create the event for shutting down
    StopRequest = CreateEvent(nullptr, false, false, nullptr);
    // Create the event for thread exit 
    ThreadExit = CreateEvent(nullptr, false, false, nullptr);
    // Create the event to use for the WASAPI callback
    WasapiEvent = CreateEvent(nullptr, false, false, nullptr);

    // Set the event handle on the AudioClient
    result = AudioClient->SetEventHandle(WasapiEvent);
    if (FAILED(result))
    {
      SetStatusAndLog(status, Zero::String::Format(
        "Failed to set event handle when initializing audio API: %d", result));
      goto ErrorExit;
    }

    if (render)
      ZPrint("Audio output successfully initialized\n");
    else
      ZPrint("Audio input successfully initialized\n");

    return;

  ErrorExit:

    ReleaseData();
    if (render)
      ZPrint("Audio output initialization failed\n");
    else
      ZPrint("Audio input initialization failed\n");
  }

  //************************************************************************************************
  void WasapiDeviceInfo::ProcessingLoop()
  {
    // Boost the thread priority to Audio
    LPCTSTR name = TEXT("Audio");
    DWORD value = 0;
    HANDLE task = AvSetMmThreadCharacteristics(name, &value);

    HRESULT result;

    // For output, set entire buffer to zero to avoid audio glitches
    if (RenderClient)
    {
      // Get the entire buffer from the RenderClient
      byte* data;
      result = RenderClient->GetBuffer(BufferFrameCount, &data);

      // Set the buffer to zero
      memset(data, 0, BufferFrameCount * sizeof(float));

      // Release the buffer
      result = RenderClient->ReleaseBuffer(BufferFrameCount, 0);
    }

    // Start the AudioClient
    AudioClient->Start();

    while (true)
    {
      // Wait for event from WASAPI
      DWORD waitResult = WaitForSingleObject(WasapiEvent, 10 * 1000);

      // Check if CloseRequest has been signaled
      if (WaitForSingleObject(StopRequest, 0) != WAIT_TIMEOUT)
        break;

      // If we timed out waiting for WASAPI, stop
      if (waitResult == WAIT_TIMEOUT)
        break;

      if (RenderClient)
      {
        // Get the number of frames that currently have audio data
        unsigned frames;
        result = AudioClient->GetCurrentPadding(&frames);

        if (result == S_OK)
        {
          // Frames to request are total frames minus frames with data
          frames = BufferFrameCount - frames;

          // Get a buffer section from WASAPI
          byte* data;
          result = RenderClient->GetBuffer(frames, &data);

          // If successful, call the client callback function
          if (result == S_OK)
            (*ClientCallback)((float*)data, nullptr, frames, ClientData);
          else
          {
            // check for device disconnected etc.
          }

          // Release the buffer
          result = RenderClient->ReleaseBuffer(frames, 0);
        }
      }
      else
      {
        // Get the size of the next input packet
        unsigned packetFrames;
        CaptureClient->GetNextPacketSize(&packetFrames);
        // Make sure it's not empty
        if (packetFrames != 0)
        {
          byte* data;
          DWORD flags;

          // Get the buffer (size will match previous call to GetNextPacketSize)
          result = CaptureClient->GetBuffer(&data, &packetFrames, &flags, nullptr, nullptr);

          // If successful, call the client callback function
          if (result == S_OK)
            (*ClientCallback)(nullptr, (float*)data, packetFrames, ClientData);
          else
          {
            // check for device disconnected etc.
          }

          // Release the buffer
          CaptureClient->ReleaseBuffer(packetFrames);
        }
      }
    }

    // Stop the audio client
    AudioClient->Stop();

    // Revert the thread priority
    AvRevertMmThreadCharacteristics(task);

    // Set the thread exit event
    SetEvent(ThreadExit);
  }

  //---------------------------------------------------------------- Audio Input Output using WASAPI

  //************************************************************************************************
  static void WASAPICallback(float* outputBuffer, float* inputBuffer, const unsigned frameCount,
    void* userData)
  {
    ((AudioIOWindows*)userData)->HandleCallback(inputBuffer, outputBuffer, frameCount);
  }

  //************************************************************************************************
  AudioIOWindows::AudioIOWindows() :
    OutputDevice(nullptr),
    InputDevice(nullptr)
  {

  }

  //************************************************************************************************
  AudioIOWindows::~AudioIOWindows()
  {
    Zero::Status status;
    ShutDownAPI(status);

    CoUninitialize();

    if (OutputDevice)
      delete OutputDevice;
    if (InputDevice)
      delete InputDevice;
  }

  //************************************************************************************************
  unsigned AudioIOWindows::GetOutputChannels()
  {
    if (OutputDevice && OutputDevice->Format)
      return OutputDevice->Format->nChannels;
    else
      return 0;
  }

  //************************************************************************************************
  unsigned AudioIOWindows::GetOutputSampleRate()
  {
    if (OutputDevice && OutputDevice->Format)
      return OutputDevice->Format->nSamplesPerSec;
    else
      return 0;
  }

  //************************************************************************************************
  unsigned AudioIOWindows::GetInputChannels()
  {
    if (InputDevice && InputDevice->Format)
      return InputDevice->Format->nChannels;
    else
      return 0;
  }

  //************************************************************************************************
  unsigned AudioIOWindows::GetInputSampleRate()
  {
    if (InputDevice && InputDevice->Format)
      return InputDevice->Format->nSamplesPerSec;
    else
      return 0;
  }

  //************************************************************************************************
  bool AudioIOWindows::IsOutputStreamOpen()
  {
    if (OutputDevice)
      return OutputDevice->StreamOpen;
    else
      return false;
  }

  //************************************************************************************************
  bool AudioIOWindows::IsInputStreamOpen()
  {
    if (InputDevice)
      return InputDevice->StreamOpen;
    else
      return false;
  }

  //************************************************************************************************
  void AudioIOWindows::StartOutputStream(Zero::Status& status)
  {
    if (!OutputDevice)
      return;

    OutputDevice->ClientCallback = WASAPICallback;
    OutputDevice->ClientData = this;

    // Start the output processing loop thread
    if (OutputDevice->RenderClient)
    {
      _beginthreadex(nullptr, 0, &AudioIOWindows::StartOutputThread, this, 0, nullptr);
      OutputDevice->StreamOpen = true;

      ZPrint("Audio output stream started\n");
    }
    else
      SetStatusAndLog(status, 
        "Error starting audio output stream: output was not previously initialized");
  }

  //************************************************************************************************
  void AudioIOWindows::StopOutputStream(Zero::Status& status)
  {
    if (OutputDevice && OutputDevice->StreamOpen)
    {
      // Signal the CloseRequest and wait on ThreadExit
      SignalObjectAndWait(OutputDevice->StopRequest, OutputDevice->ThreadExit, INFINITE, false);

      OutputDevice->StreamOpen = false;

      ZPrint("Audio output stream stopped\n");
    }
  }

  //************************************************************************************************
  void AudioIOWindows::StartInputStream(Zero::Status& status)
  {
    if (!InputDevice)
      return;

    InputDevice->ClientCallback = WASAPICallback;
    InputDevice->ClientData = this;

    // Start the input processing loop thread
    if (InputDevice->CaptureClient)
    {
      _beginthreadex(nullptr, 0, &AudioIOWindows::StartInputThread, this, 0, nullptr);
      InputDevice->StreamOpen = true;

      ZPrint("Audio input stream started\n");
    }
    else
      SetStatusAndLog(status, 
        "Error starting audio input stream: input was not previously initialized");
  }

  //************************************************************************************************
  void AudioIOWindows::StopInputStream(Zero::Status& status)
  {
    if (InputDevice && InputDevice->StreamOpen)
    {
      // Signal the CloseRequest and wait on ThreadExit
      SignalObjectAndWait(InputDevice->StopRequest, InputDevice->ThreadExit, INFINITE, false);

      InputDevice->StreamOpen = false;

      ZPrint("Audio input stream stopped\n");
    }
  }

  //************************************************************************************************
  void AudioIOWindows::InitializeAPI(Zero::Status& status)
  {
    if (!OutputDevice)
      OutputDevice = new WasapiDeviceInfo();
    if (!InputDevice)
      InputDevice = new WasapiDeviceInfo();

    HRESULT result;

    result = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(result))
    {
      SetStatusAndLog(status, Zero::String::Format(
        "CoInitialize failed when initializing audio API: %d", result));
      return;
    }

    IMMDeviceEnumerator* enumerator(nullptr);

    // Create the device enumerator
    result = CoCreateInstance(CLSID_MMDeviceEnumerator, nullptr, CLSCTX_ALL, IID_IMMDeviceEnumerator,
      (void**)&enumerator);
    if (FAILED(result))
    {
      SetStatusAndLog(status, Zero::String::Format(
        "Failed to create enumerator when initializing audio API: %d", result));
      return;
    }

    OutputDevice->Initialize(status, enumerator, true);
    InputDevice->Initialize(status, enumerator, false);

    SAFE_RELEASE(enumerator);
  }

  //************************************************************************************************
  void AudioIOWindows::ShutDownAPI(Zero::Status& status)
  {
    StopOutputStream(status);
    StopInputStream(status);

    OutputDevice->ReleaseData();
    InputDevice->ReleaseData();

    ZPrint("Audio IO shut down\n");
  }

  //************************************************************************************************
  void AudioIOWindows::HandleCallback(const void *inputBuffer, void *outputBuffer, 
    unsigned long framesPerBuffer)
  {
    if (outputBuffer)
      GetMixedOutputSamples((float*)outputBuffer, framesPerBuffer * OutputDevice->Format->nChannels);
    else if (inputBuffer)
      SaveInputSamples((const float*)inputBuffer, framesPerBuffer * InputDevice->Format->nChannels);
  }

  //************************************************************************************************
  unsigned AudioIOWindows::StartOutputThread(void* param)
  {
    if (param)
      ((AudioIOWindows*)param)->OutputDevice->ProcessingLoop();
    return 0;
  }

  //************************************************************************************************
  unsigned AudioIOWindows::StartInputThread(void* param)
  {
    if (param)
      ((AudioIOWindows*)param)->InputDevice->ProcessingLoop();
    return 0;
  }

}