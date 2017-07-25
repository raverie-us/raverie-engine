///////////////////////////////////////////////////////////////////////////////
///
/// \file RawInput.cpp
/// 
/// 
/// Authors: Chris Peters
/// Copyright 2010-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

// Usb Standard Usage Pages
namespace UsbUsagePage
{
  const uint GenericDesktop = 0x01;
}

// Usb Standard usage ids
// for GenericDesktop page
namespace UsbUsage
{
  const uint Pointer   = 0x01;
  const uint Mouse     = 0x02;
  const uint Reserved  = 0x03;
  const uint Joystick  = 0x04;
  const uint Gamepad   = 0x05;
  const uint Keyboard  = 0x06;
  const uint Keypad    = 0x07;
  const uint MultiAxis = 0x08;
  const uint Tablet    = 0x09;

  //0A-2F Reserved

  const uint X                    = 0x30;
  const uint Y                    = 0x31;
  const uint Z                    = 0x32;
  const uint Rx                   = 0x33;
  const uint Ry                   = 0x34;
  const uint Rz                   = 0x35;
  const uint Slider               = 0x36;
  const uint Dial                 = 0x37;
  const uint Wheel                = 0x38;
  const uint HatSwitch            = 0x39;
  const uint CountedBuffer        = 0x3A;
  const uint ByteCount            = 0x3B;
  const uint MotionWakeup         = 0x3C;
  const uint Start                = 0x3D;
  const uint Select               = 0x3E;
  const uint Vx                   = 0x40;
  const uint Vy                   = 0x41;
  const uint Vz                   = 0x42;
  const uint Vbrx                 = 0x43;
  const uint Vbry                 = 0x44;
  const uint Vbrz                 = 0x45;
  const uint Vno                  = 0x46;
  const uint FeatureNotification  = 0x47;
  const uint SystemControl        = 0x81;
  const uint SystemPowerDown      = 0x81;
  const uint SystemSleep          = 0x82;
  const uint SystemWake           = 0x83;
  const uint SystemContextMenu    = 0x84;
  const uint SystemMainMenu       = 0x85;
  const uint SystemAppMenu        = 0x86;
  const uint SystemMenuHelp       = 0x87;
  const uint SystemMenuExit       = 0x88;
  const uint SystemMenuSelect     = 0x89;
  const uint SystemMenuRight      = 0x8A;
  const uint SystemMenuLeft       = 0x8B;
  const uint SystemMenuUp         = 0x8C;
  const uint SystemMenuDown       = 0x8D;
  const uint SystemColdRestart    = 0x8E;
  const uint SystemWarmRestart    = 0x8F;
  const uint DpadUp               = 0x90;
  const uint DpadDown             = 0x91;
  const uint DpadRight            = 0x92;
  const uint DpadLeft             = 0x93;
}

static HashMap<uint, String> InternalGenerateUsageNames()
{
  HashMap<uint, String> names;

  names.Insert(UsbUsage::X                    , "X");
  names.Insert(UsbUsage::Y                    , "Y");
  names.Insert(UsbUsage::Z                    , "Z");
  names.Insert(UsbUsage::Rx                   , "Rx");
  names.Insert(UsbUsage::Ry                   , "Ry");
  names.Insert(UsbUsage::Rz                   , "Rz");
  names.Insert(UsbUsage::Slider               , "Slider");
  names.Insert(UsbUsage::Dial                 , "Dial");
  names.Insert(UsbUsage::Wheel                , "Wheel");
  names.Insert(UsbUsage::HatSwitch            , "Hat switch");
  names.Insert(UsbUsage::CountedBuffer        , "Counted Buffer");
  names.Insert(UsbUsage::ByteCount            , "Byte Count");
  names.Insert(UsbUsage::MotionWakeup         , "Motion Wakeup");
  names.Insert(UsbUsage::Start                , "Start");
  names.Insert(UsbUsage::Select               , "Select");
  names.Insert(UsbUsage::Vx                   , "Vx");
  names.Insert(UsbUsage::Vy                   , "Vy");
  names.Insert(UsbUsage::Vz                   , "Vz");
  names.Insert(UsbUsage::Vbrx                 , "Vbrx");
  names.Insert(UsbUsage::Vbry                 , "Vbry");
  names.Insert(UsbUsage::Vbrz                 , "Vbrz");
  names.Insert(UsbUsage::Vno                  , "Vno");
  names.Insert(UsbUsage::FeatureNotification  , "Feature Notification");
  names.Insert(UsbUsage::SystemControl        , "System Control");
  names.Insert(UsbUsage::SystemPowerDown      , "System Power Down");
  names.Insert(UsbUsage::SystemSleep          , "System Sleep");
  names.Insert(UsbUsage::SystemWake           , "System Wake");
  names.Insert(UsbUsage::SystemContextMenu    , "System Context Menu");
  names.Insert(UsbUsage::SystemMainMenu       , "System Main Menu");
  names.Insert(UsbUsage::SystemAppMenu        , "System App Menu");
  names.Insert(UsbUsage::SystemMenuHelp       , "System Menu Help");
  names.Insert(UsbUsage::SystemMenuExit       , "System Menu Exit");
  names.Insert(UsbUsage::SystemMenuSelect     , "System Menu Select");
  names.Insert(UsbUsage::SystemMenuRight      , "System Menu Right");
  names.Insert(UsbUsage::SystemMenuLeft       , "System Menu Left");
  names.Insert(UsbUsage::SystemMenuUp         , "System Menu Up");
  names.Insert(UsbUsage::SystemMenuDown       , "System Menu Down");
  names.Insert(UsbUsage::SystemColdRestart    , "System Cold Restart");
  names.Insert(UsbUsage::SystemWarmRestart    , "System Warm Restart");
  names.Insert(UsbUsage::DpadUp               , "D-pad Up");
  names.Insert(UsbUsage::DpadDown             , "D-pad Down");
  names.Insert(UsbUsage::DpadRight            , "D-pad Right");
  names.Insert(UsbUsage::DpadLeft             , "D-pad Left");
  return names;
}

HashMap<uint, String>& GetUsageNames()
{
  static HashMap<uint, String> names = InternalGenerateUsageNames();
  return names;
}

// Get the nice name of the joystick from the windows registry
// using venderId productId
String GetJoystickName(uint venderId, uint productId)
{
  // Name is in registry in key
  // HKEY_CURRENT_USER\System\CurrentControlSet\Control\MediaProperties\PrivateProperties\Joystick\OEM\VidPidName
  // VidPidName is in the format VID_045E&PID_028E
  String vidPidName = String::Format("VID_%04X&PID_%04X", venderId, productId);

  /// Open OEM Key from ...MediaProperties
  HKEY  hRoot = HKEY_CURRENT_USER;
  String keyPath = String::Format("%s\\%s", REGSTR_PATH_JOYOEM, vidPidName.c_str());

  HKEY  hKey;
  LONG result = RegOpenKeyEx(hRoot, Widen(keyPath).c_str(), 0, KEY_QUERY_VALUE, &hKey);

  if (result != ERROR_SUCCESS)
    return vidPidName;

  // Read OEM Name from the key
  char nameBuffer[256] = {0};
  DWORD length = sizeof(nameBuffer);

  result = RegQueryValueEx(hKey, REGSTR_VAL_JOYOEMNAME, 0, 0, (LPBYTE) nameBuffer, &length);
  RegCloseKey(hKey);

  if (result != ERROR_SUCCESS)
    return vidPidName;

  return nameBuffer;
}

#define MAX_BUTTONS 128

void ScanDevice(HANDLE deviceHandle, RID_DEVICE_INFO& ridDeviceInfo, Joysticks* joysticks)
{
  UINT deviceInfoSize = ridDeviceInfo.cbSize;
  ReturnIf(GetRawInputDeviceInfo(deviceHandle, RIDI_DEVICEINFO, &ridDeviceInfo, &deviceInfoSize) < 0,,
    "Unable to read device information");

  // Only map Raw Input devices that are RIM_TYPEHID (not mice or keyboards)
  if(ridDeviceInfo.dwType == RIM_TYPEHID && 
     ridDeviceInfo.hid.usUsagePage == UsbUsagePage::GenericDesktop)
  {
    // Get a nice name
    String deviceName = GetJoystickName(ridDeviceInfo.hid.dwVendorId, ridDeviceInfo.hid.dwProductId);

    UINT bufferSize = 0;
    ReturnIf(GetRawInputDeviceInfo(deviceHandle, RIDI_PREPARSEDDATA, NULL, &bufferSize) != 0,,
      "Unable to get device info length");

    PHIDP_PREPARSED_DATA preparsedData = (PHIDP_PREPARSED_DATA)alloca(bufferSize);
    ZeroMemory(preparsedData, bufferSize);

    ReturnIf((int)GetRawInputDeviceInfo(deviceHandle, RIDI_PREPARSEDDATA, preparsedData, &bufferSize) < 0,,
      "Unable to get device info");

    HIDP_CAPS caps;
    ReturnIf(HidP_GetCaps(preparsedData, &caps) != HIDP_STATUS_SUCCESS,,
      "Unable to get device capabilities");

    HIDP_BUTTON_CAPS* buttonCaps = (PHIDP_BUTTON_CAPS)alloca(sizeof(HIDP_BUTTON_CAPS) * caps.NumberInputButtonCaps);

    USHORT capsLength = caps.NumberInputButtonCaps;
    ReturnIf(HidP_GetButtonCaps(HidP_Input, buttonCaps, &capsLength, preparsedData) != HIDP_STATUS_SUCCESS,,
      "Unable to get button capabilities");

    // Basically though we don't get a caps per button, we actually do have 'button sets'
    // and thus we can have more than one set of caps per button set
    INT numberOfButtons = 0;
    for(uint i = 0; i < caps.NumberInputButtonCaps; i++)
    {
      auto& buttonCap = buttonCaps[i];
      numberOfButtons += buttonCap.Range.UsageMax - buttonCap.Range.UsageMin + 1;
    }

    // Value caps
    HIDP_VALUE_CAPS* valueCaps = (PHIDP_VALUE_CAPS)alloca(sizeof(HIDP_VALUE_CAPS) * caps.NumberInputValueCaps);
    capsLength = caps.NumberInputValueCaps;
    ReturnIf(HidP_GetValueCaps(HidP_Input, valueCaps, &capsLength, preparsedData) != HIDP_STATUS_SUCCESS,,
      "Unable to get value capabilities");

    auto& usageNames = GetUsageNames();

    auto map = new RawControlMapping();
    map->mName = deviceName;
    map->mIsParsed = true;

    for(uint i = 0; i < caps.NumberInputValueCaps; i++)
    {
      auto& valueCap = valueCaps[i];

      RawAxis& axis = map->mAxes.PushBack();
      axis.Offset = valueCap.Range.UsageMin;
      axis.Size = 0;
      axis.Max = valueCap.LogicalMax + 1;

      if (valueCap.LogicalMax == -1)
      {
        if (valueCap.BitSize == 8)
        {
          axis.Max = 0xFF;
        }
        else if (valueCap.BitSize == 16)
        {
          axis.Max = 0xFFFF;
        }
        else if (valueCap.BitSize == 32)
        {
          axis.Max = 0xFFFFFFFF;
        }
      }

      axis.Min = valueCap.LogicalMin;
      axis.Mid = (axis.Max - axis.Min) / 2;
      axis.Name = usageNames.FindValue(valueCap.Range.UsageMin, "Unknown");
      axis.DeadZonePercent = 0.16f;
      axis.CanCalibrate = true;
      axis.UseMid = true;

      // If this is a hat, we need to make sure it doesn't get calibrated
      if (valueCap.Range.UsageMin == UsbUsage::HatSwitch)
      {
        axis.CanCalibrate = false;
        axis.UseMid = false;
        axis.DeadZonePercent = 0.0f;
      }
    }

    for(int i = 0; i < numberOfButtons; i++)
    {
      auto& button = map->mButtons.PushBack();
      button.Name = String::Format("Button%d", i);
    }

    // Build a guid using the name hash, and vendor / product / version info
    auto guid = deviceName.Hash();
    guid ^= ridDeviceInfo.hid.dwVendorId * 4187;
    guid ^= ridDeviceInfo.hid.dwProductId;
    guid ^= ridDeviceInfo.hid.dwVersionNumber;

    // Tell the Joysticks system that a Joystick is present
    joysticks->AddJoystickDevice((uint)deviceHandle, guid, deviceName, map);

    const bool detailedDeviceInfo = false;

    // Print details
    if(detailedDeviceInfo)
    {
      ZPrint("Device Name: %s ", deviceName.c_str());
      ZPrint("Vendor Id: %d ",  ridDeviceInfo.hid.dwVendorId);
      ZPrint("Product Id: %d ",  ridDeviceInfo.hid.dwProductId);
      ZPrint("Version Number: %d ",  ridDeviceInfo.hid.dwVersionNumber);
      ZPrint("Usage: %X ",  ridDeviceInfo.hid.usUsage);
      ZPrint("Usage Page: %X ",  ridDeviceInfo.hid.usUsagePage);
      ZPrint("\n"); 
    } 
  }
}

void ScanRawInputDevices(uint windowHandle)
{
  // Get the Raw input device  list
  UINT deviceCount = 0;
  GetRawInputDeviceList(NULL, &deviceCount, sizeof(RAWINPUTDEVICELIST));
  PRAWINPUTDEVICELIST pRawInputDeviceList = (PRAWINPUTDEVICELIST)alloca(sizeof(RAWINPUTDEVICELIST) * deviceCount);
  GetRawInputDeviceList(pRawInputDeviceList, &deviceCount, sizeof(RAWINPUTDEVICELIST));
  
  Joysticks* joysticks = Joysticks::GetInstance();

  // DeactivateAll because joysticks may have been removed in device changed
  joysticks->DeactivateAll();

  // Iterate through all raw input devices
  RID_DEVICE_INFO ridDeviceInfo;
  ridDeviceInfo.cbSize = sizeof(RID_DEVICE_INFO);
  for(uint i = 0; i < deviceCount; i++)
  {
    HANDLE deviceHandle = pRawInputDeviceList[i].hDevice;
    ScanDevice(deviceHandle, ridDeviceInfo, joysticks);
  }

  /// Register RawInput that we accept Raw input
  /// from Joysticks and Gamepads (etc, not mice, others)

  RAWINPUTDEVICE devices[3];
  
  devices[0].usUsagePage = UsbUsagePage::GenericDesktop; 
  devices[0].usUsage = UsbUsage::Mouse; 
  devices[0].dwFlags = RIDEV_INPUTSINK;
  devices[0].hwndTarget = (HWND)windowHandle;

  devices[1].usUsagePage = UsbUsagePage::GenericDesktop;
  devices[1].usUsage     = UsbUsage::Gamepad;
  devices[1].dwFlags     = RIDEV_INPUTSINK;
  devices[1].hwndTarget  = (HWND)windowHandle;

  devices[2].usUsagePage = UsbUsagePage::GenericDesktop;
  devices[2].usUsage     = UsbUsage::Joystick;
  devices[2].dwFlags     = RIDEV_INPUTSINK;
  devices[2].hwndTarget  = (HWND)windowHandle;

  BOOL success = RegisterRawInputDevices(devices, 3, sizeof(RAWINPUTDEVICE));
  CheckWindowsErrorCode(success, "Failed to RegisterRawInputDevices.");

  RcInputOpen();

  joysticks->JoysticksChanged();
}

void PreParsed(Joystick* joystick, RAWINPUT* pRawInput)
{
  HANDLE deviceHandle = pRawInput->header.hDevice;

  UINT bufferSize;
  ReturnIf(GetRawInputDeviceInfo(deviceHandle, RIDI_PREPARSEDDATA, NULL, &bufferSize) != 0,,
    "Unable to get device info length");

  PHIDP_PREPARSED_DATA pPreparsedData = (PHIDP_PREPARSED_DATA)alloca(bufferSize);

  ReturnIf((int)GetRawInputDeviceInfo(deviceHandle, RIDI_PREPARSEDDATA, pPreparsedData, &bufferSize) < 0,,
    "Unable to get device info");

  HIDP_CAPS Caps;
  ReturnIf(HidP_GetCaps(pPreparsedData, &Caps) != HIDP_STATUS_SUCCESS,,
    "Unable to get device capabilities");

  HIDP_BUTTON_CAPS* pButtonCaps = (PHIDP_BUTTON_CAPS)alloca(sizeof(HIDP_BUTTON_CAPS) * Caps.NumberInputButtonCaps);

  USHORT capsLength = Caps.NumberInputButtonCaps;
  ReturnIf(HidP_GetButtonCaps(HidP_Input, pButtonCaps, &capsLength, pPreparsedData) != HIDP_STATUS_SUCCESS,,
    "Unable to get button capabilities");

  // Basically though we don't get a caps per button, we actually do have 'button sets'
  // and thus we can have more than one set of caps per button set
  INT numberOfButtons = 0;
  for(uint i = 0; i < Caps.NumberInputButtonCaps; i++)
  {
    auto& buttonCaps = pButtonCaps[i];
    numberOfButtons += buttonCaps.Range.UsageMax - buttonCaps.Range.UsageMin + 1;
  }

  // Value caps
  HIDP_VALUE_CAPS* pValueCaps = (PHIDP_VALUE_CAPS)alloca(sizeof(HIDP_VALUE_CAPS) * Caps.NumberInputValueCaps);
  capsLength = Caps.NumberInputValueCaps;
  ReturnIf(HidP_GetValueCaps(HidP_Input, pValueCaps, &capsLength, pPreparsedData) != HIDP_STATUS_SUCCESS,,
    "Unable to get value capabilities");

  ULONG usageLength = numberOfButtons;

  USAGE usage[MAX_BUTTONS];
  ReturnIf(HidP_GetUsages(HidP_Input, pButtonCaps->UsagePage, 0, usage, &usageLength, pPreparsedData,
           (PCHAR)pRawInput->data.hid.bRawData, pRawInput->data.hid.dwSizeHid) != HIDP_STATUS_SUCCESS,,
    "Unable to get input usages");

  
  BOOL bButtonStates[MAX_BUTTONS];
  ZeroMemory(bButtonStates, sizeof(bButtonStates));
  for(uint i = 0; i < usageLength; i++)
    bButtonStates[usage[i] - pButtonCaps->Range.UsageMin] = TRUE;

  uint buttons = 0;
  for(int i = 0; i < numberOfButtons; ++i)
  {
    if(bButtonStates[i] == TRUE)
    {
      buttons |= (1 << i);
    }
  }
  
  joystick->RawSetButtons(buttons);

  //
  // Get the state of discrete-valued-controls
  //
  auto map = joystick->GetInputMapping();

  for(uint i = 0; i < Caps.NumberInputValueCaps; i++)
  {
    auto& axis = map->mAxes[i];

    ULONG value;
    
	ReturnIf(HidP_GetUsageValue(HidP_Input, pValueCaps[i].UsagePage, 0, axis.Offset, &value, pPreparsedData,
                                (PCHAR)pRawInput->data.hid.bRawData, pRawInput->data.hid.dwSizeHid) != HIDP_STATUS_SUCCESS,,
      "Unable to get input value");
    
    joystick->RawSetAxis(i, value);

   // ZPrint("Axis with usage %x is %d\n", pValueCaps[i].Range.UsageMin, value);
  }
}

void RawInputMessage(uint wParam, uint lParam)
{
  Joysticks* joysticks = Joysticks::GetInstance();

  // Get the raw input buffer size by passing NULL
  // for the buffer
  UINT bufferSize = 0;
  GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, 
    &bufferSize, sizeof(RAWINPUTHEADER));

  // Allocate a local buffer for the data
  RAWINPUT* rawInput = (RAWINPUT*)alloca(bufferSize);

  // Get the raw input data
  GetRawInputData((HRAWINPUT)lParam, RID_INPUT, 
    rawInput, &bufferSize, sizeof(RAWINPUTHEADER));

  if (rawInput->header.dwType == RIM_TYPEMOUSE) 
  {
    Mouse* mouse = Mouse::GetInstance();
    int xMovement = rawInput->data.mouse.lLastX;
    int yMovement = rawInput->data.mouse.lLastY;
    mouse->mRawMovement += Vec2((real)xMovement, (real)yMovement);
  }
  else
  {
    auto device = (uint)rawInput->header.hDevice;
    auto joystick = joysticks->GetJoystickByDevice(device);

    // It's possible to get a joystick message before we finish mapping joysticks
    if (joystick != NULL)
    {
      // If we have a custom mapping (our own defined way of interpreting joystick data...)
      if (joystick->IsParsedInput())
      {
        PreParsed(joystick, rawInput);
      }
      else
      {
        // Send the raw buffer to the joystick to be interpreted by the custom mapping
        byte* bytes = (byte*)rawInput->data.hid.bRawData;
        joystick->RawProcess(DataBlock(bytes, bufferSize));
      }

      // Tell everyone we updated this joystick
      joystick->SignalUpdated();
    }
  }
}


void RawInputUpdate()
{
  RcInputUpdate();
  // Clear the input buffer
  Mouse* mouse = Mouse::GetInstance();
  mouse->mRawMovement = Vec2(0, 0);
}

void RawInputShutdown()
{
  RcShutdown();
}

}