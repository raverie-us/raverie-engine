///////////////////////////////////////////////////////////////////////////////
///
/// \file ComPort.hpp
/// Simple Com Port Communication
/// 
/// Authors: Chris Peters
/// Copyright 2010, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
namespace Zero
{

// Simple binary com port reader.
class ComPort
{
public:
  ComPort();
  ~ComPort();

  bool Open(StringParam name, uint baudRate);
  void Close();
  uint Read(byte* buffer, uint bytesToRead);
  void Write(byte* buffer, uint bytesToWrite);

  HANDLE mComHandle;
};

}
