///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef DEVICENOTIFY_H
#define DEVICENOTIFY_H

#ifdef _MSC_VER
#include "WindowsDeviceAccess.h"
#endif

namespace Audio
{
  //--------------------------------------------------------------------- Device Notification Object

  // Object to abstract receiving notifications on audio device changes. 
  class DeviceNotificationObject
  {
  public:
    DeviceNotificationObject(){}
    ~DeviceNotificationObject(){}

  private:
#ifdef _MSC_VER
    CMMNotificationClient Client;
#endif
  };
}

#endif