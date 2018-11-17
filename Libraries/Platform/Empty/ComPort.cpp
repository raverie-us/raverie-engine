////////////////////////////////////////////////////////////////////////////////
/// Authors: Dane Curbow
/// Copyright 2018, DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ComPort::ComPort()
{
}

ComPort::~ComPort()
{
}

void ComPort::Close()
{
}

bool ComPort::Open(StringParam name, uint baudRate)
{
  return false;
}

uint ComPort::Read(byte* buffer, uint bytesToRead)
{
  return 0;
}

void ComPort::Write(byte* buffer, uint bytesToWrite)
{
}

} // namespace Zero