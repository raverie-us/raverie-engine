///////////////////////////////////////////////////////////////////////////////
///
/// \file Debug.hpp
/// Debug support from the OS.
/// 
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

//Check class for uninitialized memory.
void CheckClassMemory(cstr className, byte* classMemory);

}
