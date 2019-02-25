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
bool LoadCoreContent(Array<String>& coreLibs);

class EditorPackageLoader : public ExplicitSingleton<EditorPackageLoader, EventObject>
{
public:
  ZilchDeclareType(EditorPackageLoader, TypeCopyMode::ReferenceType);

  EditorPackageLoader();

  void OnPackagedBuilt(ContentSystemEvent* event);

  bool LoadPackage(Editor* editor, Cog* projectCog, ContentLibrary* library, ResourcePackage* package);

  Array<ResourcePackage*> PackagesToLoad;
};

} // namespace Zero
