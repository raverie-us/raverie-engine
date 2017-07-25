///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef WindowsDevices_H
#define WindowsDevices_H

#include <ObjBase.h>
#include <mmdeviceapi.h>
#include <time.h>

namespace Audio
{
  //-------------------------------------------------------------------------- CMMNotificationClient

  // Receives notifications from Windows when audio devices change (headphones plugged or unplugged, etc.)
  class CMMNotificationClient : public IMMNotificationClient
  {
  public:
    /** Constructor registers for notifications from Windows. */
    CMMNotificationClient();
    /** Destructor un-registers for notifications. */
    ~CMMNotificationClient();

    /** Unused: required to implement. */
    ULONG STDMETHODCALLTYPE AddRef() { return 0; }
    /** Unused: required to implement. */
    ULONG STDMETHODCALLTYPE Release() { return 0; }
    /** Unused: required to implement. */
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID **ppvInterface) { return S_OK; }

    /** Callback for device change notifications. Notifies system to reset Port Audio. */
    HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDeviceId);
    
    /** Unused: required to implement. */
    HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR pwstrDeviceId) { return S_OK; }
    /** Unused: required to implement. */
    HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstrDeviceId) { return S_OK; }
    /** Unused: required to implement. */
    HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState) { return S_OK; }
    /** Unused: required to implement. */
    HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key) { return S_OK; }

  private:
    IMMDeviceEnumerator *pEnumerator;

    clock_t LastResponse;

    friend class AudioSystemInternal;
  };

}

#endif