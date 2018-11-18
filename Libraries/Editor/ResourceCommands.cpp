///////////////////////////////////////////////////////////////////////////////
///
/// \file PhysicsTestBed.cpp
/// Implementation of the PhysicsTestBed class.
/// 
/// Authors: Chris Peters, Joshua Claeys, Joshua Davis, Benjamin Strukus
/// Copyright 2010-2012, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.hpp"
#include "Widget/Command.hpp"
#include "Widget/CommandBinding.hpp"
#include "Editor.hpp"
//#include "EditorImport.hpp"
#include "ContentUploader.hpp"
#include "ContentPackageImporter.hpp"

namespace Zero
{

// Update the transforms of the object hierarchy with
// the transform from the content file like fbx.
void FixArchetypeTransforms(Editor* editor, Space* space)
{
  MetaSelection* selection = editor->GetSelection();

  Cog* object = selection->GetPrimaryAs<Cog>();

  //UpdateToContent(object, UpdateFlags::Transforms);
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

}
