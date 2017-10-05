///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <mmreg.h>
#include <Audioclient.h>
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <functiondiscoverykeys.h>
#include <process.h>
#include <avrt.h>

namespace Audio
{
  typedef void WASAPICallbackType(float* outputBuffer, float* inputBuffer, const unsigned frameCount,
    void* userData);

  //------------------------------------------------------------------------------------- DeviceInfo

  class WasapiDevice : IMMNotificationClient
  {
  public:

    WasapiDevice();
    ~WasapiDevice();

    void ReleaseData();
    void Initialize(IMMDeviceEnumerator* enumerator, bool render, StreamStatus& status,
      Zero::String& message);
    StreamStatus StartStream(WASAPICallbackType* callback, void* data);
    StreamStatus StopStream();
    void ProcessingLoop();
    unsigned GetChannels();
    unsigned GetSampleRate();

    bool StreamOpen;

  private:
    void Reset();
    bool WasapiEventHandling();
    bool OutputFallback();

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

    enum ThreadEventTypes { StopRequestEvent, ThreadExitEvent, WasapiEvent, ResetEvent, NumThreadEvents };
    HANDLE ThreadEvents[NumThreadEvents];

  public:
    HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDeviceId);

    ULONG STDMETHODCALLTYPE AddRef() { return 0; }
    ULONG STDMETHODCALLTYPE Release() { return 0; }
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID **ppvInterface) { return S_OK; }
    HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR pwstrDeviceId) { return S_OK; }
    HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstrDeviceId) { return S_OK; }
    HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR pwstrDeviceId, 
      DWORD dwNewState) { return S_OK; }
    HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR pwstrDeviceId, 
      const PROPERTYKEY key) { return S_OK; }
  };

  //------------------------------------------------------------------------------- Audio IO Windows

  class AudioIOWindows : public AudioInputOutput
  {
  public:
    AudioIOWindows();
    ~AudioIOWindows();

    // Initializes the underlying audio API
    StreamStatus InitializeAPI() override;
    // Initializes the specified audio stream
    StreamStatus InitializeStream(StreamType whichStream) override;
    // Starts the specified audio stream
    StreamStatus StartStream(StreamType whichStream) override;
    // Stops the specified audio stream
    StreamStatus StopStream(StreamType whichStream) override;
    // Shuts down the specified audio stream
    StreamStatus ShutDownStream(StreamType whichStream) override;
    // Shuts down the underlying audio API
    void ShutDownAPI() override;
    // Returns the number of channels in the specified audio stream
    unsigned GetStreamChannels(StreamType whichStream) override;
    // Returns the sample rate of the specified audio stream
    unsigned GetStreamSampleRate(StreamType whichStream) override;
    // Returns true if the specified audio stream has been started
    bool IsStreamStarted(StreamType whichStream) override;

    void HandleCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer);

  private:
    WasapiDevice* StreamDevices[NumStreamTypes];
    IMMDeviceEnumerator* Enumerator;

    static unsigned _stdcall StartOutputThread(void* param);
    static unsigned _stdcall StartInputThread(void* param);
  };
}