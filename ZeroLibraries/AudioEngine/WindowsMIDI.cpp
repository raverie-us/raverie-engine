///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.h"
#include "WindowsMIDI.h"

namespace Audio
{
  //-------------------------------------------------------------------------------- Windows Midi In

  //************************************************************************************************
  WindowsMidiIn::WindowsMidiIn() : Active(false)
  {
    // Check if there is a MIDI device connected
    if (midiInGetNumDevs() > 0)
    {
      // Open the first device in the list
      MMRESULT result = midiInOpen(&midiHandle, 0, (DWORD_PTR)MidiEvent, (DWORD_PTR)this, CALLBACK_FUNCTION);
      if (result == MMSYSERR_NOERROR)
      {
        Active = true;
        midiInStart(midiHandle);
      }
    }
  }

  //************************************************************************************************
  WindowsMidiIn::~WindowsMidiIn()
  {
    if (Active)
    {
      midiInStop(midiHandle);
      midiInClose(midiHandle);
    }
  }

  //************************************************************************************************
  void CALLBACK WindowsMidiIn::MidiEvent(HMIDIIN handle, UINT message, DWORD_PTR data, 
    DWORD_PTR param1, DWORD_PTR param2)
  {
    if (message == MIM_DATA)
    {
      MidiData* data(nullptr);
      AudioEventType type;

      // Reinterpret message
      char byte1 = LOBYTE(LOWORD(param1));
      int data1 = HIBYTE(LOWORD(param1));
      int data2 = LOBYTE(HIWORD(param1));
      int channel = byte1 & 0x0F;
      int command = byte1 & 0xF0;

      // Note off
      if (command == 0x80)
      {
        data = new MidiData(channel, (float)data1, 0);
        type = Notify_MidiNoteOff;
      }
      // Note on
      else if (command == 0x90)
      {
        if (data2 > 0)
        {
          data = new MidiData(channel, (float)data1, (float)data2);
          type = Notify_MidiNoteOn;
        }
        else
        {
          data = new MidiData(channel, (float)data1, 0);
          type = Notify_MidiNoteOff;
        }
      }
      // Pitch wheel
      else if (command == 0xE0)
      {
        float value = ((float)(data2 * (1 << 7)) + (float)data1) - (1 << 13);
        value *= 2.0f / (float)((1 << 14) - 1);
        data = new MidiData(channel, value, 0);
        type = Notify_MidiPitchWheel;
      }
      // Control
      else if (command == 0xB0)
      {
        // Volume
        if (data1 == 7)
        {
          data = new MidiData(channel, (float)data2, 0);
          type = Notify_MidiVolume;
        }
        // Modulation wheel
        else if (data1 == 1)
        {
          data = new MidiData(channel, (float)data2, 0);
          type = Notify_MidiModWheel;
        }
        else
        {
          data = new MidiData(channel, (float)data1, (float)data2);
          type = Notify_MidiControl;
        }
      }

      if (data)
        gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&ExternalSystemInterface::SendAudioEvent, 
          gAudioSystem->ExternalInterface, type, (void*)data));
    }
  }

}