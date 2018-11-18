///////////////////////////////////////////////////////////////////////////////
///
/// Authors: Joshua Davis, Chris Peters
/// Copyright 2015, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

namespace Zero
{

class Cog;

// Hacks!!! Fix these later
typedef void(*CustomLibraryLoader)(Cog* configCog);
extern CustomLibraryLoader mCustomLibraryLoader;

} // namespace Zero
