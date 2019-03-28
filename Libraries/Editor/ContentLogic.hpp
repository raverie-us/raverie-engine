// MIT Licensed (see LICENSE.md).
#pragma once
#include "Precompiled.hpp"

namespace Zero
{

class Editor;
class ResourcePackage;
class Viewport;
class Mouse;
class MouseEvent;

bool LoadContentLibrary(StringParam name, bool isCore);

void LoadContentConfig();
bool LoadCoreContent(Array<String>& coreLibs);

} // namespace Zero
