// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

class Cog;

// Hacks!!! Fix these later
typedef void (*CustomLibraryLoader)(Cog* configCog);
extern CustomLibraryLoader mCustomLibraryLoader;

} // namespace Zero
