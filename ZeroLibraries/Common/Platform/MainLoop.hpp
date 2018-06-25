///////////////////////////////////////////////////////////////////////////////
/// Authors: Dane Curbow, Chris Peters
/// Copyright 2010-2016, DigiPen Institute of Technology
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

// Some platforms do have their own loop that must run your application.
typedef void (*MainLoopFn)(void* userData);
bool SetMainLoopFunction(int fps, MainLoopFn callback, void* userData, bool stopExecutionHere);

}// namespace Zero