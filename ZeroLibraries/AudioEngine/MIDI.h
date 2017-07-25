///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifdef _MSC_VER
#include "WindowsMIDI.h"
#endif

namespace Audio
{
  //---------------------------------------------------------------------------------------- Midi In

  // Wrapper class for multi-platform MIDI access
  class MidiIn
  {
  public:
    MidiIn() {}
    ~MidiIn() {}

  private:
#ifdef _MSC_VER
    WindowsMidiIn MidiInObject;
#endif
  };

}