///////////////////////////////////////////////////////////////////////////////
///
/// \file RcInput.hpp
/// 
/// 
/// Authors: Chris Peters
/// Copyright 2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

DeclareEnum6(RcInput, Throttle, Ailerons, Elevator, Rudder, Channel5, Trainer);

struct InputData
{
  u32 Header;
  u32 Inputs[RcInput::Size];
};

const uint DigiPenRcHandle = 0xABABABAB;
const uint DigiPenRcGuid = 0xABABABAB;

String ReadSetting(HDEVINFO deviceInfo, SP_DEVINFO_DATA& data, DWORD propertyId)
{
  char properyBuffer[MAX_PATH];
  ::SetupDiGetDeviceRegistryProperty(deviceInfo, &data,
    propertyId,
    0L,
    (PBYTE)properyBuffer,
    MAX_PATH,
    0);
  return properyBuffer;
}

const String FtdiManufacturer = "FTDI";

String FindFtdiPort()
{
  String portName;

  HDEVINFO devInfo;

  // Filter to all present port devices
  devInfo = SetupDiGetClassDevs(&GUID_DEVCLASS_PORTS, 0, 0, DIGCF_PROFILE | DIGCF_PRESENT);

  // Set up the query structure
  SP_DEVINFO_DATA deviceInfoData;
  ZeroMemory(&deviceInfoData, sizeof(SP_DEVINFO_DATA));
  deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

  // Enumerate through all devices and getting data
  uint deviceIndex = 0;
  while (SetupDiEnumDeviceInfo(devInfo, deviceIndex, &deviceInfoData))
  {
    deviceIndex++;

    // Get data string from the device
    String friendlyName = ReadSetting(devInfo, deviceInfoData, SPDRP_FRIENDLYNAME);
    String manufacturer = ReadSetting(devInfo, deviceInfoData, SPDRP_MFG);
    String physical = ReadSetting(devInfo, deviceInfoData, SPDRP_PHYSICAL_DEVICE_OBJECT_NAME);

    // Parse the com from friendly name.
    // Example: 'Usb Port (COM10)' get 'COM10'
    String comPortName;
    Regex regex("\\((COM[0-9]+)\\)");
    Matches matches;
    regex.Search(friendlyName, matches);
    if(matches.Size() == 2)
      comPortName = matches[1];

    //ZPrint("Device %s %s %s : %s\n", friendlyName.c_str(), manufacturer.c_str(), physical.c_str(), comName);

    if(manufacturer == FtdiManufacturer )
      portName = comPortName;
  }

  if (devInfo)
    SetupDiDestroyDeviceInfoList(devInfo);
 
  return portName;
}

class RcInputReader
{
public:
  u32 mPacketsBytesRead;
  ComPort mComPort;
  bool mActive;
  InputData mRcInput;

  RcInputReader()
  {
    mActive = false;
  }

  ~RcInputReader()
  {
    Close();
  }

  void Close()
  {
    mComPort.Close();
    mActive = false;
  }

  void Open()
  {
    // If already active do not open again
    if(mActive)
      return;

    String portName = FindFtdiPort();

    // No port made by FTDI
    if(portName.Empty())
      return;

    // Open the com port
    mPacketsBytesRead = 0;
    bool opened = mComPort.Open(portName, 115200);

    if(!opened)
      return;

    // For right now if the com ported opened assume
    // that it is our input device
    mActive = true;

    Joysticks* joysticks = Joysticks::GetInstance();

    // Setup the default mapping
    String deviceName = "DigiPenRcInput";
    RawControlMapping* map = new RawControlMapping();
    map->mName = deviceName;
    map->mIsParsed = true;
    for(uint i=0;i<RcInput::Size;++i)
    {
      RawAxis& axis = map->mAxes.PushBack();
      axis.Name = RcInput::Names[i];
      axis.Min = 70000;
      axis.Max = 121814;
      axis.Mid = axis.Min + (axis.Max - axis.Min) / 2;
      axis.Offset = i;
      axis.DeadZonePercent = 0.001f;
      axis.Reversed = true;
      axis.UseMid = false;
    }

    joysticks->AddJoystickDevice(DigiPenRcHandle, DigiPenRcGuid, deviceName, map);
  }

  void Update()
  {
    if(!mActive)
      return;

    uint currentReads = 0;
    uint maxReadCount = 200;

    // Read all the available input every frame
    for(;;)
    {
      byte* offset = (byte*)&mRcInput;

      // Read the header
      while(mPacketsBytesRead < 4)
      {
        if(currentReads > maxReadCount)
        {
          // Failed to find the header
          // Prevent infinite loop
          return;
        }

        // Read the header one byte at a time until the full header
        // is found
        uint bytesRead = mComPort.Read(offset + mPacketsBytesRead, 1);
        ++currentReads;

        if(bytesRead == 1)
        {
          // If this is a header byte keep moving
          // else reset and try again
          if(*(offset + mPacketsBytesRead) == 0xBA)
            ++mPacketsBytesRead;
          else
            mPacketsBytesRead = 0;
        }
        else
        {
          // nothing read
          return;
        }
      }

      // Read the rest of the input package. Note we may not get all it in one update
      uint numberOfBytesToRead = sizeof(mRcInput) - mPacketsBytesRead;
      uint numBytesRead  = mComPort.Read(offset + mPacketsBytesRead, numberOfBytesToRead);
      mPacketsBytesRead += numBytesRead;

      // Check to see if the whole input package has been read
      if(mPacketsBytesRead == sizeof(InputData))
      {
        // Push the inputs to the joystick
        Joysticks* joysticks = Joysticks::GetInstance();
        Joystick* joystick = joysticks->GetJoystickByDevice(DigiPenRcGuid);

        for(uint i = 0; i < RcInput::Size; ++i)
        {
          joystick->RawSetAxis(i, mRcInput.Inputs[i]);
        }

        //for(uint i = 0; i < RcInput::Size; ++i)
        //{
        //  ZPrint("%s %d \n", RcInput::Names[i], mRcInput.Inputs[i]);
        //}

        mPacketsBytesRead = 0;
      }
      else
      {
        return;
      }
    }
  }
};

RcInputReader rcInputReader;

void RcInputOpen()
{
  rcInputReader.Close();
  rcInputReader.Open();
}

void RcInputUpdate()
{
  rcInputReader.Update();
}

void RcShutdown()
{
  rcInputReader.Close();
}

}
