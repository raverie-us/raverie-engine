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
void LoadContentLibrary(StringParam name);
void LoadContentLibrary(ContentLibrary* library);
bool LoadPackage(Editor* editor,
                 Cog* projectCog,
                 ContentLibrary* library,
                 ResourcePackage* package);

void LoadContentConfig(Cog* configCog);
bool LoadEditorContent(Cog* configCog);

// Hacks!!! Fix these later
typedef void (*ExtraLibrarySearchPathCallback)(Cog* configCog,
                                               Array<String>& libraries);
extern ExtraLibrarySearchPathCallback mExtraLibrarySearchPaths;

class EditorPackageLoader
    : public ExplicitSingleton<EditorPackageLoader, EventObject>
{
public:
  ZilchDeclareType(EditorPackageLoader, TypeCopyMode::ReferenceType);

  EditorPackageLoader();

  void OnPackagedBuilt(ContentSystemEvent* event);

  bool LoadPackage(Editor* editor,
                   Cog* projectCog,
                   ContentLibrary* library,
                   ResourcePackage* package);

  Array<ResourcePackage*> PackagesToLoad;
};

} // namespace Zero
