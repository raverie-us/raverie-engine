////////////////////////////////////////////////////////////////////////////////
/// Authors: Dane Curbow
/// Copyright 2018, DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"

namespace Zero
{

ComPort::ComPort()
{
  Error("Not implemented");
}

ComPort::~ComPort()
{
  Error("Not implemented");
}

void ComPort::Close()
{
  Error("Not implemented");
}

bool ComPort::Open(StringParam name, uint baudRate)
{
  Error("Not implemented");
  return false;
}

uint ComPort::Read(byte* buffer, uint bytesToRead)
{
  Error("Not implemented");
  return 0;
}

void ComPort::Write(byte* buffer, uint bytesToWrite)
{
  Error("Not implemented");
}

} // namespace Zero