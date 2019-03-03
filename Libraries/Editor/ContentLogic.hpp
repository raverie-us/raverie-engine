// MIT Licensed (see LICENSE.md).
#pragma once

namespace Zero
{

class Editor;
class ResourcePackage;
class Viewport;
class Mouse;
class MouseEvent;

bool LoadContentLibrary(StringParam name, bool isCore);

void LoadContentConfig();
void LoadCoreContent(Array<String>& coreLibs);

} // namespace Zero
