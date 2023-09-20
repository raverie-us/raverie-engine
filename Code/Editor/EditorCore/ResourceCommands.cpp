// MIT Licensed (see LICENSE.md).
#include "Precompiled.hpp"
#include "Extensions/Widget/Command.hpp"
#include "Extensions/Widget/CommandBinding.hpp"
#include "Editor.hpp"
//#include "EditorImport.hpp"
#include "ContentUploader.hpp"
#include "ContentPackageImporter.hpp"

namespace Raverie
{

// Update the transforms of the object hierarchy with
// the transform from the content file like fbx.
void FixArchetypeTransforms(Editor* editor, Space* space)
{
  MetaSelection* selection = editor->GetSelection();

  Cog* object = selection->GetPrimaryAs<Cog>();

  // UpdateToContent(object, UpdateFlags::Transforms);
}

void ExportContentPackage(Editor* editor)
{
  ContentPackageExporter* view = new ContentPackageExporter(editor);
  view->SetName("Export Content");
  editor->AddManagedWidget(view, DockArea::Floating);
}

void BindContentCommands(Cog* configCog, CommandManager* commands)
{
  commands->AddCommand("ExportContentPackage", BindCommandFunction(ExportContentPackage));
  commands->AddCommand("ImportContentPackage", BindCommandFunction(ImportContentPackage));
  commands->AddCommand("FixArchetypeTransforms", BindCommandFunction(FixArchetypeTransforms));
}

} // namespace Raverie
