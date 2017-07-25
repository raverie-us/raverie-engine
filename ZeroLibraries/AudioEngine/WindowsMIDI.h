///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Windows.h"
#include "Mmsystem.h"

namespace Audio
{
  //-------------------------------------------------------------------------------- Windows Midi In

  // Receives MIDI events from Windows and sends corresponding notifications
  class WindowsMidiIn
  {
  public:
    WindowsMidiIn();
    ~WindowsMidiIn();

    static void CALLBACK MidiEvent(HMIDIIN handle, UINT message, DWORD_PTR data, DWORD_PTR param1, 
      DWORD_PTR param2);


    HMIDIIN midiHandle;
    bool Active;
  };

}