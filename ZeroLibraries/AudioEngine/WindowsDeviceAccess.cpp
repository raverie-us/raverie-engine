///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.h"
#include "WindowsDeviceAccess.h"
#include <ObjBase.h>
#include "AudioSystemInternal.h"

namespace Audio
{
  //-------------------------------------------------------------------------- CMMNotificationClient

  //************************************************************************************************
  CMMNotificationClient::CMMNotificationClient() : pEnumerator(NULL), LastResponse(0)
  {
    // Register for audio device change notifications from Windows
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
    const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
    HRESULT hr = CoCreateInstance(
      CLSID_MMDeviceEnumerator, NULL,
      CLSCTX_ALL, IID_IMMDeviceEnumerator,
      (void**)&pEnumerator);
    if (pEnumerator)
      pEnumerator->RegisterEndpointNotificationCallback(this);
  }

  //************************************************************************************************
  HRESULT STDMETHODCALLTYPE CMMNotificationClient::OnDefaultDeviceChanged(EDataFlow flow, ERole role,
    LPCWSTR pwstrDeviceId)
  {
    if (clock() - LastResponse > CLOCKS_PER_SEC)
    {
      LastResponse = clock();
      
      gAudioSystem->LockObject.Lock();
      gAudioSystem->ResetPA = true;
      gAudioSystem->LockObject.Unlock();
    }

    return S_OK;
  }

  //************************************************************************************************
  CMMNotificationClient::~CMMNotificationClient()
  {
    // Stop receiving audio device notifications
    if (pEnumerator)
    {
      pEnumerator->UnregisterEndpointNotificationCallback(this);
      pEnumerator->Release();
    }
  }
}