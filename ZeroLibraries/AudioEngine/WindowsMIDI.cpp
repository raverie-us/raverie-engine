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
      // Reinterpret message
      char byte1 = LOBYTE(LOWORD(param1));
      int data1 = HIBYTE(LOWORD(param1));
      int data2 = LOBYTE(HIWORD(param1));
      int channel = byte1 & 0x0F;
      int command = byte1 & 0xF0;

      EventData3<int, float, float> data;
      data.mData1 = channel;

      // Note off
      if (command == 0x80)
      {
        data.mData2 = (float)data1;
        data.mEventType = AudioEventTypes::MidiNoteOff;
      }
      // Note on
      else if (command == 0x90)
      {
        if (data2 > 0)
        {
          data.mData2 = (float)data1;
          data.mData3 = (float)data2;
          data.mEventType = AudioEventTypes::MidiNoteOn;
        }
        else
        {
          data.mData2 = (float)data1;
          data.mEventType = AudioEventTypes::MidiNoteOff;
        }
      }
      // Pitch wheel
      else if (command == 0xE0)
      {
        float value = ((float)(data2 * (1 << 7)) + (float)data1) - (1 << 13);
        value *= 2.0f / (float)((1 << 14) - 1);
        data.mData2 = value;
        data.mEventType = AudioEventTypes::MidiPitchWheel;
      }
      // Control
      else if (command == 0xB0)
      {
        // Volume
        if (data1 == 7)
        {
          data.mData2 = (float)data2;
          data.mEventType = AudioEventTypes::MidiVolume;
        }
        // Modulation wheel
        else if (data1 == 1)
        {
          data.mData2 = (float)data2;
          data.mEventType = AudioEventTypes::MidiModWheel;
        }
        else
        {
          data.mData2 = (float)data1;
          data.mData3 = (float)data2;
          data.mEventType = AudioEventTypes::MidiControl;
        }
      }

      if (data.mEventType != AudioEventTypes::NotSet)
      {
        EventData* eventData = new EventData3<int, float, float>(data);
        gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(&ExternalSystemInterface::SendAudioEventData,
          gAudioSystem->ExternalInterface, eventData));
      }
    }
  }

}