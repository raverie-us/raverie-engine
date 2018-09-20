///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

#include <Windows.h>
#include <mmreg.h>
#include <Audioclient.h>
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <functiondiscoverykeys.h>
#include <process.h>
#include <avrt.h>

typedef void WASAPICallbackType(float* outputBuffer, float* inputBuffer, const unsigned frameCount,
  void* userData);

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

namespace Zero
{
#define SAFE_CLOSE(h) if ((h) != nullptr) { CloseHandle((h)); (h) = nullptr; }
#define SAFE_RELEASE(object) if ((object) != nullptr) { (object)->Release(); (object) = nullptr; }
#define SAFE_FREE(object) if ((object) != nullptr) { CoTaskMemFree(object); (object) = nullptr; }
  
namespace ThreadEventTypes
{
  enum Enum { StopRequestEvent, ThreadExitEvent, WasapiEvent, ResetEvent, NumThreadEvents };
}

class WasapiDevice;

String GetMessageForHresult(StringParam message, HRESULT hr)
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
    String string(String::Format("%s 0x%08x %s", message.c_str(), hr, errorMessage));
    LocalFree(errorMessage);
    return string;
  }

  return String::Format("%s 0x%08x", message.c_str(), hr);
}

//-------------------------------------------------------------------------- Audio IO Windows Data

class AudioIOWindowsData
{
public:
  AudioIOWindowsData();
  ~AudioIOWindowsData();

  WasapiDevice* StreamDevices[StreamTypes::Size];
  IMMDeviceEnumerator* Enumerator;
  IOCallbackType* Callbacks[StreamTypes::Size];
  void* CallbackData[StreamTypes::Size];

  static unsigned _stdcall StartOutputThread(void* param);
  static unsigned _stdcall StartInputThread(void* param);
};

//---------------------------------------------------------------------------------- WASAPI Device

class WasapiDevice : IMMNotificationClient
{
public:

  WasapiDevice();
  ~WasapiDevice();

  void ReleaseData();
  StreamStatus::Enum Initialize(IMMDeviceEnumerator* enumerator, bool render, String* message);
  StreamStatus::Enum StartStream(WASAPICallbackType* callback, void* data);
  StreamStatus::Enum StopStream();
  void ProcessingLoop();
  unsigned GetChannels();
  unsigned GetSampleRate();

  bool StreamOpen;

private:
  void Reset();
  bool WasapiEventHandling();
  void GetOutput();
  void GetInput();
  bool OutputFallback();
  void HandleInitializationError();

  IMMDeviceEnumerator* Enumerator;
  IMMDevice* Device;
  IAudioClient* AudioClient;
  IAudioRenderClient* RenderClient;
  IAudioCaptureClient* CaptureClient;
  tWAVEFORMATEX* Format;
  WASAPICallbackType* ClientCallback;
  void* ClientData;
  bool IsOutput;
  unsigned BufferFrameCount;
  static const unsigned NameLength = 512;
  char DeviceName[NameLength];
  static const unsigned FallbackFrames = 512;
  static const unsigned FallbackChannels = 2;
  static const unsigned FallbackSampleRate = 48000;
  float FallbackBuffer[FallbackFrames * FallbackChannels];
  unsigned FallbackSleepTime;
  String StreamTypeName;

  HANDLE ThreadEvents[ThreadEventTypes::NumThreadEvents];

public:
  HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDeviceId);

  // The following functions are not used but must be implemented
  ULONG STDMETHODCALLTYPE AddRef() { return 0; }
  ULONG STDMETHODCALLTYPE Release() { return 0; }
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID **ppvInterface) { return S_OK; }
  HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR pwstrDeviceId) { return S_OK; }
  HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstrDeviceId) { return S_OK; }
  HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState) {
    return S_OK;
  }
  HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key) {
    return S_OK;
  }
};

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
  memset(ThreadEvents, 0, sizeof(void*) * ThreadEventTypes::NumThreadEvents);
  DeviceName[0] = 0;
  StreamTypeName = "Unknown";
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

  for (int i = 0; i < ThreadEventTypes::NumThreadEvents; ++i)
    SAFE_CLOSE(ThreadEvents[i]);
}

//************************************************************************************************
StreamStatus::Enum WasapiDevice::Initialize(IMMDeviceEnumerator* enumerator, bool render,
  String* message)
{
  if (render)
    StreamTypeName = "Output";
  else
    StreamTypeName = "Input";

  ZPrint("Initializing audio %s stream\n", StreamTypeName.c_str());

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
    LogAudioIoError(GetMessageForHresult("Unable to get default device:", result), message);
    HandleInitializationError();
    return StreamStatus::DeviceProblem;
  }

  // Get the device state
  DWORD deviceState;
  result = Device->GetState(&deviceState);
  if (deviceState != DEVICE_STATE_ACTIVE)
  {
    LogAudioIoError("Default device not active:", message);
    HandleInitializationError();
    return StreamStatus::DeviceProblem;
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
    {
      WideCharToMultiByte(CP_UTF8, 0, value.pwszVal, (int)wcslen(value.pwszVal), DeviceName,
        NameLength, 0, 0);

      DeviceName[wcslen(value.pwszVal)] = 0;
    }
  }

  // Create the audio client
  result = Device->Activate(IID_IAudioClient, CLSCTX_ALL, nullptr, (void**)&AudioClient);
  if (FAILED(result))
  {
    LogAudioIoError(GetMessageForHresult("Unable to create audio client:", result), message);
    HandleInitializationError();
    return StreamStatus::DeviceProblem;
  }

  // Get the format information
  result = AudioClient->GetMixFormat(&Format);
  if (FAILED(result))
  {
    LogAudioIoError(GetMessageForHresult("Unable to get audio client format:", result), message);
    HandleInitializationError();
    return StreamStatus::DeviceProblem;
  }

  // Verify float32 output
  if ((Format->wFormatTag == WAVE_FORMAT_EXTENSIBLE && ((WAVEFORMATEXTENSIBLE*)Format)->SubFormat
    != KSDATAFORMAT_SUBTYPE_IEEE_FLOAT) || (Format->wFormatTag != WAVE_FORMAT_EXTENSIBLE &&
      Format->wFormatTag != WAVE_FORMAT_IEEE_FLOAT))
  {
    // Shouldn't hit this unless something is really weird
    LogAudioIoError(GetMessageForHresult(String::Format("Incompatible audio format 0x%04x.",
      Format->wFormatTag), result), message);
    HandleInitializationError();
    return StreamStatus::DeviceProblem;
  }

  ZPrint("Device name : %s\n", DeviceName);
  ZPrint("Channels    : %d\n", Format->nChannels);
  ZPrint("Sample rate : %d\n", Format->nSamplesPerSec);

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
    LogAudioIoError(GetMessageForHresult("Unable to initialize audio client:", result), message);
    HandleInitializationError();
    return StreamStatus::DeviceProblem;
  }

  // Get the actual buffer size
  result = AudioClient->GetBufferSize(&BufferFrameCount);
  if (FAILED(result))
  {
    LogAudioIoError(GetMessageForHresult("Unable to get buffer size from audio client:", result),
      message);
    HandleInitializationError();
    return StreamStatus::DeviceProblem;
  }

  // Create the render or capture client
  if (render)
    result = AudioClient->GetService(IID_IAudioRenderClient, (void**)&RenderClient);
  else
    result = AudioClient->GetService(IID_IAudioCaptureClient, (void**)&CaptureClient);
  if (FAILED(result))
  {
    if (render)
      LogAudioIoError(GetMessageForHresult("Unable to get audio render client:", result), message);
    else
      LogAudioIoError(GetMessageForHresult("Unable to get audio capture client:", result), message);
    HandleInitializationError();
    return StreamStatus::DeviceProblem;
  }

  // Create the thread events
  for (int i = 0; i < ThreadEventTypes::NumThreadEvents; ++i)
    ThreadEvents[i] = CreateEvent(nullptr, false, false, nullptr);

  // Set the event handle on the AudioClient
  result = AudioClient->SetEventHandle(ThreadEvents[ThreadEventTypes::WasapiEvent]);
  if (FAILED(result))
  {
    LogAudioIoError(GetMessageForHresult("Unable to set event handle on audio client:", result),
      message);
    HandleInitializationError();
    return StreamStatus::DeviceProblem;
  }

  ZPrint("Audio %s stream successfully initialized\n", StreamTypeName.c_str());

  return StreamStatus::Initialized;
}

//************************************************************************************************
StreamStatus::Enum WasapiDevice::StartStream(WASAPICallbackType* callback, void* data)
{
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
      }

      // Start the AudioClient
      AudioClient->Start();

      ZPrint("Started audio %s stream\n", StreamTypeName.c_str());
    }
    // Set up fallback stream
    else
    {
      // Create the thread events
      for (int i = 0; i < ThreadEventTypes::NumThreadEvents; ++i)
        ThreadEvents[i] = CreateEvent(nullptr, false, false, nullptr);

      ZPrint("Started audio %s stream with fallback for no device\n", StreamTypeName.c_str());
    }

    ClientCallback = callback;
    ClientData = data;

    StreamOpen = true;

    return StreamStatus::Started;
  }
  else
  {
    ZPrint("Unable to start audio %s stream: ", StreamTypeName.c_str());

    if (!Enumerator)
    {
      ZPrint("stream is uninitialized.\n");
      return StreamStatus::Uninitialized;
    }
    else
    {
      ZPrint("stream is already open.\n");
      return StreamStatus::Started;
    }
  }
}

//************************************************************************************************
StreamStatus::Enum WasapiDevice::StopStream()
{
  if (StreamOpen)
  {
    // Signal the StopRequest and wait on ThreadExit
    SignalObjectAndWait(ThreadEvents[ThreadEventTypes::StopRequestEvent],
      ThreadEvents[ThreadEventTypes::ThreadExitEvent], INFINITE, false);

    // Stop the audio client
    if (RenderClient || CaptureClient)
      AudioClient->Stop();

    StreamOpen = false;

    ZPrint("Stopped audio %s stream\n", StreamTypeName.c_str());
  }
  else
  {
    ZPrint("Unable to stop audio %s stream: not open\n", StreamTypeName.c_str());
  }

  return StreamStatus::Stopped;
}

//************************************************************************************************
void WasapiDevice::ProcessingLoop()
{
  CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

  // Boost the thread priority to Audio
  LPCTSTR name = TEXT("Audio");
  DWORD value = 0;
  HANDLE task = AvSetMmThreadCharacteristics(name, &value);

  bool processing(true);
  while (processing)
  {
    // If there is a valid input or output device, use the WASAPI function
    if (RenderClient || CaptureClient)
      processing = WasapiEventHandling();
    // Otherwise, if this is the output stream, use the fallback function
    else if (IsOutput)
      processing = OutputFallback();
    // If this is the input stream, wait for either a stop or reset signal
    else
    {
      HANDLE events[2] = { ThreadEvents[ThreadEventTypes::StopRequestEvent],
        ThreadEvents[ThreadEventTypes::ResetEvent] };

      DWORD waitResult = WaitForMultipleObjects(2, events, false, INFINITE);
      // Check for stop request
      if (waitResult == 0)
        processing = false;
      // Check for reset request
      else if (waitResult == 1)
        Reset();
    }
  }

  // Revert the thread priority
  AvRevertMmThreadCharacteristics(task);

  CoUninitialize();

  // Set the thread exit event
  SetEvent(ThreadEvents[ThreadEventTypes::ThreadExitEvent]);
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

  Initialize(Enumerator, IsOutput, nullptr);

  StartStream(ClientCallback, ClientData);
}

//************************************************************************************************
bool WasapiDevice::WasapiEventHandling()
{
  // Wait for 1000 ms or until an event is signaled
  DWORD waitResult = WaitForMultipleObjects(ThreadEventTypes::NumThreadEvents, ThreadEvents, false, 1000);

  // If the StopRequestEvent was signaled, return and stop
  if (waitResult == ThreadEventTypes::StopRequestEvent)
    return false;
  // If the ResetEvent was signaled, reset and return
  else if (waitResult == ThreadEventTypes::ResetEvent)
  {
    Reset();
    return true;
  }
  // If we timed out waiting for WASAPI, try to reset (will use fallback if it fails)
  else if (waitResult == WAIT_TIMEOUT)
  {
    Reset();
    return true;
  }

  if (RenderClient)
    GetOutput();
  else
    GetInput();

  return true;
}

//************************************************************************************************
void WasapiDevice::GetOutput()
{
  // Get the number of frames that currently have audio data
  unsigned frames;
  HRESULT result = AudioClient->GetCurrentPadding(&frames);

  if (result == S_OK)
  {
    // Frames to request are total frames minus frames with data
    frames = BufferFrameCount - frames;

    // Get a buffer section from WASAPI
    byte* data;
    result = RenderClient->GetBuffer(frames, &data);

    // If successful, call the client callback function
    if (result == S_OK && frames > 0)
      (*ClientCallback)((float*)data, nullptr, frames, ClientData);

    // Release the buffer
    result = RenderClient->ReleaseBuffer(frames, 0);
  }
}

//************************************************************************************************
void WasapiDevice::GetInput()
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
    HRESULT result = CaptureClient->GetBuffer(&data, &packetFrames, &flags, nullptr, nullptr);

    // If successful, call the client callback function
    if (result == S_OK)
      (*ClientCallback)(nullptr, (float*)data, packetFrames, ClientData);

    // Release the buffer
    CaptureClient->ReleaseBuffer(packetFrames);
  }
}

//************************************************************************************************
bool WasapiDevice::OutputFallback()
{
  // Wait for the FallbackSleepTime or until an event is signaled
  DWORD waitResult = WaitForMultipleObjects(ThreadEventTypes::NumThreadEvents, ThreadEvents,
    false, FallbackSleepTime);
  // Check for signaled thread events
  if (waitResult != WAIT_TIMEOUT)
  {
    if (waitResult == ThreadEventTypes::StopRequestEvent)
      return false;
    else if (waitResult == ThreadEventTypes::ResetEvent)
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
void WasapiDevice::HandleInitializationError()
{
  ReleaseData();

  ZPrint("Audio %s initialization unsuccessful. Using fallback.\n", StreamTypeName.c_str());
}

//************************************************************************************************
HRESULT STDMETHODCALLTYPE WasapiDevice::OnDefaultDeviceChanged(EDataFlow flow, ERole role,
  LPCWSTR pwstrDeviceId)
{
  if (role == ERole::eConsole)
  {
    if ((IsOutput && flow == EDataFlow::eRender) || (!IsOutput && flow == EDataFlow::eCapture))
      SetEvent(ThreadEvents[ThreadEventTypes::ResetEvent]);
  }

  return S_OK;
}

//---------------------------------------------------------------- Audio Input Output using WASAPI

//************************************************************************************************
static void WASAPICallback(float* outputBuffer, float* inputBuffer, const unsigned frameCount,
  void* userData)
{
  if (outputBuffer)
    ((AudioIOWindowsData*)userData)->Callbacks[StreamTypes::Output](outputBuffer, inputBuffer,
      frameCount, ((AudioIOWindowsData*)userData)->CallbackData[StreamTypes::Output]);
  else
    ((AudioIOWindowsData*)userData)->Callbacks[StreamTypes::Input](outputBuffer, inputBuffer,
      frameCount, ((AudioIOWindowsData*)userData)->CallbackData[StreamTypes::Input]);
}

//************************************************************************************************
AudioInputOutput::AudioInputOutput()
{
  PlatformData = (void*)new AudioIOWindowsData();
}

//************************************************************************************************
AudioInputOutput::~AudioInputOutput()
{
  if (((AudioIOWindowsData*)PlatformData)->Enumerator)
  {
    StopStream(StreamTypes::Output, nullptr);
    StopStream(StreamTypes::Input, nullptr);
    ShutDownAPI();
  }

  delete (AudioIOWindowsData*)PlatformData;
}

//************************************************************************************************
StreamStatus::Enum AudioInputOutput::InitializeAPI(String* resultMessage)
{
  ZPrint("Initializing WASAPI\n");

  HRESULT result = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
  if (FAILED(result))
  {
    LogAudioIoError(GetMessageForHresult("CoInitialize unsuccessful.", result), resultMessage);
    return StreamStatus::ApiProblem;
  }

  // Create the device enumerator
  result = CoCreateInstance(CLSID_MMDeviceEnumerator, nullptr, CLSCTX_ALL, IID_IMMDeviceEnumerator,
    (void**)&((AudioIOWindowsData*)PlatformData)->Enumerator);
  if (FAILED(result))
  {
    LogAudioIoError(GetMessageForHresult("Unable to create enumerator.", result), resultMessage);
    return StreamStatus::ApiProblem;
  }

  return StreamStatus::Initialized;
}

//************************************************************************************************
StreamStatus::Enum AudioInputOutput::InitializeStream(StreamTypes::Enum whichStream,
  String* resultMessage)
{
  WasapiDevice* device = ((AudioIOWindowsData*)PlatformData)->StreamDevices[whichStream];

  // Create the WasapiDevice object if needed
  if (!device)
  {
    device = new WasapiDevice();
    ((AudioIOWindowsData*)PlatformData)->StreamDevices[whichStream] = device;
  }

  // Initialize the appropriate device
  StreamStatus::Enum status = device->Initialize(((AudioIOWindowsData*)PlatformData)->Enumerator,
    whichStream == StreamTypes::Output, resultMessage);

  return status;
}

//************************************************************************************************
StreamStatus::Enum AudioInputOutput::StartStream(StreamTypes::Enum whichStream,
  String* resultMessage, IOCallbackType* callback, void* callbackData)
{
  ((AudioIOWindowsData*)PlatformData)->Callbacks[whichStream] = callback;
  ((AudioIOWindowsData*)PlatformData)->CallbackData[whichStream] = callbackData;

  // Get the appropriate device object
  WasapiDevice* device = ((AudioIOWindowsData*)PlatformData)->StreamDevices[whichStream];
  StreamStatus::Enum status;

  if (device)
    status = device->StartStream(WASAPICallback, PlatformData);
  else
    status = StreamStatus::Uninitialized;

  // Start the processing loop thread (needs to start even if uninitialized)
  if (whichStream == StreamTypes::Output)
    _beginthreadex(nullptr, 0, &AudioIOWindowsData::StartOutputThread, PlatformData, 0, nullptr);
  else if (whichStream == StreamTypes::Input)
    _beginthreadex(nullptr, 0, &AudioIOWindowsData::StartInputThread, PlatformData, 0, nullptr);

  return status;
}

//************************************************************************************************
StreamStatus::Enum AudioInputOutput::StopStream(StreamTypes::Enum whichStream, String* resultMessage)
{
  // Get the appropriate device object
  WasapiDevice* device = ((AudioIOWindowsData*)PlatformData)->StreamDevices[whichStream];

  if (device)
  {
    device->StopStream();
    device->ReleaseData();

    return StreamStatus::Stopped;
  }

  if (whichStream == StreamTypes::Input)
    LogAudioIoError("Unable to stop audio input stream: not currently started", resultMessage);
  else
    LogAudioIoError("Unable to stop audio output stream: not currently started", resultMessage);

  return StreamStatus::Uninitialized;
}

//************************************************************************************************
void AudioInputOutput::ShutDownAPI()
{
  if (((AudioIOWindowsData*)PlatformData)->Enumerator)
  {
    SAFE_RELEASE(((AudioIOWindowsData*)PlatformData)->Enumerator);
    CoUninitialize();

    ZPrint("WASAPI audio IO was shut down\n");
  }
}

//************************************************************************************************
unsigned AudioInputOutput::GetStreamChannels(StreamTypes::Enum whichStream)
{
  if (((AudioIOWindowsData*)PlatformData)->StreamDevices[whichStream])
    return ((AudioIOWindowsData*)PlatformData)->StreamDevices[whichStream]->GetChannels();
  else
    return 0;
}

//************************************************************************************************
unsigned AudioInputOutput::GetStreamSampleRate(StreamTypes::Enum whichStream)
{
  if (((AudioIOWindowsData*)PlatformData)->StreamDevices[whichStream])
    return ((AudioIOWindowsData*)PlatformData)->StreamDevices[whichStream]->GetSampleRate();
  else
    return 0;
}

//************************************************************************************************
float AudioInputOutput::GetBufferSizeMultiplier()
{
  return 0.04f;
}

//-------------------------------------------------------------------------- Audio IO Windows Data

//************************************************************************************************
AudioIOWindowsData::AudioIOWindowsData() : Enumerator(nullptr)
{
  memset(StreamDevices, 0, sizeof(WasapiDevice*) * StreamTypes::Size);
}

//************************************************************************************************
AudioIOWindowsData::~AudioIOWindowsData()
{
  if (StreamDevices[StreamTypes::Output])
    delete StreamDevices[StreamTypes::Output];
  if (StreamDevices[StreamTypes::Input])
    delete StreamDevices[StreamTypes::Input];
}

//************************************************************************************************
unsigned AudioIOWindowsData::StartOutputThread(void* param)
{
  if (param)
    ((AudioIOWindowsData*)param)->StreamDevices[StreamTypes::Output]->ProcessingLoop();
  return 0;
}

//************************************************************************************************
unsigned AudioIOWindowsData::StartInputThread(void* param)
{
  if (param && ((AudioIOWindowsData*)param)->StreamDevices[StreamTypes::Input])
    ((AudioIOWindowsData*)param)->StreamDevices[StreamTypes::Input]->ProcessingLoop();
  return 0;
}

//--------------------------------------------------------------------------------------- MIDI Input

//************************************************************************************************
void CALLBACK MidiInProcCallback(HMIDIIN handle, UINT message, DWORD_PTR dwInstance,
  DWORD_PTR param1, DWORD_PTR param2)
{
  if (message != MIM_DATA)
    return;

  // Reinterpret message
  char byte1 = LOBYTE(LOWORD(param1));
  int data1 = HIBYTE(LOWORD(param1));
  int data2 = LOBYTE(HIWORD(param1));
  int channel = byte1 & 0x0F;
  int command = byte1 & 0xF0;

  MidiData data;
  data.mEventType = MidiEventType::NotSet;
  data.mData1 = channel;
  data.mData2 = 0;
  data.mData3 = 0;

  // Note off
  if (command == 0x80)
  {
    data.mData2 = (float)data1;
    data.mEventType = MidiEventType::MidiNoteOff;
  }
  // Note on
  else if (command == 0x90)
  {
    if (data2 > 0)
    {
      data.mData2 = (float)data1;
      data.mData3 = (float)data2;
      data.mEventType = MidiEventType::MidiNoteOn;
    }
    else
    {
      data.mData2 = (float)data1;
      data.mEventType = MidiEventType::MidiNoteOff;
    }
  }
  // Pitch wheel
  else if (command == 0xE0)
  {
    float value = ((float)(data2 * (1 << 7)) + (float)data1) - (1 << 13);
    value *= 2.0f / (float)((1 << 14) - 1);
    data.mData2 = value;
    data.mEventType = MidiEventType::MidiPitchWheel;
  }
  // Control
  else if (command == 0xB0)
  {
    // Volume
    if (data1 == 7)
    {
      data.mData2 = (float)data2;
      data.mEventType = MidiEventType::MidiVolume;
    }
    // Modulation wheel
    else if (data1 == 1)
    {
      data.mData2 = (float)data2;
      data.mEventType = MidiEventType::MidiModWheel;
    }
    else
    {
      data.mData2 = (float)data1;
      data.mData3 = (float)data2;
      data.mEventType = MidiEventType::MidiControl;
    }
  }

  if (data.mEventType != MidiEventType::NotSet)
  {
    MidiInput* input = (MidiInput*)dwInstance;
    if (input->mOnMidiData)
      input->mOnMidiData(&data, input);
  }
}

//************************************************************************************************
MidiInput::MidiInput() :
  mOnMidiData(nullptr),
  mHandle(nullptr),
  mUserData(nullptr)
{
  // Check if there is a MIDI device connected
  if (midiInGetNumDevs() > 0)
  {
    // Open the first device in the list
    MMRESULT result = midiInOpen((HMIDIIN*)&mHandle, 0, (DWORD_PTR)&MidiInProcCallback, (DWORD_PTR)this,
      CALLBACK_FUNCTION);
    if (result == MMSYSERR_NOERROR)
    {
      midiInStart((HMIDIIN)mHandle);
    }
  }
}

//************************************************************************************************
MidiInput::~MidiInput()
{
  if (mHandle)
  {
    midiInStop((HMIDIIN)mHandle);
    midiInClose((HMIDIIN)mHandle);
  }
}

} // namespace Zero
