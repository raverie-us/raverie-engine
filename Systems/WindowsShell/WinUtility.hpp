///////////////////////////////////////////////////////////////////////////////
///
/// \file WinUtility.hpp
/// Windows Utility Functions.
///
/// Authors: Chris Peters
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

const uint ZeroDefaultIcon = 101;

//Window classes a pointer to extra class data in GWL_USERDATA
//to give each instance of a window a 'this' pointer. PointerFromWindow
//and SetWindowPointer access this data on the HWND.
inline void *PointerFromWindow(HWND hWnd) 
{
  return reinterpret_cast<void *>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
}

inline void SetWindowPointer(HWND hWnd, void *ptr) 
{
  ::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(ptr));
}

template<typename rangeType>
void SetStrings(HWND comboBox, rangeType inputRange)
{
  //Clear the list
  SendMessage(comboBox,(UINT) CB_RESETCONTENT,0,0);

  for(;!inputRange.Empty();inputRange.PopFront())
  {
    // Add string to combobox.
    SendMessage(comboBox,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) Widen(ToString(inputRange.Front())).c_str()); 
  }
}

//Stripes quotes in place.
LPSTR StripQuotes(LPSTR input);

LPCSTR* CommandLineToArgvA(LPCSTR CmdLine, int* _argc);
PWCHAR* CommandLineToArgvW(PWCHAR CmdLine, int* _argc);

class Image;
//Create a bitmap buffer (a device independent bitmap in memory) from a Image Buffer
void CreateBitmapBuffer(Image* image, byte*& outputBuffer, uint& outSize);

}//namespace Zero



