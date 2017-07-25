///////////////////////////////////////////////////////////////////////////////
///
/// \file RawInput.hpp
/// 
/// 
/// Authors: Chris Peters
/// Copyright 2010-2013, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

/// Find valid joysticks and gamepads and add them to the engine joystick system.
/// Called in windows startup or device changed.
void ScanRawInputDevices(uint windowHandle);

/// Update Raw Input from a WM_INPUT message
void RawInputMessage(uint wParam, uint lParam);

// Update Raw input
void RawInputUpdate();

// Shutdown Raw input
void RawInputShutdown();

}
