///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.h"

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

namespace Audio
{
#define SAFE_CLOSE(h) if ((h) != nullptr) { CloseHandle((h)); (h) = nullptr; }
#define SAFE_RELEASE(object) if ((object) != nullptr) { (object)->Release(); (object) = nullptr; }
#define SAFE_FREE(object) if ((object) != nullptr) { CoTaskMemFree(object); (object) = nullptr; }

  Zero::String GetMessageForHresult(Zero::StringParam message, HRESULT hr)
  { 
    if (HRESULT_FACILITY(hr) == FACILITY_WINDOWS)
      hr = HRESULT_CODE(hr);
    TCHAR* errorMessage;

    if (FormatMessage(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
      nullptr,
      hr,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      (LPTSTR)&errorMessage,
      0,
      nullptr) != 0)
    {
      Zero::String string(Zero::String::Format("%s 0x%08x: %s", message.c_str(), hr, errorMessage));
      LocalFree(errorMessage);
      return string;
    }

    return Zero::String::Format("%s 0x%08x", message.c_str(), hr);
  }

  //---------------------------------------------------------------------------------- WASAPI Device

  //************************************************************************************************
  WasapiDevice::WasapiDevice() :
    AudioClient(nullptr),
    Device(nullptr),
    RenderClient(nullptr),
    CaptureClient(nullptr),
    Format(nullptr),
    Enumerator(nullptr),
    IsOutput(false),
    BufferFrameCount(0),
    StreamOpen(false),
    ClientCallback(nullptr),
    ClientData(nullptr),
    FallbackSleepTime((unsigned)((float)FallbackFrames / (float)FallbackSampleRate * 1000.0f))
  {
    memset(ThreadEvents, 0, sizeof(void*) * NumThreadEvents);
  }

  //************************************************************************************************
  WasapiDevice::~WasapiDevice()
  {
    ReleaseData(); 
    if (Enumerator)
      Enumerator->UnregisterEndpointNotificationCallback(this);
  }

  //************************************************************************************************
  void WasapiDevice::ReleaseData()
  {
    SAFE_RELEASE(AudioClient);
    SAFE_RELEASE(Device);
    SAFE_RELEASE(RenderClient);
    SAFE_RELEASE(CaptureClient);

    SAFE_FREE(Format);

    for (int i = 0; i < NumThreadEvents; ++i)
      SAFE_CLOSE(ThreadEvents[i]);
  }

  //************************************************************************************************
  void WasapiDevice::Initialize(IMMDeviceEnumerator* enumerator, bool render, StreamStatus& status,
    Zero::String& message)
  {
    if (!Enumerator)
    {
      Enumerator = enumerator;
      IsOutput = render;
      if (enumerator)
        enumerator->RegisterEndpointNotificationCallback(this);
    }

    HRESULT result;

    // Get the default output or input device
    if (render)
      result = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &Device);
    else
      result = enumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &Device);
    if (FAILED(result))
    {
      status = DeviceProblem;
      if (render)
        LogAudioIoError(GetMessageForHresult("Unable to get default output device.", result), &message);
      else
        LogAudioIoError(GetMessageForHresult("Unable to get default input device.", result), &message);
      goto ErrorExit;
    }

    // Get the device state
    DWORD deviceState;
    result = Device->GetState(&deviceState);
    if (deviceState != DEVICE_STATE_ACTIVE)
    {
      status = DeviceProblem;
      if (render)
        LogAudioIoError("Default output device not active.", &message);
      else
       LogAudioIoError("Default input device not active.", &message);
      goto ErrorExit;
    }

    // Get access to the device properties
    IPropertyStore* property(nullptr);
    result = Device->OpenPropertyStore(STGM_READ, &property);
    if (result == S_OK)
    {
      // Get the FriendlyName property
      PROPVARIANT value;
      PropVariantInit(&value);
      result = property->GetValue(PKEY_Device_FriendlyName, &value);

      // Translate the name format
      if (value.pwszVal)
        WideCharToMultiByte(CP_UTF8, 0, value.pwszVal, (int)wcslen(value.pwszVal), DeviceName,
          NameLength, 0, 0);
    }

    // Create the audio client
    result = Device->Activate(IID_IAudioClient, CLSCTX_ALL, nullptr, (void**)&AudioClient);
    if (FAILED(result))
    {
      status = DeviceProblem;
      LogAudioIoError(GetMessageForHresult("Unable to create audio client.", result), &message);
      goto ErrorExit;
    }

    // Get the format information
    result = AudioClient->GetMixFormat(&Format);
    if (FAILED(result))
    {
      status = DeviceProblem;
      LogAudioIoError(GetMessageForHresult("Unable to get audio client format.", result), &message);
      goto ErrorExit;
    }

    // Verify float32 output
    if (Format->wFormatTag != WAVE_FORMAT_EXTENSIBLE ||
      ((WAVEFORMATEXTENSIBLE*)Format)->SubFormat != KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)
    {
      // Shouldn't hit this unless something is really weird
      status = DeviceProblem;
      LogAudioIoError(GetMessageForHresult("Incompatible audio format.", result), &message);
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
      status = DeviceProblem;
      LogAudioIoError(GetMessageForHresult("Unable to initialize audio client.", result), &message);
      goto ErrorExit;
    }

    // Get the actual buffer size
    result = AudioClient->GetBufferSize(&BufferFrameCount);
    if (FAILED(result))
    {
      status = DeviceProblem;
      LogAudioIoError(GetMessageForHresult("Unable to get buffer size from audio client.", result),
        &message);
      goto ErrorExit;
    }

    // Create the render or capture client
    if (render)
      result = AudioClient->GetService(IID_IAudioRenderClient, (void**)&RenderClient);
    else
      result = AudioClient->GetService(IID_IAudioCaptureClient, (void**)&CaptureClient);
    if (FAILED(result))
    {
      status = DeviceProblem;
      if (render)
        LogAudioIoError(GetMessageForHresult("Unable to get audio render client.", result), &message);
      else
        LogAudioIoError(GetMessageForHresult("Unable to get audio capture client.", result), &message);
      goto ErrorExit;
    }

    // Create the thread events
    for (int i = 0; i < NumThreadEvents; ++i)
      ThreadEvents[i] = CreateEvent(nullptr, false, false, nullptr);

    // Set the event handle on the AudioClient
    result = AudioClient->SetEventHandle(ThreadEvents[WasapiEvent]);
    if (FAILED(result))
    {
      status = DeviceProblem;
      LogAudioIoError(GetMessageForHresult("Unable to set event handle on audio client.", result),
        &message);
      goto ErrorExit;
    }

    status = Initialized;

    if (render)
      ZPrint("Audio output successfully initialized\n");
    else
      ZPrint("Audio input successfully initialized\n");

    return;

  ErrorExit:

    ReleaseData();
    if (render)
      ZPrint("Audio output initialization unsuccessful. Using fallback.\n");
    else
      ZPrint("Audio input initialization unsuccessful\n");
  }

  //************************************************************************************************
  StreamStatus WasapiDevice::StartStream(WASAPICallbackType* callback, void* data)
  {
    StreamStatus status = Uninitialized;

    // Make sure the stream hasn't already been started and has been initialized
    if (!StreamOpen && Enumerator)
    {
      // Check if the input or output device has been initialized
      if (RenderClient || CaptureClient)
      {
        // For output, set entire buffer to zero to avoid audio glitches
        if (RenderClient)
        {
          // Get the entire buffer from the RenderClient
          byte* data;
          HRESULT result = RenderClient->GetBuffer(BufferFrameCount, &data);

          // Set the buffer to zero
          memset(data, 0, BufferFrameCount * sizeof(float));

          // Release the buffer
          result = RenderClient->ReleaseBuffer(BufferFrameCount, 0);

          ZPrint("Started audio output stream\n");
        }
        else
          ZPrint("Started audio input stream\n");

        // Start the AudioClient
        AudioClient->Start();
      }
      // If not, and this is the input stream, do nothing
      else if (!IsOutput)
      {
        ZPrint("Unable to start input stream: uninitialized\n");
        return Uninitialized;
      }
      // Set up fallback output stream
      else
      {
        // Create the thread events
        for (int i = 0; i < NumThreadEvents; ++i)
          ThreadEvents[i] = CreateEvent(nullptr, false, false, nullptr);

        ZPrint("Started audio output stream with fallback for no device\n");
      }

      ClientCallback = callback;
      ClientData = data;
      
      StreamOpen = true;
      status = Started;
    }
    else
    {
      if (IsOutput)
        ZPrint("Unable to start output stream: ");
      else
        ZPrint("Unable to start input stream: ");
      
      ZPrint("stream is either uninitialized or already open\n");
    }

    return status;
  }

  //************************************************************************************************
  StreamStatus WasapiDevice::StopStream()
  {
    StreamStatus status = Uninitialized;

    if (StreamOpen)
    {
      // Signal the StopRequest and wait on ThreadExit
      SignalObjectAndWait(ThreadEvents[StopRequestEvent], ThreadEvents[ThreadExitEvent], INFINITE, false);

      // Stop the audio client
      if (RenderClient || CaptureClient)
        AudioClient->Stop();

      StreamOpen = false;
      status = Stopped;

      if (IsOutput)
        ZPrint("Stopped audio output stream\n");
      else
        ZPrint("Stopped audio input stream\n");
    }
    else
    {
      if (IsOutput)
        ZPrint("Unable to stop output stream: not open\n");
      else
        ZPrint("Unable to stop input stream: not open\n");
    }

    return status;
  }

  //************************************************************************************************
  void WasapiDevice::ProcessingLoop()
  {
    // Boost the thread priority to Audio
    LPCTSTR name = TEXT("Audio");
    DWORD value = 0;
    HANDLE task = AvSetMmThreadCharacteristics(name, &value);

    bool processing(true);
    while (processing)
    {
      if (RenderClient || CaptureClient)
        processing = WasapiEventHandling();
      else if (IsOutput)
        processing = OutputFallback();
      else
        processing = false;
    }

    // Revert the thread priority
    AvRevertMmThreadCharacteristics(task);

    // Set the thread exit event
    SetEvent(ThreadEvents[ThreadExitEvent]);
  }

  //************************************************************************************************
  unsigned WasapiDevice::GetChannels()
  {
    if (Format)
      return Format->nChannels;
    else if (IsOutput)
      return FallbackChannels;
    else
      return 0;
  }

  //************************************************************************************************
  unsigned WasapiDevice::GetSampleRate()
  {
    if (Format)
      return Format->nSamplesPerSec;
    else if (IsOutput)
      return FallbackSampleRate;
    else
      return 0;
  }

  //************************************************************************************************
  void WasapiDevice::Reset()
  {
    ZPrint("Resetting audio due to default device change\n");

    // Stop the audio client
    if (RenderClient || CaptureClient)
      AudioClient->Stop();

    StreamOpen = false;

    ReleaseData();

    StreamStatus status;
    Zero::String string;
    Initialize(Enumerator, IsOutput, status, string);

    StartStream(ClientCallback, ClientData);
  }

  //************************************************************************************************
  bool WasapiDevice::WasapiEventHandling()
  {
    // Wait for 1000 ms or until an event is signaled
    DWORD waitResult = WaitForMultipleObjects(NumThreadEvents, ThreadEvents, false, 1000);

    // If the StopRequestEvent was signaled, return and stop
    if (waitResult == StopRequestEvent)
      return false;
    // If the ResetEvent was signaled, reset and return
    else if (waitResult == ResetEvent)
    {
      Reset();
      return true;
    }
    // If we timed out waiting for WASAPI, return and stop
    else if (waitResult == WAIT_TIMEOUT)
      return false;

    HRESULT result;

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

        // Release the buffer
        CaptureClient->ReleaseBuffer(packetFrames);
      }
    }

    return true;
  }

  //************************************************************************************************
  bool WasapiDevice::OutputFallback()
  {
    // Wait for the FallbackSleepTime or until an event is signaled
    DWORD waitResult = WaitForMultipleObjects(NumThreadEvents, ThreadEvents, false, FallbackSleepTime);
    // Check for signaled thread events
    if (waitResult != WAIT_TIMEOUT)
    {
      if (waitResult == StopRequestEvent)
        return false;
      else if (waitResult == ResetEvent)
      {
        Reset();
        return true;
      }
    }

    // Call the client callback function with the dummy buffer
    (*ClientCallback)(FallbackBuffer, nullptr, FallbackFrames, ClientData);

    return true;
  }

  //************************************************************************************************
  HRESULT STDMETHODCALLTYPE WasapiDevice::OnDefaultDeviceChanged(EDataFlow flow, ERole role, 
    LPCWSTR pwstrDeviceId)
  {
    if (role == ERole::eConsole)
    {
      if ((IsOutput && flow == EDataFlow::eRender) || (!IsOutput && flow == EDataFlow::eCapture))
        SetEvent(ThreadEvents[ResetEvent]);
    }

    return S_OK;
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
    Enumerator(nullptr)
  {
    memset(StreamDevices, 0, sizeof(WasapiDevice*) * NumStreamTypes);
  }

  //************************************************************************************************
  AudioIOWindows::~AudioIOWindows()
  {
    ShutDownStream(OutputStream);
    ShutDownStream(InputStream);
    ShutDownAPI();

    if (StreamDevices[OutputStream])
      delete StreamDevices[OutputStream];
    if (StreamDevices[InputStream])
      delete StreamDevices[InputStream];
  }

  //************************************************************************************************
  StreamStatus AudioIOWindows::InitializeAPI()
  {
    ZPrint("Initializing WASAPI\n");

    HRESULT result = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(result))
    {
      LogAudioIoError(GetMessageForHresult("CoInitialize unsuccessful.", result), &LastErrorMessage);
      return ApiProblem;
    }

    // Create the device enumerator
    result = CoCreateInstance(CLSID_MMDeviceEnumerator, nullptr, CLSCTX_ALL, IID_IMMDeviceEnumerator,
      (void**)&Enumerator);
    if (FAILED(result))
    {
      LogAudioIoError(GetMessageForHresult("Unable to create enumerator.", result), &LastErrorMessage);
      return ApiProblem;
    }
    
    return Initialized;
  }

  //************************************************************************************************
  StreamStatus AudioIOWindows::InitializeStream(StreamType whichStream)
  {
    // Create the WasapiDevice object if needed
    if (!StreamDevices[whichStream])
      StreamDevices[whichStream] = new WasapiDevice();

    // Initialize the appropriate device
    StreamDevices[whichStream]->Initialize(Enumerator, whichStream == OutputStream,
      StreamInfoList[whichStream].Status, StreamInfoList[whichStream].ErrorMessage);

    // If this was the output stream, also initialize the buffers
    if (whichStream == OutputStream)
      InitializeOutputBuffers();

    return StreamInfoList[whichStream].Status;
  }

  //************************************************************************************************
  StreamStatus AudioIOWindows::StartStream(StreamType whichStream)
  {
    // Get the appropriate device object
    WasapiDevice* device = StreamDevices[whichStream];

    if (device)
      StreamInfoList[whichStream].Status = device->StartStream(WASAPICallback, this);
    else
      StreamInfoList[whichStream].Status = Uninitialized;

    // Check if the device exists and was successfully initialized
    if (StreamInfoList[whichStream].Status == Started)
    {
      // Start the processing loop thread
      if (whichStream == OutputStream)
        _beginthreadex(nullptr, 0, &AudioIOWindows::StartOutputThread, this, 0, nullptr);
      else if (whichStream == InputStream)
        _beginthreadex(nullptr, 0, &AudioIOWindows::StartInputThread, this, 0, nullptr);
    }

    return StreamInfoList[whichStream].Status;
  }

  //************************************************************************************************
  StreamStatus AudioIOWindows::StopStream(StreamType whichStream)
  {
    // Get the appropriate device object
    WasapiDevice* device = StreamDevices[whichStream];

    if (device)
    {
      device->StopStream();
      StreamInfoList[whichStream].Status = Stopped;

      if (whichStream == OutputStream)
        ZPrint("Audio output stream stopped\n");
      else if (whichStream == InputStream)
        ZPrint("Audio input stream stopped\n");

      return Stopped;
    }

    if (whichStream == InputStream)
      LogAudioIoError("Unable to stop audio input stream: not currently started", 
        &StreamInfoList[whichStream].ErrorMessage);
    else
      LogAudioIoError("Unable to stop audio output stream: not currently started",
        &StreamInfoList[whichStream].ErrorMessage);

    return Uninitialized;
  }

  //************************************************************************************************
  StreamStatus AudioIOWindows::ShutDownStream(StreamType whichStream)
  {
    // Release stream data if stream stopped successfully
    if (StopStream(whichStream) == Stopped)
      StreamDevices[whichStream]->ReleaseData();

    return Stopped;
  }

  //************************************************************************************************
  void AudioIOWindows::ShutDownAPI()
  {
    if (Enumerator)
    {
      SAFE_RELEASE(Enumerator);
      CoUninitialize();

      ZPrint("Shut down WASAPI audio IO\n");
    }
  }

  //************************************************************************************************
  unsigned AudioIOWindows::GetStreamChannels(StreamType whichStream)
  {
    if (StreamDevices[whichStream])
      return StreamDevices[whichStream]->GetChannels();
    else
      return 0;
  }

  //************************************************************************************************
  unsigned AudioIOWindows::GetStreamSampleRate(StreamType whichStream)
  {
    if (StreamDevices[whichStream])
      return StreamDevices[whichStream]->GetSampleRate();
    else
      return 0;
  }

  //************************************************************************************************
  bool AudioIOWindows::IsStreamStarted(StreamType whichStream)
  {
    if (StreamDevices[whichStream])
      return StreamDevices[whichStream]->StreamOpen;
    else
      return false;
  }

  //************************************************************************************************
  void AudioIOWindows::HandleCallback(const void *inputBuffer, void *outputBuffer, 
    unsigned long framesPerBuffer)
  {
    if (outputBuffer)
      GetMixedOutputSamples((float*)outputBuffer, framesPerBuffer * 
        StreamDevices[OutputStream]->GetChannels());
    else if (inputBuffer)
      SaveInputSamples((const float*)inputBuffer, framesPerBuffer * 
        StreamDevices[InputStream]->GetChannels());
  }

  //************************************************************************************************
  unsigned AudioIOWindows::StartOutputThread(void* param)
  {
    if (param)
      ((AudioIOWindows*)param)->StreamDevices[OutputStream]->ProcessingLoop();
    return 0;
  }

  //************************************************************************************************
  unsigned AudioIOWindows::StartInputThread(void* param)
  {
    if (param)
      ((AudioIOWindows*)param)->StreamDevices[InputStream]->ProcessingLoop();
    return 0;
  }

}