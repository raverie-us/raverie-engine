///////////////////////////////////////////////////////////////////////////////
///
/// \file ComPort.hpp
/// Simple Com Port Communication
/// 
/// Authors: Chris Peters
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ComPort::ComPort()
{
  mComHandle = INVALID_HANDLE_VALUE;
}

ComPort::~ComPort()
{
  Close();
}

void ComPort::Close()
{
  if(mComHandle != INVALID_HANDLE_VALUE)
  {
    ::CloseHandle(mComHandle);
    mComHandle = INVALID_HANDLE_VALUE;
  }
}

bool ComPort::Open(StringParam name, uint baudRate)
{
  String comportName = String::Format("\\\\.\\%s", name.c_str());

  ErrorIf(mComHandle != INVALID_HANDLE_VALUE, "Com port already open.");

  mComHandle = CreateFile(Widen(comportName).c_str(), GENERIC_READ | GENERIC_WRITE, NULL, NULL, OPEN_EXISTING, 0, NULL);
  //CheckWindowsErrorCode(comHandle != INVALID_HANDLE_VALUE, "Check com port");
  if(mComHandle == INVALID_HANDLE_VALUE)
    return false;

  DCB PortDCB;

  // Initialize the DCBlength member.
  PortDCB.DCBlength = sizeof(DCB);

  // Get the default port setting information.
  GetCommState(mComHandle, &PortDCB);

  // Change the DCB structure settings.
  PortDCB.BaudRate = baudRate;         // Current baud 
  PortDCB.fBinary = TRUE;               // Binary mode; no EOF check 
  PortDCB.fParity = FALSE;               // Enable parity checking 
  PortDCB.fOutxCtsFlow = FALSE;         // No CTS output flow control 
  PortDCB.fOutxDsrFlow = FALSE;         // No DSR output flow control 
  PortDCB.fDtrControl = DTR_CONTROL_ENABLE; 
  // DTR flow control type 
  PortDCB.fDsrSensitivity = FALSE;      // DSR sensitivity 
  PortDCB.fTXContinueOnXoff = TRUE;     // XOFF continues Tx 
  PortDCB.fOutX = FALSE;                // No XON/XOFF out flow control 
  PortDCB.fInX = FALSE;                 // No XON/XOFF in flow control 
  PortDCB.fErrorChar = FALSE;           // Disable error replacement 
  PortDCB.fNull = FALSE;                // Disable null stripping 
  PortDCB.fRtsControl = RTS_CONTROL_ENABLE; 
  // RTS flow control 
  PortDCB.fAbortOnError = FALSE;        // Do not abort reads/writes on 
  // error
  PortDCB.ByteSize = 8;                 // Number of bits/byte, 4-8 
  PortDCB.Parity = NOPARITY;            // 0-4=no,odd,even,mark,space 
  PortDCB.StopBits = ONESTOPBIT;        // 0,1,2 = 1, 1.5, 2 

  VerifyWin(SetCommState(mComHandle, &PortDCB));

  // Retrieve the timeout parameters for all read and write operations
  // on the port. 
  COMMTIMEOUTS CommTimeouts;
  GetCommTimeouts(mComHandle, &CommTimeouts);

  // Change the COMMTIMEOUTS structure settings.
  CommTimeouts.ReadIntervalTimeout = MAXDWORD;
  CommTimeouts.ReadTotalTimeoutMultiplier = 0;
  CommTimeouts.ReadTotalTimeoutConstant = 0;
  CommTimeouts.WriteTotalTimeoutMultiplier = 10;
  CommTimeouts.WriteTotalTimeoutConstant = 1000;

  // Set the timeout parameters for all read and write operations
  // on the port. 
  VerifyWin(SetCommTimeouts(mComHandle, &CommTimeouts));

  return true;
}

uint ComPort::Read(byte* buffer, uint bytesToRead)
{
  DWORD numBytesRead =  0;
  // Read from com handle
  ReadFile (mComHandle,     // Port handle
            buffer,        // Pointer to data to read
            bytesToRead,   // Number of bytes to read
            &numBytesRead, // Pointer to number of bytes
            NULL           // No overlapped
            );
  return numBytesRead;
}

void ComPort::Write(byte* buffer, uint bytesToWrite)
{
  DWORD numberOfBytesWritten = 0;
  WriteFile(mComHandle, buffer, bytesToWrite, &numberOfBytesWritten, NULL );
}

}